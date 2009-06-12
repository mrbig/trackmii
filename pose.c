#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cwiid.h>
#include <math.h>
#include "pose.h"

#define sqr(z)	((z) * (z))

float R01, R02, R12;

float r1m_norm[3], r2m_norm[3], r3m_norm[3];

float yawAlignModel, pitchAlignModel;

float dist1, dist2;

float FZScalar = 1320;

TPose oldpose;

int gSmoothing = 15;

#define GLOBAL_SMOOTHING 100

void Initialize3PCapModel(point3Df dimensions3PtsCap[3]) {
    int i;
    float dot;
    float top[3], R01_v[3], R02_v[3], R12_v[3];

    oldpose.yaw = 0;
    oldpose.roll = 0;
    oldpose.pitch = 0;

    R01_v[0] = -1 * dimensions3PtsCap[0].x;
    R01_v[1] = -1 * dimensions3PtsCap[0].y;
    R01_v[2] = -1 * dimensions3PtsCap[0].z;
    R01 = sqrt(sqr(R01_v[0]) + sqr(R01_v[1]) + sqr(R01_v[2]));

    R02_v[0] = dimensions3PtsCap[0].x;
    R02_v[1] = -1 * dimensions3PtsCap[0].y;
    R02_v[2] = -1 * dimensions3PtsCap[0].z;
    R02 = sqrt(sqr(R02_v[0]) + sqr(R02_v[1]) + sqr(R02_v[2]));

    R12_v[0] = 2 * dimensions3PtsCap[0].x;
    R12_v[1] = 0;
    R12_v[2] = 0;
    R12 = sqrt(sqr(R12_v[0]) + sqr(R12_v[1]) + sqr(R12_v[2]));

    // Model triad
    for ( i=0; i<3; i++ ) {
        r1m_norm[i] = R01_v[i] * (1/R01);
    }

    dot = R02_v[0] * r1m_norm[0] +
          R02_v[1] * r1m_norm[1] +
          R02_v[2] * r1m_norm[2];

    for ( i=0; i<3; i++ ) {
        top[i] = R02_v[i] - dot * r1m_norm[i];
    }

    for ( i=0; i<3; i++ ) {
        r2m_norm[i] = top[i] * (1/sqrt(sqr(top[0]) + sqr(top[1]) + sqr(top[2])));
    }

    //cross product x1_norm X y1_norm
    r3m_norm[0] = r1m_norm[1] * r2m_norm[2] - r1m_norm[2] * r2m_norm[1];
    r3m_norm[1] = r1m_norm[2] * r2m_norm[0] - r1m_norm[0] * r2m_norm[2];
    r3m_norm[2] = r1m_norm[0] * r2m_norm[1] - r1m_norm[1] * r2m_norm[0];

    yawAlignModel = atan( dimensions3PtsCap[0].x / dimensions3PtsCap[0].z );
    pitchAlignModel = atan( dimensions3PtsCap[0].y / dimensions3PtsCap[0].z );
}

/**
 * Sorts the screen points
 * Topmost is first
 * other two sorted by x
 */
void SortCap(point2D img[3]) {
    int i, idtop = 0;
    point2D sorted[3];
    
    sorted[0].y = 0;
    sorted[1].x = 6000;
    sorted[2].x = 0;
    
    for (i=0; i<3; i++) {
        if (img[i].y>sorted[0].y) {
            sorted[0].x = img[i].x;
            sorted[0].y = img[i].y;
            idtop = i;
        }
    }
    for (i=0; i<3; i++) {
        if ( i == idtop ) continue;
        if (img[i].x<sorted[1].x) {
            sorted[1].x = img[i].x;
            sorted[1].y = img[i].y;
        }
        if (img[i].x>sorted[2].x) {
            sorted[2].x = img[i].x;
            sorted[2].y = img[i].y;
        }
    }
    
    for (i=0; i<3; i++) {
        img[i].x = sorted[i].x;
        img[i].y = sorted[i].y;
    }
}

/**
 * Calculates current head position
 */
int AlterPose(point2D img[3], TPose *pose) {
    int i, j;
    float Rot[3][3];
    float top[3], est_obj01[3], est_obj02[3], r1e_norm[3], r2e_norm[3], r3e_norm[3];
    float d01_v[2], d02_v[2], d12_v[2];
    float d01, d02, d12, a, b, c, s, inv_s, h1, h2, dot;

    SortCap(img);

    /*
    for (i=0; i<3; i++) {
        printf("(%d, %d) ", img[i].x, img[i].y);
    }
    printf("\n");
     */

    d01_v[0] = img[1].x - img[0].x;
    d01_v[1] = img[1].y - img[0].y;
    d01 = sqrt(sqr(d01_v[0]) + sqr(d01_v[1]));

    d02_v[0] = img[2].x - img[0].x;
    d02_v[1] = img[2].y - img[0].y;
    d02 = sqrt(sqr(d02_v[0]) + sqr(d02_v[1]));

    d12_v[0] = img[2].x - img[1].x;
    d12_v[1] = img[2].y - img[1].y;
    d12 = sqrt(sqr(d12_v[0]) + sqr(d12_v[1]));

    // Alter's magic equations
    a = (R01 + R02 + R12) * (-R01 + R02 + R12) * (R01 - R02 + R12) * (R01 + R02 - R12);
    b =  sqr(d01) * (-1 * sqr(R01) + sqr(R02) + sqr(R12))
       + sqr(d02) * (sqr(R01) - sqr(R02) + sqr(R12))
       + sqr(d12) * (sqr(R01) + sqr(R02) - sqr(R12));
    c = (d01 + d02 + d12) * (-d01 + d02 + d12) * (d01 - d02 + d12) * (d01 + d02 - d12);
    s = sqrt((b + sqrt(sqr(b) - a * c)) * (1/a));
    inv_s = 1/s;

    h1 = sqr(s*R01) - sqr(d01);
    h2 = sqr(s*R02) - sqr(d02);
    // sqrt(negative number) sometimes occurs for extreme poses, more often with 3point cap.
    if ( h1<0 || h2<0 ) return 1;
    
    h1 = - sqrt(h1);
    h2 = - sqrt(h2);

    // Calculate rotation matrix given estimated depth information
    est_obj01[0] = inv_s * d01_v[0];
    est_obj01[1] = inv_s * d01_v[1];
    est_obj01[2] = inv_s * h1;  // distance drops out


    est_obj02[0] = inv_s * d02_v[0];
    est_obj02[1] = inv_s * d02_v[1];
    est_obj02[2] = inv_s * h2;

    //

    dist1 = est_obj01[2];
    dist2 = est_obj02[2];

    //**********image triad*********
    for ( i=0; i<3; i++ ) {
        r1e_norm[i] = est_obj01[i] * (1/sqrt(sqr(est_obj01[0]) + sqr(est_obj01[1]) + sqr(est_obj01[2])));
    }

    dot = est_obj02[0] * r1e_norm[0] +
          est_obj02[1] * r1e_norm[1] +
          est_obj02[2] * r1e_norm[2] ;

    for ( i=0; i<3; i++ ) {
        top[i] = (est_obj02[i] - dot * r1e_norm[i]);
    }

    for ( i=0; i<3; i++ ) {
        r2e_norm[i] = top[i] * (1/sqrt(sqr(top[0]) + sqr(top[1]) + sqr(top[2])));
    }

    //cross product x1_norm x y1_norm
    r3e_norm[0] = r1e_norm[1] * r2e_norm[2] - r1e_norm[2] * r2e_norm[1];
    r3e_norm[1] = r1e_norm[2] * r2e_norm[0] - r1e_norm[0] * r2e_norm[2];
    r3e_norm[2] = r1e_norm[0] * r2e_norm[1] - r1e_norm[1] * r2e_norm[0];

    // R = M2 * (M1)^T
    for ( i=0; i<3; i++ ) {
        for ( j=0; j<3; j++ ) {
            Rot[j][i] = r1e_norm[j] * r1m_norm[i] + r2e_norm[j] * r2m_norm[i] + r3e_norm[j] * r3m_norm[i];
        }
    }

    pose->yaw = atan2(Rot[2][0], Rot[0][0]);
    pose->pitch = atan2(-Rot[1][2], Rot[1][1]);
    pose->roll = atan2(-Rot[1][0], Rot[0][0]); // use arctan instead of arcsin to avoid NAN when > 1

    pose->panX = inv_s * img[0].x;
    pose->panY = inv_s * img[0].y;
    pose->panZ = inv_s * FZScalar;

    return 0;
}

/**
 * Converts angles in current pose objet to degress
 */
void PoseToDegrees(TPose *pose) {
    pose->yaw = pose->yaw * 180 / M_PI;
    pose->roll = pose->roll * 180 / M_PI;
    pose->pitch = pose->pitch * 180 / M_PI;
}

/**
 * Smooth position changes
 */
void SmoothPose(TPose *pose) {
    float SmoothingSize;
    TPose delta;

    SmoothingSize =  20 * gSmoothing * GLOBAL_SMOOTHING * 0.001;

    delta.yaw = abs(pose->yaw - oldpose.yaw);
    delta.roll = abs(pose->roll - oldpose.roll);
    delta.pitch = abs(pose->pitch - oldpose.pitch);

    pose->yaw = ((pose->yaw * delta.yaw) / (delta.yaw + SmoothingSize))
           + ((oldpose.yaw * SmoothingSize) / (delta.yaw + SmoothingSize));
    pose->roll = ((pose->roll * delta.roll) / (delta.roll + SmoothingSize))
           + ((oldpose.roll * SmoothingSize) / (delta.roll + SmoothingSize));
    pose->pitch = ((pose->pitch * delta.pitch) / (delta.pitch + SmoothingSize))
           + ((oldpose.pitch * SmoothingSize) / (delta.pitch + SmoothingSize));


    oldpose.yaw = pose->yaw;
    oldpose.roll = pose->roll;
    oldpose.pitch = pose->pitch;
}

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

int getSmoothing()
{
    return gSmoothing;
}
void setSmoothing( value )
{
    gSmoothing = value;
}
