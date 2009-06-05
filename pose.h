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


float CalculateHeadYaw(point2D pnts[3]);



#ifdef	__cplusplus
}
#endif

#endif	/* _POSE_H */

