#include "boot__array.h"
#include "ts800x.h"
#include "system_init.h"
#include "ts800x_sleep_2g4.h"
//#include "ts800x_vtimer.h"

#ifdef APP_CFG_DBG_LOG
#include "log.h"
#endif

#define EN_RTC() (*(volatile uint32_t *)0x40080050 = 0x1)
#define IRQ_NVIC_PRIO(IRQn,priority) (((priority << (8U - __NVIC_PRIO_BITS)) & (uint32_t)0xFFUL) << _BIT_SHIFT(IRQn))

void ts800x_Otp_LoadCfgInfo(void);
void ts800x_Otp_ApplyCfgInfo(void);

#ifndef TS_ON_FPGA
// only call one time when power on
void PMU_OnChip_Init();
#endif

//#ifdef TS_EN_DEEP_SLEEP
void unused_gpio_mask_parse_and_set(unsigned int m);

#ifdef TS_OTP_ROM
#undef __RAM_CODE_SECTION
#define __RAM_CODE_SECTION
#endif
#define UNUSED_GPIO_FUNC 0xA0 // 0xA0: high-impedance, 0x20: pull-up, 0x40: pull-down
// Description: parse GPIO mask, set gpio oe '0', set gpio function to UNUSED_GPIO_FUNC
__RAM_CODE_SECTION
void unused_gpio_mask_parse_and_set(unsigned int m)
{
    int i, j, t, fclr, fset;
    int *p;
    p = (int *)&(GPIO_ATF->GPIO_CFG_0);
    //---------------------------------------
    // GPIO OE
    //---------------------------------------
    t = GPIO_INOUT->GPIO_OE;
    t = t & (~m);
    GPIO_INOUT->GPIO_OE = t;
    //---------------------------------------
    // GPIO FUNC
    //---------------------------------------
    for (i = 0; i < 6; i++)
    {
        // read
        t = *(p + i);
        fclr = 0xff;
        fset = UNUSED_GPIO_FUNC;
        // modify
        for (j = 0; j < 4; j++)
        {
            if (m & 1)
            {
                t = t & (~fclr);
                t = t | (fset);
            }
            m = m >> 1;
            fclr = fclr << 8;
            fset = fset << 8;
        }
        // write
        *(p + i) = t;
    }
}
//#endif
void ts800x_SWD_Close(void)
{
    GPIO_ATF->GPIO_CFG_0 &= 0xFFFF0000;
}
static void irq_priority()
{
    NVIC_SetPriority(SVCall_IRQn,IRQ_LOW_PRIORITY);
    NVIC_SetPriority(SysTick_IRQn, IRQ_MED_PRIORITY);
    NVIC_SetPriority(PendSV_IRQn,IRQ_LOW_PRIORITY);
    NVIC->IP[0] = IRQ_NVIC_PRIO(WDT_IRQn,IRQ_HIGH_PRIORITY) | IRQ_NVIC_PRIO(Interrupt1_IRQn,IRQ_LOW_PRIORITY) | IRQ_NVIC_PRIO(RADIO_DMA_IRQn,IRQ_CRITICAL_PRIORITY) | IRQ_NVIC_PRIO(RADIO_TICK_IRQn,IRQ_HIGH_PRIORITY);
    NVIC->IP[1] = IRQ_NVIC_PRIO(Interrupt4_IRQn,IRQ_LOW_PRIORITY) | IRQ_NVIC_PRIO(Interrupt5_IRQn,IRQ_LOW_PRIORITY) | IRQ_NVIC_PRIO(Interrupt6_IRQn,IRQ_LOW_PRIORITY) | IRQ_NVIC_PRIO(ADC_IRQn,IRQ_MED_PRIORITY);
    NVIC->IP[2] = IRQ_NVIC_PRIO(UART0_IRQn,IRQ_MED_PRIORITY) | IRQ_NVIC_PRIO(UART1_IRQn,IRQ_MED_PRIORITY) | IRQ_NVIC_PRIO(SPIM0_IRQn,IRQ_MED_PRIORITY) | IRQ_NVIC_PRIO(SPIM1_IRQn,IRQ_MED_PRIORITY);
    NVIC->IP[3] = IRQ_NVIC_PRIO(I2C0_IRQn,IRQ_MED_PRIORITY) | IRQ_NVIC_PRIO(Interrupt13_IRQn,IRQ_LOW_PRIORITY) | IRQ_NVIC_PRIO(SPIS0_IRQn,IRQ_MED_PRIORITY) | IRQ_NVIC_PRIO(USB_IRQn,IRQ_LOW_PRIORITY);
    NVIC->IP[4] = IRQ_NVIC_PRIO(GPIO_IRQn,IRQ_MED_PRIORITY) | IRQ_NVIC_PRIO(QDEC_IRQn,IRQ_LOW_PRIORITY) | IRQ_NVIC_PRIO(AES_IRQn,IRQ_MED_PRIORITY) | IRQ_NVIC_PRIO(CAPTURE_IRQn,IRQ_LOW_PRIORITY);
    NVIC->IP[5] = IRQ_NVIC_PRIO(CLKCAL_IRQn,IRQ_LOW_PRIORITY) | IRQ_NVIC_PRIO(RTC_IRQn,IRQ_HIGH_PRIORITY) | IRQ_NVIC_PRIO(SDMA_IRQn,IRQ_LOW_PRIORITY) | IRQ_NVIC_PRIO(Interrupt23_IRQn,IRQ_LOW_PRIORITY);
    NVIC->IP[6] = IRQ_NVIC_PRIO(TIMER0_IRQn,IRQ_MED_PRIORITY) | IRQ_NVIC_PRIO(TIMER1_IRQn,IRQ_MED_PRIORITY) | IRQ_NVIC_PRIO(TIMER2_IRQn,IRQ_MED_PRIORITY) | IRQ_NVIC_PRIO(TIMER3_IRQn,IRQ_MED_PRIORITY);
    NVIC->IP[7] = IRQ_NVIC_PRIO(TIMER4_IRQn,IRQ_MED_PRIORITY) | IRQ_NVIC_PRIO(TIMER5_IRQn,IRQ_MED_PRIORITY) | IRQ_NVIC_PRIO(TIMER6_IRQn,IRQ_MED_PRIORITY) | IRQ_NVIC_PRIO(TIMER7_IRQn,IRQ_MED_PRIORITY);
}

void SystemInit(void)
{
    #ifdef APP_FW_PROTECT_EN
    ts800x_SWD_Close();
    #endif
    SCB->VTOR = 0x20000000; // __RAM_BASE
    //SCB->VTOR = 0x10000000;
    /***********reset handler end***********/

    /***********system init config start***********/
    EN_RTC();

    REG_WRT(0x42000100, ((XTAL_FREQ - 1) << 8) + 1); // enable BB us tick 0x1701

    ts800x_Otp_LoadCfgInfo();
    ts800x_Otp_ApplyCfgInfo();

    #ifndef TS_ON_FPGA
    #ifndef TS_MCU_ONLY
    PMU_OnChip_Init();
    #endif
    #endif

    #ifdef TS_EN_DEEP_SLEEP
    #ifndef TS_RAM_SAVE_24K // TS_OTP_ROM_TEST
    AON_CTRL->AON_CTRL0 = 0x7A;
    #else
    AON_CTRL->AON_CTRL0 = 0xFA;
    #endif
    AON_CTRL->GOSLEEP = 0;

    #ifdef TS_EN_DEEP_SLEEP
    unused_gpio_mask_parse_and_set(TS_UNUSED_GPIO_MASK);
    #endif

    #ifdef TS_EN_FORCE_SLEEP
    TS800X_SLEEP_FORCE_CHECK();
    #endif

    (void)ts800x_Sleep_Clear_Wakeup_Record();
    #endif

    #if(SDK_HCLK_MHZ == 48000000)
    REG_WRT(0x42002000, 2);
    REG_WRT_MSK(0x40000024, ((3 << 18) | (3 << 16) | (3 << 10) | (0x3F << 4)), ((2 << 18) | (2 << 16) | (1 << 10) | (0x23 << 4)));
    #else
    REG_WRT_MSK(0x40000024, (3 << 16 | 3 << 10), (1 << 16 | 1 << 10));
    #endif

    #ifndef TS_EN_WDG
    // wdt_en=0
    SYS_CTRL->WDT_CFG = 0x00;
    #endif

    /***********system init config end***********/
    irq_priority();
    __enable_irq();

    /***********system global config start before app main app init***********/
    #ifdef APP_CFG_DBG_LOG
    TS_LOG_INIT();
    #endif

    /***********system global config end before app main app init***********/
}


#ifdef TS_BLE_V0
void rom_hook_register(int index, void *func)
{
    rom_hook_table[index] = func;
}
#endif

#if defined(TS_BLE_V0) || defined(TS_BLE_V0_2_0)
// ref: https://www.cnblogs.com/hancm/p/3638039.html
// ref: http://ftp.gnu.org/gnu/glibc/glibc-2.21.tar.gz
#undef RAND_MAX
#define RAND_MAX (32768)
// static unsigned long int next = 1;

//int xrand(void)
//{
//    next = next * 1103515245 + 12345;

//    return (unsigned int)(next>>16) % (RAND_MAX);
//}

//void xsrand(unsigned int seed)
//{
//    next = seed;
//}
#endif

#ifdef TS_EN_DEEP_SLEEP
__RAM_CODE_SECTION
#endif
void delay_us(uint32_t us)
{
    volatile uint32_t cur_us = REG_RD(0x42000104);
    while(REG_RD(0x42000104) - cur_us < us);
}

void delay_ms(uint32_t ms)
{
    delay_us(ms * 1000);
}

//void ts800x_Hw_Chk_And_EnIrq(void)
//{
//#if defined(__CC_ARM)
//    register uint32_t primask  __ASM("primask");
//#else
//    //work round: for irq issue
//    uint32_t primask;
//    __ASM volatile ("mrs %0, primask" : "=r" (primask) : : "memory");
//#endif
//    if (0 == (~primask & 1)){
//        __enable_irq();
//    }
//}

uint32_t system_Enter_Critical(void)
{
    uint32_t stat = __get_PRIMASK();
    __disable_irq();
    return stat;
}

void system_Exit_Critical(uint32_t prev_stat)
{
    if (0 == prev_stat)
    {
        __enable_irq();
    }
}
