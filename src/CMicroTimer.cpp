#include "CMicroTimer.h"
#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint32_t CMicroTimer::mMSeconds;    // milli seconds
volatile uint16_t CMicroTimer::mUSeconds;    // micro seconds

// ----------------------------------------------------------------------------

ISR(TIMER0_OVF_vect)
{
   CMicroTimer::mUSeconds += 128;       // Timer overflows every 128 microseconds

   if (CMicroTimer::mUSeconds >= 1000)
   {
      CMicroTimer::mMSeconds++;
      CMicroTimer::mUSeconds -= 1000;
   }
}

// ----------------------------------------------------------------------------

void CMicroTimer::init(void)
{
   mMSeconds = 0;
   mUSeconds = 0;

   TCCR0 |= _BV(CS01);  // Set prescaler to F_OSC/8

   TIMSK |= _BV(TOIE0);  // enable overflow interrupt
}

// ----------------------------------------------------------------------------

uint32_t CMicroTimer::getMilliSeconds(void)
{
   uint32_t milliseconds;
   uint8_t sreg = SREG;

   cli();

   milliseconds = mMSeconds;

   SREG = sreg;

   return milliseconds;
}

// ----------------------------------------------------------------------------

uint32_t CMicroTimer::getMicroSeconds(void)
{
   uint32_t micros = 0;
   uint8_t sreg = SREG;

   cli();
   
   micros = mUSeconds + (TCNT0>>1);  // TCNT0 increments every 0.5 microsecond
   
   if (TIFR & _BV(TOV0))
   {     // TCNT0 has just overflowed
      micros = mUSeconds + 128;
   }

   micros += mMSeconds * 1000;
 
   SREG = sreg;

   return micros;
}

// ----------------------------------------------------------------------------

void CMicroTimer::sleep(const uint32_t milliseconds)
{
   uint32_t startTime = getMilliSeconds();

   while ( (startTime + milliseconds) > getMilliSeconds() );
}

// ----------------------------------------------------------------------------

void CMicroTimer::sleep_us(const uint32_t microseconds)
{
   uint32_t startTime = getMicroSeconds();

   while ( (startTime + microseconds) > getMicroSeconds() );
}