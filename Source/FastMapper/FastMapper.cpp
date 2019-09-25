/*
 ==============================================================================

 Copyright © Organic Orchestra, 2017

 This file is part of LGML. LGML is a software to manipulate sound in real-time

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation (version 3 of the License).

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 ==============================================================================
 */

#include "FastMapper.h"

#if !ENGINE_HEADLESS
#include "../Controllable/Parameter/UI/ParameterUI.h"
#endif
#include "../Controllable/Parameter/UI/ParameterUIHelpers.h" // keep after ParameterUI.h

#include "../Controller/ControllerManager.h"
juce_ImplementSingleton (FastMapper)
IMPL_OBJ_TYPE (FastMapper);

template<>
void ParameterContainer::OwnedFeedbackListener<FastMapper>::parameterFeedbackUpdate (ParameterContainer* notif, ParameterBase* p,ParameterBase::Listener * notifier) {
    static uint32 lastAddedTime = 0;
    if ( p)
    {
        if(owner->autoAddFastMaps){
            if(notif== ControllerManager::getInstance()){
                auto now=Time::getMillisecondCounter();
//                jassert(now>=owner->lastFMAddedTime);
                // debounce control changes, to avoid setting potentialOutput back
                if (p->isUserDefined && ((now-lastAddedTime)>500)){
                    lastAddedTime = now;
                    owner->setPotentialInput (p);
                }
            }
        }


    }
}



FastMapper::FastMapper (StringRef name) :
ParameterContainer (name),
autoAddFastMaps(false),
lastFMAddedTime(0),
pSync(this)
{
    canHaveUserDefinedContainers = true;
    nameParam->setInternalOnlyFlags(true,false);
    potentialIn = addNewParameter<ParameterProxy> ("Input", "potential input for new fastMap,\nto assing :\n- move a controller in mapping mode\n- alt click on other LGML parameter\n- use this popup");
    potentialOut = addNewParameter<ParameterProxy> ("Output", "potential output for new fastMap\nto assign :\n- click on parameter in mapping mode\n- navigate through this popup");

    potentialIn->addParameterProxyListener(this);
    potentialOut->addParameterProxyListener(this);
#if !ENGINE_HEADLESS
    LGMLDragger::getInstance()->addSelectionListener (this);
#endif

    potentialIn->setSavable(false);
    potentialOut->setSavable(false);
    


}

FastMapper::~FastMapper()
{
#if !ENGINE_HEADLESS
    if (auto* dr = LGMLDragger::getInstanceWithoutCreating())
    {
        dr->removeSelectionListener (this);
    }
#endif
    if(auto cm = ControllerManager::getInstanceWithoutCreating()){
        cm->removeFeedbackListener(&pSync);
    }

    clear();
}



void FastMapper::setPotentialInput ( ParameterBase* p)
{
    if( p!=potentialIn->getLinkedParam()){
        potentialIn->setParamToReferTo (p);

    }
}
void FastMapper::setPotentialOutput ( ParameterBase* p )
{
    if( p!=potentialOut->getLinkedParam()){
        potentialOut->setParamToReferTo (p);
    }
}
void FastMapper::createNewFromPotentials()
{
    if (potentialIn->getLinkedParam() && potentialOut->getLinkedParam() )
    {
        addFastMap();

    }
}
void FastMapper::clear()
{
    while (maps.size())
    {
        removeFastmap (maps[0]);
    }

    potentialOut->setParamToReferTo (nullptr);
    potentialIn->setParamToReferTo (nullptr);

}


FastMap* FastMapper::addFastMap()
{





    if(!checkDuplicates (potentialIn->getLinkedParam(),potentialOut->getLinkedParam())){
        lastFMAddedTime = Time::getMillisecondCounter();
        std::unique_ptr<FastMap> f ( new FastMap());
        f->nameParam->isEditable = true;
        f->referenceIn->setParamToReferTo (potentialIn->getLinkedParam());
        f->referenceOut->setParamToReferTo (potentialOut->getLinkedParam());
        addChildControllableContainer (f.get());
        maps.add (f.get());
        f->referenceIn->addParameterProxyListener(this);
        f->referenceOut->addParameterProxyListener(this);


        auto addedFastMap = f.release();
#if !ENGINE_HEADLESS

        WeakReference<FastMap> wkf(addedFastMap);
        // avoid listener feedback
        MessageManager::callAsync([this,wkf](){
            if(!wkf.get()){return;}
            auto pIns = wkf->referenceIn->linkedParam?AllParamType::getAllParameterUIs().getForParameter(wkf->referenceIn->linkedParam):AllParamType::ArrayType();
            auto pOuts = wkf->referenceOut->linkedParam?AllParamType::getAllParameterUIs().getForParameter(wkf->referenceOut->linkedParam):AllParamType::ArrayType();

            for(auto p : pIns){p->setHasMappedParameter(true);}
            for(auto p : pOuts){p->setHasMappedParameter(true);}

            if(auto dr = LGMLDragger::getInstance()){
                dr->setSelected(nullptr,this);
            }
            potentialIn->setParamToReferTo (nullptr);
            potentialOut->setParamToReferTo (nullptr);
        });
#endif
        return addedFastMap;
    }
    else{
        return nullptr;
    }
}

bool FastMapper::checkDuplicates (ParameterBase* referenceInParam,ParameterBase* referenceOutParam)
{
    bool dup = false;

    if (referenceInParam == nullptr && referenceOutParam == nullptr)
        return false;

    for (auto& ff : maps)
    {
//        if (ff == f)continue;

        if (ff->referenceIn->getLinkedParam() == referenceInParam &&
            ff->referenceOut->getLinkedParam() == referenceOutParam)
        {
            dup = true;
        }
        else if (ff->referenceIn->getLinkedParam() == referenceOutParam &&
                 ff->referenceOut->getLinkedParam() == referenceInParam)
        {
            dup = true;
        }

        if (dup)
        {
            LOGE(juce::translate("can't duplicate fastMap"));

            return true;

        }
    }

    return false;
}

void FastMapper::removeFastmap (FastMap* f)
{
    f->referenceIn->setParamToReferTo(nullptr);
    f->referenceOut->setParamToReferTo(nullptr);
    jassert (f);
    removeChildControllableContainer (f);
    maps.removeObject (f);
}


bool FastMapper::removeMappingIncluding(ParameterBase *p){
    Array<FastMap*> toRemove;
    for(auto & m:maps){
        if(m && m->getProxyForParameter(p)){
            toRemove.add(m);
        }
    }

    for(auto m:toRemove){
        removeFastmap(m);
    }
    return toRemove.size()>0;
}

ParameterContainer*   FastMapper::addContainerFromObject (const String& /*name*/, DynamicObject*   fData)
{
    FastMap* f = addFastMap();
    if(f){
        f->configureFromObject(fData);
    }
    return f;


}



#if !ENGINE_HEADLESS
void FastMapper::selectionChanged ( ParameterBase* c )
{

    auto ms = Desktop::getInstance().getMouseSource(0);
    if (ms&& ms->getCurrentModifiers().isAltDown()){
        setPotentialInput ( ParameterBase::fromControllable (c));
    }
    else{
        setPotentialOutput ( ParameterBase::fromControllable (c));
    }

};


void FastMapper::mappingModeChanged(bool state){
    autoAddFastMaps = state;
    if(auto cm = ControllerManager::getInstance()){
        if(state){
            cm->addFeedbackListener(&pSync);
        }
        else{
            cm->removeFeedbackListener(&pSync);
            setPotentialInput(nullptr);
            setPotentialOutput(nullptr);
        }
    }


};

#endif
void  FastMapper::linkedParamChanged (ParameterProxy* p ) {
    if(p== potentialIn || p== potentialOut){
        createNewFromPotentials();
    }
    else{
#if !ENGINE_HEADLESS
        // remove old mapped uis
        String lastAddressS = p->lastValue;
        if(lastAddressS.isNotEmpty()){
            ControlAddressType lastAddress =  ControlAddressType::fromString(lastAddressS);
            auto lastP = dynamic_cast<ParameterBase*>(ControllableContainer::getRoot(true)->getControllableForAddress(lastAddress));
            if(lastP){
                bool isStillMapped=false;
                for(auto fm : maps){
                    if(fm->getProxyForParameter(lastP)){
                        isStillMapped = true;
                        break;
                    }
                }
                if(!isStillMapped){
                    auto & allUis (AllParamType::getAllParameterUIs());
                    auto pUIs = allUis.getForParameter(lastP);
                    for(auto ui:pUIs){
                        ui->setHasMappedParameter(false);
                    }
                }
            }
        }

        // add newMapped uis
        String addrS = p->stringValue();
        if(addrS.isNotEmpty()){
            ControlAddressType addr =  ControlAddressType::fromString(addrS);
            auto lastP = dynamic_cast<ParameterBase*>(ControllableContainer::getRoot(true)->getControllableForAddress(addr));
            if(lastP){
                auto pUIs = AllParamType::getAllParameterUIs().getForParameter(lastP);
                for(auto ui:pUIs){
                    ui->setHasMappedParameter(true);
                }
            }
        }


#endif
    }

}

bool  FastMapper::isParameterMapped (ParameterBase * p){
    if(!p)return false;
    //    jassert(p);
    for (const auto * f : maps){
        if(f->getProxyForParameter(p)){
            return true;
        }
    }

    return false;
}
