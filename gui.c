#include <stdio.h>
#include <string.h>
#include "XPLMMenus.h"
#include "XPWidgets.h"
#include "XPStandardWidgets.h"

int SetupWindowHandler(XPWidgetMessage inMessage,
                       XPWidgetID      inWidget,
                       long            inParam1,
                       long            inParam2);

void MenuHandler(void *, void *);

// Widgets we use
XPLMMenuID menuId;
XPWidgetID setupWindowWidget = NULL;


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
    //fprintf(stderr, "Got event\n");
    if (inMessage == xpMessage_CloseButtonPushed)
    {
        fprintf(stderr, "Got close button event\n");
        XPHideWidget(setupWindowWidget);
    }
    return 0;
}

/**
 * Here we create the setup window, and all it's sub widgets
 */
void CreateSetupWindow()
{
    setupWindowWidget = XPCreateWidget(100, 650, 450, 400,
                                       1, // Visible
                                       "TrackMii Setup", // desc
                                       1, // root
                                       NULL,
                                       xpWidgetClass_MainWindow);
    XPSetWidgetProperty(setupWindowWidget, xpProperty_MainWindowHasCloseBoxes, 1);

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
