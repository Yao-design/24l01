#ifndef __TS800X_SLEEP_H__
#define __TS800X_SLEEP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ts800x.h"

#define ADC_FIXED_GAIN   0xFF

#ifndef TS_OTP_ROM_USE_OTP_INIT

#define SET_OTP_CTRL(v)    (*(volatile uint32_t*)(0x40000050) = (v))
#define PENVDD2_VDD2(v)    (v << 0)
#define PLDO(v)            (v << 1)
#define PDSTB(v)           (v << 2)
#define PCE(v)             (v << 3)
#define PTM(v)             (v << 4)
#define PPROG(v)           (v << 8)
#define PTR(v)             (v << 9)
#define PWE(v)             (v << 10)

#endif

/**
 * @addtogroup ts800x_StdPeriph_Driver
 * @{
 */

/**
 * @addtogroup SLEEP_Driver
 * @brief 	   SLEEP driver modules
 * @{
 */

/** @brief Enumerations for the possible microcontroller sleep modes.
 * - SLEEPMODE_RUNNING
 *     Everything is active and running.  In practice this mode is not
 *     used, but it is defined for completeness of information.
 * - SLEEPMODE_CPU_HALT
 *     Only the CPU is halted.  The rest of the chip continues running
 *     normally.  The chip will wake from any interrupt.
 * - SLEEPMODE_WAKETIMER
 *     The device is in deep sleep and the timer clock sources remain running.
 *     Wakeup is possible from both GPIO and the Hal Virtual Timers.
 * - SLEEPMODE_NOTIMER
 *     The device is in deep sleep. All the peripherals and clock sources are turned off.
 *     Wakeup is possible only from GPIOs IO9-IO10-IO11-IO12-IO13.
 */
typedef enum {
    SLEEPMODE_RUNNING       = 0,
    SLEEPMODE_CPU_HALT      = 1,
    SLEEPMODE_WAKETIMER     = 2,
    SLEEPMODE_NOTIMER       = 3,
} SleepModes;

#ifdef TS_EN_DEEP_SLEEP
/**
 * @brief This function allows to put the TS800X in low power state.
 *
 * This function allows to enter a desired sleep mode that is negotiated between BT stack needs and application needs.
 * The application can provide the desired sleep mode using the parameter sleepMode. In addition to this, the application can
 * optionally use the @ref SLEEP_AppSleepModeCheck to provide its desired sleep mode.
 * The function will compute the sleep mode by combining the different requests and choosing the lowest possible power mode.
 *
 * The device can be configured with different wake sources
 * according to the sleepMode parameter. The lowest power mode is obtained configuring the device in
 * SLEEPMODE_NOTIMER.
 * For all sleep modes, ts800x_Sleep_Check() will return when the wakeup occours.
 *
 * @param sleepMode Sleep mode (see SleepModes enum)
 *
 * @param gpioWakeBitMask  A bit mask of the GPIO that are allowed to wake
 * the chip from deep sleep.  A high bit in the mask will enable waking
 * the chip if the corresponding GPIO changes state.
 * @param gpioWakeLevelMask  A mask used to setup the active wakeup level:
 * - 0: the system wakes up when IO is low
 * - 1: the system wakes up when IO is high
 * The level sensitive bit mask is the same of the gpioWakeBitMask parameter.
 *
 * @retval Status of the call
 */
void ts800x_Sleep_Check(SleepModes sleepMode, uint32_t gpioWakeBitMask, uint32_t gpionWakeBitMask);

/**
 * @brief This function allows the application to define its desired sleep mode.
 *
 * The ts800x_Sleep_IsAllowed allows the application to set its desired sleep mode based on the application power management policy.
 * When user calls @ref ts800x_Sleep_Check, a negotiation occurs to define the sleep mode and this function is called to get
 * inputs from application.
 * It is important to notice that this function is executed with interrupts disabled
 *
 * @param sleepMode Sleep mode (see SleepModes enum)
 *
 * @retval Return the sleep mode possible and safe from the application point of view
 *
 * @note A weak implementation always returning SLEEPMODE_NOTIMER is provided as default
 * when no application specifc behaviour is required.
 */
uint16_t ts800x_Sleep_WakeupSource(void);
uint32_t ts800x_Sleep_Clear_Wakeup_Record(void);
uint32_t ts800x_Sleep_GpioWakeupSource(void);
uint32_t ts800x_Sleep_Get_ProtectTime(void);
void tsBle_Sleep_Set_Pre_Wakeup_Time(uint16_t cycles_32k);
void ts800x_Sleep_Set_ForceSleep(uint32_t wakeup_iobit_mask, uint32_t nwakeup_iobit_mask, uint32_t slp_unused_iobit_mask);
void ts800x_Sleep_Check_ForceSleep(uint32_t wakeup_iobit_mask, uint32_t nwakeup_iobit_mask, uint32_t slp_unused_iobit_mask);
void ts800x_Sleep_Init(void);
void ts800x_Sleep_Set_Enable(bool sleep_enable);
bool ts800x_Sleep_IsEnable(void);

#define TS800X_SLEEP_INIT()         ts800x_Sleep_Init()
#define TS800X_SLEEP_CHECK()        ts800x_Sleep_Check(SLEEPMODE_NOTIMER, TS_GPIO_WAKEUP_MASK, TS_GPIO_nWAKEUP_MASK)
#define TS800X_SLEEP_FORCE_CHECK()  ts800x_Sleep_Check_ForceSleep(TS_GPIO_WAKEUP_MASK, TS_GPIO_nWAKEUP_MASK, TS_WHEN_SLEEP_GPIO_MASK)
#define TS800X_SLEEP_FLAG_SET()     ts800x_Sleep_Set_Enable(true)
#define TS800X_SLEEP_FLAG_CLR()     ts800x_Sleep_Set_Enable(false)
#define TS800X_SLEEP_FLAG_GET()     ts800x_Sleep_IsEnable()
#define TS800X_SLEEP_WKUP_SRC_GET()         ts800x_Sleep_WakeupSource()
#define TS800X_SLEEP_WKUP_SRC_GET_AND_CLR() ts800x_Sleep_Clear_Wakeup_Record()
#define TS800X_SLEEP_WKUP_SRC_GPIO_GET()    ts800x_Sleep_GpioWakeupSource()
#define TS800X_SLEEP_SET_PRE_WKUP_TIME(CYCLES_32K_NUM)  tsBle_Sleep_Set_Pre_Wakeup_Time(CYCLES_32K_NUM)
#else
#define TS800X_SLEEP_INIT()
#define TS800X_SLEEP_CHECK()
#define TS800X_SLEEP_FORCE_CHECK()
#define TS800X_SLEEP_FLAG_SET()
#define TS800X_SLEEP_FLAG_CLR()
#define TS800X_SLEEP_FLAG_GET()             (0)
#define TS800X_SLEEP_WKUP_SRC_GET()         (0)
#define TS800X_SLEEP_WKUP_SRC_GET_AND_CLR() (0)
#define TS800X_SLEEP_WKUP_SRC_GPIO_GET()    (0)
#define TS800X_SLEEP_SET_PRE_WKUP_TIME(CYCLES_32K_NUM)  (CYCLES_32K_NUM)
#endif


#ifdef __cplusplus
}
#endif
#endif