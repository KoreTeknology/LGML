/*
  ==============================================================================

    Controllable.h
    Created: 8 Mar 2016 1:08:56pm
    Author:  bkupe

  ==============================================================================
*/

#ifndef CONTROLLABLE_H_INCLUDED
#define CONTROLLABLE_H_INCLUDED

#include "JuceHeader.h"
#include "StringUtil.h"

class ControllableContainer;

class Controllable
{
public:
	Controllable(const String &niceName, const String &description, bool enabled = true);
	virtual ~Controllable() {}

	bool enabled;
	String niceName;
	String shortName;
	bool hasCustomShortName;
	
	bool isControllableExposed;

	String description;

	ControllableContainer * parentContainer;

	void setNiceName(const String &niceName) {
		this->niceName = niceName;
		if (!hasCustomShortName) setAutoShortName();
	}

	void setCustomShortName(const String &shortName)
	{
		this->shortName = shortName;
		hasCustomShortName = true;
	}

	void setAutoShortName() {
		hasCustomShortName = false;
		shortName = StringUtil::toShortName(niceName);
	}

	void setEnabled(bool value, bool silentSet = false, bool force = false)
	{
		if (!force && value == enabled) return;

		enabled = value;
		if(!silentSet) listeners.call(&Listener::controllableStateChanged, this);
	}

	void setParentContainer(ControllableContainer * container)
	{
		this->parentContainer = container;
	}

	String getControlAddress();

public:
	class  Listener
	{
	public:
		/** Destructor. */
		virtual ~Listener() {}
		virtual void controllableStateChanged(Controllable * c) = 0;
	};

	ListenerList<Listener> listeners;
	void addListener(Listener* newListener) { listeners.add(newListener); }
	void removeListener(Listener* listener) { listeners.remove(listener); }

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Controllable)
};

#endif  // CONTROLLABLE_H_INCLUDED
