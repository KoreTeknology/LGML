/*
 ==============================================================================

 JavascriptControllerUI.h
 Created: 7 May 2016 7:28:40am
 Author:  Martin Hermant

 ==============================================================================
 */

#ifndef JAVASCRIPTCONTROLLERUI_H_INCLUDED
#define JAVASCRIPTCONTROLLERUI_H_INCLUDED


class JavascriptController;
#include "OSCControllerUI.h"

class JavascriptControllerUI : public OSCControllerUI{
public:
	JavascriptControllerUI(JavascriptController * jsController);
	virtual ~JavascriptControllerUI();

	JavascriptController * jsController;

	InspectorEditor * getEditor() override;
};




#endif  // JAVASCRIPTCONTROLLERUI_H_INCLUDED