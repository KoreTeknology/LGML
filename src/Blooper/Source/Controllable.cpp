/*
  ==============================================================================

    Controllable.cpp
    Created: 8 Mar 2016 1:08:56pm
    Author:  bkupe

  ==============================================================================
*/

#include "Controllable.h"
#include "ControllableContainer.h"

Controllable::Controllable(const String & niceName, const String &description, bool enabled) :
	description(description),
	parentContainer(nullptr),
	hasCustomShortName(false),
	isControllableExposed(true)
{
	setEnabled(enabled);
	setNiceName(niceName);
	DBG("Add controllable :" + niceName + " >> " + shortName + " (" + description + ")");
}

String Controllable::getControlAddress()
{
	StringArray addressArray;
	addressArray.add(shortName);

	ControllableContainer * pc = parentContainer;

	while (pc != nullptr)
	{
		if(!pc->skipControllableNameInAddress) addressArray.insert(0, pc->shortName);
		pc = pc->parentContainer;
		
	}

	return "/" + addressArray.joinIntoString("/");
}