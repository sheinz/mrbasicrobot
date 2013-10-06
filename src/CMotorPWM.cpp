/*
   The class uses Timer/Counter1 to orginise 2 channel PWM in order to control 2 motors

*/


#include "CMotorPWM.h"
#include <avr/io.h>


// hardware configuration
#define OUT_1_DDR      DDRD
#define OUT_1_PORT     PORTD
#define OUT_1_PIN      _BV(PD7)

#define OUT_2_DDR      DDRB
#define OUT_2_PORT     PORTB
#define OUT_2_PIN      _BV(PB0)

#define OUT_3_DDR      DDRD
#define OUT_3_PORT     PORTD
#define OUT_3_PIN      _BV(PD6)

#define OUT_4_DDR      DDRD
#define OUT_4_PORT     PORTD
#define OUT_4_PIN      _BV(PD5)

#define PWM_DDR        DDRB
#define PWM_PORT       PORTB
#define PWM_A_PIN      _BV(PB1)
#define PWM_B_PIN      _BV(PB2)

//    Motor PWM configuration
//    Motor A  - PB1
//    Motor B  - PB2

CMotorPWM::eMotorMode CMotorPWM::mMotorMode[CMotorPWM::MOTOR_COUNT];
uint8_t CMotorPWM::mMotorSpeed[CMotorPWM::MOTOR_COUNT];

// ----------------------------------------------------------------------------

void CMotorPWM::init(void)
{
   // Set all control port to output
   OUT_1_DDR |= OUT_1_PIN;
   OUT_2_DDR |= OUT_2_PIN;
   OUT_3_DDR |= OUT_3_PIN;
   OUT_4_DDR |= OUT_4_PIN;

   PWM_DDR |= PWM_A_PIN | PWM_B_PIN; // the PWM pins must be set as output


   for (uint8_t i = 0; i < MOTOR_COUNT; i++)
   {
      mMotorSpeed[i] = 0;
      mMotorMode[i] = IDLE; 
   }

   OCR1AL = 0;
   OCR1BL = 0;

   // Clear OC1A/OC1B on Compare Match
   // Fast PWM, 8-bit
   TCCR1A = _BV(COM1A1) | _BV(COM1B1) | _BV(WGM10);
   // Frequency = F_CPU/8
   TCCR1B = _BV(WGM12) | _BV(CS11);// | _BV(CS10);
}

// ----------------------------------------------------------------------------

void CMotorPWM::setMotor(const CMotorPWM::eMotor motor, const CMotorPWM::eMotorMode mode)
{
   if (motor < MOTOR_COUNT)
   {
      mMotorMode[motor] = mode;

      if (mode == IDLE)
      {
         setMotorPwm(motor, 0);
      }
      else if (mode == FORWARD)
      {
         setDirection(motor, true, false);
         setMotorPwm(motor, mMotorSpeed[motor]);
      }
      else if (mode == BACKWARD)
      {
         setDirection(motor, false, true);
         setMotorPwm(motor, mMotorSpeed[motor]);
      }
      else if (mode == BRAKE)
      {
         setDirection(motor, false, false);
         setMotorPwm(motor, 0xFF);     // full power stop
      }
   }   
}

// ----------------------------------------------------------------------------

void CMotorPWM::setMotor(const CMotorPWM::eMotor motor, const CMotorPWM::eMotorMode mode, const uint8_t speed)
{
   if (motor < MOTOR_COUNT)
   {
      mMotorSpeed[motor] = speed;
      setMotor(motor, mode);
   }
}

// ----------------------------------------------------------------------------

void CMotorPWM::setMotorPwm(const CMotorPWM::eMotor motor, uint8_t speed)
{
   if (motor == MOTOR_A)
   {
      OCR1AL = speed;
   }
   else
   {
      OCR1BL = speed;
   }
}

// ----------------------------------------------------------------------------

void CMotorPWM::setDirection(const eMotor motor, const bool out1, const bool out2)
{
   if (motor == MOTOR_A)
   {
      if (out1 == true)
      {
         OUT_1_PORT |= OUT_1_PIN;      // set pin
      }
      else
      {
         OUT_1_PORT &= ~OUT_1_PIN;     // clear pin
      }

      if (out2 == true)
      {
         OUT_2_PORT |= OUT_2_PIN;      // set pin
      }
      else
      {
         OUT_2_PORT &= ~OUT_2_PIN;     // clear pin
      }
   }
   else if (motor == MOTOR_B)
   {
      if (out1 == true)
      {
         OUT_3_PORT |= OUT_3_PIN;      // set pin
      }
      else
      {
         OUT_3_PORT &= ~OUT_3_PIN;     // clear pin
      }

      if (out2 == true)
      {
         OUT_4_PORT |= OUT_4_PIN;      // set pin
      }
      else
      {
         OUT_4_PORT &= ~OUT_4_PIN;     // clear pin
      }
   }
}

// ----------------------------------------------------------------------------

uint8_t CMotorPWM::getMotorSpeed(const eMotor motor)
{
   if (motor < MOTOR_COUNT)
   {
      return mMotorSpeed[motor];
   }
   else
   {
      return 0;
   }
}

// ----------------------------------------------------------------------------

uint8_t CMotorPWM::increaseMotorSpeed(const eMotor motor, uint8_t step)
{
   if (motor < MOTOR_COUNT)
   {
      if ( (0xFF - mMotorSpeed[motor]) > step )
      {
         mMotorSpeed[motor] = 0xFF;
      }
      else
      {
         mMotorSpeed[motor] += step;
      }

      setMotorPwm(motor, mMotorSpeed[motor]);

      return mMotorSpeed[motor];
   }
   else
   {
      return 0;
   }
}

// ----------------------------------------------------------------------------

uint8_t CMotorPWM::decreaseMotorSpeed(const eMotor motor, uint8_t step)
{
   if (motor < MOTOR_COUNT)
   {
      if (mMotorSpeed[motor] < step)
      {
         mMotorSpeed[motor] = 0;
      }
      else
      {
         mMotorSpeed[motor] -= step;
      }

      setMotorPwm(motor, mMotorSpeed[motor]);

      return mMotorSpeed[motor];
   }
   else
   {
      return 0;
   }
}
