#ifndef __CRADIOCTRLSIGNAL_H__
#define __CRADIOCTRLSIGNAL_H__

#include <inttypes.h>
#include <avr/interrupt.h>

// forward declarations for interrupts
extern "C" void INT0_vect(void) __attribute__ ((signal));
extern "C" void INT1_vect(void) __attribute__ ((signal));
void fallingEdgeProcessing(const uint8_t channel, const uint32_t timeStamp);


class CRadioCtrlSignal
{
private:
   CRadioCtrlSignal();


public:
   
   static const uint8_t MAX_CHANNELS = 2;
   static const uint16_t MAX_POSSIBLE_VAL = 2200;     // pulse of 2.2milliseconds
   static const uint16_t MIN_POSSIBLE_VAL = 800;      // pulse of 0.8millisecond
   static const uint16_t DEFAULT_VAL      = 1500;     // pulse of 1.5millisecond


   static void init(void);

   static void startCallibration(void);
   static void stopCallibration(void);
   static void setNeutral(void);

   static uint16_t getValue(uint8_t channel);

   static uint16_t getMax(uint8_t channel);

   static uint16_t getMin(uint8_t channel);

   static uint16_t getNeutral(uint8_t channel);

private:
   friend void INT0_vect(void);
   friend void INT1_vect(void);
   friend void fallingEdgeProcessing(const uint8_t channel, const uint32_t timeStamp);


   struct ChannelInfo
   {
      uint32_t pulseStartTime;

      uint16_t max;
      uint16_t min;
      uint16_t neutral;
      uint16_t value;      // actual value of channel
   };

   static ChannelInfo mChannels[MAX_CHANNELS];

   static bool mCallibMode;
   static bool mSettingNeutralMode;
};


#endif // __CRADIOCTRLSIGNAL_H__