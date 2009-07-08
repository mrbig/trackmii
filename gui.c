#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cwiid.h>
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

XPWidgetID yawDeadzoneValueCaption = NULL;
XPWidgetID yawDeadzoneScrollbar = NULL;
XPWidgetID yawResponseValueCaption = NULL;
XPWidgetID yawResponseScrollbar = NULL;
XPWidgetID yawAmplificationValueCaption = NULL;
XPWidgetID yawAmplificationScrollbar = NULL;

XPWidgetID pitchDeadzoneValueCaption = NULL;
XPWidgetID pitchDeadzoneScrollbar = NULL;
XPWidgetID pitchResponseValueCaption = NULL;
XPWidgetID pitchResponseScrollbar = NULL;
XPWidgetID pitchAmplificationValueCaption = NULL;
XPWidgetID pitchAmplificationScrollbar = NULL;

XPWidgetID wiimoteConnected = NULL;
XPWidgetID wiimoteDisconnected = NULL;
XPWidgetID connectButton = NULL;

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
    basicTranslationCfg trcfg;
    if (inMessage == xpMessage_CloseButtonPushed)
    {
        XPHideWidget(setupWindowWidget);
    }
    else if (inMessage == xpMsg_ScrollBarSliderPositionChanged)
    {
        

        if (inParam1 == (long)smoothingScrollbar) {
            tmp = XPGetWidgetProperty(smoothingScrollbar, xpProperty_ScrollBarSliderPosition, NULL);

            setSmoothing(tmp);

            sprintf(buff, "%ld", tmp);
            XPSetWidgetDescriptor(smoothingValueCaption, buff);
        }

        /* yaw scrollbars */
        else if (inParam1 == (long)yawDeadzoneScrollbar) {
            tmp = XPGetWidgetProperty(yawDeadzoneScrollbar, xpProperty_ScrollBarSliderPosition, NULL);

            trcfg = getTranslationCfg(DOF_YAW);
            trcfg.deadzone = tmp;
            setTranslationCfg(DOF_YAW, &trcfg);

            sprintf(buff, "%ld", tmp);
            XPSetWidgetDescriptor(yawDeadzoneValueCaption, buff);
        }
        else if (inParam1 == (long)yawResponseScrollbar) {
            tmp = XPGetWidgetProperty(yawResponseScrollbar, xpProperty_ScrollBarSliderPosition, NULL);

            trcfg = getTranslationCfg(DOF_YAW);
            trcfg.response = tmp;
            setTranslationCfg(DOF_YAW, &trcfg);

            sprintf(buff, "%ld", tmp);
            XPSetWidgetDescriptor(yawResponseValueCaption, buff);
        }
        else if (inParam1 == (long)yawAmplificationScrollbar) {
            tmp = XPGetWidgetProperty(yawAmplificationScrollbar, xpProperty_ScrollBarSliderPosition, NULL);

            trcfg = getTranslationCfg(DOF_YAW);
            trcfg.amplification = tmp;
            setTranslationCfg(DOF_YAW, &trcfg);

            sprintf(buff, "%ld", tmp);
            XPSetWidgetDescriptor(yawAmplificationValueCaption, buff);
        }

        /* pitch scrollbars */
        else if (inParam1 == (long)pitchDeadzoneScrollbar) {
            tmp = XPGetWidgetProperty(pitchDeadzoneScrollbar, xpProperty_ScrollBarSliderPosition, NULL);

            trcfg = getTranslationCfg(DOF_PITCH);
            trcfg.deadzone = tmp;
            setTranslationCfg(DOF_PITCH, &trcfg);

            sprintf(buff, "%ld", tmp);
            XPSetWidgetDescriptor(pitchDeadzoneValueCaption, buff);
        }
        else if (inParam1 == (long)pitchResponseScrollbar) {
            tmp = XPGetWidgetProperty(pitchResponseScrollbar, xpProperty_ScrollBarSliderPosition, NULL);

            trcfg = getTranslationCfg(DOF_PITCH);
            trcfg.response = tmp;
            setTranslationCfg(DOF_PITCH, &trcfg);

            sprintf(buff, "%ld", tmp);
            XPSetWidgetDescriptor(pitchResponseValueCaption, buff);
        }
        else if (inParam1 == (long)pitchAmplificationScrollbar) {
            tmp = XPGetWidgetProperty(pitchAmplificationScrollbar, xpProperty_ScrollBarSliderPosition, NULL);

            trcfg = getTranslationCfg(DOF_PITCH);
            trcfg.amplification = tmp;
            setTranslationCfg(DOF_PITCH, &trcfg);

            sprintf(buff, "%ld", tmp);
            XPSetWidgetDescriptor(pitchAmplificationValueCaption, buff);
        }
    }
    else if (inMessage == xpMsg_PushButtonPressed)
    {
        if (inParam1 == (long)connectButton) {
            ConnectWiimote();
            if (getConnectionState()) {
                XPHideWidget(wiimoteDisconnected);
                XPShowWidget(wiimoteConnected);
            } else {
                XPShowWidget(wiimoteDisconnected);
                XPHideWidget(wiimoteConnected);
            }
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
void CreateScrollbar(XPWidgetID *setupWindowWidget,
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

    *scrollbar = XPCreateWidget(x + 80, y, x2 - 30, y - 20,
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
 * Here we create the setup window, and all it's sub widgets
 */
void CreateSetupWindow()
{
    int x, y, x2, y2, w, h;
    basicTranslationCfg trcfg;

    x = 100;
    y = 650;
    w = 600;
    h = 270;
    x2 = x + w;
    y2 = y - h;

    setupWindowWidget = XPCreateWidget(x, y, x2, y2,
                                       1, // Visible
                                       "TrackMii Setup", // desc
                                       1, // root
                                       NULL,
                                       xpWidgetClass_MainWindow);
    XPSetWidgetProperty(setupWindowWidget, xpProperty_MainWindowHasCloseBoxes, 1);

    /* smoothing scrollbar */
    CreateScrollbar(setupWindowWidget, &smoothingScrollbar, &smoothingValueCaption,
            "Smoothing:",
            x+10, y-40, x+350,
            getSmoothing(),
            1, 100);


    /* yaw deadzone scrollbar */
    XPCreateWidget(x+10, y - 70, x + 80, y-90, 1, "Yaw translation setup", 0, setupWindowWidget, xpWidgetClass_Caption);

    trcfg = getTranslationCfg(DOF_YAW);

    CreateScrollbar(setupWindowWidget, &yawDeadzoneScrollbar, &yawDeadzoneValueCaption,
            "Deadzone:",
            x+10, y-90, x+280,
            trcfg.deadzone,
            0, 30);

    CreateScrollbar(setupWindowWidget, &yawResponseScrollbar, &yawResponseValueCaption,
            "Response:",
            x+10, y-120, x+280,
            trcfg.response,
            0, 80);

    CreateScrollbar(setupWindowWidget, &yawAmplificationScrollbar, &yawAmplificationValueCaption,
            "Amplification:",
            x+10, y-150, x+280,
            trcfg.amplification,
            0, 100);

    /* pitch deadzone scrollbar */
    XPCreateWidget(x+300, y - 70, x + 380, y-90, 1, "Pitch translation setup", 0, setupWindowWidget, xpWidgetClass_Caption);

    trcfg = getTranslationCfg(DOF_PITCH);

    CreateScrollbar(setupWindowWidget, &pitchDeadzoneScrollbar, &pitchDeadzoneValueCaption,
            "Deadzone:",
            x+310, y-90, x2-10,
            trcfg.deadzone,
            0, 30);

    CreateScrollbar(setupWindowWidget, &pitchResponseScrollbar, &pitchResponseValueCaption,
            "Response:",
            x+310, y-120, x2-10,
            trcfg.response,
            0, 80);

    CreateScrollbar(setupWindowWidget, &pitchAmplificationScrollbar, &pitchAmplificationValueCaption,
            "Amplification:",
            x+310, y-150, x2-10,
            trcfg.amplification,
            0, 100);

    /* Wiimote connected subwindows */
    wiimoteConnected = XPCreateWidget(x+40, y-185, x2-40, y2+25, 0,
                                      "Wiimote is connected",
                                      0, setupWindowWidget, xpWidgetClass_SubWindow);

    XPSetWidgetProperty(wiimoteConnected, xpProperty_SubWindowType, xpSubWindowStyle_SubWindow);
    XPAddWidgetCallback(wiimoteConnected, XPUFixedLayout);
    XPCreateWidget(x+230, y - 210, x2 - 230, y-230, 1,
                   "Wiimote is connected",
                   0, wiimoteConnected, xpWidgetClass_Caption);


    /* Wiimote disconnected subwindow, with connect button */
    wiimoteDisconnected = XPCreateWidget(x+40, y-170, x2-40, y2+10, 0,
                                      "Wiimote is currently not connected",
                                      0, setupWindowWidget, xpWidgetClass_SubWindow);

    XPSetWidgetProperty(wiimoteDisconnected, xpProperty_SubWindowType, xpSubWindowStyle_SubWindow);
    XPAddWidgetCallback(wiimoteDisconnected, XPUFixedLayout);

    XPCreateWidget(x+215, y - 180, x2 - 230, y-193, 1,
                   "Wiimote is currently not connected",
                   0, wiimoteDisconnected, xpWidgetClass_Caption);
    XPCreateWidget(x+90, y - 200, x2 - 10, y-213, 1,
                   "To connect the wiimote you should put it first in discoverable mode (press 1+2),",
                   0, wiimoteDisconnected, xpWidgetClass_Caption);
    XPCreateWidget(x+228, y - 213, x2 - 235, y-226, 1,
                   "then click the button below.",
                   0, wiimoteDisconnected, xpWidgetClass_Caption);
    connectButton = XPCreateWidget(x + 220, y-235, x2 - 220, y - 252,
                                        1,
                                        "Connect to Wiimote",
                                        0,
                                        wiimoteDisconnected,
                                        xpWidgetClass_Button);
    XPSetWidgetProperty(connectButton, xpProperty_ButtonType, xpPushButton);
    XPSetWidgetProperty(connectButton, xpProperty_ButtonBehavior, xpButtonBehaviorPushButton);

    
    /* Adding callback for window events */
    XPAddWidgetCallback(setupWindowWidget, SetupWindowHandler);
    //XPAddWidgetCallback(setupWindowWidget, XPUFixedLayout);
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
