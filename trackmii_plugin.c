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
#include <string.h>
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


#include "linux-track/tir4_driver.h"
#include "linux-track/wiimote_driver.h"

/*
 * Global Variables.  We will store our single window globally.
 */

XPLMWindowID    gWindow = NULL;

// Connection status
bool connected = false;

// Connection handler
struct camera_control_block ccb;


int gFreeView = 0;

// This is the current head pose
TPose gPose;

int gValid = 0;

int gVersion = TRACKMII_VERSION;


// Translation config
basicTranslationCfg gTranslationCfg[6];

XPLMCommandRef cmdOnOff = NULL;
XPLMCommandRef cmdCenter = NULL;

// Different callbacks
void MyDrawWindowCallback( XPLMWindowID inWindowID,    
                           void * inRefcon);

int MyOnOffHandler(XPLMCommandRef   inCommand,
                   XPLMCommandPhase inPhase,
                   void *           inRefcon);

int MyCenteringHandler(XPLMCommandRef   inCommand,
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
XPLMDataRef gPilotHeadX = NULL;
XPLMDataRef gPilotHeadY = NULL;
XPLMDataRef gPilotHeadZ = NULL;
XPLMDataRef gFrameRatePeriodDf = NULL;

XPLMDataRef gViewType = NULL;

point3Df gDefaultPilotHead;

#define VIEW_FORWARDS 1000

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
                    0, 700, 350, 625,            /* Area of the window. */
                    0,                           /* Start invisible. */
                    MyDrawWindowCallback,        /* Callbacks */
                    NULL,
                    NULL,
                    NULL);                        /* Refcon - not used. */
                    

    /*
     * Wiimote initialization
     */
    point3Df dimensions3PtsCap[3];

    ccb.mode = operational_3dot;
    
    SetCamDevice(wiimote);

    ConnectCam();
    
    
    /* Register our hot key for the new view. */
    cmdOnOff = XPLMCreateCommand("trackmii/operation/toggle_tracking", "Toggle head tracking");
    XPLMRegisterCommandHandler(cmdOnOff,
                               MyOnOffHandler,
                               1,                    // Receive input before plugin windows
                               (void *) 0);          // inRefcon.

    /* And hotkey for centering */
    cmdCenter = XPLMCreateCommand("trackmii/operation/centering", "Setting current position as center");
    XPLMRegisterCommandHandler(cmdCenter,
                               MyCenteringHandler,
                               1,                    // Receive input before plugin windows
                               (void *) 0);          // inRefcon.
    
    /* Register drawing callback */
    XPLMRegisterDrawCallback(MyDrawingCallback, xplm_Phase_FirstScene, 1, NULL);

    /* Necessary datarefs */
    gPilotHeadYawDf = XPLMFindDataRef("sim/graphics/view/pilots_head_psi");
    gPilotHeadPitchDf = XPLMFindDataRef("sim/graphics/view/pilots_head_the");
    gPilotHeadX = XPLMFindDataRef("sim/graphics/view/pilots_head_x");
    gPilotHeadY = XPLMFindDataRef("sim/graphics/view/pilots_head_y");
    gPilotHeadZ = XPLMFindDataRef("sim/graphics/view/pilots_head_z");
    gFrameRatePeriodDf = XPLMFindDataRef("sim/operation/misc/frame_rate_period");

    gViewType = XPLMFindDataRef("sim/graphics/view/view_type");

    /* Gui initialization */
    InitGui();
    
    

    /* Initializing cap model */
    dimensions3PtsCap[0].x = 70;
    dimensions3PtsCap[0].y = 80;
    dimensions3PtsCap[0].z = 100;

    Initialize3PCapModel(dimensions3PtsCap);

    // Initialize translation
    gTranslationCfg[DOF_YAW].deadzone = 10;
    gTranslationCfg[DOF_YAW].response = 9;
    gTranslationCfg[DOF_YAW].amplification = 30;

    gTranslationCfg[DOF_PITCH].deadzone = 3;
    gTranslationCfg[DOF_PITCH].response = 14;
    gTranslationCfg[DOF_PITCH].amplification = 23;

    gTranslationCfg[DOF_PANX].deadzone = 15;
    gTranslationCfg[DOF_PANX].response = 2;
    gTranslationCfg[DOF_PANX].amplification = 4;

    gTranslationCfg[DOF_PANY].deadzone = 15;
    gTranslationCfg[DOF_PANY].response = 2;
    gTranslationCfg[DOF_PANY].amplification = 4;

    gTranslationCfg[DOF_PANZ].deadzone = 15;
    gTranslationCfg[DOF_PANZ].response = 2;
    gTranslationCfg[DOF_PANZ].amplification = 12;

    LoadSettings();

    SetupTranslationCurve(DOF_YAW, gTranslationCfg[DOF_YAW]);
    SetupTranslationCurve(DOF_PITCH, gTranslationCfg[DOF_PITCH]);

    SetupTranslationCurve(DOF_PANX, gTranslationCfg[DOF_PANX]);
    SetupTranslationCurve(DOF_PANY, gTranslationCfg[DOF_PANY]);
    SetupTranslationCurve(DOF_PANZ, gTranslationCfg[DOF_PANZ]);
    
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
    XPLMUnregisterCommandHandler(cmdCenter, MyCenteringHandler, 0, 0);
    DestroyGui();
    DisconnectCam();
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
 * Set current cam category
 */
void SetCamDevice(enum cal_device_category_type category) {
    ccb.device.category = category;
}

/**
 * Get current cam category
 */
enum cal_device_category_type GetCamDevice() {
    return ccb.device.category;
}


/**
 * Connecting to the camera device
 */
void ConnectCam() {

    connected = !cal_init(&ccb);
}

/**
 * Disconnecting from camera device
 */
void DisconnectCam() {
    if (connected) {
        if (!cal_shutdown(&ccb)) {
            connected = false;
        } else {
            fprintf(stderr, "Error closing camera connection\n");
        }
    }
    fprintf(stderr, "Camera disconnected\n");
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
    if (!connected) {
        strcat(buf, "(tracking off)");
    }
    else if (gValid == 3) {
        strcat(buf, "(ok)");
    } else {
        strcat(buf, "(weak)");
    }

    XPLMDrawString(color, left + 5, top - 45,
                   buf, NULL, xplmFont_Basic);
    sprintf(buf, "TrackMii: panX: %f panY: %f panZ: %f\n", gPose.panX, gPose.panY, gPose.panZ);
    XPLMDrawString(color, left + 5, top - 60,
                   buf, NULL, xplmFont_Basic);
}                                   

/**
 * Drawing callback, here we do the headtracking processing
 */
int MyDrawingCallback (
                                   XPLMDrawingPhase     inPhase,    
                                   int                  inIsBefore,    
                                   void *               inRefcon)
{
    int i;
    float fps;
    point2D pnts[3];
    struct frame_type frame;

    if (!gFreeView || !connected) return 1;

    cal_get_frame(&ccb, &frame);

    for (i=0; i<frame.bloblist.num_blobs; i++) {
        pnts[i].x = frame.bloblist.blobs[i].x;
        pnts[i].y = frame.bloblist.blobs[i].y;
    }

    if (frame.bloblist.num_blobs == 3 && !AlterPose(pnts, &gPose)) {
        PoseToDegrees(&gPose);
        fps = XPLMGetDataf(gFrameRatePeriodDf);
        // During pause fps is zero
        fps = fps ? 1/fps : 20;
        SmoothPose(&gPose, fps);
        
        XPLMSetDataf(gPilotHeadYawDf, -gPose.yaw);
        XPLMSetDataf(gPilotHeadPitchDf, gPose.pitch);

        XPLMSetDataf(gPilotHeadX, gDefaultPilotHead.x-gPose.panX);
        XPLMSetDataf(gPilotHeadY, gDefaultPilotHead.y+gPose.panY);
        XPLMSetDataf(gPilotHeadZ, gDefaultPilotHead.z+gPose.panZ);
    }
    gValid = frame.bloblist.num_blobs;
    frame_free(&ccb, &frame);

    return 1;
}

/**
 * Command for turning the headtracking on/off
 */
int MyOnOffHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void* inRefcon)
{
    if ( inPhase == 2 ) {
        if (!gFreeView) SetCenter();
        if (XPLMGetDatai(gViewType) == VIEW_FORWARDS) {
            // We save current head position for reference
            gDefaultPilotHead.x = XPLMGetDataf(gPilotHeadX);
            gDefaultPilotHead.y = XPLMGetDataf(gPilotHeadY);
            gDefaultPilotHead.z = XPLMGetDataf(gPilotHeadZ);
        }
        gFreeView = 1-gFreeView;
    }
    
    return 0;
}

/**
 * Command for turning the headtracking on/off
 */
int MyCenteringHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void* inRefcon)
{
    if ( inPhase == 2 ) {
        SetCenter();
    }

    return 0;
}

void SetupTranslationCurve(int dof, basicTranslationCfg cfg) {
    translationCfg trcfg;

    trcfg.P1.x = 0;   trcfg.P1.y = 0;
    //fprintf(stderr, "=============================\ndz: %d, resp: %d, ampl: %d\n", cfg.deadzone, cfg.response, cfg.amplification);

    if (dof < DOF_PANX) {
        trcfg.C1.x = cfg.deadzone;  trcfg.C1.y = 2;

        trcfg.C2.x = min(cfg.deadzone + cfg.response, 99);  trcfg.C2.y = 2+sqrt(cfg.response);

        trcfg.P2.x = 99;  trcfg.P2.y = 220+4*cfg.amplification;
    } else {
        // translation need much smaller results
        trcfg.C1.x = cfg.deadzone;  trcfg.C1.y = 0.1;

        trcfg.C2.x = min(cfg.deadzone + cfg.response, 99);  trcfg.C2.y = sqrt((float)cfg.response/10);

        trcfg.P2.x = 99;  trcfg.P2.y = 0.5+(float)cfg.amplification/100;
    }

    /*
    fprintf(stderr, "New translation config for dof %d is:\n", dof);
    fprintf(stderr, "P1.x=%f P1.y=%f\n", trcfg.P1.x, trcfg.P1.y);
    fprintf(stderr, "C1.x=%f C1.y=%f\n", trcfg.C1.x, trcfg.C1.y);
    fprintf(stderr, "C2.x=%f C2.y=%f\n", trcfg.C2.x, trcfg.C2.y);
    fprintf(stderr, "P2.x=%f P2.y=%f\n", trcfg.P2.x, trcfg.P2.y);
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
    return connected;
}

/**
 * Saving current settings into preferences file
 */
void SaveSettings() {
    char path[1024];
    int fd = 0;
    int smoothing;
    //fprintf(stderr, "Saving settings\n");

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
