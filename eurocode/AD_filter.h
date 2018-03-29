#ifndef __AD_FILTER__
#define __AD_FILTER__

#include "def.h"


extern uint16_t filterMedianAverageSample (uint16_t *adValueBuf, int sampleNum);
extern uint16_t filterMedianSample (uint16_t *adValueBuf, int sampleNum);

#endif

