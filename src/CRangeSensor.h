#ifndef __CRANGESENSOR_H__
#define __CRANGESENSOR_H__

#include <inttypes.h>

class CRangeSensor
{
private:
   CRangeSensor();

   

public:

   static void init(void);

   static void startMeasurement(void);

   static uint32_t getMeasurement(void);

   static bool isReady(void);
};


#endif