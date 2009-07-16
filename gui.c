/* Copyright (C) 2009 Nagy Attila Gabor <nagy.attila.gabor@gmail.com>
 *
 *     This file is part of TrackMii.
 *
 *  TrackMii is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TrackMii is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cwiid.h>
#include <math.h>

#define XPLM200 = 1;
#include "XPLMGraphics.h"
#include "XPLMMenus.h"
#include "XPWidgets.h"
#include "XPStandardWidgets.h"
#include "XPWidgetUtils.h"
#include "trackmii_plugin.h"
#include "pose.h"

int SetupWindowHandler(XPWidgetMessage inMessage,
                       XPWidgetID      inWidget,
                       long            inParam1,
                       long            inParam2);

void MenuHandler(void *, void *);

// Widgets we use
XPLMMenuID menuId;
XPWidgetID setupWindowWidget = NULL;
XPWidgetID smoothingScrollbar = NULL;
XPWidgetID smoothingValueCaption = NULL;


XPWidgetID wiimoteConnected = NULL;
XPWidgetID wiimoteDisconnected = NULL;
XPWidgetID connectButton = NULL;
XPWidgetID debugCheckbox = NULL;

XPWidgetID radioWiimote = NULL;
XPWidgetID radioTIR = NULL;

typedef enum TrParams {deadzone, response, amplification} trParam;

typedef struct trSetup {
    XPWidgetID scrollbar;
    XPWidgetID caption;
    trParam trParam;
    int dof;
} trSetup;

trSetup translationWindows[5][3];

/**
 * Initializing menu, and other gui stuff
 */
void InitGui()
{
    int item;

    /* Registering into the menu */
    item = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "TrackMii", NULL, 1);
    menuId = XPLMCreateMenu("TrackMii setup", XPLMFindPluginsMenu(), item, MenuHandler, NULL);
    XPLMAppendMenuItem(menuId, "Setup TrackMii", (void *)"setup", 1);
}

/**
 * Cleaning up all gui items before exit
 */
void DestroyGui()
{
    XPLMDestroyMenu(menuId);
}

/**
 * Setup window handler - receives all events
 * to the setup window.
 */
int SetupWindowHandler(XPWidgetMessage inMessage,
                       XPWidgetID      inWidget,
                       long            inParam1,
                       long            inParam2)
{
    char buff[255];
    long tmp;
    int i, j;
    basicTranslationCfg trcfg;
    if (inMessage == xpMessage_CloseButtonPushed)
    {
        XPHideWidget(setupWindowWidget);
        SaveSettings();
    }
    else if (inMessage == xpMsg_ScrollBarSliderPositionChanged)
    {
        

        for (i=0; i<5; i++) {
            for (j=0; j<3; j++) {
                if (inParam1 == (long)translationWindows[i][j].scrollbar) {
                    tmp = XPGetWidgetProperty(translationWindows[i][j].scrollbar,
                            xpProperty_ScrollBarSliderPosition,
                            NULL);

                    trcfg = getTranslationCfg(translationWindows[i][j].dof);
                    switch (translationWindows[i][j].trParam) {
                        case deadzone:
                            trcfg.deadzone = tmp; break;
                        case response:
                            trcfg.response = tmp; break;
                        case amplification:
                            trcfg.amplification = tmp; break;
                    }
                    setTranslationCfg(translationWindows[i][j].dof, &trcfg);
                    sprintf(buff, "%ld", tmp);
                    XPSetWidgetDescriptor(translationWindows[i][j].caption, buff);
                    return 0;
                }
            }
        }
        if (inParam1 == (long)smoothingScrollbar) {
            tmp = XPGetWidgetProperty(smoothingScrollbar, xpProperty_ScrollBarSliderPosition, NULL);

            setSmoothing(tmp);

            sprintf(buff, "%ld", tmp);
            XPSetWidgetDescriptor(smoothingValueCaption, buff);
        }
    }
    else if (inMessage == xpMsg_PushButtonPressed)
    {
        if (inParam1 == (long)connectButton) {
            ConnectCam();
            if (getConnectionState()) {
                XPHideWidget(wiimoteDisconnected);
                XPShowWidget(wiimoteConnected);
            } else {
                XPShowWidget(wiimoteDisconnected);
                XPHideWidget(wiimoteConnected);
            }
        }
    }
    else if (inMessage == xpMsg_ButtonStateChanged) {
        if (inParam1 == (long)debugCheckbox) {
            ToggleDebugWindowVisible(inParam2);
        }
        else if (inParam1 == (long)radioWiimote) {
            XPSetWidgetProperty(radioTIR, xpProperty_ButtonState, 0);
            DisconnectCam();
            SetCamDevice(wiimote);
            XPShowWidget(wiimoteDisconnected);
            XPHideWidget(wiimoteConnected);
        }
        else if (inParam1 == (long)radioTIR) {
            XPSetWidgetProperty(radioWiimote, xpProperty_ButtonState, 0);
            DisconnectCam();
            SetCamDevice(tir4_camera);
            XPShowWidget(wiimoteDisconnected);
            XPHideWidget(wiimoteConnected);
        }
    }
    return 0;
}

/**
 * Generic handler for subwindows, dispatches everything upwards
 */
int SubwindowHandler(XPWidgetMessage inMessage,
                     XPWidgetID      inWidget,
                     long            inParam1,
                     long            inParam2)
{
    return 0;
}


/**
 * Creating a scrollbar handle, as we like it
 */
void CreateScrollbar(XPWidgetID setupWindowWidget,
                     XPWidgetID *scrollbar,
                     XPWidgetID *valueCaption,
                     char *label,
                     int x, int y, int x2,
                     int value,
                     int min,
                     int max)
{
    char buff[255];

    XPCreateWidget(x, y, x + 80, y-20, 1, label, 0, setupWindowWidget, xpWidgetClass_Caption);
    XPCreateWidget(x2-30, y, x2, y-20, 1, "max", 0, setupWindowWidget, xpWidgetClass_Caption);

    sprintf(buff, "%d", value);
    *valueCaption =
            XPCreateWidget(x+160, y+5, x+170, y, 1, buff, 0, setupWindowWidget, xpWidgetClass_Caption);

    *scrollbar = XPCreateWidget(x + 61, y, x2 - 30, y - 20,
                                        1,
                                        label,
                                        0,
                                        setupWindowWidget,
                                        xpWidgetClass_ScrollBar);

    XPSetWidgetProperty(*scrollbar, xpProperty_ScrollBarType, xpScrollBarTypeSlider);
    XPSetWidgetProperty(*scrollbar, xpProperty_ScrollBarMin, min);
    XPSetWidgetProperty(*scrollbar, xpProperty_ScrollBarMax, max);
    XPSetWidgetProperty(*scrollbar, xpProperty_ScrollBarSliderPosition, value);
}

/**
 * Create a window with three scrollbars: deadzone, response, amplification
 * It can be used to control a translation function for one dof
 */
void CreateTranslationSetupWindow(XPWidgetID parent, int x, int y, char *label, int dof, trSetup *setup) {
    XPWidgetID w;
    basicTranslationCfg trcfg;
    
    w = XPCreateWidget(x, y, x+250, y-110, 1,
                       "", 0, parent, xpWidgetClass_SubWindow);
    XPSetWidgetProperty(w, xpProperty_SubWindowType, xpSubWindowStyle_SubWindow);

    /* yaw deadzone scrollbar */
    XPCreateWidget(x+5, y, x + 70, y-20, 1, label, 0, parent, xpWidgetClass_Caption);

    trcfg = getTranslationCfg(dof);

    CreateScrollbar(parent, &setup[0].scrollbar, &setup[0].caption,
            "Deadzone:",
            x+10, y-25, x+250,
            trcfg.deadzone,
            0, 30);

    CreateScrollbar(parent, &setup[1].scrollbar, &setup[1].caption,
            "Response:",
            x+10, y-55, x+250,
            trcfg.response,
            0, 80);

    CreateScrollbar(parent, &setup[2].scrollbar, &setup[2].caption,
            "Ampl.:",
            x+10, y-85, x+250,
            trcfg.amplification,
            0, 100);

    setup[0].dof       = dof;
    setup[0].trParam   = deadzone;

    setup[1].dof       = dof;
    setup[1].trParam   = response;

    setup[2].dof       = dof;
    setup[2].trParam   = amplification;
}

/**
 * Create a label widget, with text centered between left and
 * right x coordinates
 */
XPWidgetID LabelCentered(XPWidgetID *parent, int x, int y, int x2, char *text) {
    float len;
    int pos;
    len = XPLMMeasureString(xplmFont_Proportional, text, strlen(text));
    pos = (x2-x)/2-ceil(len/2);
    return XPCreateWidget(x+pos, y, x+pos+(int)len, y-12, 1,
            text,
            0, parent, xpWidgetClass_Caption);
}

/**
 * Here we create the setup window, and all it's sub widgets
 */
void CreateSetupWindow()
{
    int x, y, x2, y2, w, h;

    x = 100;
    y = 650;
    w = 790;
    h = 330;
    x2 = x + w;
    y2 = y - h;

    // Gray windows
    XPWidgetID w1, w2;

    setupWindowWidget = XPCreateWidget(x, y, x2, y2,
                                       1, // Visible
                                       "TrackMii Setup", // desc
                                       1, // root
                                       NULL,
                                       xpWidgetClass_MainWindow);
    XPSetWidgetProperty(setupWindowWidget, xpProperty_MainWindowHasCloseBoxes, 1);

    /* Upper subwindow */
    w1 = XPCreateWidget(x+10, y-30, x+520, y-80, 1,
                        "", 0, setupWindowWidget, xpWidgetClass_SubWindow);
    XPSetWidgetProperty(w1, xpProperty_SubWindowType, xpSubWindowStyle_SubWindow);
    
    /* smoothing scrollbar */
    CreateScrollbar(setupWindowWidget, &smoothingScrollbar, &smoothingValueCaption,
            "Smoothing:",
            x+20, y-50, x+350,
            getSmoothing(),
            1, 100);

    /* Radio button to enable/disable debugging */
    debugCheckbox = XPCreateWidget(x + 385, y-50, x + 400, y - 70,
                                        1,
                                        "",
                                        0,
                                        setupWindowWidget,
                                        xpWidgetClass_Button);
    XPSetWidgetProperty(debugCheckbox, xpProperty_ButtonType, xpRadioButton);
    XPSetWidgetProperty(debugCheckbox, xpProperty_ButtonBehavior, xpButtonBehaviorCheckBox);
    XPCreateWidget(x+400, y-50, x+510, y-65, 1, "show debug window", 0, setupWindowWidget, xpWidgetClass_Caption);

    /* yaw subwindow */
    CreateTranslationSetupWindow(setupWindowWidget,
            x+10, y-90,
            "Yaw translation setup",
            DOF_YAW,
            translationWindows[0]);

    /* pitch subwindow */
    CreateTranslationSetupWindow(setupWindowWidget,
            x+270, y-90,
            "Pitch translation setup",
            DOF_PITCH,
            translationWindows[1]);

    /* panX subwindow */
    CreateTranslationSetupWindow(setupWindowWidget,
            x+10, y-210,
            "Horizontal translation setup",
            DOF_PANX,
            translationWindows[2]);

    /* panY subwindow */
    CreateTranslationSetupWindow(setupWindowWidget,
            x+270, y-210,
            "Vertical translation setup",
            DOF_PANY,
            translationWindows[3]);

    /* panZ subwindow */
    CreateTranslationSetupWindow(setupWindowWidget,
            x+530, y-210,
            "Depth translation setup",
            DOF_PANZ,
            translationWindows[4]);

    /* linux-track device selector */
    w2 = XPCreateWidget(x+530, y-30, x2-10, y-80, 1,
                        "", 0, setupWindowWidget, xpWidgetClass_SubWindow);
    XPSetWidgetProperty(w2, xpProperty_SubWindowType, xpSubWindowStyle_SubWindow);

    radioWiimote = XPCreateWidget(x + 540, y-40, x + 555, y - 55,
                                        1,
                                        "",
                                        0,
                                        setupWindowWidget,
                                        xpWidgetClass_Button);
    XPSetWidgetProperty(radioWiimote, xpProperty_ButtonType, xpRadioButton);
    XPSetWidgetProperty(radioWiimote, xpProperty_ButtonBehavior, xpButtonBehaviorRadioButton);
    XPCreateWidget(x+560, y-40, x2-20, y-55, 1, "use Wiimote", 0, setupWindowWidget, xpWidgetClass_Caption);
    if (GetCamDevice() == wiimote) {
        XPSetWidgetProperty(radioWiimote, xpProperty_ButtonState, 1);
    }
    
    radioTIR = XPCreateWidget(x + 540, y-55, x + 555, y - 70,
                                        1,
                                        "",
                                        0,
                                        setupWindowWidget,
                                        xpWidgetClass_Button);
    XPSetWidgetProperty(radioTIR, xpProperty_ButtonType, xpRadioButton);
    XPSetWidgetProperty(radioTIR, xpProperty_ButtonBehavior, xpButtonBehaviorRadioButton);
    XPCreateWidget(x+560, y-55, x2-20, y-70, 1, "use Track IR4", 0, setupWindowWidget, xpWidgetClass_Caption);
    if (GetCamDevice() == tir4_camera) {
        XPSetWidgetProperty(radioTIR, xpProperty_ButtonState, 1);
    }


    /* Wiimote connected subwindows */
    wiimoteConnected = XPCreateWidget(x+530, y-90, x2-10, y-200, 0,
                                      "Wiimote is connected",
                                      0, setupWindowWidget, xpWidgetClass_SubWindow);

    XPSetWidgetProperty(wiimoteConnected, xpProperty_SubWindowType, xpSubWindowStyle_SubWindow);
    XPAddWidgetCallback(wiimoteConnected, XPUFixedLayout);
    LabelCentered(wiimoteConnected, x+540, y-140, x2-20, "Wiimote is connected");

    /* Wiimote disconnected subwindow, with connect button */
    wiimoteDisconnected = XPCreateWidget(x+530, y-90, x2-10, y-200, 0,
                                      "Wiimote is currently not connected",
                                      0, setupWindowWidget, xpWidgetClass_SubWindow);

    XPSetWidgetProperty(wiimoteDisconnected, xpProperty_SubWindowType, xpSubWindowStyle_SubWindow);
    XPAddWidgetCallback(wiimoteDisconnected, XPUFixedLayout);

    LabelCentered(wiimoteDisconnected, x+540, y-100, x2-20, "Wiimote is currently not connected");
    LabelCentered(wiimoteDisconnected, x+540, y-120, x2-20, "To connect the wiimote you should put it");
    LabelCentered(wiimoteDisconnected, x+540, y-133, x2-20, "first in discoverable mode (press 1+2),");
    LabelCentered(wiimoteDisconnected, x+540, y-145, x2-20, "then click the button below.");
    connectButton = XPCreateWidget(x + 540 + 40, y-170, x2 - 10-40, y - 187,
                                        1,
                                        "Connect to Wiimote",
                                        0,
                                        wiimoteDisconnected,
                                        xpWidgetClass_Button);
    XPSetWidgetProperty(connectButton, xpProperty_ButtonType, xpPushButton);
    XPSetWidgetProperty(connectButton, xpProperty_ButtonBehavior, xpButtonBehaviorPushButton);

    
    /* Adding callback for window events */
    XPAddWidgetCallback(setupWindowWidget, SetupWindowHandler);
}



/**
 * Menu callback
 */
void MenuHandler(void *mRef, void * iRef)
{
    if (!strcmp((char *) iRef, "setup")) {
        if (!setupWindowWidget) {
            CreateSetupWindow();
        } else {
            XPShowWidget(setupWindowWidget);
        }
        if (getConnectionState()) {
            XPHideWidget(wiimoteDisconnected);
            XPShowWidget(wiimoteConnected);
        } else {
            XPShowWidget(wiimoteDisconnected);
            XPHideWidget(wiimoteConnected);
        }
    }
}
