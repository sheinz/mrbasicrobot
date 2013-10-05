#ifndef __CSERVOCTRL_HPP__
#define __CSERVOCTRL_HPP__

#include <inttypes.h>

class CServoCtrl;  // forward declaration
extern CServoCtrl ServoCtrl; // only one object, and it is defined here

// ----------------------------------------------------------------------------

const static uint8_t  MAX_SERVOS = 19;  // the number of servos
const static int8_t  MAX_SERVO_VALUE = 55;
const static int8_t  MIN_SERVO_VALUE = -55;

class CServoCtrl
{
private:



   void sortServoValueList(void);

public:

   void init(void);

   void setServo(uint8_t servo_index, int8_t servo_value);

   void setAllServo(int8_t servo_value);

   int8_t getServo(uint8_t servo_index);
   
};



#endif // __CSERVOCTRL_HPP__