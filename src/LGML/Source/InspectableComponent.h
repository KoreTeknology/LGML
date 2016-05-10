/*
  ==============================================================================

    InspectableComponent.h
    Created: 9 May 2016 6:51:16pm
    Author:  bkupe

  ==============================================================================
*/

#ifndef INSPECTABLECOMPONENT_H_INCLUDED
#define INSPECTABLECOMPONENT_H_INCLUDED

#include "JuceHeader.h"

class CustomEditor;
class ControllableContainer;
class Inspector;
class InspectorEditor;

class InspectableComponent : public Component
{
public:
	InspectableComponent();
	InspectableComponent(ControllableContainer * relatedContainer);
	InspectableComponent(Inspector * targetInspector);
	InspectableComponent(ControllableContainer * relatedContainer,Inspector * targetInspector);


	virtual ~InspectableComponent();

	Inspector * inspector;

	virtual InspectorEditor * getEditor();
	
	ControllableContainer * relatedControllableContainer;
	bool recursiveInspectionLevel;

	bool repaintOnSelectionChanged;

	bool isSelected;
	virtual void selectThis();
	virtual void setSelected(bool value);

	virtual void setSelectedInternal(bool value); //to be overriden

public:
	//Listener
	class  InspectableListener
	{
	public:
		/** Destructor. */
		virtual ~InspectableListener() {}
		virtual void inspectableSelectionChanged(InspectableComponent * c) = 0;
	};

	ListenerList<InspectableListener> listeners;
	void addInspectableListener(InspectableListener* newListener) { listeners.add(newListener); }
	void removeInspectableListener(InspectableListener* listener) { listeners.remove(listener); }

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InspectableComponent)
};



#endif  // INSPECTABLECOMPONENT_H_INCLUDED
