#include <stdio.h>
#include <string.h>
#include <cwiid.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define XPLM200 = 1;
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMCamera.h"
#include "XPLMDataAccess.h"
#include "XPLMUtilities.h"
#include "trackmii_plugin.h"
#include "pose.h"
#include "gui.h"

/*
 * Global Variables.  We will store our single window globally.
 */

XPLMWindowID    gWindow = NULL;

// Wiimote handler
cwiid_wiimote_t *gWiimote = NULL;

int gFreeView = 0;

// This is the current head pose
TPose gPose;

int gValid = 0;

int gVersion = TRACKMII_VERSION;

int gStateCheckIn = STATE_CHECK_INTERVAL;

// Translation config
basicTranslationCfg gTranslationCfg[2];

XPLMCommandRef cmdOnOff = NULL;

// Different callbacks
void MyDrawWindowCallback( XPLMWindowID inWindowID,    
                           void * inRefcon);

int MyOnOffHandler(XPLMCommandRef   inCommand,
                   XPLMCommandPhase inPhase,
                   void *           inRefcon);

int MyDrawingCallback (
                                   XPLMDrawingPhase     inPhase,    
                                   int                  inIsBefore,    
                                   void *               inRefcon);

// Forward references
void SetupTranslationCurve(int, basicTranslationCfg cfg);

void LoadSettings();

// Datarefs in use
XPLMDataRef gPilotHeadYawDf = NULL;
XPLMDataRef gPilotHeadPitchDf = NULL;
XPLMDataRef gFrameRatePeriodDf = NULL;


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
                    0,                           /* Start invisible. */
                    MyDrawWindowCallback,        /* Callbacks */
                    NULL,
                    NULL,
                    NULL);                        /* Refcon - not used. */
                    

    /*
     * Wiimote initialization
     */
    point3Df dimensions3PtsCap[3];

    ConnectWiimote();
    
    
    /* Register our hot key for the new view. */
    cmdOnOff = XPLMCreateCommand("trackmii/operation/toggle_tracking", "Toggle head tracking");
    XPLMRegisterCommandHandler(cmdOnOff,
                               MyOnOffHandler,
                               1,                    // Receive input before plugin windows
                               (void *) 0);          // inRefcon.
    
    /* Register drawing callback */
    XPLMRegisterDrawCallback(MyDrawingCallback, xplm_Phase_FirstScene, 1, NULL);

    /* Necessary datarefs */
    gPilotHeadYawDf = XPLMFindDataRef("sim/graphics/view/pilots_head_psi");
    gPilotHeadPitchDf = XPLMFindDataRef("sim/graphics/view/pilots_head_the");
    gFrameRatePeriodDf = XPLMFindDataRef("sim/operation/misc/frame_rate_period");

    /* Gui initialization */
    InitGui();
    
    

    /* Initializing cap model */
    dimensions3PtsCap[0].x = 70;
    dimensions3PtsCap[0].y = 80;
    dimensions3PtsCap[0].z = 100;

    Initialize3PCapModel(dimensions3PtsCap);

    // Initialize translation
    gTranslationCfg[DOF_YAW].deadzone = 15;
    gTranslationCfg[DOF_YAW].response = 5;
    gTranslationCfg[DOF_YAW].amplification = 25;

    gTranslationCfg[DOF_PITCH].deadzone = 15;
    gTranslationCfg[DOF_PITCH].response = 5;
    gTranslationCfg[DOF_PITCH].amplification = 25;

    LoadSettings();

    SetupTranslationCurve(DOF_YAW, gTranslationCfg[DOF_YAW]);
    SetupTranslationCurve(DOF_PITCH, gTranslationCfg[DOF_PITCH]);
    
    /* We must return 1 to indicate successful initialization, otherwise we
     * will not be called back again. */
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
    XPLMUnregisterCommandHandler(cmdOnOff, MyOnOffHandler, 0, 0);
    DestroyGui();
    if (gWiimote) cwiid_close(gWiimote);
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

/**
 * Connecting to the wiimote
 */
void ConnectWiimote() {
    bdaddr_t bdaddr;

    bdaddr = *BDADDR_ANY;
#ifndef WIIMOTE_DISABLED

    fprintf(stderr, "Put Wiimote in discoverable mode now (press 1+2)...\n");

    if (!(gWiimote = cwiid_open(&bdaddr, 0))) {
        fprintf(stderr, "Wiimote not found\n");
    } else {
        cwiid_set_led(gWiimote, CWIID_LED1_ON | CWIID_LED4_ON);
        cwiid_set_rpt_mode(gWiimote, CWIID_RPT_STATUS | CWIID_RPT_IR);
        fprintf(stderr, "Wiimote connected\n");
    }
#endif
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
    
    char buf[1024];
    
    
    /* First we get the location of the window passed in to us. */
    XPLMGetWindowGeometry(inWindowID, &left, &top, &right, &bottom);
    
    /* We now use an XPLMGraphics routine to draw a translucent dark
     * rectangle that is our window's shape. */
    XPLMDrawTranslucentDarkBox(left, top, right, bottom);

    sprintf(buf, "TrackMii:");
    XPLMDrawString(color, left + 5, top - 15,
                   buf, NULL, xplmFont_Basic);

    sprintf(buf, "    pitch: %f yaw: %f", gPose.pitch, gPose.yaw);
    XPLMDrawString(color, left + 5, top - 30,
                   buf, NULL, xplmFont_Basic);

    sprintf(buf, "    leds: %d ", gValid);
    if (!gWiimote) {
        strcat(buf, "(tracking off)");
    }
    else if (gValid == 3) {
        strcat(buf, "(ok)");
    } else {
        strcat(buf, "(weak)");
    }

    XPLMDrawString(color, left + 5, top - 45,
                   buf, NULL, xplmFont_Basic);
    //sprintf(buf, "TrackMii: panX: %f panY: %f panZ: %f\n", gPose.panX, gPose.panY, gPose.panZ);
    //XPLMDrawString(color, left + 5, top - 40,
    //               buf, NULL, xplmFont_Basic);
}                                   

/**
 * Drawing callback, here we do the headtracking processing
 */
int MyDrawingCallback (
                                   XPLMDrawingPhase     inPhase,    
                                   int                  inIsBefore,    
                                   void *               inRefcon)
{
    // TODO: disable callback, when not needed
    if (!gFreeView || !gWiimote) return 1;

    int i, valid;
    float fps;
    struct cwiid_state state;
    point2D pnts[3];

    if (!gStateCheckIn--) {
        gStateCheckIn = STATE_CHECK_INTERVAL;

        if (cwiid_request_status(gWiimote)) {
            fprintf(stderr, "Requesting status failed, disconnecting\n");
            cwiid_close(gWiimote);
            gWiimote = NULL;
            return 1;
        }
    }

    if (cwiid_get_state(gWiimote, &state)) {
        // Treat connection as disconnected on error
        fprintf(stderr, "Error reading wiimote state\n");
        cwiid_close(gWiimote);
        gWiimote = NULL;
        return 1;
    }

    valid = 0;
    for (i=0; i<CWIID_IR_SRC_COUNT; i++) {
        if (state.ir_src[i].valid) {
            if (valid<3) {
                pnts[valid].x = state.ir_src[i].pos[CWIID_X];
                pnts[valid].y = state.ir_src[i].pos[CWIID_Y];
            }
            valid++;
        }
    }

    if (valid == 3 && !AlterPose(pnts, &gPose)) {
        PoseToDegrees(&gPose);
        fps = XPLMGetDataf(gFrameRatePeriodDf);
        // During pause fps is zero
        fps = fps ? 1/fps : 20;
        SmoothPose(&gPose, fps);
        
        XPLMSetDataf(gPilotHeadYawDf, -gPose.yaw);
        XPLMSetDataf(gPilotHeadPitchDf, gPose.pitch);
    }
    gValid = valid;

    return 1;
}

/**
 * Command for turning the headtracking on/off
 */
int MyOnOffHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void* inRefcon)
{
    if ( inPhase == 2 ) {
        gFreeView = 1-gFreeView;
    }
    
    return 0;
}

void SetupTranslationCurve(int dof, basicTranslationCfg cfg) {
    translationCfg trcfg;

    trcfg.P1.x = 0;   trcfg.P1.y = 0;

    trcfg.C1.x = cfg.deadzone;  trcfg.C1.y = 2;

    trcfg.C2.x = min(cfg.deadzone + cfg.response, 99);  trcfg.C2.y = 2+sqrt(cfg.response);

    trcfg.P2.x = 99;  trcfg.P2.y = 220+4*cfg.amplification;

    /*
    fprintf(stderr, "New translation config for dof %d is:\n", dof);
    fprintf(stderr, "P1.x=%d P1.y=%d\n", trcfg.P1.x, trcfg.P1.y);
    fprintf(stderr, "C1.x=%d C1.y=%d\n", trcfg.C1.x, trcfg.C1.y);
    fprintf(stderr, "C2.x=%d C2.y=%d\n", trcfg.C2.x, trcfg.C2.y);
    fprintf(stderr, "P2.x=%d P2.y=%d\n", trcfg.P2.x, trcfg.P2.y);
     //*/

    InitializeCurve(dof, trcfg);
}

/**
 * Returns current translation setup for the given DoF
 */
basicTranslationCfg getTranslationCfg(int dof) {
    return gTranslationCfg[dof];
}

/**
 * Sets new translation setup fot the given DoF
 * Reinitializes translation array inmediately
 */
void setTranslationCfg(int dof, basicTranslationCfg *cfg) {
    gTranslationCfg[dof].deadzone = cfg->deadzone;
    gTranslationCfg[dof].response = cfg->response;
    gTranslationCfg[dof].amplification = cfg->amplification;

    SetupTranslationCurve(dof, gTranslationCfg[dof]);
}

/**
 * Returns 1 if connection is ok, 0 if not connected
 */
int getConnectionState() {
    if (gWiimote && cwiid_request_status(gWiimote)) {
        fprintf(stderr, "Requesting status failed, disconnecting\n");
        cwiid_close(gWiimote);
        gWiimote = NULL;
    }
    return gWiimote ? 1 : 0;
}

/**
 * Saving current settings into preferences file
 */
void SaveSettings() {
    char path[1024];
    int fd = 0;
    int smoothing;

    XPLMGetSystemPath(path);

    strcat(path, "Output/preferences/trackmii.prf");

    fd = open( path, O_RDWR|O_CREAT|O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );

    smoothing = getSmoothing();
    write( fd, &gVersion, sizeof(int));
    write( fd, &smoothing, sizeof(int));
    write( fd, gTranslationCfg, sizeof( gTranslationCfg ));
    
    close( fd );

}

/**
 * Load current settings from binary properties file
 * File has a version on the begin, if it does not match, fail silenty
 */
void LoadSettings() {
    char path[1024];
    int fd = 0;
    int smoothing;
    int ver;

    XPLMGetSystemPath(path);

    strcat(path, "Output/preferences/trackmii.prf");

    fd = open( path, O_RDONLY);

    if (!read(fd, &ver, sizeof(int))) {
        close(fd);
        return;
    }
    if (ver != gVersion) {
        fprintf(stderr, "Found invalid version of preferences file, ignoring.\n");
        close(fd);
        return;
    }
    if (!read(fd, &smoothing, sizeof(int))) {
        close(fd);
        return;
    }
    if (!read(fd, gTranslationCfg, sizeof( gTranslationCfg ))) {
        close(fd);
        return;
    }

    setSmoothing(smoothing);

    fprintf(stderr, "Settings loaded\n");

    close( fd );
}

/**
 * Set debug window state, if param int 1, the window is visible
 * else it's hidden
 */
void ToggleDebugWindowVisible(int state) {
    XPLMSetWindowIsVisible(gWindow, state);
}
