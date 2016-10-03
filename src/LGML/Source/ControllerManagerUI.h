/*
  ==============================================================================

    ControllerManagerUI.h
    Created: 8 Mar 2016 10:13:00pm
    Author:  bkupe

  ==============================================================================
*/

#ifndef CONTROLLERMANAGERUI_H_INCLUDED
#define CONTROLLERMANAGERUI_H_INCLUDED


#include "ControlManager.h"
#include "ControllerUI.h"

#include "ShapeShifterContent.h"
//==============================================================================
/*
*/

class ControllerManagerUI : public ShapeShifterContent, ControllerManager::Listener
{
public:
    ControllerManagerUI(const String &contentName, ControllerManager * manager);
    ~ControllerManagerUI();

    ControllerManager * manager;

    OwnedArray<ControllerUI> controllersUI;

    ControllerUI * addControllerUI(Controller * controller);
    void removeControllerUI(Controller * controller);

    ControllerUI * getUIForController(Controller * controller);

    void paint (Graphics&)override;
    void resized()override;

    void mouseDown(const MouseEvent &e) override;

	void clear();
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ControllerManagerUI)

    // Inherited via Listener
    virtual void controllerAdded(Controller *) override;
    virtual void controllerRemoved(Controller *) override;
};

#endif  // CONTROLLERMANAGERUI_H_INCLUDED