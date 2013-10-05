/*
   The class uses Timer/Counter1 to orginise 2 channel PWM in order to control 2 motors

*/


#include "CMotorPWM.h"
#include <avr/io.h>
#include <avr/interrupt.h>


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


#define START_TIMER()      TCCR2 |= _BV(CS21) | _BV(CS22)   // Normal mode, prescaler F_CPU/8
#define STOP_TIMER()       TCCR2 = 0            // Stop timer

// clear the interrupt flag in case the interrupt has already occurred
#define ENABLE_OVERFLOW_INT()      TIFR |= _BV(TOV2); TIMSK |= _BV(TOIE2); TIMSK &= ~_BV(OCIE2)        // enable overflow
#define ENABLE_COMPARE_INT()       TIFR |= _BV(OCF2); TIMSK |= _BV(OCIE2); TIMSK &= ~_BV(TOIE2)    

CMotorPWM::eMotorMode CMotorPWM::mMotorMode[CMotorPWM::MOTOR_COUNT];
uint8_t CMotorPWM::mMotorSpeed[CMotorPWM::MOTOR_COUNT];
CMotorPWM::PwmChannel CMotorPWM::mPwm[CMotorPWM::MOTOR_COUNT];
int8_t CMotorPWM::mCurrentPwm;
CMotorPWM::PwmChannel CMotorPWM::mPwmUpd[CMotorPWM::MOTOR_COUNT];
volatile bool CMotorPWM::mUpdate;


// ----------------------------------------------------------------------------

ISR(TIMER2_COMP_vect)
{
   STOP_TIMER();
   TCNT2 = OCR2;     // reset the interrupt overhead

   PWM_PORT &= ~(CMotorPWM::mPwm[CMotorPWM::mCurrentPwm].mask);
  
   CMotorPWM::mCurrentPwm++;

   if (  (CMotorPWM::mCurrentPwm < CMotorPWM::MOTOR_COUNT) && 
         (CMotorPWM::mPwm[CMotorPWM::mCurrentPwm].mask != 0)   )
   {
      OCR2 = CMotorPWM::mPwm[CMotorPWM::mCurrentPwm].value;
   }
   else
   {
      //if (OCR2 == 0xFF) TCNT2 = 0xFF;     // so the next interrupt will be immediately 

      CMotorPWM::mCurrentPwm = 0;
      ENABLE_OVERFLOW_INT();
   }  


   START_TIMER();
}

// ----------------------------------------------------------------------------

ISR(TIMER2_OVF_vect)
{   
   uint8_t mask = 0;
   STOP_TIMER();
   TCNT2 = 0;        // reset interrupt overhead

   // update 
   if (CMotorPWM::mUpdate)
   {
      PORTC |= _BV(PC3);

      for (uint8_t i = 0; i < CMotorPWM::MOTOR_COUNT; i++)
      {
         CMotorPWM::mPwm[i].mask = CMotorPWM::mPwmUpd[i].mask;
         CMotorPWM::mPwm[i].value = CMotorPWM::mPwmUpd[i].value;
      }
      CMotorPWM::mUpdate = false;
   }
   
   if (CMotorPWM::mPwm[0].value == 0)
   {
      mask = CMotorPWM::mPwm[0].mask;
      CMotorPWM::mCurrentPwm = 1;   
   }
   
   OCR2 = CMotorPWM::mPwm[CMotorPWM::mCurrentPwm].value;    // it can be 0xFF

   PWM_PORT |= (PWM_A_PIN | PWM_B_PIN) & (~mask);  // set all pins except for the value=0
   
   ENABLE_COMPARE_INT();
   START_TIMER();
}

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
      mMotorSpeed[i] = 127;
      mMotorMode[i] = IDLE; 
   }
   
   mPwm[0].mask = PWM_A_PIN | PWM_B_PIN;
   mPwm[0].value = 127;
   mPwm[1].mask = 0;    // 0 means this is the terminal element
   mPwm[1].value = 0xFF;
   mPwmUpd[0] = mPwm[0];
   mPwmUpd[1] = mPwm[1];
   mUpdate = false;

   CMotorPWM::mCurrentPwm = 0;

   TCNT2 = 250;         // next interrupt will be the overflow
   OCR2 = 127;          // somewhere in the middle
   ENABLE_OVERFLOW_INT();
   START_TIMER();
}

// ----------------------------------------------------------------------------

void CMotorPWM::setMotor(const CMotorPWM::eMotor motor, const CMotorPWM::eMotorMode mode)
{
   if (motor < MOTOR_COUNT)
   {
      mMotorMode[motor] = mode;

      if (mode == IDLE)
      {
         mMotorSpeed[motor] = 0;
         updateMotorsSpeed();
      }
      else if (mode == FORWARD)
      {
         setDirection(motor, true, false);
         updateMotorsSpeed();
      }
      else if (mode == BACKWARD)
      {
         setDirection(motor, false, true);
         updateMotorsSpeed();
      }
      else if (mode == BRAKE)
      {
         setDirection(motor, false, false);
         mMotorSpeed[motor] = 0xFF;
         updateMotorsSpeed();
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

void CMotorPWM::updateMotorsSpeed(void)
{
   while (mUpdate);     // wait untel the previous update cycle ends

   if (mMotorSpeed[0] < mMotorSpeed[1])
   {
      mPwmUpd[0].value = mMotorSpeed[0];
      mPwmUpd[0].mask = PWM_A_PIN;
      mPwmUpd[1].value = mMotorSpeed[1];
      mPwmUpd[1].mask = PWM_B_PIN;
   }
   else if (mMotorSpeed[0] > mMotorSpeed[1])
   {
      mPwmUpd[0].value = mMotorSpeed[1];
      mPwmUpd[0].mask = PWM_B_PIN;
      mPwmUpd[1].value = mMotorSpeed[0];
      mPwmUpd[1].mask = PWM_A_PIN;
   }
   else     // equal
   {
      mPwmUpd[0].value = mMotorSpeed[0];
      mPwmUpd[0].mask = PWM_B_PIN | PWM_A_PIN;
      mPwmUpd[1].value = 0xFF;
      mPwmUpd[1].mask = 0;
   }

   uint8_t sreg = SREG;
   cli();
   mUpdate = true;      // safe set the flag
   SREG = sreg;
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

      updateMotorsSpeed();

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

      updateMotorsSpeed();

      return mMotorSpeed[motor];
   }
   else
   {
      return 0;
   }
}