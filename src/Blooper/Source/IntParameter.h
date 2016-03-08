/*
  ==============================================================================

    IntParameter.h
    Created: 8 Mar 2016 1:22:23pm
    Author:  bkupe

  ==============================================================================
*/

#ifndef INTPARAMETER_H_INCLUDED
#define INTPARAMETER_H_INCLUDED

#include "Parameter.h"

class IntParameter : public Parameter
{
public:
	IntParameter(const String &shortName, const int &initialValue, const int &minValue = 0, const int &maxValue = 1, bool enabled = true);

	int minValue;
	int maxValue;
	int value;

	void setValue(const int &value, bool silentSet = false, bool force = false)
	{
		if (!force && this->value == value) return;
		this->value = jlimit<int>(minValue, maxValue, value);
		if (!silentSet) notifyValueChanged();
	}

	float getNormalizedValue() override
	{
		return jmap<float>(value, minValue, maxValue, 0, 1);
	}
};


#endif  // INTPARAMETER_H_INCLUDED
