#include "CServoCtrl.hpp"

#include <avr/interrupt.h>


CServoCtrl ServoCtrl;

// ----------------------------------------------------------------------------

#define SERVO_PORT_0    PORTB   
#define SERVO_PORT_1    PORTC
#define SERVO_PORT_2    PORTD

#define SERVO_DDR_0     DDRB
#define SERVO_DDR_1     DDRC
#define SERVO_DDR_2     DDRD


#define PRESCALER_FAST     0b00000100        //prescaler = 256
#define PRESCALER_SLOW     0b00000101        // prescaler = 1024

// ----------------------------------------------------------------------------

#define SERVO_DELAY_OFFSET       90

struct TServoTiming
{
   uint8_t port_0_mask;
   uint8_t port_1_mask;
   uint8_t port_2_mask;

   uint8_t delay;
};

struct TServoValue
{
   uint8_t servo_index;
   int8_t  servo_value;
};

// ----------------------------------------------------------------------------

static TServoTiming servoTimingList[MAX_SERVOS];
static volatile uint8_t currentServo;        // 0..MAX_SERVOS-1
static volatile bool needsUpdate;      // true if mServoValueList has been changed and servoTimingList needs update


TServoValue servoValueList[MAX_SERVOS];
TServoValue* pServoValueSortedList[MAX_SERVOS];

// ----------------------------------------------------------------------------

void updateServoList(void);

// ----------------------------------------------------------------------------

SIGNAL(TIMER0_COMP_vect)
{
   TCCR0 = 0;  // stop the timer

   if (currentServo)
   {
      OCR0 = servoTimingList[currentServo].delay;	  

      SERVO_PORT_0 &= ~(servoTimingList[currentServo-1].port_0_mask);
      SERVO_PORT_1 &= ~(servoTimingList[currentServo-1].port_1_mask);
      SERVO_PORT_2 &= ~(servoTimingList[currentServo-1].port_2_mask);

      currentServo++;

      if ( (OCR0 == 0xFF) || (currentServo >= MAX_SERVOS) )
      {				
         OCR0 = 0xFF;      // set it anyway
         TCNT0 = 0;
                 
         TCCR0 = PRESCALER_SLOW;
         currentServo = 0;

         if (needsUpdate)
         {
            updateServoList();		
            needsUpdate = false;	
         }
      }
      else
      {
         TCCR0 = PRESCALER_FAST;
      }
   }
   else
   {  // first cycle
      OCR0 = servoTimingList[currentServo].delay;
      TCNT0 = 0;
      
      SERVO_PORT_0 = 0xFF;
      SERVO_PORT_1 = 0xFF;
      SERVO_PORT_2 |= (1<<4)|(1<<5)|(1<<6);    // set all servo pins to 1 (* on "port_2" only 4, 5, 6 pins are used)

      currentServo++;      

      TCCR0 = PRESCALER_FAST;
   }
}

// ----------------------------------------------------------------------------

// Bubble sort
void sortServoList(void)
{
   TServoValue* pTemp;

   for (uint8_t i = (MAX_SERVOS - 1); i > 0; i--)
   {
      for (uint8_t j = 1; j <= i; j++)
      {
         if (pServoValueSortedList[j-1]->servo_value > pServoValueSortedList[j]->servo_value)
         {
            pTemp = pServoValueSortedList[j-1];
            pServoValueSortedList[j-1] = pServoValueSortedList[j];
            pServoValueSortedList[j] = pTemp;
         }
      }
   }
}

// ----------------------------------------------------------------------------

void addPortMask(uint8_t servo_index, uint8_t pin_index)
{
   if (pin_index < 8)
   {
      servoTimingList[servo_index].port_0_mask |= 1<<pin_index;
   }
   else if (pin_index < 16)
   {
      servoTimingList[servo_index].port_1_mask |= 1<<(pin_index-8);
   }
   else if (pin_index < 24)
   {
      servoTimingList[servo_index].port_2_mask |= 1<<(4 + pin_index-16);      // In "port_2" the real pins start from 4
   }
}

// ----------------------------------------------------------------------------

void clearPortMasks(void)
{
   for (uint8_t i = 0; i < MAX_SERVOS; i++)
   {
      servoTimingList[i].port_0_mask = 0;
      servoTimingList[i].port_1_mask = 0;
      servoTimingList[i].port_2_mask = 0;
   }
}

// ----------------------------------------------------------------------------

void updateServoList(void)
{
   clearPortMasks(); // clear all masks

   servoTimingList[0].delay = pServoValueSortedList[0]->servo_value + SERVO_DELAY_OFFSET;  // first delay is always from the firs servo in the sorted list
   addPortMask(0, pServoValueSortedList[0]->servo_index);      // add mask for the first servo

   uint8_t lastUniqueServoIndex = 0;      // mark it as the first unique
   for (uint8_t i = 1; i < MAX_SERVOS; i++)     // start with the next servo
   {
      if (pServoValueSortedList[lastUniqueServoIndex]->servo_value == pServoValueSortedList[i]->servo_value)
      {     // if the next servo has the same value than just add its port mask 
         addPortMask(lastUniqueServoIndex, pServoValueSortedList[i]->servo_index);
      }
      else
      {  // if the next servo has unique value than go to the next position and mark it as an unique servo
         lastUniqueServoIndex++;
         servoTimingList[lastUniqueServoIndex].delay = pServoValueSortedList[i]->servo_value + SERVO_DELAY_OFFSET;
         addPortMask(lastUniqueServoIndex, pServoValueSortedList[i]->servo_index);
      }
   }

   if ( (lastUniqueServoIndex+1) < MAX_SERVOS ) // there is less records than number of servos
   {     // mark the end of list
      servoTimingList[lastUniqueServoIndex+1].delay = 0xFF;
   }
}

// ----------------------------------------------------------------------------

void CServoCtrl::init(void)
{
   TIMSK |= 1<<OCIE0;	
   //TCCR0 = PRESCALER_FAST;
   TCCR0 |= 0x04;

   OCR0 = 0xFF;

   SERVO_DDR_0 = 0xFF;
   SERVO_DDR_1 = 0xFF;
   SERVO_DDR_2 |= (1<<4)|(1<<5)|(1<<6); // only pins 4, 5, 6 are used

   for (uint8_t i = 0; i < MAX_SERVOS; i++)
   {
      servoValueList[i].servo_index = i;
      servoValueList[i].servo_value = 0;
      pServoValueSortedList[i] = &(servoValueList[i]);
   }
   // don't need to sort the list because all elements are equal
   updateServoList();

   currentServo = 0;
   needsUpdate = false;
}

// ----------------------------------------------------------------------------

void CServoCtrl::setServo(uint8_t servo_index, int8_t servo_value)
{
   if (needsUpdate)
   {
      return;
   }

   if (servo_value > MAX_SERVO_VALUE)
   {
      servo_value = MAX_SERVO_VALUE;
   }
   else if (servo_value < MIN_SERVO_VALUE)
   {
      servo_value = MIN_SERVO_VALUE;
   }

   if (servo_index < MAX_SERVOS)
   {
      servoValueList[servo_index].servo_value = servo_value;
   }
   
   sortServoList();
   needsUpdate = true;
}

// ----------------------------------------------------------------------------

void CServoCtrl::setAllServo(int8_t servo_value)
{
   if (servo_value > MAX_SERVO_VALUE)
   {
      servo_value = MAX_SERVO_VALUE;
   }
   else if (servo_value < MIN_SERVO_VALUE)
   {
      servo_value = MIN_SERVO_VALUE;
   }

   if (needsUpdate)
   {
      return;
   }

   for (uint8_t i = 0; i < MAX_SERVOS; i++)
   {
      servoValueList[i].servo_value = servo_value;
   }

   // don't need to sort
   needsUpdate = true;
}

// ----------------------------------------------------------------------------

int8_t CServoCtrl::getServo(uint8_t servo_index)
{
   if (servo_index < MAX_SERVOS)
   {
      return servoValueList[servo_index].servo_value;
   }

   return 0;      // index is out of range
}
