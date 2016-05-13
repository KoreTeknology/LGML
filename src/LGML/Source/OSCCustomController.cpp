/*
  ==============================================================================

    OSCCustomController.cpp
    Created: 10 May 2016 2:29:25pm
    Author:  bkupe

  ==============================================================================
*/

#include "OSCCustomController.h"
#include "OSCCustomControllerUI.h"

OSCCustomController::OSCCustomController() :
	OSCDirectController("OSC Custom Controller")
{
}

OSCCustomController::~OSCCustomController()
{
}

var OSCCustomController::getJSONData()
{
	var data = OSCController::getJSONData();

	var vDataArray;
	for (auto &v : variables)
	{
		var vData(new DynamicObject());
		vData.getDynamicObject()->setProperty("name", v->parameter->niceName);
		vDataArray.append(vData);
	}

	data.getDynamicObject()->setProperty("variables", vDataArray);

	return data;
}

void OSCCustomController::loadJSONData(var data)
{
	OSCController::loadJSONData(data);
	Array<var>* vDataArray = data.getDynamicObject()->getProperty("variables").getArray();

	for (auto &v : *vDataArray)
	{
		addVariable(new FloatParameter(v.getDynamicObject()->getProperty("name"),"variable",0));
	}
}

ControllerUI * OSCCustomController::createUI()
{
	DBG("Create Custom UI");
	return new OSCCustomControllerUI(this);
}

Result OSCCustomController::processMessageInternal(const OSCMessage & msg)
{
	String address = msg.getAddressPattern().toString();
	DBG("Process message : " << address << " / " << msg.size() << "/" << String(msg[0].isFloat32()) );
	
	ControlVariable * v = getVariableForAddress(address);
	
	Result r(Result::ok());

	if (v == nullptr) r = Result::fail("Variable not found");
	if (msg.size() == 0) r = Result::fail("No argument");
	if (!msg[0].isFloat32()) r = Result::fail("Argument is not a float");

	if (r != Result::ok())
	{
		return OSCDirectController::processMessageInternal(msg);
	}

	DBG("Passed  ! : " << address);
	v->parameter->setValue(msg[0].getFloat32());

	return r;

}
