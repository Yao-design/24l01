#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ts800x_sleep_2g4.h"

#ifdef TS_EN_DEEP_SLEEP

#include "ts800x_rtc.h"
#include "ts800x_hal_uart.h"
#include "ts800x_ll_gpio.h"



#define TS800X_SLEEP_ADDR_LIST_SIZE     (sizeof(addr_list) / sizeof(uint32_t))



static uint32_t * g_save_buf;

#ifdef TS_SAVE_UART0_REGS
static uint32_t * uart0_save_buf;
#endif
#ifdef TS_SAVE_UART1_REGS
static uint32_t * uart1_save_buf;
#endif

static volatile uint32_t sleep_wakeup_reason;


static uint32_t addr_list[] =
{
    /////////////////////////////////////////////// REGS///////////////////////////////////
    /* GPIO regs */
    0x40010200, 0x40010204, 0x40010208, 0x4001020C, 0x40010210, 0x40010214, 0x40010300, 0x40010304,
    0x40010010, 0x40010030, 0x40010040, 0x40010170, 0x40010180, 0x40010190, 0x400101A0, 0x40010140,
    (uint32_t)&NVIC->IP[0], (uint32_t)&NVIC->IP[1], (uint32_t)&NVIC->IP[2], (uint32_t)&NVIC->IP[3],
    (uint32_t)&NVIC->IP[4], (uint32_t)&NVIC->IP[5], (uint32_t)&NVIC->IP[6], (uint32_t)&NVIC->IP[7],
    /* Device regs */

    /* SPIS regs */

    /* ... */
};


#if defined(TS_SAVE_UART0_REGS) || defined(TS_SAVE_UART1_REGS)

#define IER_DLL 5
#define IER_DLF (sizeof(uart_addr_idx_list) - 1)

uint8_t uart_addr_idx_list[] =
{
    TS_UART_LCR, TS_UART_LCR_EXT,  TS_UART_FCR, TS_UART_MCR,  TS_UART_IER,
    TS_UART_DLL, TS_UART_DLH, TS_UART_DLF,
};


static void ts800x_sleep_save_uart_reg_info(uint32_t * uart, uint32_t * save_buf)
{
    volatile uint32_t * uart_reg_base = (volatile uint32_t *)uart;

    for (int i = 0; i < sizeof(uart_addr_idx_list); i++)
    {
        if (i == IER_DLL)
        {
            ts800x_HAL_Uart_Wait_Idle(uart);

            uart_reg_base[TS_UART_LCR] |= 0x80; //LCR_DLAB
        }

        save_buf[i] = uart_reg_base[uart_addr_idx_list[i]];
    }
}

void ts800x_sleep_restore_uart_reg_info(uint32_t * uart, uint32_t * save_buf)
{
    volatile uint32_t * uart_reg_base = (volatile uint32_t *)uart;

    for (int i = 0; i < sizeof(uart_addr_idx_list); i++)
    {
        if (i == IER_DLL)
        {
            ts800x_HAL_Uart_Wait_Idle(uart);

            uart_reg_base[TS_UART_LCR] |= 0x80; //LCR_DLAB
        }

        uart_reg_base[uart_addr_idx_list[i]] = save_buf[i];

        if (i == IER_DLF)
        {
            uart_reg_base[TS_UART_LCR] &= (~0x80); //LCR_DLAB
        }
    }
}
#endif

static void ts800x_sleep_save_reg_info_ext()
{
    uint8_t i = 0;

    for (i = 0; i < TS800X_SLEEP_ADDR_LIST_SIZE; i++)
    {
        g_save_buf[i] = *(uint32_t *)addr_list[i];
    }

#ifdef TS_SAVE_UART0_REGS
    //maybe need to de-init uart gpio
    ts800x_sleep_save_uart_reg_info((uint32_t*)UART0_BASE, uart0_save_buf);
#endif

#ifdef TS_SAVE_UART1_REGS
    //maybe need to de-init uart gpio
    ts800x_sleep_save_uart_reg_info((uint32_t*)UART1_BASE, uart1_save_buf);
#endif
}

static void _TS800X_Sleep_restore_reg_info_ext()
{
    #ifdef TS_SAVE_UART0_REGS
    ts800x_sleep_restore_uart_reg_info((uint32_t*)UART0_BASE, uart0_save_buf);
    #endif

    #ifdef TS_SAVE_UART1_REGS
    ts800x_sleep_restore_uart_reg_info((uint32_t*)UART1_BASE, uart1_save_buf);
    #endif

    for (uint8_t i = 0; i < TS800X_SLEEP_ADDR_LIST_SIZE; i++)
    {
        *(uint32_t *)addr_list[i] = g_save_buf[i];
    }
}

#ifdef CONFIG_OTP_PROGRAM
#ifdef TS_CPU_E902
void (*otp_init)(void) = (void *)0x00000102;
#else
void (*otp_init)(void) = (void *)(0x000002f8 | 1);
#endif
#endif

#ifdef CONFIG_FLASH_PROGRAM
// Description: set flash power GPIO state, from output '1' to '0'
// NOTE:
//   1) if more than one GPIO, invoke this function more than one times
__RAM_CODE_SECTION
void flash_power_off(int p){
		#ifdef CONFIG_FLASH_PROGRAM
    int t;
    t = GPIO_INOUT->GPIO_O;
    t = t & (~(1<<p));
    GPIO_INOUT->GPIO_O = t;
    #endif
}
#endif

__RAM_CODE_SECTION
void qspi_regs_restore()
{
		#ifdef CONFIG_FLASH_PROGRAM
//    REG_WRT(0x40010208, QSPI_VDD_FUNC_REG_VAL);
    REG_WRT(0x40010214, QSPI_VDD_FUNC_REG_VAL);
    REG_WRT(0x4001020C, QSPI_FLASH_FUNC_REG1_VAL);
    REG_WRT(0x40010210, QSPI_FLASH_FUNC_REG2_VAL);
    REG_WRT(0x40010300, QSPI_REMAP_REG_VAL);

    #ifndef TS_RSTR_MORE_CR_REGS
    REG_WRT(0x40010030, QSPI_VDD_OE_REG_VAL);
    REG_WRT(0x40010040, QSPI_VDD_O_REG_VAL);
    #endif
    #endif
}


void ts800x_ChipSleepCriticalWork(void);

#define MAX_GPIO_NR  24
 uint32_t   unused_gpio_mask_when_sleep;

void ts800x_cfg_wakeup_gpio(uint32_t gpio_mask, uint8_t is_n_wakeup)
{
    // unused_gpio_mask_when_sleep &= ~gpio_mask;

    for (int i = 0; i < MAX_GPIO_NR; i++)
    {
        if ((gpio_mask >> i) & 0x1)
        {
            uint32_t func_val = is_n_wakeup ? 0x20 : 0x40;
            //TODO: if has remap, need to cfg remap regs
            ts800x_LL_Gpio_Set_Func(i, func_val);
        }
    }

    /* Setup the Wakeup Source */
    if (!is_n_wakeup)
    {
        AON_CTRL->WAKEUP_CTRL0 = gpio_mask;
    }
    else
    {
        AON_CTRL->WAKEUP_CTRL1 = gpio_mask;
    }
}


static uint8_t _TS800X_InternalSleep( uint32_t gpioWakeBitMask, uint32_t gpionWakeBitMask)
{
    uint32_t reg_save_buf[TS800X_SLEEP_ADDR_LIST_SIZE];

    g_save_buf = reg_save_buf;


	unused_gpio_mask_when_sleep = TS_WHEN_SLEEP_GPIO_MASK;

    #ifdef TS_SAVE_UART0_REGS
    uint32_t uart0_save_buf_lc[sizeof(uart_addr_idx_list)];
    uart0_save_buf = uart0_save_buf_lc;
    #endif

    #ifdef TS_SAVE_UART1_REGS
    uint32_t uart1_save_buf_lc[sizeof(uart_addr_idx_list)];
    uart1_save_buf = uart1_save_buf_lc;
    #endif

    ts800x_sleep_save_reg_info_ext();
    if (gpioWakeBitMask)  ts800x_cfg_wakeup_gpio(gpioWakeBitMask, 0);
    if (gpionWakeBitMask) ts800x_cfg_wakeup_gpio(gpionWakeBitMask, 1);

    // Clear sleep flag.
    AON_CTRL->CPU_RST_CLR = 0x3ff;
    AON_CTRL->WAKEUP_CLEAR = 0x1000000;


   // REG_WRT_BITS(0x40080110, 14, 11, 15);  // xtal current, before sleep

   // REG_WRT(0x40080060, (0x96 << 8) | (4 << 4) | 0); //ram error patch, before sleep

    ts800x_ChipSleepCriticalWork();

    __disable_irq();

    //gpio hold has been released
    _TS800X_Sleep_restore_reg_info_ext();

    // save wakeup info, avoid error in avtive, must clear here.
    sleep_wakeup_reason = AON_CTRL->CPU_RST_RCD;

    // Clear sleep flag.
    AON_CTRL->CPU_RST_CLR = 0x3ff;

    __enable_irq();

     return 1;
}

void force_sleep_wakeup_check()
{

}

uint32_t ts800x_Sleep_Clear_Wakeup_Record()
{
    uint32_t  w_rec = AON_CTRL->WAKEUP_RECORD;

    if (w_rec)
    {
        AON_CTRL->WAKEUP_CLEAR = w_rec;
        for (volatile int i = 0; i < 500; i++);
        AON_CTRL->WAKEUP_CLEAR = 0;
    }

    return w_rec;
}
__RAM_CODE_SECTION
void gpio_regs_restore_before_rel_gpio_hold()
{//the gpio regs need to be restored before gpio hold released, set here
    REG_WRT(0x40010030, g_save_buf[9]);
    REG_WRT(0x40010040, g_save_buf[10]);

}

#ifdef CONFIG_FLASH_PROGRAM
extern uint8_t  gpio_vdd_pin1;
extern uint8_t  gpio_vdd_pin2;
extern uint8_t  has_flash;
#endif

#ifdef CONFIG_OTP_PROGRAM
extern uint8_t has_otp;
#endif
uint32_t ts2G4_sleep_goto_sleep()
{
    #ifdef CONFIG_FLASH_PROGRAM
    //has_flash = CONFIG_FLASH_PROGRAM;
		has_flash = 1;
    gpio_vdd_pin1 = TS_FLASH_VDDIO_PIN1;
    gpio_vdd_pin2 = TS_FLASH_VDDIO_PIN2;
    #endif

    #ifdef CONFIG_OTP_PROGRAM
    //has_otp = CONFIG_OTP_PROGRAM;
		has_otp = 0;
    #endif

    _TS800X_InternalSleep(TS_GPIO_WAKEUP_MASK, TS_GPIO_nWAKEUP_MASK);

    return AON_CTRL->WAKEUP_RECORD;
}
#endif