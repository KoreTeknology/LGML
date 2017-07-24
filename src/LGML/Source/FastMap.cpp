/*
  ==============================================================================

    FastMap.cpp
    Created: 17 May 2016 6:05:27pm
    Author:  bkupe

  ==============================================================================
*/

#include "FastMap.h"
#include "NodeManager.h"

#include "Engine.h"


FastMap::FastMap() :
	referenceIn(nullptr),
	referenceOut(nullptr),
	ControllableContainer("FastMap")
{
//	reference = new ControlVariableReference();
//	referenceIn.get()->addReferenceListener(this);

	enabledParam = addBoolParameter("Enabled", "Enabled / Disable Fast Map", true);

	minInputVal = addFloatParameter("In Min", "Minimum Input Value", 0, 0, 1);
	maxInputVal = addFloatParameter("In Max", "Maximum Input Value", 1, 0, 1);
	minOutputVal = addFloatParameter("Out Min", "Minimum Output Value", 0, 0, 1);
	maxOutputVal = addFloatParameter("Out Max", "Maximum Output Value", 1, 0, 1);

	invertParam = addBoolParameter("Invert", "Invert the output signal", false);

}

FastMap::~FastMap()
{

	setReference(nullptr);
	setTarget(nullptr);
//	setGhostAddress(String::empty);

	fastMapListeners.call(&FastMapListener::fastMapRemoved, this);
	if(getEngine())  getEngine()->removeControllableContainerListener(this);
}

void FastMap::process()
{
	if (!enabledParam->boolValue()) return;
  if(!referenceIn.get() || !referenceOut.get()) return;

  auto inRef = dynamic_cast<Parameter*>(referenceIn.get());
  if(inRef){
	float sourceVal = (float)inRef->value;

	bool newIsInRange = (sourceVal > minInputVal->floatValue() && sourceVal <= maxInputVal->floatValue());

	if (invertParam->boolValue()) newIsInRange = !newIsInRange;

	if (referenceOut.get()->type == Controllable::TRIGGER)
	{
		if (newIsInRange != isInRange && newIsInRange) ((Trigger*)referenceOut.get())->trigger();
	}
	else
	{
		if (referenceOut.get()->type == Controllable::BOOL)
		{
			((BoolParameter *)referenceOut.get())->setValue(newIsInRange);
		}else
		{
			if (minOutputVal->floatValue() < maxOutputVal->floatValue())
			{
				float targetVal = juce::jmap<float>(sourceVal, minInputVal->floatValue(), maxInputVal->floatValue(), minOutputVal->floatValue(), maxOutputVal->floatValue());
				targetVal = juce::jlimit<float>(minOutputVal->floatValue(), maxOutputVal->floatValue(), targetVal);
				if (invertParam->boolValue()) targetVal = maxOutputVal->floatValue() - (targetVal - minOutputVal->floatValue());
				((Parameter *)referenceOut.get())->setNormalizedValue(targetVal);
			}
		}
	}

	isInRange = newIsInRange;
  }
}

void FastMap::setReference(Controllable * r)
{
	if (referenceIn == r) return;
	if (referenceIn != nullptr)
	{
		referenceIn.get()->removeControllableListener(this);
	}
	referenceIn = r;
	if (referenceIn != nullptr)
	{
		referenceIn.get()->addControllableListener(this);
		
		float normMin = minInputVal->getNormalizedValue();
		float normMax = maxInputVal->getNormalizedValue();
//		minInputVal->setRange(referenceIn.get()->currentVariable->parameter->minimumValue, referenceIn.get()->currentVariable->parameter->maximumValue);
//		maxInputVal->setRange(referenceIn.get()->currentVariable->parameter->minimumValue, referenceIn.get()->currentVariable->parameter->maximumValue);
//		
		minInputVal->setNormalizedValue(normMin);
		maxInputVal->setNormalizedValue(normMax);
		
	}

	fastMapListeners.call(&FastMapListener::fastMapReferenceChanged, this);
}

void FastMap::setTarget(Controllable * c)
{

	if (referenceOut == c) return;

	if (referenceOut != nullptr)
	{
		referenceOut.get()->removeControllableListener(this);
	}
	referenceOut = c;

	if (referenceOut != nullptr)
	{
		referenceOut.get()->addControllableListener(this);
	}

	fastMapListeners.call(&FastMapListener::fastMapTargetChanged, this);
}

void FastMap::setGhostAddress(const String & address)
{
	if (ghostAddress == address) return;
	ghostAddress = address;
	if (ghostAddress.isNotEmpty()) getEngine()->addControllableContainerListener(this);
	else getEngine()->removeControllableContainerListener(this);
}

var FastMap::getJSONData()
{
	var data = ControllableContainer::getJSONData();
	if (referenceIn != nullptr)
	{
//		data.getDynamicObject()->setProperty("reference", referenceIn.get()->getJSONData());
	}

	if (referenceOut != nullptr)
	{
		data.getDynamicObject()->setProperty("target", referenceOut.get()->getControlAddress()); //Need to be global
		setGhostAddress(String::empty);
	}

	return data;
}

void FastMap::loadJSONDataInternal(var data)
{
	if (data.getDynamicObject()->hasProperty("reference"))
	{
//		referenceIn.get()->loadJSONData(data.getDynamicObject()->getProperty("reference"));
	}

	if (data.getDynamicObject()->hasProperty("target"))
	{
		String cAddress = data.getDynamicObject()->getProperty("target").toString();
		//Need to be global
		cAddress = cAddress.substring(1);
		Controllable * c = NodeManager::getInstance()->getControllableForAddress(cAddress);
		if (c == nullptr) setGhostAddress(cAddress);
		else setTarget(c);
	}
}

void FastMap::remove()
{
	fastMapListeners.call(&FastMapListener::askForRemoveFastMap, this);
}

void FastMap::childStructureChanged(ControllableContainer *, ControllableContainer *)
{
	
	if (ghostAddress.isNotEmpty() && referenceOut.get() == nullptr)
	{
    // shouldn't use that
    jassertfalse;
		if (NodeManager::getInstanceWithoutCreating() == nullptr) return;
		Controllable * c = NodeManager::getInstance()->getControllableForAddress(ghostAddress);
		if (c != nullptr) setTarget(c);
	}
}


void FastMap::parameterValueChanged(Parameter * p){
  if(referenceIn && referenceOut){
  if(p==referenceIn){
    process();
    return;
  }
  }
  ControllableContainer::parameterValueChanged(p);



}

void FastMap::controllableRemoved(Controllable * c)
{
	if (c == referenceOut)
	{
		setTarget(nullptr);
	}
}
