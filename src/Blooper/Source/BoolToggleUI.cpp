/*
  ==============================================================================

    BoolToggleUI.cpp
    Created: 8 Mar 2016 3:47:01pm
    Author:  bkupe

  ==============================================================================
*/

#include "BoolToggleUI.h"
#include "Style.h"

//==============================================================================
BoolToggleUI::BoolToggleUI(Parameter * parameter) :
	ParameterUI(parameter),
	boolParam((BoolParameter *)parameter)
{
	setSize(10,10);
}

BoolToggleUI::~BoolToggleUI()
{
	
}

void BoolToggleUI::paint(Graphics & g)
{
	Colour c = (boolParam->value ? HIGHLIGHT_COLOR : NORMAL_COLOR).withAlpha(boolParam->enabled?1:.3f);
	g.setGradientFill(ColourGradient(c.brighter(), getLocalBounds().getCentreX(), getLocalBounds().getCentreY(), c.darker(), 2,2,true));
	g.fillRoundedRectangle(getLocalBounds().toFloat(),2);
}

void BoolToggleUI::mouseDown(const MouseEvent & e)
{
	boolParam->setValue(!boolParam->value);
}

void BoolToggleUI::parameterValueChanged(Parameter *)
{
	repaint();
}

