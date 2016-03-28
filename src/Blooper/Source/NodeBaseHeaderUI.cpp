/*
 ==============================================================================

 NodeBaseHeaderUI.cpp
 Created: 8 Mar 2016 5:53:52pm
 Author:  bkupe

 ==============================================================================
 */

#include "NodeBaseHeaderUI.h"

NodeBaseHeaderUI::NodeBaseHeaderUI()
{
    node = nullptr;
    nodeUI = nullptr;

    setSize(20, 30);
}

NodeBaseHeaderUI::~NodeBaseHeaderUI()
{
    if (node != nullptr && node->hasAudioOutputs) {
        node->audioProcessor->removeRMSListener(&vuMeter);
    }

}

void NodeBaseHeaderUI::setNodeAndNodeUI(NodeBase * node, NodeBaseUI * nodeUI)
{
    this->node = node;
    this->nodeUI = nodeUI;

    if (node != nullptr && node->hasAudioOutputs) {
        node->audioProcessor->addRMSListener(&vuMeter);
        addAndMakeVisible(vuMeter);
    }

    titleUI = node->nameParam->createStringParameterUI();
    titleUI->setNameLabelVisible(false);
    titleUI->setInterceptsMouseClicks(false, false);
    addAndMakeVisible(titleUI);

    enabledUI = node->enabledParam->createToggle();
    addAndMakeVisible(enabledUI);

    init();

}
void NodeBaseHeaderUI::mouseDoubleClick(const MouseEvent & e){
    if(titleUI){titleUI->valueLabel.showEditor();}
}

void NodeBaseHeaderUI::init()
{
    //to override

}

void NodeBaseHeaderUI::resized()
{
    if (enabledUI != nullptr)
    {
        Rectangle<int> r = getLocalBounds();
        r.reduce(5, 2);
        r.removeFromLeft(enabledUI->getWidth());
        r.removeFromRight(50);
        titleUI->setBounds(r);
        enabledUI->setTopLeftPosition(5, 5);
    }

    if (node && node->hasAudioOutputs) {
        Rectangle<int> vuMeterRect = getLocalBounds().removeFromRight(14).reduced(4);
        vuMeter.setBounds(vuMeterRect);
    }
}
