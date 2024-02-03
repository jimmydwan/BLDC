#ifndef __SENSOR_H
#define __SENSOR_H

#include "userMain.h"
#define SENSOR_DIRECTION -1 // 根据硬件方案，角度值随逆时针旋转减小
/******************************************************************************/
#define AS5600_CPR 4096    // 12bit
#define AS5047P_CPR 16384  // 14bit
#define TLE5012B_CPR 32768 // 15bit
#define MA730_CPR 65536    // 14bit,左对齐,低两位补0,所以是65536
#define MT6701_CPR 65536   // 14bit,左对齐,低两位补0,所以是65536
/**
 *  Direction structure
 */
typedef enum
{
    CW = 1,     // clockwise
    CCW = -1,   // counter clockwise
    UNKNOWN = 0 // not yet known or invalid state
} Direction;

extern long cpr;
extern float angle_prev;
extern Direction sensor_direction;
extern int pole_pairs;
extern float zero_electric_angle;
extern float sensor_offset;

/******************************************************************************/
float shaftAngle(void);
float shaftVelocity(void);
float electricalAngle(void);
/******************************************************************************/

/******************************************************************************/
void MagneticSensor_Init(float zero_electric_offset, Direction _sensor_direction);
float getAngle(void);
float getVelocity(void);
int alignSensor(void);
/******************************************************************************/
#endif
