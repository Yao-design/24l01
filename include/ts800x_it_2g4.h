
#ifndef __TS800X_IT_H__
#define __TS800X_IT_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include "ts800x.h"

ATTRIBUTE_ISR void RADIO_DMA_Handler(void);
ATTRIBUTE_ISR void RADIO_TICK_Handler(void);
ATTRIBUTE_ISR void RTC_Handler(void);

#ifdef __cplusplus
}
#endif

#endif

