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
 *   along with TrackMii.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cwiid.h>
#include <math.h>
#include <time.h>
#include "pose.h"

struct point {
    uint16_t x;
    uint16_t y;
};

void print_state(struct cwiid_state *state) {

    printf("Report Mode:");
    if (state->rpt_mode & CWIID_RPT_STATUS) printf(" STATUS");
    if (state->rpt_mode & CWIID_RPT_BTN) printf(" BTN");
    if (state->rpt_mode & CWIID_RPT_ACC) printf(" ACC");
    if (state->rpt_mode & CWIID_RPT_IR) printf(" IR");
    printf("\n");
    printf("Battery: %d%%\n", (int)(100.0 * state->battery / CWIID_BATTERY_MAX));
};

void cwiid_callback(cwiid_wiimote_t *wiimote, int mesg_count,
                    union cwiid_mesg mesg[], struct timespec *timestamp) {
    int i, j, valid;
    struct point lft, top, rgt;
    //double yaw;
    point2D pnts[3];
    TPose pose;

    for (i=0; i<mesg_count; i++) {
        if (mesg[i].type != CWIID_MESG_IR) continue;

        valid = lft.y = top.x = top.y = rgt.x = rgt.y = 0;
        lft.x = 2000;

        valid = 0;
        
        for (j=0; j<CWIID_IR_SRC_COUNT; j++) {
            if (!mesg[i].ir_mesg.src[j].valid) continue;
            
            pnts[valid].x = mesg[i].ir_mesg.src[j].pos[CWIID_X];
            pnts[valid].y = mesg[i].ir_mesg.src[j].pos[CWIID_Y];

            valid++;
        }
        if (valid < 3) {
            printf("---\n");
            continue;
        }

        /*
        yaw = CalculateHeadYaw(pnts);
        printf(" yaw: %2f\n", yaw);
         */

        if (AlterPose(pnts, &pose)) {
            printf("TrackMii: ---\n");
        } else {
            PoseToDegrees(&pose);
            SmoothPose(&pose);
            printf("TrackMii: pitch: %f roll: %f yaw: %f\n", pose.pitch, pose.roll, pose.yaw);
        }
        printf("\n");


    }
};
/*
 * 
 */
int main(int argc, char** argv) {
    cwiid_wiimote_t *wiimote;
    struct cwiid_state state;
    bdaddr_t bdaddr;
    int exit = 0;
    point3Df dimensions3PtsCap[3];

    bdaddr = *BDADDR_ANY;

    printf("connecting!\n");
    
    if (!(wiimote = cwiid_open(&bdaddr, 0))) {
        fprintf(stderr, "Unable to connect to wiimote\n");
        return -1;
    }

    printf("connected\n");

    cwiid_set_led(wiimote, CWIID_LED1_ON | CWIID_LED4_ON);
    cwiid_set_rpt_mode(wiimote, CWIID_RPT_STATUS | CWIID_RPT_IR);

    //*

    if (cwiid_set_mesg_callback(wiimote, cwiid_callback)) {
        fprintf(stderr, "Unable to set message callback\n");
    }
    if (cwiid_enable(wiimote, CWIID_FLAG_MESG_IFC)) {
        fprintf(stderr, "Error enabling messages\n");
    }
    // */


    dimensions3PtsCap[0].x = 70;
    dimensions3PtsCap[0].y = 80;
    dimensions3PtsCap[0].z = 100;

    Initialize3PCapModel(dimensions3PtsCap);

    while (!exit) {
        switch(getchar()) {
            case 'q':
                exit = 1;
                break;
            default:
                if (cwiid_get_state(wiimote, &state)) {
                    printf("Error getting state\n");
                } else {
                    print_state(&state);
                }
        }
    }

    /*
    if (getchar()) {
        printf("Exiting\n");
    }
     **/

    if (cwiid_close(wiimote)) {
        printf("Error on wiimote disconnect\n");
        return -1;
    }
    
    return (EXIT_SUCCESS);
}

