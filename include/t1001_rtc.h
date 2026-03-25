
#ifndef _T1001_RTC_H
#define _T1001_RTC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "t1001.h"

#define RTC_IT_Overflow                           ((uint16_t)1 << SYS_CTRL_AON_RTC_IRQ_AON_RTC_OVFL_IE_Pos)
#define RTC_IT_Timeout                            ((uint16_t)1 << SYS_CTRL_AON_RTC_IRQ_AON_RTC_TO_IE_Pos)

#define RTC0_IT_STATUS_Overflow                   ((uint16_t)1 << SYS_CTRL_AON_RTC_IRQ_AON_RTC_OVFL_IS_Pos)
#define RTC1_IT_STATUS_Timeout                    ((uint16_t)1 << SYS_CTRL_AON_RTC_IRQ_AON_RTC_TO_IS_Pos)

/** Check RTC_IT valid. */
#define IS_RTC_IT(_IT)                 ((_IT) == RTC_IT_Overflow ||\
										(_IT) == RTC_IT_Timeout)

/** Check RTC_IT_STATUS valid. */
#define IS_RTC_IT_STATUS(_IT)                 ((_IT) == RTC0_IT_STATUS_Overflow ||\
											   (_IT) == RTC1_IT_STATUS_Timeout)

uint32_t RTC_GetCurrentSysTime(void);
void RTC_SetTimerTarget(uint32_t timeout);
uint32_t RTC_GetTimerTarget(void);

void RTC_ITConfig(uint16_t RTC_IT, FunctionalState NewState);
ITStatus RTC_GetITStatus(uint16_t RTC_IT);
void RTC_ClearITPendingBit(uint16_t RTC_IT);


#ifdef __cplusplus
}
#endif
#endif
