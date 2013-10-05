#include "CCarCtrl.h"


int8_t CCarCtrl::mSpeed;
int8_t CCarCtrl::mAngle;

// ----------------------------------------------------------------------------

void CCarCtrl::init(void)
{
   CMotorPWM::init();

   mAngle = 0;
   mSpeed = 0;
}

// ----------------------------------------------------------------------------

void CCarCtrl::setMotor(CMotorPWM::eMotor motor, int16_t speed)
{
   if (speed > 255)
   {
      speed = 255;
   }
   else if (speed < -255)
   {
      speed = -255;
   }

   if (speed > 0)
   {
      CMotorPWM::setMotor(motor, CMotorPWM::BACKWARD, static_cast<uint8_t>(speed));
   }
   else if (speed < 0)
   {
      CMotorPWM::setMotor(motor, CMotorPWM::FORWARD, static_cast<uint8_t>(speed * -1) );
   }
   else
   {
      CMotorPWM::setMotor(motor, CMotorPWM::IDLE);
   }
}

// ----------------------------------------------------------------------------

void CCarCtrl::setMovement(const int8_t angle, const int8_t speed)
{
   int16_t leftMotor, rightMotor;
   float angleFactor = static_cast<float>(mAngle) / 128;  // angle from -1..1

   mSpeed = speed;
   mAngle = angle;

   leftMotor = rightMotor = static_cast<int16_t>(mSpeed) * 2;

   if (mSpeed > 0)
   {
      leftMotor += static_cast<float>(leftMotor) * angleFactor;
      rightMotor -= static_cast<float>(rightMotor) * angleFactor;
   }
   else
   {
      leftMotor += static_cast<float>(leftMotor) * angleFactor;
      rightMotor -= static_cast<float>(rightMotor) * angleFactor;
   }

   setMotor(CMotorPWM::MOTOR_A, leftMotor);
   setMotor(CMotorPWM::MOTOR_B, rightMotor);
}


// ----------------------------------------------------------------------------

void CCarCtrl::setRotation(const int8_t angle)
{
   // todo
}

// ----------------------------------------------------------------------------

void CCarCtrl::setSpeed(const int8_t speed)
{
   setMovement(mAngle, speed);
}

// ----------------------------------------------------------------------------

void CCarCtrl::brake(void)
{
   CMotorPWM::setMotor(CMotorPWM::MOTOR_A, CMotorPWM::BRAKE);
   CMotorPWM::setMotor(CMotorPWM::MOTOR_B, CMotorPWM::BRAKE);
}

// ----------------------------------------------------------------------------

void CCarCtrl::setTurnAngle(const int8_t angle)
{
   setMovement(angle, mSpeed);
}