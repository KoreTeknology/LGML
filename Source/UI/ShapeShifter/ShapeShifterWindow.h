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


#pragma once

#include "ShapeShifterPanel.h"
#include "../Style.h" // for USE_GL

// significantly slower on osx
// TODO benchmark use of GL
#define USE_GL_FOR_EACH_WINDOW 0

class ShapeShifterWindow :
    public ResizableWindow,
    public ShapeShifterPanel::Listener
{
public:
    ShapeShifterWindow (ShapeShifterPanel* _panel, Rectangle<int> bounds);
    ~ShapeShifterWindow();

    enum DragMode { NONE, TAB, PANEL };

    DragMode dragMode;

    ShapeShifterPanel* panel;

    bool checking;

    void paintOverChildren (Graphics& g)override;
    void resized() override;

    ComponentDragger dragger;
    void mouseDown (const MouseEvent& e) override;
    void mouseDrag (const MouseEvent& e) override;
    void mouseUp (const MouseEvent& e) override;


    void clear();

    void userTriedToCloseWindow() override;

    virtual var getCurrentLayout();

    void panelEmptied (ShapeShifterPanel*) override;


#if USE_GL_FOR_EACH_WINDOW
    OpenGLContext openGLContext;
#endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ShapeShifterWindow)
};


