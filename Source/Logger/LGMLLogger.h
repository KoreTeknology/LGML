/*
 ==============================================================================

 Copyright © Organic Orchestra, 2017

 This file is part of LGML. LGML is a software to manipulate sound in real-time

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation (version 3 of the License).

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 ==============================================================================
 */

#ifndef LGMLLOGGER_H_INCLUDED
#define LGMLLOGGER_H_INCLUDED

#include "../Utils/QueuedNotifier.h"
#include "../Utils/DebugHelpers.h"
// do not use file logger atm
// TODO figure out true utility of such
#define USE_FILE_LOGGER 0



class LogElement
{
public:
    LogElement (const String& log);
    Time time;
    String content;
    String source;
    enum Severity {LOG_NONE = -1, LOG_DBG = 0, LOG_WARN = 1, LOG_ERR = 2};
    Severity severity;
    int getNumLines() const {return  _arr->size();}
    const String& getLine (int i) const {return _arr->getReference (i); }
    void incrementNumAppearances(){numAppearances++; time=Time::getCurrentTime();}
    int getNumAppearances() const{return numAppearances;}
    bool operator == (const LogElement & other) const{
        return (other.severity == severity) &&
        (other.source == source) &&
        (other.content ==content);
    }

private:
    int numAppearances;
    ScopedPointer<StringArray> _arr;
    friend class LinkedListPointer<LogElement>;
    LogElement * nextItem;
};



class LGMLLogger : public Logger
{
    public :

    juce_DeclareSingleton (LGMLLogger, true);

    LGMLLogger();
    ~LGMLLogger();

    void logMessage (const String& message) override;

    struct Listener : private AsyncUpdater{
        Listener():readIdx(0),lastIdx(0){}
        virtual ~Listener(){cancelPendingUpdate();}

        virtual void newMessage(const LogElement * el)  = 0;
        virtual void logCleared() {};
    private:
        void handleAsyncUpdate() final{
            int end = lastIdx.get();
            if(end<readIdx){logCleared();readIdx = 0;}
            for(int i = readIdx ; i <= end ; i++){
                newMessage(LGMLLogger::getInstance()->loggedElements.getUnchecked(i));
            }
            readIdx=jmax(0,end);
        }
        friend class LGMLLogger;
        void newMessageAtIdx(int idx){
            lastIdx.set(idx);
            triggerAsyncUpdate();
        }

        int readIdx;
        Atomic<int> lastIdx;

    };

    ListenerList<Listener> listeners;


    class CoalescedListener : private Listener,private Timer{
    public:
        CoalescedListener(int _time):time(_time),readIdxCoal(0){

        }
        virtual ~CoalescedListener(){stopTimer();};
        virtual void newMessages(int from, int to)  = 0;

    private:
        void newMessage(const LogElement * el){
            if(!isTimerRunning()){
                startTimer(time);
            }
        }

        void timerCallback() final{
            int tend = LGMLLogger::getInstance()->getNumLogs();
            if(readIdxCoal> tend){ readIdxCoal = 0;}// has been cleared
            else if(tend>0 && readIdxCoal==tend)readIdxCoal=tend-1; // if only updating last
            newMessages(readIdxCoal,tend);
            readIdxCoal = tend;
            // check if any modification occured during this callback
            if(tend == LGMLLogger::getInstance()->getNumLogs()){stopTimer();}
        }
        int readIdxCoal;
        const int time;
        friend class LGMLLogger;

    };

    const String & getWelcomeMessage();
    void addLogListener (Listener* l) {listeners.add (l);}
    void addLogCoalescedListener (CoalescedListener* l) {listeners.add (l);}
    void removeLogListener (CoalescedListener* l) {listeners.remove (l);}
    void removeLogListener (Listener* l) {listeners.remove (l);}

#if USE_FILE_LOGGER
    class FileWriter : public StringListener
    {
    public:
        FileWriter() {fileLog = FileLogger::createDefaultAppLogger ("LGML", "log", "");}

        void newMessage (const String& s) override {if (fileLog && !s.isEmpty()) {fileLog->logMessage (s);}}
        String getFilePath() {return fileLog->getLogFile().getFullPathName();}
        ScopedPointer<FileLogger> fileLog;
    };

    FileWriter fileWriter;
#endif

    static int maxLoggedElements;
    Atomic<int> writeCursor;
    OwnedArray<LogElement,CriticalSection> loggedElements;
    int getNumLogs();
    void clearLog();
private:
    const String welcomeMessage;
};



#endif  // LOGGER_H_INCLUDED
