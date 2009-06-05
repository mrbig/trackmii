#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cwiid.h>
#include <math.h>
#include "pose.h"


/**
 * Quick & dirty head position estimation
 * Used as proof of concept.
 */
float CalculateHeadYaw(point2D pnts[3]) {
    int i, found;

    struct point2D lft, top, rgt;
    float yaw;

    /** yaw szamitasa **/
    lft.y = top.x = top.y = rgt.x = rgt.y = 0;
    lft.x = 2000;

    for (i=0; i<3; i++) {
        if (pnts[i].y > top.y) {
            top.x = pnts[i].x;
            top.y = pnts[i].y;
            found = i;
        }

    }

    for (i=0; i<3; i++) {
        if (i == found) continue;

        if (pnts[i].x < lft.x) {
            lft.x = pnts[i].x;
            lft.y = pnts[i].y;
        }
        if (pnts[i].x > rgt.x) {
            rgt.x = pnts[i].x;
            rgt.y = pnts[i].y;
        }

    }

    printf("left: (%d, %d) top: (%d, %d) right: (%d, %d)\n",
            lft.x, lft.y, top.x, top.y, rgt.x, rgt.y);

    float l1 = top.x-lft.x;
    float l2 = rgt.x-top.x;

    yaw = asin((l1-l2)/(fabs(l2)+fabs(l1)));

    yaw = yaw * 180 / M_PI;

    yaw = yaw * (yaw/90) * (yaw/90);

    return yaw;
}
