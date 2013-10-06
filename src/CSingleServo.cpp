#include "CSingleServo.h"

#include <avr/io.h>
#include <avr/interrupt.h>


// configuration
#define SERVO_DDR    DDRC
#define SERVO_PORT   PORTC
#define SERVO_PIN    PC5



// Set frequency to F_CPU/64 and compare to 250 which gives 1ms delay
#define START_1MS_DELAY()  delay_type = DELAY_1MS; OCR2 = 250; TCNT2 = 0; TCCR2 = _BV(CS22)
// Start timer for actual servo position 1ms + 0..1024us
#define START_POS_DELAY()  delay_type = DELAY_POS; TCNT2 = 0; TCCR2 = _BV(CS22)
// Start timer for the delay between pulses. Roughly 16ms.
#define START_LONG_DELAY() delay_type = DELAY_LONG; TCNT2 = 0; OCR2 = 0xFF; \
                           TCCR2 = _BV(CS22) | _BV(CS21) | _BV(CS20)

#define STOP_TIMER()       TCCR2 = 0;


// Error correction (magic number)
#define ERROR_TRIM      3

#define SERVO_PIN_HIGH()   SERVO_PORT |= _BV(SERVO_PIN)
#define SERVO_PIN_LOW()    SERVO_PORT  &= ~_BV(SERVO_PIN)

// range 0..255, neutral=127
static volatile uint8_t servo_position;

enum eDelay
{
   DELAY_1MS,
   DELAY_POS,
   DELAY_LONG
};
static volatile eDelay delay_type;
// ----------------------------------------------------------------------------

ISR(TIMER2_COMP_vect)
{
   STOP_TIMER();
   if (delay_type == DELAY_1MS)
   {
      OCR2 = servo_position;
      START_POS_DELAY();
   }
   else if (delay_type == DELAY_POS)
   {
      SERVO_PIN_LOW();
      START_LONG_DELAY();
   }
   else  // DELAY_LONG
   {
      SERVO_PIN_HIGH();
      START_1MS_DELAY();
   }
}

// ----------------------------------------------------------------------------

void CSingleServo::init()
{
   servo_position = 127;

   SERVO_DDR |= _BV(SERVO_PIN);

   // starting signal generation
   SERVO_PIN_HIGH();
   START_1MS_DELAY();

   // enable compare match interrupt
   TIMSK |= _BV(OCIE2);
}

// ----------------------------------------------------------------------------

// position range is -127..127
void CSingleServo::set(int8_t position)
{
   // can't handle to small values
   if (position < (-127 + ERROR_TRIM))
   {
      position = -127 + ERROR_TRIM;
   }
   servo_position = (uint8_t)((int16_t)position + 128  - ERROR_TRIM);
}

void CSingleServo::set_raw(uint8_t position)
{
   servo_position = position;
}
