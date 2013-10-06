#ifndef __CMotorPWM_H__
#define __CMotorPWM_H__

#include <inttypes.h>

class CMotorPWM
{
public:
   static void init(void);

   enum eMotorMode
   {
      IDLE = 0,
      BRAKE,
      FORWARD,
      BACKWARD
   };

   enum eMotor
   {
      MOTOR_A = 0,
      MOTOR_B,
      MOTOR_COUNT,
   };

   static void setMotor(const eMotor motor, const eMotorMode mode);

   /**
    * @param speed Speed of the motor from 0 to 256
    */
   static void setMotor(const eMotor motor, const eMotorMode mode, const uint8_t speed);

   static uint8_t getMotorSpeed(const eMotor motor);

   /**
    * The methods return the resulting motor speed
    */
   static uint8_t increaseMotorSpeed(const eMotor motor, uint8_t step);
   static uint8_t decreaseMotorSpeed(const eMotor motor, uint8_t step);

private:

   CMotorPWM();

   static void setDirection(const eMotor motor, const bool out1, const bool out2);

   static void setMotorPwm(const eMotor motor, uint8_t speed);


   static eMotorMode mMotorMode[MOTOR_COUNT];
   static uint8_t mMotorSpeed[MOTOR_COUNT];
   
};

#endif // __CMotorPWM_H__

