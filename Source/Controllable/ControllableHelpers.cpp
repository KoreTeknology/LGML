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


#include "ControllableHelpers.h"
#include "../Utils/DebugHelpers.h"

#include "../Node/Manager/NodeManager.h"

ControllableContainerPopupMenu::ControllableContainerPopupMenu (ControllableContainer* rootContainer, Array<Controllable*>* _filterOut ):
    filterOutControllable (_filterOut)
{
    int id = 1;

    if (rootContainer == nullptr) rootContainer = NodeManager::getInstance(); //to replace with global app container containing nodes, controllers, rules, etc...

    populateMenu (this, rootContainer, id);
}

ControllableContainerPopupMenu::~ControllableContainerPopupMenu()
{
}

void ControllableContainerPopupMenu::populateMenu (PopupMenu* subMenu, ControllableContainer* container, int& currentId)
{
    if (subMenu != this)
    {
        for (auto& c : container->controllables)
        {
            if (c->isControllableExposed && (!filterOutControllable || !filterOutControllable->contains (c)))
            {
                subMenu->addItem (currentId, c->niceName);
                controllableList.add (c);
                currentId++;
            }
        }
    }

    for (auto& cc : container->controllableContainers)
    {
        PopupMenu p;
        populateMenu (&p, cc, currentId);
        subMenu->addSubMenu (cc->getNiceName(), p);
    }
}

Controllable* ControllableContainerPopupMenu::showAndGetControllable()
{
    int result = show();

    if (result == 0) return nullptr;

    return controllableList[result - 1];
}



ControllableReferenceUI::ControllableReferenceUI (ControllableContainer* container, Array<Controllable*> filterOut) :
    rootContainer (container),
    TextButton ("Target"),
    filterOutControllable (filterOut)
{
    addListener (this);
    setTooltip ("Choose a target");
}

ControllableReferenceUI::~ControllableReferenceUI()
{
    removeListener (this);
}


void ControllableReferenceUI::setCurrentControllale (Controllable* c)
{

    if (currentControllable.get() == c) return;

    currentControllable = c;

    if (c != nullptr)
    {
        setTooltip ("Current Controllable :" + c->niceName + String ("\n") + c->controlAddress);
        setButtonText (c->niceName);
    }
    else
    {
        setTooltip ("Choose a controllable");
        setButtonText ("Target");
    }


    listeners.call (&Listener::choosedControllableChanged, this, c);
}

void ControllableReferenceUI::buttonClicked (Button*)
{
    ControllableContainerPopupMenu p (rootContainer, &filterOutControllable);
    Controllable* c = p.showAndGetControllable();

    if (c != nullptr) setCurrentControllale (c);
}
