#ifndef TS800X_OTP_CFG_2G4_H
#define TS800X_OTP_CFG_2G4_H
/////////////////////////////////////////////////////////////////////////////
// INC
/////////////////////////////////////////////////////////////////////////////
#include <stdint.h>
#include "system.h"

typedef struct {
    union {
        uint8_t raw;
        struct {
            uint8_t value  :4;
            uint8_t unused :4;
        };
    } rf_bg_vtrim;                  // 0x1F803F30
    uint8_t  rf_bg_volt_offset;     // 0x1F803F31
    union {
        uint8_t raw;
        struct {
            uint8_t value   :4;
            uint8_t unused  :4;
        };
    } aon_rg_bg_vref_trim;          // 0x1F803F32
    uint8_t  aon_bg_volt_offset;    // 0x1F803F33
    union {
        uint16_t raw;
        struct {
            uint8_t value;
            uint8_t unused;
        };
    } aon_rco24m_cc;                // 0x1F803F34
    union {
        uint16_t raw;
        struct {
            uint8_t value;
            uint8_t unused;
        };
    } aon_rco32k_cc;                // 0x1F803F36
    union {
        uint16_t raw;
        struct {
            uint16_t value :10;
            uint8_t unused :6;
        };
    } corner_tgt_freq;              // 0x1F803F38
    uint16_t unused0;               // 0x1F803F3A

    union {
        uint32_t raw;               // 0x1F803F3C
        struct {
            uint16_t voltage  :12;
            uint8_t unused0 :4;
            uint16_t sample :10;
            uint8_t unused1 :6;
        };
    } adc_high_div1;
    union {
        uint32_t raw;
        struct {
            uint16_t voltage  :12;
            uint8_t unused0 :4;
            uint16_t sample :10;
            uint8_t unused1 :6;
        };
    } adc_low_div1;               // 0x1F803F40
    union {
        uint32_t raw;
        struct {
            uint16_t voltage  :12;
            uint8_t unused0 :4;
            uint16_t sample :10;
            uint8_t unused1 :6;
        };
    } adc_high_div2;              // 0x1F803F44
    union {
        uint32_t raw;
        struct {
            uint16_t voltage  :12;
            uint8_t unused0 :4;
            uint16_t sample :10;
            uint8_t unused1 :6;
        };
    } adc_low_div2;               // 0x1F803F48
    union {
        uint32_t raw;
        struct {
            uint16_t voltage  :12;
            uint8_t unused0 :4;
            uint16_t sample :10;
            uint8_t unused1 :6;
        };
    } adc_high_div3;              // 0x1F803F4C
    union {
        uint32_t raw;
        struct {
            uint16_t voltage  :12;
            uint8_t unused0 :4;
            uint16_t sample :10;
            uint8_t unused1 :6;
        };
    } adc_low_div3;               // 0x1F803F50
    union {
        uint32_t raw;
        struct {
            uint16_t voltage  :12;
            uint8_t unused0 :4;
            uint16_t sample :10;
            uint8_t unused1 :6;
        };
    } adc_high_div4;              // 0x1F803F54
    union {
        uint32_t raw;
        struct {
            uint16_t voltage  :12;
            uint8_t unused0 :4;
            uint16_t sample :10;
            uint8_t unused1 :6;
        };
    } adc_low_div4;               // 0x1F803F58

    union {
        uint32_t raw;
        struct {
            uint32_t corner_offset_slope    :5;
            uint32_t hp_offset_slope        :5;
            uint32_t vtrim_offset           :4;
            uint32_t sx_offset              :4;
            uint32_t room_adc_value       :10;
            uint32_t unused                 :4;
        };
    } temperature;                  // 0x1F803F5C

    union {
        uint16_t raw;
        struct {
            uint16_t value  :12;
            uint16_t unused :4;
        };
    } rx_rssi_high;                 // 0x1F803F60
    union {
        uint16_t raw;
        struct {
            uint16_t value  :12;
            uint16_t unused :4;
        };
    } rx_rssi_low;                  // 0x1F803F62
    union {
        uint16_t raw;
        struct {
            uint16_t value  :12;
            uint16_t unused :4;
        };
    } tx_pwr_rx_rssi;               // 0x1F803F64
    uint16_t unused2;               // 0x1F803F66
    union {
        uint32_t raw;
        struct {
            uint32_t value  :31;
            uint8_t  unused :1;
        };
    } crystal_cal;                  // 0x1F803F68
    union {
        uint32_t raw;
        struct {
            uint32_t value  :31;
            uint8_t  unused :1;
        };
    } adc_res;                    // 0x1F803F6C
    union {
        uint32_t raw;
        struct {
            uint16_t voltage  :12;
            uint8_t unused0 :4;
            uint16_t sample :10;
            uint8_t unused1 :6;
        };
    } adc_vbat_high;              // 0x1F803F70
    union {
        uint32_t raw;
        struct {
            uint16_t voltage  :12;
            uint8_t unused0 :4;
            uint16_t sample :10;
            uint8_t unused1 :6;
        };
    } adc_vbat_low;               // 0x1F803F74
} otp_cfg_t;

/////////////////////////////////////////////////////////////////////////////
// VARIABLE
/////////////////////////////////////////////////////////////////////////////

extern otp_cfg_t g_otp_cfg;

/////////////////////////////////////////////////////////////////////////////
// FUNCTION
/////////////////////////////////////////////////////////////////////////////
// Read OTP configs to memory
void ts800x_Otp_LoadCfgInfo(void);
void ts800x_Otp_ApplyCfgInfo(void);

#endif // TS800X_OTP_CFG_2G4_H
