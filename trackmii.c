/* 
 * File:   trackmii.c
 * Author: mrbig
 *
 * Created on 2009. junius 3., 21:26
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cwiid.h>
#include <math.h>
#include <time.h>

struct point {
    uint16_t x;
    uint16_t y;
};

void print_state(struct cwiid_state *state) {
    int i, valid;

    printf("Report Mode:");
    if (state->rpt_mode & CWIID_RPT_STATUS) printf(" STATUS");
    if (state->rpt_mode & CWIID_RPT_BTN) printf(" BTN");
    if (state->rpt_mode & CWIID_RPT_ACC) printf(" ACC");
    if (state->rpt_mode & CWIID_RPT_IR) printf(" IR");
    printf("\n");
    printf("Battery: %d%%\n", (int)(100.0 * state->battery / CWIID_BATTERY_MAX));
    printf("IR: ");
    for (i=0; i<CWIID_IR_SRC_COUNT; i++) {
        if (state->ir_src[i].valid) {
            printf("(%d, %d) ", state->ir_src[i].pos[CWIID_X],
                                state->ir_src[i].pos[CWIID_Y]);
        } else {
            printf("(_, _) ");
        }
    }
    printf("\n");
};

void cwiid_callback(cwiid_wiimote_t *wiimote, int mesg_count,
                    union cwiid_mesg mesg[], struct timespec *timestamp) {
    int i, j, valid;
    struct point lft, top, rgt;
    double yaw;

    for (i=0; i<mesg_count; i++) {
        if (mesg[i].type != CWIID_MESG_IR) continue;

        valid = lft.y = top.x = top.y = rgt.x = rgt.y = 0;
        lft.x = 2000;

        valid = 0;
        for (j=0; j<CWIID_IR_SRC_COUNT; j++) {
            if (!mesg[i].ir_mesg.src[j].valid) continue;
            
            if (mesg[i].ir_mesg.src[j].pos[CWIID_Y] > top.y) {
                top.x = mesg[i].ir_mesg.src[j].pos[CWIID_X];
                top.y = mesg[i].ir_mesg.src[j].pos[CWIID_Y];
            }

            valid++;
        }
        if (valid < 3) {
            printf("---\n");
            continue;
        }

        for (j=0; j<CWIID_IR_SRC_COUNT; j++) {
            if (mesg[i].ir_mesg.src[j].valid) {
                
                if (mesg[i].ir_mesg.src[j].pos[CWIID_X] == top.x &&
                        mesg[i].ir_mesg.src[j].pos[CWIID_Y] == top.y) continue;

                if (mesg[i].ir_mesg.src[j].pos[CWIID_X] < lft.x) {
                    lft.x = mesg[i].ir_mesg.src[j].pos[CWIID_X];
                    lft.y = mesg[i].ir_mesg.src[j].pos[CWIID_Y];
                }
                if (mesg[i].ir_mesg.src[j].pos[CWIID_X] > rgt.x) {
                    rgt.x = mesg[i].ir_mesg.src[j].pos[CWIID_X];
                    rgt.y = mesg[i].ir_mesg.src[j].pos[CWIID_Y];
                }
                
                valid++;

            }
        }
        printf("%d ", time(NULL));
        printf("left: %d,%d top: %d,%d right: %d,%d",
                lft.x, lft.y,
                top.x, top.y,
                rgt.x, rgt.y);

        double l1 = top.x-lft.x;
        double l2 = rgt.x-top.x;
        /*
        if (l1>l2) {
            yaw = acos(l2/l1);
        } else {
            yaw = -1 * acos(l1/l2);
        }
         */
        yaw = asin((l2-l1)/(fabs(l2)+fabs(l1)));
            
        printf(" yaw: %2f", yaw*180/M_PI);

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

    bdaddr = *BDADDR_ANY;

    printf("connecting!\n");
    
    if (!(wiimote = cwiid_open(&bdaddr, 0))) {
        fprintf(stderr, "Unable to connect to wiimote\n");
        return -1;
    }

    printf("connected\n");

    cwiid_set_led(wiimote, CWIID_LED1_ON | CWIID_LED4_ON);
    cwiid_set_rpt_mode(wiimote, CWIID_RPT_STATUS | CWIID_RPT_IR);

    if (cwiid_set_mesg_callback(wiimote, cwiid_callback)) {
        fprintf(stderr, "Unable to set message callback\n");
    }
    if (cwiid_enable(wiimote, CWIID_FLAG_MESG_IFC)) {
        fprintf(stderr, "Error enabling messages\n");
    }

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

