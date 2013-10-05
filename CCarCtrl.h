#ifndef __CCARCTRL_H__
#define __CCARCTRL_H__

#include <inttypes.h>

#include "CMotorPWM.h"


class CCarCtrl
{
private:
   CCarCtrl();

   static int8_t mSpeed;
   static int8_t mAngle;

   static void setMotor(CMotorPWM::eMotor motor, int16_t speed);

public:

   static void init(void);


   // speed -128..127
   // 0 - means idle (not brake)
   static void setSpeed(const int8_t speed);
   

   static void brake(void);

   // angle -128..127
   // 0 means no turn
   static void setTurnAngle(const int8_t angle);

   // angle -128..127
   // speed -128..127
   static void setMovement(const int8_t angle, const int8_t speed);


   // Rotation on its place
   // angle -128..127
   static void setRotation(const int8_t angle);

};


#endif