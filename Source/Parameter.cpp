/* Copyright © Organic Orchestra, 2017
*
* This file is part of LGML.  LGML is a software to manipulate sound in realtime
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation (version 3 of the License).
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
*/


#include "Parameter.h"
#include "JsHelpers.h"


const Identifier Parameter::valueIdentifier("value");
const Identifier Parameter::minValueIdentifier("min");
const Identifier Parameter::maxValueIdentifier("max");


Parameter::Parameter(const Type &type, const String &niceName, const String &description, var initialValue, var minValue = var(), var maxValue = var(), bool enabled) :
Controllable(type, niceName, description, enabled),
isEditable(true),
isPresettable(true),
isOverriden(false),
queuedNotifier(100),
hasCommitedValue(false),
isCommitableParameter(false),
isSettingValue(false),
isLocking(true)
{
  minimumValue = minValue;
  maximumValue = maxValue;
  defaultValue = initialValue;

  resetValue(true);
}
void Parameter::setFromVarObject(DynamicObject & ob){
  if(ob.hasProperty("maximumValue")){minimumValue = ob.getProperty("maximumValue");}
  if(ob.hasProperty("minimumValue")){minimumValue = ob.getProperty("minimumValue");}
  if(ob.hasProperty("initialValue")){defaultValue = ob.getProperty("initialValue");}
  if(ob.hasProperty("value")){setValue(ob.getProperty("value"));}
}
void Parameter::resetValue(bool silentSet)
{
  isOverriden = false;
  setValue(defaultValue, silentSet,true);
}

void Parameter::setValue(var _value, bool silentSet, bool force)
{
  if(isCommitableParameter && !force){
    commitValue(_value);
  }
  else{
    tryToSetValue(_value,silentSet,force);
  }

}

bool Parameter::waitOrDeffer(const var & _value, bool silentSet , bool force ){
  if(!force&&isSettingValue){
    if(isLocking){
      int overflow = 1000000;
      auto startWait = Time::currentTimeMillis();
      while(isSettingValue && overflow>0){
        //        Thread::sleep(1);
        Thread::yield();
        overflow--;
      }

      if(isSettingValue && overflow<=0){
        DBG("locked for : " << Time::currentTimeMillis()-startWait);
      }
    }
    // force defering if locking too long or not locking
    if (isSettingValue){
      if(auto *mm = MessageManager::getInstanceWithoutCreating()){
        mm->callAsync([this,_value, silentSet, force](){tryToSetValue(_value, silentSet, force);});
        return true;
      }
    }

    //    jassertfalse;
  }
  return false;
}
void Parameter::tryToSetValue(var _value, bool silentSet , bool force ){

  if (!force && checkValueIsTheSame(_value, value)) return;

  if(!waitOrDeffer(_value, silentSet, force)){
    isSettingValue = true;
    lastValue = var(value);
    setValueInternal(_value);
    if(_value != defaultValue) isOverriden = true;
    if (!silentSet) notifyValueChanged(false);
    isSettingValue = false;
  }

}
void Parameter::setRange(var min, var max){
  minimumValue = min;
  maximumValue = max;
  listeners.call(&Listener::parameterRangeChanged,this);
  var arr;arr.append(minimumValue);arr.append(maximumValue);
  queuedNotifier.addMessage(new ParamWithValue(this,arr,true));
}

void Parameter::commitValue(var _value){
  hasCommitedValue = value!=_value;
  commitedValue  =_value;

}

void Parameter::pushValue(bool force){
  if(!hasCommitedValue && !force)return;
  tryToSetValue(commitedValue,false,true);
  hasCommitedValue = false;
}


void Parameter::setValueInternal(var & _value) //to override by child classes
{

  value = _value;
#ifdef JUCE_DEBUG
  checkVarIsConsistentWithType();
#endif
}

bool Parameter::checkValueIsTheSame(var newValue, var oldValue)
{
  return newValue.hasSameTypeAs(oldValue) && (newValue == oldValue);
}

void Parameter::checkVarIsConsistentWithType(){
  if      (type == Type::STRING && !value.isString()) { value = value.toString();}
  else if (type == Type::INT && !value.isInt())       { value = int(value);}
  else if (type == Type::BOOL && !value.isBool())     { value = bool(value);}
  else if (type == Type::FLOAT && !value.isDouble())  { value = double(value);}
  else if (type == Type::POINT2D && !value.isArray()) { value = Array<var>{0,0};}
  else if (type == Type::POINT3D && !value.isArray()) { value = Array<var>{0,0,0};}
}

void Parameter::setNormalizedValue(const float & normalizedValue, bool silentSet, bool force)
{
  setValue(jmap<float>(normalizedValue, (float)minimumValue, (float)maximumValue), silentSet, force);
}

float Parameter::getNormalizedValue() {
  if(minimumValue==maximumValue){
    return 0.0;
  }
  else
    return jmap<float>((float)value, (float)minimumValue, (float)maximumValue, 0.f, 1.f);
}

void Parameter::notifyValueChanged(bool defferIt) {
  if(defferIt)
    triggerAsyncUpdate();
  else
    listeners.call(&Listener::parameterValueChanged, this);

  queuedNotifier.addMessage(new ParamWithValue(this,value,false));
}


//JS Helper


DynamicObject * Parameter::createDynamicObject()
{
  DynamicObject * dObject = Controllable::createDynamicObject();
  static const Identifier _jsSetIdentifier("set");
  dObject->setMethod(_jsSetIdentifier, setControllableValueFromJS);
  return dObject;
}

var Parameter::getVarObject(){
  var res = new DynamicObject();
  res.getDynamicObject()->setProperty(varTypeIdentifier, getTypeIdentifier().toString());
  res.getDynamicObject()->setProperty(valueIdentifier, value);
  if(isNumeric()){
    res.getDynamicObject()->setProperty(minValueIdentifier, minimumValue);
    res.getDynamicObject()->setProperty(maxValueIdentifier, maximumValue);
  }
  return res;
}
var Parameter::getVarState(){
  return value;
}



void Parameter::handleAsyncUpdate(){
  listeners.call(&Listener::parameterValueChanged, this);
};



