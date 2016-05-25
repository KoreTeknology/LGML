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
	public ChangeListener
{

public:

	AudioDeviceInNode();
	~AudioDeviceInNode();


	void processBlockInternal(AudioBuffer<float>& buffer, MidiBuffer& midiMessages)override;

	void changeListenerCallback(ChangeBroadcaster* source)override;
	void updateIO();

	
	virtual ConnectableNodeUI * createUI() override;


	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioDeviceInNode)
};







#endif  // AudioDeviceInNode_H_INCLUDED
