#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cwiid.h>
#include "XPLMMenus.h"
#include "XPWidgets.h"
#include "XPStandardWidgets.h"
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
 * Setup window handler
 */
int SetupWindowHandler(XPWidgetMessage inMessage,
                       XPWidgetID      inWidget,
                       long            inParam1,
                       long            inParam2)
{
    char buff[255];
    long tmp;
    if (inMessage == xpMessage_CloseButtonPushed)
    {
        fprintf(stderr, "Got close button event\n");
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
    }
    return 0;
}

/**
 * Here we create the setup window, and all it's sub widgets
 */
void CreateSetupWindow()
{
    int x, y, x2, y2, w, h;
    char buff[255];
    x = 100;
    y = 650;
    w = 350;
    h = 250;
    x2 = x + w;
    y2 = y - h;

    setupWindowWidget = XPCreateWidget(x, y, x2, y2,
                                       1, // Visible
                                       "TrackMii Setup", // desc
                                       1, // root
                                       NULL,
                                       xpWidgetClass_MainWindow);
    XPSetWidgetProperty(setupWindowWidget, xpProperty_MainWindowHasCloseBoxes, 1);

    /* scrollbar */
    XPCreateWidget(x+10, y - 40, x + 80, y-60, 1, "Smoothing:", 0, setupWindowWidget, xpWidgetClass_Caption);
    XPCreateWidget(x2-40, y - 40, x2 - 10, y-60, 1, "max", 0, setupWindowWidget, xpWidgetClass_Caption);

    sprintf(buff, "%d", getSmoothing());
    smoothingValueCaption =
            XPCreateWidget(x+170, y-30, x+180, y-40, 1, buff, 0, setupWindowWidget, xpWidgetClass_Caption);
    
    smoothingScrollbar = XPCreateWidget(x + 80, y - 40, x2 - 40, y - 60,
                                        1,
                                        "smoothing",
                                        0,
                                        setupWindowWidget,
                                        xpWidgetClass_ScrollBar);

    XPSetWidgetProperty(smoothingScrollbar, xpProperty_ScrollBarType, xpScrollBarTypeSlider);
    XPSetWidgetProperty(smoothingScrollbar, xpProperty_ScrollBarMin, 1);
    XPSetWidgetProperty(smoothingScrollbar, xpProperty_ScrollBarMax, 100);
    XPSetWidgetProperty(smoothingScrollbar, xpProperty_ScrollBarSliderPosition, getSmoothing());

    XPAddWidgetCallback(setupWindowWidget, SetupWindowHandler);
}


/**
 * Menu callback
 */
void MenuHandler(void *mRef, void * iRef)
{
    if (!strcmp((char *) iRef, "setup")) {
        fprintf(stderr, "menu clicked\n");
        if (!setupWindowWidget) {
            fprintf(stderr, "Creating menu\n");
            CreateSetupWindow();
        } else {
            fprintf(stderr, "Showing widget\n");
            XPShowWidget(setupWindowWidget);
        }
    }
}
