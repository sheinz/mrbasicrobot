#ifndef __CMotorPWM_H__
#define __CMotorPWM_H__

#include <inttypes.h>
#include <avr/interrupt.h>

// forward declarations for interrupts
extern "C" void TIMER2_COMP_vect(void) __attribute__ ((signal));
extern "C" void TIMER2_OVF_vect(void) __attribute__ ((signal));



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
   // friend interrupts
   friend void TIMER2_COMP_vect(void);
   friend void TIMER2_OVF_vect(void);

   static void setDirection(const eMotor motor, const bool out1, const bool out2);

   static void updateMotorsSpeed(void);


   static eMotorMode mMotorMode[MOTOR_COUNT];
   static uint8_t mMotorSpeed[MOTOR_COUNT];
   
   struct PwmChannel
   {
      uint8_t value;
      uint8_t mask;     // the bits which is set should be clear at next compare match
   };

   static PwmChannel mPwm[MOTOR_COUNT];
   static int8_t mCurrentPwm; 

   static PwmChannel mPwmUpd[MOTOR_COUNT];
   static volatile bool mUpdate;    // if true the mPwm will be update

};

#endif // __CMotorPWM_H__

