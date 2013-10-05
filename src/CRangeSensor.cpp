#include "CRangeSensor.h"
#include <avr/interrupt.h>

#include "CMicroTimer.h"


// ------ configuration ---------
// PD2 (INT0) - trigger pin
// PD3 (INT1) - echo pin

// ----------------------------------------------------------------------------

#define BAD_MEASUREMENT       0xFFFFFFFF

static volatile uint32_t start_time;
static volatile uint32_t last_measurement;


ISR(INT1_vect)
{
   uint32_t timeStamp = CMicroTimer::getMicroSeconds();

   if (start_time == 0)
   {  // rising edge
      PORTD |= (1<<4);

      start_time = timeStamp;

      MCUCR &= ~_BV(ISC10);      // falling edge generates interrupt
   }
   else
   {
      PORTD &= ~(1<<4);
      if (timeStamp > start_time)
      {
         last_measurement = timeStamp - start_time;
      }
      else
      {
         last_measurement = BAD_MEASUREMENT;
      }

      GICR &= ~_BV(INT1);  // disable interrupt from INT1
   }
}

// -----------------------------------------------

void CRangeSensor::init(void)
{
   DDRD |= _BV(PD2);    // trigger pin to output
   PORTD &=  ~_BV(PD2);

   DDRD &= ~_BV(PD3);   // echo pin to input
   PORTD &= ~_BV(PD3);  // disable pull-up resistor

   GICR &= ~_BV(INT1);  // disable interrupt from INT1 at start
}

// -----------------------------------------------

void CRangeSensor::startMeasurement(void)
{
   last_measurement = 0;      // clear the measurement
   start_time = 0;

   MCUCR |= _BV(ISC11) | _BV(ISC10);      // rising edge of INT1 generates an interrupt

   // generate 10 us trigger pulse
   PORTD |= _BV(PD2);      // set trigger pin to HIGH
   CMicroTimer::sleep_us(10);       // 10us pause
   PORTD &= ~_BV(PD2);      // set trigger pin to LOW

   GICR |= _BV(INT1);      // enable interrupt from INT1
}

// ----------------------------------------------------------------------------

uint32_t CRangeSensor::getMeasurement(void)
{
   return last_measurement;
}

// ----------------------------------------------------------------------------

bool CRangeSensor::isReady(void)
{
   return (last_measurement != 0);
}