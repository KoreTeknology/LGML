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


#include "EnumParameter.h"

#include "ParameterFactory.h"
#include <juce_gui_basics/juce_gui_basics.h>

REGISTER_PARAM_TYPE (EnumParameter)

var EnumParameter::None;
Identifier EnumParameter::modelIdentifier ("model");
Identifier EnumParameter::selectedSetIdentifier ("selected");
//////////////
//

#if JUCE_DEBUG && 0
#define DBGENUM(x) DBG(x)
#define LOG_ENUM DBG("////////////////");DBG(dumpVarObj(enumData));
String dumpVarObj(const var & v ,const int indent=0){
    String res;
    if(auto obj = v.getDynamicObject()){
        res += "\n";
        for (auto vv:obj->getProperties()){

            for(int i = 0 ; i < indent ; i++){
                res+=" ";
            }
            res+= vv.name.toString() +" : " + dumpVarObj(vv.value,indent+1);
            res += "\n";
        }
    }
    else if(auto arr = v.getArray()){
        StringArray sarr;
        for(auto  a:*arr){
            sarr.add(dumpVarObj(a,indent));
        }
        res = "[" + sarr.joinIntoString(",") + "]";
    }
    else{
        res = v.toString();
    }

    return res;

}

#else
#define LOG_ENUM
#define DBGENUM(x)
#endif
///////////////////
// EnumParameter

EnumParameter::EnumParameter (const String& niceName, const String& description, EnumParameterModel* modelInstance, const var& defaultValue, bool _userCanEnterText, bool enabled) :
ParameterBase ( niceName, description, defaultValue, enabled),
asyncNotifier (1000),
userCanEnterText (_userCanEnterText),
enumData (new DynamicObject())
{

    enumData->setProperty (selectedSetIdentifier, Array<var>());
    ownModel = modelInstance == nullptr;
    model = ownModel ? new EnumParameterModel() : modelInstance;
    enumData->setProperty (modelIdentifier, var(model.get()->getObject() ));


    model->listeners.add (this);

    asyncNotifier.addListener (this);

    value = var (enumData);

}
EnumParameter::~EnumParameter()
{
    //  value = var::null;
    model->listeners.remove (this);
    asyncNotifier.removeListener (this);
    enumData = nullptr;

    if (ownModel)
    {
        delete model;
    }
}

EnumParameterModel* EnumParameter::getModel() const
{
    jassert (model.get());
    return model.get();

}

void EnumParameter::addOption (Identifier key, var data, bool autoGenerated)
{
    //  adding option thru parameter is not supported when using a shared model
    jassert (!autoGenerated || ownModel || userCanEnterText);
    getModel()->addOption (key, data, autoGenerated);
}

void EnumParameter::addOrSetOption (Identifier key, var data, bool autoGenerated)
{
    //  adding option thru parameter is not supported when using a shared model
    jassert (!autoGenerated || ownModel || userCanEnterText);
    getModel()->addOrSetOption (key, data, autoGenerated);
}
void EnumParameter::removeOption (Identifier key, bool autoGenerated)
{
    //  removing option thru parameter is not supported when using a shared model
    jassert (!autoGenerated || ownModel || userCanEnterText);
    selectId (key, false, true);
    getModel()->removeOption (key, autoGenerated);



}
Array<Identifier> EnumParameter::getSelectedIds() const
{
    return getSelectedSetIds (value);
}

Identifier EnumParameter::getFirstSelectedId() const
{
    Array<Identifier> arr = getSelectedIds();

    if (arr.size())return arr[0];

    return Identifier::null;

}
var EnumParameter::getFirstSelectedValue (var _defaultValue) const
{
    Array<var> arr = getSelectedValues();

    if (arr.size())return arr[0];

    return _defaultValue;

}


bool EnumParameter::selectionIsNotEmpty()
{
    Array<var> arr = getSelectedValues();
    return arr.size() > 0;
}


void EnumParameter::selectId (Identifier key, bool shouldSelect, bool appendSelection)
{
    if (!appendSelection)
    {
        auto oldS = getSelectedIds();

        for (auto& s : oldS)
        {
            if (s != key)
            {
                selectId (s, false, true);
            }
        }

    }

    Array<var>* selection = getSelectedSet (value);
    jassert (selection);
    jassert (appendSelection || (selection && (selection->size() == 0 || selection->getReference (0).toString() == key.toString())));
    //  if(!appendSelection)selection->clear();
    int numSelectionChange = 0;

    if (shouldSelect  && !selection->contains (key.toString()))
    {
        selection->add (key.toString());
        numSelectionChange = 1;
    }
    else if (!shouldSelect)
    {
        numSelectionChange = selection->removeAllInstancesOf (key.toString());
        jassert( getSelectedSet (value)->size() == selection->size());

    }

    if (numSelectionChange > 0)
    {
        auto msg = EnumChangeMessage::newSelectionMessage (key, shouldSelect, getModel()->isValidId (key));
        processForMessage (*msg, enumListeners);
        asyncNotifier.addMessage (msg);
        DBGENUM ("enum : " << key.toString() << (!shouldSelect ? " not" : "") << " selected" << (!msg->isValid ? " not" : "") << " valid ");


    }

    LOG_ENUM
}

bool EnumParameter::selectFromVar (const var& _value, bool shouldSelect, bool appendSelection)
{
    // select based on integer (<=0 unselect all)
    if (_value.isInt() || _value.isDouble())
    {

        const int idx = (int)_value ;
        auto props = getModel()->getProperties();

        if (idx >= 0 && idx < props.size())
        {
            Identifier key = props.getName (idx);
            selectId (key, shouldSelect, appendSelection);
        }
        else
        {
            unselectAll();
        }

        return true;

    }

    // select based on string
    else if (_value.isString())
    {
        String sV = _value.toString();

        if (sV.isEmpty())
        {
            unselectAll();
        }
        else
        {
            selectId (sV, shouldSelect, appendSelection);
        }

        return true;
    }
    else
    {
        return false;
    }
}


void EnumParameter::unselectAll()
{
    for (auto& s : getSelectedSetIds (value))
    {
        selectId (s, false, true);
    }

    jassert (getSelectedSet (value)->size() == 0) ;
}




Array<var> EnumParameter::getSelectedValues() const
{
    Array<var> res;
    auto vm = getModel()->getProperties();

    for (auto& i : getSelectedSetIds (value))
    {
        var* s =  vm.getVarPointer (i);

        if (s) {res.add (*s );}
        //        else  {jassertfalse;}
    }

    return res;
};

var EnumParameter::getValueForId (const Identifier& i)
{
    return getModel()->getValueForId (i);
}

NamedValueSet getModelPropsFromVar (const var& v)
{
    if (auto dob = v.getProperty (EnumParameter::modelIdentifier, EnumParameter::None).getDynamicObject())
    {
        return dob->getProperties();
    }

    return NamedValueSet();
}
bool EnumParameter::checkValueIsTheSame (const var& v1, const var& v2)
{


    if ( !v1.getDynamicObject() || !v2.getDynamicObject()) return false;
    auto sId1 =getSelectedSetIds (v1);
    auto sId2 = getSelectedSetIds (v2);
    bool selectionChanged = sId1 !=sId2;
    bool modelChanged = getModelPropsFromVar (v1) != getModelPropsFromVar (v2);
    bool hasChanged = (selectionChanged || modelChanged);

    //    DBGENUM("sel");
    //    for(auto d:getSelectedSetIds(v1)){
    //      DBGENUM("v1 : " << d);
    //    }
    //    for(auto d:getSelectedSetIds(v2)){
    //      DBGENUM("v2 : " << d);
    //    }
    //    DBGENUM("model");
    //    for(auto d:getModelPropsFromVar(v1)){
    //      DBGENUM("v1 : " << d.name);
    //    }
    //    for(auto d:getModelPropsFromVar(v2)){
    //      DBGENUM("v2 : " << d.name);
    //    }


    return !hasChanged;

}


void EnumParameter::setValueInternal (const var& _value)
{
    // try to select
    DBGENUM ("enum value : " << _value.toString());

    if (selectFromVar (_value, true, false)) {}
    else if (_value.isUndefined())
    {
        unselectAll();
    }
    else if (_value.isArray())
    {
        unselectAll();

        for (auto& v : *_value.getArray())
        {
            jassert (selectFromVar (v, true, true));
        }
    }

    // rebuild the whole model if needed and select
    else if (DynamicObject* dvalues = _value.getProperty (modelIdentifier, EnumParameter::None).getDynamicObject())
    {

        // if model is stored, this param should own it

        if (auto model = getModel())
        {
            auto oldP = model->getProperties();

            for (auto v : oldP)
            {
                removeOption (v.name);
            }

        }

        for (auto v : dvalues->getProperties())
        {
            addOption (v.name.toString(), v.value);
        }



        for (auto sel : getSelectedSetIds (value))
        {
            selectId (sel.toString(), false, true);
        }

        for (auto& sel : getSelectedSetIds (_value))
        {
            selectId (sel.toString(), true, true);
        }

    }
    else if (Array<var>* vl = getSelectedSet (_value))
    {
        for (auto s : *vl) {selectId (s.toString(), true, true);}

        if (vl->size() == 0) {unselectAll();}
    }
    else
    {
        // var not suported
        jassertfalse;

    }


}




Array<Identifier> EnumParameter::getSelectedSetIds (const juce::var& v) const
{
    Array<Identifier>res;

    if (Array<var>*   arr =  getSelectedSet (v))
    {
        for (auto& e : *arr)
        {
            res.add (Identifier (e.toString()));
        }
    }

    return res;
}

Array<var>* EnumParameter::getSelectedSet (const juce::var& v) const
{
    if (DynamicObject* dob = v.getDynamicObject())
    {
        return dob->getProperty (selectedSetIdentifier).getArray();
    }

    // wrong var passed in
    //  jassertfalse;
    return nullptr;
}


void EnumParameter::modelOptionAdded (EnumParameterModel*, Identifier& key )
{
    auto msg = EnumChangeMessage::newStructureChangeMessage (key, true);
    msg->isValid = getModel()->isValidId (msg->key);
    if (getSelectedIds().contains (key))
    {
        msg->isSelectionChange = true;
        msg->isSelected = true;
        ParameterBase::notifyValueChanged();
    }

    processForMessage (*msg, enumListeners);
    asyncNotifier.addMessage (msg);

};
void EnumParameter::modelOptionRemoved (EnumParameterModel*, Identifier& key)
{
    auto msg = EnumChangeMessage::newStructureChangeMessage (key, false);
    msg->isValid = getModel()->isValidId (msg->key);
    if (getSelectedIds().contains (key))
    {
        msg->isSelectionChange = true;
        msg->isSelected = false;
        ParameterBase::notifyValueChanged();
    }

    processForMessage (*msg, enumListeners);
    asyncNotifier.addMessage (msg);
};

void EnumParameter::processForMessage (const EnumChangeMessage& msg, ListenerList<EnumListener>& _listeners)
{
    if (msg.isStructureChange)
    {
        if (msg.isAdded)
        {
            _listeners.call (&EnumListener::enumOptionAdded, this, msg.key);
        }
        else
        {
            _listeners.call (&EnumListener::enumOptionRemoved, this, msg.key);
        }

    }

    if (msg.isSelectionChange)
    {
        // check validity state has not changed
        bool curValidState = getModel()->isValidId (msg.key);

        // if changed call synchronous another time with validity state updated
        if (msg.isValid != curValidState )
        {
            jassert (&_listeners == &asyncEnumListeners);
            enumListeners.call (&EnumListener::enumOptionSelectionChanged, this, msg.isSelected, curValidState, msg.key);
        }

        _listeners.call (&EnumListener::enumOptionSelectionChanged, this, msg.isSelected, curValidState, msg.key);


    }
}

void EnumParameter::newMessage (const EnumChangeMessage& msg)
{

    processForMessage (msg, asyncEnumListeners);

};

String EnumParameter::stringValue() const
{
    auto  selected =  getSelectedIds();

    if (selected.size() == 0)
    {
        return "";
    }
    else if (selected.size() == 1)
    {
        return selected[0].toString();
    }
    else
    {
        String res = selected[0].toString();

        for (int i = 1 ; i < selected.size() ; i++)
        {
            res += "," + selected[i].toString();
        }

        return res;
    }
}


///////////////
// EnumParameterModel



EnumParameterModel::EnumParameterModel(): addFunction (nullptr)
{
    autoGenerated = new DynamicObject();
    userGenerated = new DynamicObject();
}

EnumParameterModel::~EnumParameterModel()
{
    EnumParameterModel::masterReference.clear();

}

void EnumParameterModel::setIsFileBased (bool _isFileBased)
{
    if (_isFileBased)
    {
        addFunction = [] (EnumParameter * ep)
        {
            FileChooser fc ("choose file : " + ep->niceName);
            bool res = false;
            Identifier key;
            String value;

            if (fc.browseForFileToOpen())
            {
                File f ( fc.getResult());

                if (f.exists())
                {
                    res = true;
                    key = f.getFileNameWithoutExtension();
                    value = f.getFullPathName();
                }
            }

            return std::tuple<bool, Identifier, var> (res, key, value);
        };
    }
    else
    {
        addFunction = nullptr;
    }
}

const Identifier& EnumParameterModel::getIdForValue(const var & v){
    static Identifier notFoundId("notFound");


    for(auto & k:userGenerated->getProperties()){
        if(k.value == v){return k.name;}
    }

    for(auto & k:autoGenerated->getProperties()){
        if(k.value == v){return k.name;}
    }


    return notFoundId;

}

void EnumParameterModel::addOption (Identifier key, var data, bool isAutoGenerated)
{
    // we don't want to override existing
    jassert (!isValidId (key));
    addOrSetOption (key, data, isAutoGenerated);

}

void EnumParameterModel::addOrSetOption (Identifier key, var data, bool isAutoGenerated)
{


    DynamicObject* d = isAutoGenerated ? autoGenerated.get() : userGenerated.get();
    DynamicObject* otherd = !isAutoGenerated ? autoGenerated.get() : userGenerated.get();
    // watchout duplicates in user defined
    jassert (!otherd->hasProperty (key));
    bool isNew = !d->hasProperty (key);
    d->setProperty (key, data);

    if (isNew)
    {
        listeners.call (&Listener::modelOptionAdded, this, key);
    }
}

void EnumParameterModel::removeOption (Identifier key, bool isAutoGenerated)
{
    DynamicObject* d = isAutoGenerated ? autoGenerated.get() : userGenerated.get();
    bool hadOption = false;
    hadOption = d->hasProperty (key);
    d->removeProperty (key);

    if (hadOption)
    {
        listeners.call (&Listener::modelOptionRemoved, this, key);
    }

}

var EnumParameterModel::getValueForId (const Identifier& key)
{
    if (userGenerated->hasProperty (key))
    {
        return userGenerated->getProperty (key);
    }
    else
    {
        return autoGenerated->getProperty (key);
    }
}

bool EnumParameterModel::isValidId (Identifier key)
{
    return userGenerated->hasProperty (key) || autoGenerated->hasProperty (key);
}
const NamedValueSet EnumParameterModel::getProperties()noexcept
{
    NamedValueSet res (userGenerated->getProperties());
    
    for (auto k : autoGenerated->getProperties())
    {
        jassert (!res.contains (k.name));
        res.set (k.name, k.value);
    }
    
    return res;
}
DynamicObject* EnumParameterModel::getObject()
{
    return userGenerated;
    
}



