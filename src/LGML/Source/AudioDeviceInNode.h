/*
==============================================================================

AudioDeviceInNode.h
Created: 7 Mar 2016 8:03:48pm
Author:  Martin Hermant

==============================================================================
*/

#ifndef AudioDeviceInNode_H_INCLUDED
#define AudioDeviceInNode_H_INCLUDED



#include "NodeBase.h"


class AudioDeviceInNode :
	public NodeBase,
	public juce::AudioProcessorGraph::AudioGraphIOProcessor,
	public ChangeListener,
	public NodeBase::NodeBaseListener
{

public:

	AudioDeviceInNode();
	~AudioDeviceInNode();

	void processBlockInternal(AudioBuffer<float>& buffer, MidiBuffer& midiMessages)override;

	void changeListenerCallback(ChangeBroadcaster* source)override;
    void onContainerParameterChanged(Parameter *)override;
	void updateIO();

	Array<BoolParameter *> inMutes;
    Array<FloatParameter * > volumes;
    Array<float > logVolumes,lastVolumes;


	void addVolMute();
	void removeVolMute();

	virtual ConnectableNodeUI * createUI() override;

	virtual void audioOutputAdded(NodeBase *, int) override;
	virtual void audioOutputRemoved(NodeBase *, int) override;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioDeviceInNode)
};







#endif  // AudioDeviceInNode_H_INCLUDED
