/* 
 * File:   pose.h
 * Author: mrbig
 *
 * Created on 2009. j�nius 5., 18:29
 */

#ifndef _POSE_H
#define	_POSE_H

#ifdef	__cplusplus
extern "C" {
#endif


typedef struct point2D {
    uint16_t x;
    uint16_t y;
} point2D;

typedef struct point3Df {
    float x;
    float y;
    float z;
} point3Df;

typedef struct {
    float yaw;
    float pitch;
    float roll;

    float panX;
    float panY;
    float panZ;
} TPose;


void Initialize3PCapModel(point3Df dimensions3PtsCap[3]);

int AlterPose(point2D pnts[3], TPose *pose);

void PoseToDegrees(TPose *pose);

void SmoothPose(TPose *pose);

float CalculateHeadYaw(point2D pnts[3]);



#ifdef	__cplusplus
}
#endif

#endif	/* _POSE_H */

