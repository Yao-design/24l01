#include "ts800x_it_2g4.h"


#ifdef TS_2G4_DMA_EN_IRQ
extern void RF_2G4_RADIO_Handler(void);

#ifdef TS_EN_DEEP_SLEEP
__RAM_CODE_SECTION
#endif
ATTRIBUTE_ISR void  RADIO_DMA_Handler(void)
{
    RF_2G4_RADIO_Handler();
}
#endif
