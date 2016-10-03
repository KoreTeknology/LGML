/*
  ==============================================================================

    JsGlobalEnvironment.cpp
    Created: 9 May 2016 5:17:33pm
    Author:  Martin Hermant

  ==============================================================================
*/

#include "JsGlobalEnvironment.h"
#include "DebugHelpers.h"
#include "TimeManager.h"
#include "NodeManager.h"
#include "ControlManager.h"

#include "JsHelpers.h"
juce_ImplementSingleton(JsGlobalEnvironment);


JsGlobalEnvironment::JsGlobalEnvironment(){
    env = new DynamicObject();
    static const Identifier jsPostIdentifier("post");
	static const Identifier jsGetMillisIdentifier("getMillis");
    getEnv()->setMethod(jsPostIdentifier, JsGlobalEnvironment::post);
	getEnv()->setMethod(jsGetMillisIdentifier, JsGlobalEnvironment::getMillis);
    // default in global namespace
    linkToControllableContainer("time",TimeManager::getInstance());
    linkToControllableContainer("node",NodeManager::getInstance());
    linkToControllableContainer("controller",ControllerManager::getInstance());


}

void JsGlobalEnvironment::removeNamespace(const String & ns){removeNamespaceFromObject(ns,getEnv());}

DynamicObject * JsGlobalEnvironment::getNamespaceObject(const String & ns){return getNamespaceFromObject(ns,getEnv());}

DynamicObject * JsGlobalEnvironment::getEnv(){return env.getDynamicObject();}

var JsGlobalEnvironment::post(const juce::var::NativeFunctionArgs& a){
    for(int i = 0 ; i < a.numArguments ;i++){
        LOG(a.arguments[i].toString());
    }
    return var();
}

var JsGlobalEnvironment::getMillis(const juce::var::NativeFunctionArgs& /*a*/) {
	return var((int)Time::getMillisecondCounter());
}