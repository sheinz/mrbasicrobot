#ifndef __CMICROTIMER_H__
#define __CMICROTIMER_H__

#include <inttypes.h>
#include <avr/interrupt.h>

// forward declarations for interrupts
extern "C" void TIMER0_OVF_vect(void) __attribute__ ((signal));

class CMicroTimer
{
private:
   CMicroTimer();

   // friend interrupt
   friend void TIMER0_OVF_vect(void);

   static volatile uint32_t mMSeconds;    // milli seconds
   static volatile uint16_t mUSeconds;    // micro seconds

public:

   static void init(void);

   static uint32_t getMilliSeconds(void);

   static uint32_t getMicroSeconds(void);

   static void sleep(const uint32_t milliseconds);

   static void sleep_us(const uint32_t microseconds);
};


#endif  // __CMICROTIMER_H__