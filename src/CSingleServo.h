#ifndef __CSINGLESERVO_H__
#define __CSINGLESERVO_H__
/*
 * Desciption: Implementation of single hobby servo controller.
 * Using one 8-bit hardware timer.
 */

#include <inttypes.h>

class CSingleServo
{
private:
   CSingleServo();

public:
   static void init();

   // position range is -127..127
   static void set(int8_t position);

};

#endif // __CSINGLESERVO_H__
