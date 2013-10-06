#include "CMotorPWM.h"
#include "CMicroTimer.h"
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
//#include "CRadioCtrlSignal.h"
#include "CCarCtrl.h"
#include "CRangeSensor.h"

#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include <avr/pgmspace.h>

extern "C"
{
#include "uart.h"
};

#undef FDEV_SETUP_STREAM 
#define FDEV_SETUP_STREAM(p, g, f) { 0, 0, f, 0, 0, p, g, 0 }

static FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_WRITE);


int main(void)
{
   CMicroTimer::init();
   //CRadioCtrlSignal::init();
   CCarCtrl::init();

   CRangeSensor::init();

   uart_init();

   stdout = stdin = &uart_str;

   sei();  // enable global interrupt

   DDRD |= (1<<4);

   if ((MCUCSR & (1<<BORF)) || (MCUCSR & (1<<EXTRF)) )
   {
      //while (1)
      {
         PORTD |= (1<<4);
         CMicroTimer::sleep_us(500000);
         PORTD &= ~(1<<4);
         CMicroTimer::sleep_us(500000);
      }
   }

   PORTD |= (1<<4);
   CMicroTimer::sleep(5000);
   PORTD &= ~(1<<4);

   while (1)
   {
	   CCarCtrl::setMovement(0, 100);
	   CMicroTimer::sleep(1000);
	   CCarCtrl::setMovement(60, 100);
	   CMicroTimer::sleep(1000);
	   CCarCtrl::setMovement(-60, 100);
	   CMicroTimer::sleep(1000);
	   CCarCtrl::setMovement(0, -100);
	   CMicroTimer::sleep(1000);
	   CCarCtrl::setMovement(-60, -100);
	   CMicroTimer::sleep(2000);
   }

   while (1)
   {
      uint32_t distance;

      CRangeSensor::startMeasurement();

      while (!CRangeSensor::isReady());      // wait till it ready

      distance = CRangeSensor::getMeasurement();

      printf("Distance: %d\n", distance);

      CMicroTimer::sleep(100);
   }

   //while (1)
   //{
   //   int16_t speed = CRadioCtrlSignal::getValue(0);
   //   int16_t stearing = CRadioCtrlSignal::getValue(1);
   //   int16_t neutral = CRadioCtrlSignal::getNeutral(0); 

   //   speed -= neutral;
   //   stearing -= neutral;

   //   speed = speed / 4;
   //   stearing = stearing / 4;

   //   CCarCtrl::setMovement(stearing, speed);

   //   CMicroTimer::sleep(50);
   //   
   //}

   return 0;
}
