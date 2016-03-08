/*
  ==============================================================================

    DummyNode.cpp
    Created: 3 Mar 2016 12:31:33pm
    Author:  bkupe

  ==============================================================================
*/

#include "DummyNode.h"
#include "NodeBaseUI.h"
#include "DummyNodeContentUI.h"

DummyNode::DummyNode(NodeManager * nodeManager,uint32 nodeId) :
	NodeBase(nodeManager,nodeId, "DummyNode", new DummyAudioProcessor, new DummyDataProcessor)
{

	DBG("Dummy node add Data Input");
	dataProcessor->addInputData("IN Number", DataProcessor::DataType::Number);
	dataProcessor->addInputData("IN Position", DataProcessor::DataType::Position);

	dataProcessor->addOutputData("OUT Number", DataProcessor::DataType::Number);
	dataProcessor->addOutputData("OUT Orientation", DataProcessor::DataType::Orientation);

	testFloatParam = addFloatParameter("Test Float slider", 2.3f, .1f, 5);
	testFloatParam->addListener(this);
}

 DummyNode::~DummyNode()
{
	DBG("delete dummy node");
}

 void DummyNode::parameterValueChanged(Parameter * p)
 {
	 if (p == testFloatParam) ((DummyAudioProcessor*)audioProcessor)->amp = p->getNormalizedValue();
 }

 NodeBaseUI * DummyNode::createUI()
{

	NodeBaseUI * ui = new NodeBaseUI(this,new DummyNodeContentUI());
	return ui;
	
}
