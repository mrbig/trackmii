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

#ifndef _POSE_H
#define	_POSE_H

#ifdef	__cplusplus
extern "C" {
#endif

#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))
#define DegToRad(a) (a/180*M_PI)
#define RadToDeg(a) (a*180/M_PI)

typedef struct point2D {
    int16_t x;
    int16_t y;
} point2D;

typedef struct point2Df {
    float x;
    float y;
} point2Df;

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

typedef struct translationCfg {
    point2Df P1;
    point2Df C1;
    point2Df C2;
    point2Df P2;
} translationCfg;

#define DOF_YAW 0
#define DOF_PITCH 1
#define DOF_ROLL 2
#define DOF_PANX 3
#define DOF_PANY 4
#define DOF_PANZ 5


void Initialize3PCapModel(point3Df dimensions3PtsCap[3]);

void InitializeCurve();

int AlterPose(point2D pnts[3], TPose *pose);

void PoseToDegrees(TPose *pose);

void SmoothPose(TPose *pose, float fps);

float CalculateHeadYaw(point2D pnts[3]);

void SetCenter();

int getSmoothing();

void setSmoothing(int value);


#ifdef	__cplusplus
}
#endif

#endif	/* _POSE_H */

