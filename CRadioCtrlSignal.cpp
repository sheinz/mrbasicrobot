#include "CRadioCtrlSignal.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include "CMicroTimer.h"

CRadioCtrlSignal::ChannelInfo CRadioCtrlSignal::mChannels[CRadioCtrlSignal::MAX_CHANNELS];
bool CRadioCtrlSignal::mCallibMode;
bool CRadioCtrlSignal::mSettingNeutralMode;

// ----------------------------------------------------------------------------

inline void fallingEdgeProcessing(const uint8_t channel, const uint32_t timeStamp)
{
   CRadioCtrlSignal::mChannels[channel].value = 
      timeStamp - CRadioCtrlSignal::mChannels[channel].pulseStartTime;

   if (CRadioCtrlSignal::mChannels[channel].value > CRadioCtrlSignal::MAX_POSSIBLE_VAL ||
      CRadioCtrlSignal::mChannels[channel].value < CRadioCtrlSignal::MIN_POSSIBLE_VAL)
   {  // basic filtering
      CRadioCtrlSignal::mChannels[channel].value = 0;       // error value
      
   }

   if (CRadioCtrlSignal::mChannels[channel].value != 0)
   {
      if (CRadioCtrlSignal::mCallibMode)
      {
         if (CRadioCtrlSignal::mChannels[channel].value > CRadioCtrlSignal::mChannels[channel].max)
         {
            CRadioCtrlSignal::mChannels[channel].max = CRadioCtrlSignal::mChannels[channel].value;
         }
         else if (CRadioCtrlSignal::mChannels[channel].value < CRadioCtrlSignal::mChannels[channel].min)
         {
            CRadioCtrlSignal::mChannels[channel].min = CRadioCtrlSignal::mChannels[channel].value;
         }
      }
      else if (CRadioCtrlSignal::mSettingNeutralMode)
      {
         CRadioCtrlSignal::mSettingNeutralMode = false;

         CRadioCtrlSignal::mChannels[channel].neutral = CRadioCtrlSignal::mChannels[channel].value;
      }
   }
}

// ----------------------------------------------------------------------------

ISR(INT0_vect)
{   
   uint32_t timeStamp = CMicroTimer::getMicroSeconds();

   sei();      // enable interrupt inside this interrupt

   if (PIND & _BV(PD2))
   {  // rising edge of impulse
      CRadioCtrlSignal::mChannels[0].pulseStartTime = timeStamp;
   }
   else
   {
      fallingEdgeProcessing(0, timeStamp);
   }
}

// ----------------------------------------------------------------------------

ISR(INT1_vect)
{
   uint32_t timeStamp = CMicroTimer::getMicroSeconds();

   sei();      // enable interrupt inside this interrupt

   if (PIND & _BV(PD3))
   {  // rising edge of impulse
      CRadioCtrlSignal::mChannels[1].pulseStartTime = timeStamp;
   }
   else
   {
      fallingEdgeProcessing(1, timeStamp);
   }
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void CRadioCtrlSignal::init(void)
{
   DDRD &= ~(_BV(PD3) | _BV(PD2));     // set pin as input (not really necessary) 
   PORTD |= _BV(PD3) | _BV(PD2);       // connect pull up resistors

   MCUCR |= _BV(ISC10) | _BV(ISC00);      // any logical changes on INT0 and INT1 generates interrupt

   GICR |= _BV(INT1) | _BV(INT0);

   
   mCallibMode = false;
   mSettingNeutralMode = false;

   for (uint8_t i = 0; i < MAX_CHANNELS; i++)
   {
      mChannels[i].max = MAX_POSSIBLE_VAL;
      mChannels[i].min = MIN_POSSIBLE_VAL;
      mChannels[i].value = 0;
      mChannels[i].neutral = DEFAULT_VAL;
      mChannels[i].pulseStartTime = 0;
   }
}

// ----------------------------------------------------------------------------

void CRadioCtrlSignal::startCallibration(void)
{
   mCallibMode = true;
}

// ----------------------------------------------------------------------------

void CRadioCtrlSignal::stopCallibration(void)
{
   mCallibMode = false;
}

// ----------------------------------------------------------------------------

uint16_t CRadioCtrlSignal::getValue(uint8_t channel)
{
   if (channel < MAX_CHANNELS)
   {
      return mChannels[channel].value;
   }

   return 0;
}

// ----------------------------------------------------------------------------

uint16_t CRadioCtrlSignal::getMax(uint8_t channel)
{
   if (channel < MAX_CHANNELS)
   {
      return mChannels[channel].max;
   }

   return 0;
}

// ----------------------------------------------------------------------------

uint16_t CRadioCtrlSignal::getMin(uint8_t channel)
{
   if (channel < MAX_CHANNELS)
   {
      return mChannels[channel].min;
   }

   return 0;
}

// ----------------------------------------------------------------------------

uint16_t CRadioCtrlSignal::getNeutral(uint8_t channel)
{
   if (channel < MAX_CHANNELS)
   {
      return mChannels[channel].neutral;
   }

   return 0;
}

