/******************************************************************************
 * @file:   cfg_def_app.h
 * @author: Techstorm Semi Application Team
 * @brief:  cfg_def_app head file
 *
 * @date:   Created on 2024-09-19
 ******************************************************************************/

#ifndef CFG_DEF_APP_H
#define	CFG_DEF_APP_H

/******************************************************************************
 Include
 ******************************************************************************/

#ifdef	__cplusplus
extern "C" {
#endif

/******************************************************************************
 Provide External Macro Definition
 ******************************************************************************/
#define TS_2G4_DMA_EN_IRQ
#define TS_2G4_EN_AUTO_LEN
#define TS_2G4_EN_AUTO_ACK


#define TS_2G4_VER

/**
 * @brief Debug log config
 */
#define APP_CFG_DBG_LOG

#ifdef APP_CFG_DBG_LOG
#define LOG_RTT             1
#define LOG_UART            2   // default use uart0
#define APP_CFG_LOG_TYPE    LOG_UART
#endif
/****************************SYSTEM CFG****************************************/
/**
 * @brief Project is run 2.4G, not on ble, when this macro definition is enabled
 */
#define TS_2G4_ONLY
/**
 * @brief Woking main frequency
 */
#define SDK_HCLK_MHZ        24000000u

/**
 * @brief Low power
 */
 #define TS_EN_DEEP_SLEEP

#ifdef TS_EN_DEEP_SLEEP
/* open the define of UART0/1 when sleep, if uartX is enable. LOG_UART default use UART0 */
 #define TS_SAVE_UART0_REGS
// #define TS_SAVE_UART1_REGS

#define TS_EN_CLK_CAL
#define TS_GPIO_WAKEUP_MASK  0
#define TS_GPIO_nWAKEUP_MASK 0x4   //GPIO2
#define TS_EN_UNUSED_GPIO

    #ifdef TS_EN_UNUSED_GPIO
    /* QFN24: 13,14,16,18, 20,21,22,0,1 JTAG, HIGH */
    #define TS_UNUSED_GPIO_MASK        (0x8A9FFC)      // forever unused IO mask
    #define TS_WHEN_SLEEP_GPIO_MASK    (0x356000)      // SLEEP unused IO mask
    #else
    #define TS_FAKE_GPIO_HOLD
    #define TS_UNUSED_GPIO_MASK        0
    #define TS_WHEN_SLEEP_GPIO_MASK    0
    #endif
#endif

/**
 * @brief Watch dog
 */
//#define TS_EN_WDG
#ifdef TS_EN_WDG
#define TS_FEED_WDG_TIME    5
#endif


#ifdef	__cplusplus
}
#endif

/*******************************************************************************
 End of File
 ******************************************************************************/

#endif	/* CFG_DEF_APP_H */
