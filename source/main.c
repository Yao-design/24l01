/******************************************************************************
 * @file:   main.c
 * @author: Techstorm Semi Application Team
 * @brief:  main source file - nRF24L01 compatibility debug version
 ******************************************************************************/

/******************************************************************************
 Include
 ******************************************************************************/
#include "cfg_def_app.h"
#include "ts800x.h"
#include "log.h"
#include "ts800x_single_2g4.h"
#include "ts800x_sleep_2g4.h"
#include "ts800x_ll_gpio.h"

/******************************************************************************
 Quote External Function Declaration
 ******************************************************************************/
void SystemInit(void);

/******************************************************************************
 Local Macro Definition
 ******************************************************************************/
#define NRF24L01_PAYLOAD_LEN   32

/******************************************************************************
 Local Variables
 ******************************************************************************/
extern RF_2G4_PARAM_ts rf_2g4_mgr;
static uint16_t freq;

uint8_t tx_buf[NRF24L01_PAYLOAD_LEN];
uint8_t rx_buf[NRF24L01_PAYLOAD_LEN];

volatile uint8_t rf_rx_busy = 0;
volatile uint8_t rf_tx_busy = 0;
volatile uint32_t sync_count = 0;
volatile uint32_t rx_count = 0;
volatile uint32_t tx_count = 0;
volatile uint32_t crc_err_count = 0;

/******************************************************************************
 Function Prototypes
 ******************************************************************************/
void ts800x_Set_High_Perf(uint8_t en);
void RF_2G4_PrepareStart(void);
void RF_2G4_UpdateDesc_TxPkt(void);
void RF_2G4_UpdateDesc_RxPkt(void);
void delay_us(uint32_t);
void delay_ms(uint32_t);

/******************************************************************************
 * Callbacks
 ******************************************************************************/
__RAM_CODE_SECTION
void ts2G4_Sync_End_It()
{
    sync_count++;
    LOG_I("[SYNC] sync_count=%d", sync_count);
}

__RAM_CODE_SECTION
void ts2G4_Rx_End_It(uint8_t is_crc_err)
{
    if(!is_crc_err)
    {
        rx_count++;
        LOG_I("[RX_OK] rx_count=%d", rx_count);
        LOG_I("[RX] len=%d", rf_2g4_mgr.fifoLen);
        LOG_HEX(rx_buf, 32);
    }
    else
    {
        crc_err_count++;
    }

    rf_rx_busy = 0;
}

__RAM_CODE_SECTION
void ts2G4_Tx_End_It(void)
{
    tx_count++;
    LOG_I("[TX_DONE] tx_count=%d", tx_count);
    
    rf_2g4_mgr.autoLen_preSize = 3;
    rf_2g4_mgr.autoLen_size = 6;
    rf_2g4_mgr.fifoLen = NRF24L01_PAYLOAD_LEN;
    
    rf_tx_busy = 0;
}

__RAM_CODE_SECTION
uint8_t ts2G4_Sleep_Wakeup_worker(uint32_t wkup_type)
{
    return 1;
}

/******************************************************************************
 * nRF24L01格式发送函数
 ******************************************************************************/
void nrf24l01_send(uint8_t *payload, uint8_t payload_len)
{
    static uint8_t pid = 0;
    uint8_t no_ack = 0;

    memset(tx_buf, 0, NRF24L01_PAYLOAD_LEN);

    uint8_t bits[256];
    int bit_idx = 0;

    // PCF: Length(6) + PID(2) + NO_ACK(1)
    for (int i = 0; i < 6; i++) {
        bits[bit_idx++] = (payload_len >> i) & 1;
    }
    for (int i = 0; i < 2; i++) {
        bits[bit_idx++] = (pid >> i) & 1;
    }
    bits[bit_idx++] = no_ack & 1;

    // Payload
    for (int i = 0; i < payload_len; i++) {
        for (int j = 0; j < 8; j++) {
            bits[bit_idx++] = (payload[i] >> j) & 1;
        }
    }

    int byte_len = (bit_idx + 7) / 8;
    for (int i = 0; i < byte_len; i++) {
        tx_buf[i] = 0;
        for (int j = 0; j < 8; j++) {
            int idx = i * 8 + j;
            if (idx < bit_idx) {
                tx_buf[i] |= (bits[idx] << j);
            }
        }
    }

    LOG_I("[TX] PCF: Len=%d, PID=%d, NO_ACK=%d, Bytes=%d",
          payload_len, pid, no_ack, byte_len);
    LOG_HEX(tx_buf, byte_len);

    rf_2g4_mgr.autoLen_preSize = 0;
    rf_2g4_mgr.autoLen_size = 0;
    rf_2g4_mgr.fifoLen = byte_len;

    rf_tx_busy = 1;
    ts2G4_Single_TxData(tx_buf, byte_len, freq, 38);

    pid = (pid + 1) & 0x03;
}

/******************************************************************************
 * 测试发送函数
 ******************************************************************************/
void test_send(void)
{
    uint8_t payload[32];
    memset(payload, 0, 32);
    payload[0] = 0x01;
    payload[1] = 0x02;
    payload[2] = 0x03;
    payload[3] = 0x04;
    payload[4] = 0x05;
    payload[5] = 0x06;
    payload[6] = 0x07;
    payload[7] = 0x08;
    
    LOG_I("[TX] Test send:");
    nrf24l01_send(payload, 8);
}

/******************************************************************************
 * Main Function
 ******************************************************************************/
int main(void)
{
    SystemInit();
    delay_ms(2000);
    ts800x_Rf_Init();

    LOG_I("========================================");
    LOG_I("nRF24L01 Debug Version");
    LOG_I("========================================");
    
    RF_2G4_PARAM_ts tmp_rf_cfg = {
        .channel = 88,
        .phyMode = RF_2G4_RATE_1M,
        .preambleLen = 2,
        .accessCode = {0xC3, 0x3C, 0x5A, 0xA5},
        .accessLen = 4,
        .whiteEn = 0,
        .whiteInit = 0,
        .crcEn = 0,           // 开启CRC
        .crcLen = 2,          // 16位CRC
        .crcInit = 0xFFFF,    // 标准初始值
        .crcPoly = 0x1021,     // 标准CRC-16多项式
        .autoLen_preSize = 3,
        .autoLen_size = 6,
        .autoLen_headerSign = 0,
        .autoLen_extSize = 0,
        .autoLen_ext = 0,
        .autoLen_maxAllow_rxBits = 32 * 8,
        .fifoOffset = 0,
        .fifoLen = NRF24L01_PAYLOAD_LEN,
        .rxTimeout = 30 * 1000,
        .autoAck_resendEn = 0,
        .autoAck_rxTimeout = 500,
        .autoAck_resendDelay = 250,
        .autoAck_txDelay = 0,
        .autoAck_resendLimit = 0,
    };

    LOG_I("[CFG] channel=%d (freq=%dMHz)", tmp_rf_cfg.channel, 2364 + tmp_rf_cfg.channel);
    LOG_I("[CFG] accessCode=%02X %02X %02X %02X", 
          tmp_rf_cfg.accessCode[0], tmp_rf_cfg.accessCode[1],
          tmp_rf_cfg.accessCode[2], tmp_rf_cfg.accessCode[3]);
    
    ts2G4_Single_Init(&tmp_rf_cfg);
    
    ts2G4_Single_Set_DataFormat(1);   
    
    freq = 88;

    RF_2G4_PrepareStart();
    
    uint8_t last_prmble = 0x96;
    ts2G4_Set_Preamble_N_Last(&last_prmble, 1);
    
    RF_2G4_UpdateDesc_TxPkt();
    RF_2G4_UpdateDesc_RxPkt();
    
    ts800x_Rf_Set_TxPower(0);
    
    LOG_I("========================================");
    LOG_I("RF Init Complete!");
    LOG_I("========================================");
    
    
    
    // TX暂时关闭
    // delay_ms(1000);
    // LOG_I("[TX] First test send after init...");
    // test_send();
    
    // 直接进入RX模式
    rf_rx_busy = 1;
    ts2G4_Single_RxData(rx_buf, NRF24L01_PAYLOAD_LEN, freq, 38);
    
    uint32_t tx_timer = 0;
    
    while (1)
    {
        // TX定时发送暂时关闭
        /*
        tx_timer++;
        if(tx_timer >= 500000)
        {
            tx_timer = 0;
            if(!rf_rx_busy && !rf_tx_busy)
            {
                test_send();
            }
        }
        */
        
        // RX完成，继续接收
        if(!rf_rx_busy && !rf_tx_busy)
        {
            rf_rx_busy = 1;
            ts2G4_Single_RxData(rx_buf, NRF24L01_PAYLOAD_LEN, freq, 38);
        }
    }
}

/*******************************************************************************
 End of File
 ******************************************************************************/