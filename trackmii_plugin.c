/*
 * HellWorld.c
 * 
 * This plugin implements the canonical first program.  In this case, we will 
 * create a window that has the text hello-world in it.  As an added bonus
 * the  text will change to 'This is a plugin' while the mouse is held down
 * in the window.  
 * 
 * This plugin demonstrates creating a window and writing mouse and drawing
 * callbacks for that window.
 * 
 */

#include <stdio.h>
#include <string.h>
#include <cwiid.h>
#include <math.h>
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMCamera.h"
#include "XPLMDataAccess.h"

/*
 * Global Variables.  We will store our single window globally.  We also record
 * whether the mouse is down from our mouse handler.  The drawing handler looks
 * at this information and draws the appropriate display.
 * 
 */

XPLMHotKeyID	gHotKey = NULL;

XPLMWindowID    gWindow = NULL;

// Wiimote handler
cwiid_wiimote_t *gWiimote;

// Current heading
float gYaw;

int gFreeView = 0;

void MyDrawWindowCallback(
                                   XPLMWindowID         inWindowID,    
                                   void *               inRefcon);    

void    MyHotKeyCallback(void *               inRefcon);    

XPLMDataRef gPilotHeadDataRef = NULL;

struct point {
    uint16_t x;
    uint16_t y;
};

/*
 * XPluginStart
 * 
 * Our start routine registers our window and does any other initialization we 
 * must do.
 * 
 */
PLUGIN_API int XPluginStart(
                        char *        outName,
                        char *        outSig,
                        char *        outDesc)
{
    /* First we must fill in the passed in buffers to describe our
     * plugin to the plugin-system. */

    strcpy(outName, "TrackMii");
    strcpy(outSig, "trackmii.tracker");
    strcpy(outDesc, "Head tracking based on wiimote.");

    /* Now we create a window.  We pass in a rectangle in left, top,
     * right, bottom screen coordinates.  We pass in three callbacks. */

    gWindow = XPLMCreateWindow(
                    0, 700, 300, 650,            /* Area of the window. */
                    1,                            /* Start visible. */
                    MyDrawWindowCallback,        /* Callbacks */
                    NULL,
                    NULL,
                    NULL);                        /* Refcon - not used. */
                    
    /* We must return 1 to indicate successful initialization, otherwise we
     * will not be called back again. */
    
    bdaddr_t bdaddr;
    
    bdaddr = *BDADDR_ANY;
    fprintf(stderr, "Connecting to wiimote\n");
    
    if (!(gWiimote = cwiid_open(&bdaddr, 0))) {
        return 0;
    }
    cwiid_set_led(gWiimote, CWIID_LED1_ON | CWIID_LED4_ON);
    cwiid_set_rpt_mode(gWiimote, CWIID_RPT_STATUS | CWIID_RPT_IR);
    
    fprintf(stderr, "Wiimote connected\n");
    
    /* Register our hot key for the new view. */
    gHotKey = XPLMRegisterHotKey(XPLM_VK_F8, xplm_DownFlag, 
                                 "Head tracking on/off",
                                 MyHotKeyCallback,
                                 NULL);
    
    gPilotHeadDataRef = XPLMFindDataRef("sim/graphics/view/pilots_head_psi");
    return 1;
}

/*
 * XPluginStop
 * 
 * Our cleanup routine deallocates our window.
 * 
 */
PLUGIN_API void    XPluginStop(void)
{
    XPLMDestroyWindow(gWindow);
    cwiid_close(gWiimote);
    fprintf(stderr, "Wiimote closed\n");
}

/*
 * XPluginDisable
 * 
 * We do not need to do anything when we are disabled, but we must provide the handler.
 * 
 */
PLUGIN_API void XPluginDisable(void)
{
}

/*
 * XPluginEnable.
 * 
 * We don't do any enable-specific initialization, but we must return 1 to indicate
 * that we may be enabled at this time.
 * 
 */
PLUGIN_API int XPluginEnable(void)
{
    return 1;
}

/*
 * XPluginReceiveMessage
 * 
 * We don't have to do anything in our receive message handler, but we must provide one.
 * 
 */
PLUGIN_API void XPluginReceiveMessage(
                    XPLMPluginID    inFromWho,
                    long            inMessage,
                    void *            inParam)
{
}

float CalculateHeadPosition(struct cwiid_state *state) {
    int valid, i, j;
    
    struct point lft, top, rgt;
    float yaw;

    /** yaw szamitasa **/
    valid = lft.y = top.x = top.y = rgt.x = rgt.y = 0;
    lft.x = 2000;

    valid = 0;
    for (j=0; j<CWIID_IR_SRC_COUNT; j++) {
        if (!state->ir_src[j].valid) continue;
        
        if (state->ir_src[j].pos[CWIID_Y] > top.y) {
            top.x = state->ir_src[j].pos[CWIID_X];
            top.y = state->ir_src[j].pos[CWIID_Y];
        }

        valid++;
    }
    if (valid < 3) {
        return 0;
    }

    for (j=0; j<CWIID_IR_SRC_COUNT; j++) {
        if (state->ir_src[j].valid) {
            
            if (state->ir_src[j].pos[CWIID_X] == top.x &&
                state->ir_src[j].pos[CWIID_Y] == top.y) continue;

            if (state->ir_src[j].pos[CWIID_X] < lft.x) {
                lft.x = state->ir_src[j].pos[CWIID_X];
                lft.y = state->ir_src[j].pos[CWIID_Y];
            }
            if (state->ir_src[j].pos[CWIID_X] > rgt.x) {
                rgt.x = state->ir_src[j].pos[CWIID_X];
                rgt.y = state->ir_src[j].pos[CWIID_Y];
            }
            
            valid++;

        }
    }
    
    float l1 = top.x-lft.x;
    float l2 = rgt.x-top.x;
    
    yaw = asin((l1-l2)/(fabs(l2)+fabs(l1)));
    
    yaw = yaw * 180 / M_PI;
    
    yaw = yaw * (yaw/90) * (yaw/90);
    
    gYaw = yaw;
    
    if (gFreeView) {
        XPLMSetDataf(gPilotHeadDataRef, yaw);
    }
    
    return yaw;
}

/*
 * MyDrawingWindowCallback
 * 
 * This callback does the work of drawing our window once per sim cycle each time
 * it is needed.  It dynamically changes the text depending on the saved mouse
 * status.  Note that we don't have to tell X-Plane to redraw us when our text
 * changes; we are redrawn by the sim continuously.
 * 
 */
void MyDrawWindowCallback(
                                   XPLMWindowID         inWindowID,    
                                   void *               inRefcon)
{
    int        left, top, right, bottom;
    float    color[] = { 1.0, 1.0, 1.0 };     /* RGB White */
    
    struct cwiid_state state;
    char buf[1024];
    int i;
    float yaw;
    
    
    /* First we get the location of the window passed in to us. */
    XPLMGetWindowGeometry(inWindowID, &left, &top, &right, &bottom);
    
    /* We now use an XPLMGraphics routine to draw a translucent dark
     * rectangle that is our window's shape. */
    XPLMDrawTranslucentDarkBox(left, top, right, bottom);

    if (cwiid_get_state(gWiimote, &state)) return;
    
    sprintf(buf, "state: ");
    for (i=0; i<CWIID_IR_SRC_COUNT; i++) {
        if (state.ir_src[i].valid) {
            sprintf(buf+strlen(buf), "(%d, %d) ", state.ir_src[i].pos[CWIID_X],
                                      state.ir_src[i].pos[CWIID_Y]);
        } else {
            sprintf(buf+strlen(buf), "(_, _) ");
        }
    }
    XPLMDrawString(color, left + 5, top - 10,
                   buf, NULL, xplmFont_Basic);
    
    yaw = CalculateHeadPosition( &state );
    
    sprintf(buf, "yaw: %2f", yaw);
    XPLMDrawString(color, left + 5, top - 20,
                   buf, NULL, xplmFont_Basic);
    
    sprintf(buf, "phead: %2f", XPLMGetDataf(gPilotHeadDataRef));
    XPLMDrawString(color, left + 5, top - 30,
                       buf, NULL, xplmFont_Basic);
}                                   


void	MyHotKeyCallback(void *               inRefcon)
{
    /* Now we control the camera until the view changes. */
    gFreeView = 1-gFreeView;
}

