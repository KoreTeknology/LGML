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


#if !ENGINE_HEADLESS

#include "GapGrabber.h"
#include "../Style.h"
#include "ShapeShifterContainer.h"


struct MiniHandle : public Component,SettableTooltipClient{
    MiniHandle(GapGrabber * o,bool dir):owner(o),direction(dir){
        setRepaintsOnMouseActivity (true);
        bool isH = owner->direction==GapGrabber::Direction::HORIZONTAL;
        if(isH){
            if(dir){
                setTooltip(juce::translate("toggles mini mode for panel on right"));
                setMouseCursor(MouseCursor::RightEdgeResizeCursor);
            }
            else{
                setTooltip(juce::translate("toggles mini mode for panel on left"));
                setMouseCursor(MouseCursor::LeftEdgeResizeCursor);
            }
        }
        else{
            if(dir){
                setTooltip(juce::translate("toggles mini mode for panel on bottom"));
                setMouseCursor(MouseCursor::BottomEdgeResizeCursor);
            }
            else{
                setTooltip(juce::translate("toggles mini mode for panel on top"));
                setMouseCursor(MouseCursor::TopEdgeResizeCursor);
            }
        }
    }

    void paint(Graphics & g)final{
        Colour c = findColour (ResizableWindow::backgroundColourId).brighter (.1f);
        if (isMouseOver(false)){ c = findColour (TextButton::buttonOnColourId);}

        g.setColour(c);
        auto r = getLocalBounds();

        if(owner->direction==GapGrabber::Direction::VERTICAL){
            int yh = r.getY() + r.getHeight()/2;
            g.drawLine(r.getX(), yh, r.getRight(), yh);
        }
        else{
            int x = r.getX() + r.getWidth()/2;
            g.drawLine(x, r.getY()  , x, r.getBottom());
        }

    }
    void mouseUp (const MouseEvent& e) {
        if(e.getDistanceFromDragStart()>0)return;

        ShapeShifterContainer * parent = findParentComponentOfClass<ShapeShifterContainer>();
        if(!parent){
            jassertfalse;
            return;
        }

        bool otherMinimized = false;
        ShapeShifter * toReduce = parent->shifters[parent->grabbers.indexOf(owner)+(direction?1:0)];
        for(auto & s:parent->shifters){if(s!=toReduce)otherMinimized|=s->isMini;}
        if(!otherMinimized){
            toReduce->setMini(!toReduce->isInAutoMiniMode());
            owner->updateMiniHandleVisibility();
        }
        else{
            jassertfalse;
        }
    }

    GapGrabber * owner;
    bool direction;
};

struct StretchHandle : public Component{
    StretchHandle(GapGrabber * o):owner(o){
        setRepaintsOnMouseActivity (true);
        setMouseCursor (owner->direction == GapGrabber::Direction::HORIZONTAL ?
                        MouseCursor::LeftRightResizeCursor :
                        MouseCursor::UpDownResizeCursor);
    }

    void paint(Graphics & g)final{
        Rectangle<int> r = getLocalBounds().reduced(1);
        Colour c = findColour (ResizableWindow::backgroundColourId).brighter (.1f);
        if (isMouseOver(false)){ c = findColour (TextButton::buttonOnColourId);}
        if (isMouseButtonDown()){c = c.brighter();}
        g.setColour (c);
        g.fillRoundedRectangle (r.toFloat(), 2);
    }

    void mouseDrag (const MouseEvent& e) override
    {
        owner->listeners.call (&GapGrabber::Listener::grabberGrabUpdate,owner,
                               owner->direction == GapGrabber::Direction::HORIZONTAL ?
                               e.getPosition().x : e.getPosition().y);
    }
    GapGrabber * owner;
};


GapGrabber::GapGrabber (Direction _direction) : direction (_direction)
{
    goMiniHandle = new MiniHandle(this,true);
    addAndMakeVisible(goMiniHandle);
    goMiniHandle2 = new MiniHandle(this,false);
    addAndMakeVisible(goMiniHandle2);
    stretchHandle = new StretchHandle(this);
    addAndMakeVisible(stretchHandle );

}

GapGrabber::~GapGrabber(){}

void GapGrabber::resized() {
    Rectangle<int> r = getLocalBounds().reduced (1);
    Rectangle<int> minR ,grabR,minR2;
    if(direction==Direction::HORIZONTAL){
        minR = r.withTrimmedBottom(r.getHeight()*.75);
        minR.reduce(0, minR.getHeight()/4);
        minR2 = r.withTrimmedTop(r.getHeight()*.75);
        minR2.reduce(0, minR2.getHeight()/4);
        grabR = r.reduced(0,r.getHeight()/4);
    }
    else{
        minR = r.withTrimmedRight(r.getWidth()*.75);
        minR.reduce(minR.getWidth()/4,0);
        minR2 = r.withTrimmedLeft(r.getWidth()*.75);
        minR2.reduce(minR2.getWidth()/4,0);
        grabR = r.reduced(r.getWidth()/4, 0);

    }
    goMiniHandle->setBounds(minR);
    goMiniHandle2->setBounds(minR2);
    stretchHandle->setBounds(grabR);

};

void GapGrabber::updateMiniHandleVisibility(){
    ShapeShifterContainer * parent = findParentComponentOfClass<ShapeShifterContainer>();
    if(!parent){
        jassertfalse;
        return;
    }


    for(int i = 0 ; i < parent->grabbers.size() ; i++){
        auto s = parent->shifters.getUnchecked(i);
        auto s2 = parent->shifters.getUnchecked(i+1);
        goMiniHandle ->setVisible(!s->isInAutoMiniMode());
        goMiniHandle2->setVisible(!s2->isInAutoMiniMode());
    }

}

void GapGrabber::parentHierarchyChanged(){
    updateMiniHandleVisibility();
}


#endif
