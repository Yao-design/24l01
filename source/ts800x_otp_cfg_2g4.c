#include "string.h"
#include "ts800x_otp_cfg_2g4.h"

/////////////////////////////////////////////////////////////////////////////
// MACRO
/////////////////////////////////////////////////////////////////////////////
#define OTP_CFG_ADDR                    0x1F803F30  // OTP parameter start address
#define RF_BG_VTRIM_DEFAULT             10          // If OTP do not program or parity check failed, use this value!!!
#define RF_AON_BG_VTRIM_DEFAULT         3           //
#define RCO_24M_CC_DEF                  0x8D
#define RCO_32K_CC_DEF                  0xB7
#define RF_CORNER_CLK_DEF               280         // Default Corner CLK Reference Value
#define RF_XO_CTUNE_DEFAULT             53          // Default Crystal XO CTUNE Value
#define RF_POWER_RX_RSSI_REF            1036        // Default 0dBm RX RSSI Reference Value
#define CORNER_OFFSET_SLOPE_DEF         8           // Default Corner CLK offset slope at various temperature
#define HP_OFFSET_SLOPE_DEF             25          // Default HP offset slope at various temperature
#define ADC_ROOM_TEMP_DEFAULT         0x3FF       // Default ADC value of room temperature
#define ADC_DIV1_HI_VOLT_DEF          600
#define ADC_DIV1_HI_SAMP_DEF          683
#define ADC_DIV1_LO_VOLT_DEF          200
#define ADC_DIV1_LO_SAMP_DEF          228
#define ADC_DIV2_HI_VOLT_DEF          1200
#define ADC_DIV2_HI_SAMP_DEF          683
#define ADC_DIV2_LO_VOLT_DEF          400
#define ADC_DIV2_LO_SAMP_DEF          228
#define ADC_DIV3_HI_VOLT_DEF          1800
#define ADC_DIV3_HI_SAMP_DEF          683
#define ADC_DIV3_LO_VOLT_DEF          600
#define ADC_DIV3_LO_SAMP_DEF          228
#define ADC_DIV4_HI_VOLT_DEF          2400
#define ADC_DIV4_HI_SAMP_DEF          683
#define ADC_DIV4_LO_VOLT_DEF          800
#define ADC_DIV4_LO_SAMP_DEF          228
#define ADC_VBAT_HI_VOLT_DEF          3000
#define ADC_VBAT_HI_SAMP_DEF          853
#define ADC_VBAT_LO_VOLT_DEF          2000
#define ADC_VBAT_LO_SAMP_DEF          569
#define ADC_RES_DEF                   240000      // 240 Kohm

#ifdef TS_EN_OTP_PAR_CHK
// 0: check failed
// 1: check pass
static int otp_check_parity(uint8_t *q, int len){
    int i, j, p;
    p = 0;
    for(i=0; i<len; i++)
        for(j=0; j<8; j++)
            p ^= (((*(q+i))>>j) & 1);
    return p;
}

#define OTP_CHECK_PARITY(val, sz)       (otp_check_parity((uint8_t*)&val, sz))
#define OTP_CHECK_WORD_PARITY(val, sz)  OTP_CHECK_PARITY(val, sz)
#else
#define OTP_CHECK_PARITY(val, sz)       ((val) != ((1 << ((sz) * 8)) - 1))
#define OTP_CHECK_WORD_PARITY(val, sz)  ((val) != 0xFFFFFFFF)
#endif

otp_cfg_t g_otp_cfg;

#ifdef TS_FT_APP
// FT power calibration
uint32_t rf_rssi_cal;
#endif

void ts800x_Otp_LoadCfgInfo()
{
    // Read from OTP
    memcpy((void*)&g_otp_cfg, (void *)OTP_CFG_ADDR, sizeof(otp_cfg_t));

    // RF
    if (!OTP_CHECK_PARITY(g_otp_cfg.rf_bg_vtrim.raw, 1)) {
        g_otp_cfg.rf_bg_vtrim.value = RF_BG_VTRIM_DEFAULT;
    }
    // if (!OTP_CHECK_PARITY(g_otp_cfg.rf_bg_volt_offset, 1)) {
    //     g_otp_cfg.rf_bg_volt_offset = 0;
    // }
    if (!OTP_CHECK_PARITY(g_otp_cfg.aon_rg_bg_vref_trim.raw, 1)) {
        g_otp_cfg.aon_rg_bg_vref_trim.value = RF_AON_BG_VTRIM_DEFAULT;
    }
    // if (!OTP_CHECK_PARITY(g_otp_cfg.aon_bg_volt_offset, 1)) {
    //     g_otp_cfg.aon_bg_volt_offset = 0;
    // }
// #ifndef TS_NO_RC_CAL
    if (!OTP_CHECK_PARITY(g_otp_cfg.aon_rco24m_cc.raw, 2)) {
        g_otp_cfg.aon_rco24m_cc.value = RCO_24M_CC_DEF;
    }
    if (!OTP_CHECK_PARITY(g_otp_cfg.aon_rco32k_cc.raw, 2)) {
        g_otp_cfg.aon_rco32k_cc.value = RCO_32K_CC_DEF;
    }
// #endif // !TS_NO_RC_CAL

    if (!OTP_CHECK_PARITY(g_otp_cfg.corner_tgt_freq.raw, 2)) {
        g_otp_cfg.corner_tgt_freq.value = RF_CORNER_CLK_DEF;
    }
#ifndef TS_NO_ADC_CAL
    // ADC DIV1
    if (!OTP_CHECK_WORD_PARITY(g_otp_cfg.adc_high_div1.raw, 4)) {
        g_otp_cfg.adc_high_div1.voltage = ADC_DIV1_HI_VOLT_DEF;
        g_otp_cfg.adc_high_div1.sample = ADC_DIV1_HI_SAMP_DEF;
    }
    if (!OTP_CHECK_WORD_PARITY(g_otp_cfg.adc_low_div1.raw, 4)) {
        g_otp_cfg.adc_low_div1.voltage = ADC_DIV1_LO_VOLT_DEF;
        g_otp_cfg.adc_low_div1.sample = ADC_DIV1_LO_SAMP_DEF;
    }
    // ADC DIV2
    if (!OTP_CHECK_WORD_PARITY(g_otp_cfg.adc_high_div2.raw, 4)) {
        g_otp_cfg.adc_high_div2.voltage = ADC_DIV2_HI_VOLT_DEF;
        g_otp_cfg.adc_high_div2.sample = ADC_DIV2_HI_SAMP_DEF;
    }
    if (!OTP_CHECK_WORD_PARITY(g_otp_cfg.adc_low_div2.raw, 4)) {
        g_otp_cfg.adc_low_div2.voltage = ADC_DIV2_LO_VOLT_DEF;
        g_otp_cfg.adc_low_div2.sample = ADC_DIV2_LO_SAMP_DEF;
    }
    // ADC DIV3
    if (!OTP_CHECK_WORD_PARITY(g_otp_cfg.adc_high_div3.raw, 4)) {
        g_otp_cfg.adc_high_div3.voltage = ADC_DIV3_HI_VOLT_DEF;
        g_otp_cfg.adc_high_div3.sample = ADC_DIV3_HI_SAMP_DEF;
    }
    if (!OTP_CHECK_WORD_PARITY(g_otp_cfg.adc_low_div3.raw, 4)) {
        g_otp_cfg.adc_low_div3.voltage = ADC_DIV3_LO_VOLT_DEF;
        g_otp_cfg.adc_low_div3.sample = ADC_DIV3_LO_SAMP_DEF;
    }
    // ADC DIV4
    if (!OTP_CHECK_WORD_PARITY(g_otp_cfg.adc_high_div4.raw, 4)) {
        g_otp_cfg.adc_high_div4.voltage = ADC_DIV4_HI_VOLT_DEF;
        g_otp_cfg.adc_high_div4.sample = ADC_DIV4_HI_SAMP_DEF;
    }
    if (!OTP_CHECK_WORD_PARITY(g_otp_cfg.adc_low_div4.raw, 4)) {
        g_otp_cfg.adc_low_div4.voltage = ADC_DIV4_LO_VOLT_DEF;
        g_otp_cfg.adc_low_div4.sample = ADC_DIV4_LO_SAMP_DEF;
    }
#endif
    // TEMP
    if (!OTP_CHECK_WORD_PARITY(g_otp_cfg.temperature.raw, 4)) {
        g_otp_cfg.temperature.room_adc_value = ADC_ROOM_TEMP_DEFAULT;
        g_otp_cfg.temperature.corner_offset_slope = CORNER_OFFSET_SLOPE_DEF;
        g_otp_cfg.temperature.hp_offset_slope = HP_OFFSET_SLOPE_DEF;
        g_otp_cfg.temperature.vtrim_offset = 0xF;
        g_otp_cfg.temperature.sx_offset = 0xF;
    }
    // RX_RSSI
    if (!OTP_CHECK_PARITY(g_otp_cfg.rx_rssi_high.raw, 2)) {
        g_otp_cfg.rx_rssi_high.value = 0;
    }
    if (!OTP_CHECK_PARITY(g_otp_cfg.rx_rssi_low.raw, 2)) {
        g_otp_cfg.rx_rssi_low.value = 0;
    }
    // TX_RSSI
    if (!OTP_CHECK_PARITY(g_otp_cfg.tx_pwr_rx_rssi.raw, 2)) {
        g_otp_cfg.tx_pwr_rx_rssi.value = RF_POWER_RX_RSSI_REF;
    }
#ifndef TS_NO_XO_CTUNE_CAL
    // XO CTUNE
    if (!OTP_CHECK_WORD_PARITY(g_otp_cfg.crystal_cal.raw, 4)) {
        g_otp_cfg.crystal_cal.value = RF_XO_CTUNE_DEFAULT;
    }
#endif // !TS_NO_XO_CTUNE_CAL

#ifndef TS_NO_ADC_CAL
    // ADC_RES
    if (!OTP_CHECK_WORD_PARITY(g_otp_cfg.adc_res.raw, 4)) {
        g_otp_cfg.adc_res.value = ADC_RES_DEF;
    }
    // ADC_VBAT
    if (!OTP_CHECK_WORD_PARITY(g_otp_cfg.adc_vbat_high.raw, 4)) {
        g_otp_cfg.adc_vbat_high.voltage = ADC_VBAT_HI_VOLT_DEF;
        g_otp_cfg.adc_vbat_high.sample = ADC_VBAT_HI_SAMP_DEF;
    }
    if (!OTP_CHECK_WORD_PARITY(g_otp_cfg.adc_vbat_low.raw, 4)) {
        g_otp_cfg.adc_vbat_low.voltage = ADC_VBAT_LO_VOLT_DEF;
        g_otp_cfg.adc_vbat_low.sample = ADC_VBAT_LO_SAMP_DEF;
    }
#endif  // !TS_NO_ADC_CAL
#ifdef TS_FT_APP
    rf_rssi_cal = REG_RD(0x1F803FF0);
#endif
}

void ts800x_Otp_ApplyCfgInfo(void)
{
    //-----------------------------------------------------
    // AON
    //-----------------------------------------------------
    // aon_rg_bg_vref_trim
    REG_WRT_BITS(0x40080104, 17, 14, g_otp_cfg.aon_rg_bg_vref_trim.value);
    // aon_rco24m_cc
    REG_WRT_BITS(0x40080108, 15, 8, g_otp_cfg.aon_rco24m_cc.value);
    // aon_rco32k_cc
    REG_WRT_BITS(0x4008010C, 14, 7, g_otp_cfg.aon_rco32k_cc.value);
    // xo ctune
    REG_WRT_BITS(0x40080110, 23, 18, g_otp_cfg.crystal_cal.value);
    //-----------------------------------------------------
    // CORE
    //-----------------------------------------------------
    // rf_bg_ctrim
    REG_WRT_BITS(0x42002008, 30, 27, g_otp_cfg.rf_bg_vtrim.value);
}


