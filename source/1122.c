/******************************************************************************
 * @file:   main.c
 * @author: Techstorm Semi Application Team
 * @brief:  main source file - nRF24L01 RX validation
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
#define NRF24L01_RF_CHANNEL    88
#define NRF24L01_WHITEN_VAL    38

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
static void nrf24l01_restore_rx_autolen(void);
static void nrf24l01_start_rx(void);
static void log_nrf24l01_rx_packet(void);
static void decode_nrf24l01_payload_from_rx(const uint8_t *rx, int rx_bytes);
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
        log_nrf24l01_rx_packet();
        
        // // 调用三层扫描解析函数
        // decode_nrf24l01_payload_from_rx(rx_buf, 32);
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
    
    // 恢复autoLen配置用于接收
    nrf24l01_restore_rx_autolen();
    
    rf_tx_busy = 0;
}

__RAM_CODE_SECTION
uint8_t ts2G4_Sleep_Wakeup_worker(uint32_t wkup_type)
{
    return 1;
}

static void nrf24l01_restore_rx_autolen(void)
{
    rf_2g4_mgr.autoLen_preSize = 0;
    rf_2g4_mgr.autoLen_size = 6;
    rf_2g4_mgr.autoLen_headerSign = 0;
    rf_2g4_mgr.autoLen_extSize = 3;
    rf_2g4_mgr.autoLen_ext = 0;
    rf_2g4_mgr.autoLen_maxAllow_rxBits = NRF24L01_PAYLOAD_LEN * 8;
    rf_2g4_mgr.fifoLen = NRF24L01_PAYLOAD_LEN;
}

static void nrf24l01_start_rx(void)
{
    nrf24l01_restore_rx_autolen();
    rf_rx_busy = 1;
    ts2G4_Single_RxData(rx_buf, NRF24L01_PAYLOAD_LEN, freq, NRF24L01_WHITEN_VAL);
}

static void log_nrf24l01_rx_packet(void)
{
    uint8_t payload_len = (uint8_t)rf_2g4_mgr.fifoLen;
    uint8_t pcf_ext = rf_2g4_mgr.autoLen_ext & 0x07;
    uint8_t pid = pcf_ext & 0x03;
    uint8_t no_ack = (pcf_ext >> 2) & 0x01;

    LOG_I("[RX] len=%d pid=%d no_ack=%d ext=0x%02X",
          payload_len, pid, no_ack, pcf_ext);
    LOG_I("[RX_RAW] dump_len=%d", NRF24L01_PAYLOAD_LEN);
    LOG_HEX(rx_buf, NRF24L01_PAYLOAD_LEN);
    decode_nrf24l01_payload_from_rx(rx_buf, NRF24L01_PAYLOAD_LEN);
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

    // 禁用autoLen，因为我们手动构造了PCF
    rf_2g4_mgr.autoLen_preSize = 0;
    rf_2g4_mgr.autoLen_size = 0;
    rf_2g4_mgr.fifoLen = byte_len;

    rf_tx_busy = 1;
    ts2G4_Single_TxData(tx_buf, byte_len, freq, 38);

    pid = (pid + 1) & 0x03;
}
/******************************************************************************
 * 三层扫描解析 nRF24L01 数据包
 * 第一层：两种 bit 展开方式 (MSB-first 和 LSB-first)
 * 第二层：扫描所有可能的 PCF 起点
 * 第三层：验证 payload 是否匹配已知值 01 02 03 04
 ******************************************************************************/
static void decode_nrf24l01_payload_from_rx(const uint8_t *rx, int rx_bytes)
{
    // 已知的期望 payload
    const uint8_t expected_payload[4] = {0x01, 0x02, 0x03, 0x04};
    const int expected_len = 4;
    
    // 两种 bit 流
    uint8_t bits_M[512];  // MSB-first: 每字节 bit7->bit0
    uint8_t bits_L[512];  // LSB-first: 每字节 bit0->bit7
    int bit_cnt_M = 0;
    int bit_cnt_L = 0;
    
    // ===== 第一层：生成两种 bit 流 =====
    for (int i = 0; i < rx_bytes; i++)
    {
        // 方案 M：MSB-first (bit7 -> bit0)
        for (int b = 7; b >= 0; b--)
        {
            bits_M[bit_cnt_M++] = (rx[i] >> b) & 0x01;
        }
        // 方案 L：LSB-first (bit0 -> bit7)
        for (int b = 0; b < 8; b++)
        {
            bits_L[bit_cnt_L++] = (rx[i] >> b) & 0x01;
        }
    }
    
    LOG_I("========== THREE-LAYER SCAN START ==========");
    
    int found = 0;
    
    // 遍历两种 bit 流方案
    const char *mode_names[2] = {"MSB-first", "LSB-first"};
    uint8_t *bit_streams[2] = {bits_M, bits_L};
    int bit_counts[2] = {bit_cnt_M, bit_cnt_L};
    
    for (int mode = 0; mode < 2; mode++)
    {
        uint8_t *bits = bit_streams[mode];
        int bit_cnt = bit_counts[mode];
        
        // ===== 第二层：扫描所有可能的 PCF 起点 =====
        // PCF = 9 bits (Length:6 + PID:2 + NO_ACK:1)
        // 扫描范围：前 32 bits 内所有可能的起点
        for (int pcf_start = 0; pcf_start <= 32 && pcf_start + 9 <= bit_cnt; pcf_start++)
        {
            // 解析 PCF (按 nRF24L01 LSB-first 格式)
            uint8_t len = 0;
            for (int i = 0; i < 6; i++)
            {
                len |= (bits[pcf_start + i] << i);  // LSB-first
            }
            uint8_t pid = 0;
            for (int i = 0; i < 2; i++)
            {
                pid |= (bits[pcf_start + 6 + i] << i);
            }
            uint8_t no_ack = bits[pcf_start + 8];
            
            // 只关注 len == 4 的候选
            if (len != expected_len)
            {
                continue;
            }
            
            // ===== 第三层：提取并验证 payload =====
            int payload_start = pcf_start + 9;  // PCF 之后
            
            if (payload_start + expected_len * 8 > bit_cnt)
            {
                continue;
            }
            
            // 提取 4 字节 payload (两种字节序)
            uint8_t payload_msb[4] = {0};  // payload 按 MSB 组装
            uint8_t payload_lsb[4] = {0};  // payload 按 LSB 组装
            
            for (int i = 0; i < expected_len; i++)
            {
                for (int b = 0; b < 8; b++)
                {
                    int idx = payload_start + i * 8 + b;
                    payload_msb[i] |= (bits[idx] << (7 - b));  // MSB-first
                    payload_lsb[i] |= (bits[idx] << b);        // LSB-first
                }
            }
            
            // 检查是否匹配期望值
            int match_msb = (memcmp(payload_msb, expected_payload, expected_len) == 0);
            int match_lsb = (memcmp(payload_lsb, expected_payload, expected_len) == 0);
            
            if (match_msb || match_lsb)
            {
                found = 1;
                LOG_I("");
                LOG_I("========== FOUND! ==========");
                LOG_I("[MODE] %s", mode_names[mode]);
                LOG_I("[PCF]  start=%d, len=%d, pid=%d, no_ack=%d", pcf_start, len, pid, no_ack);
                LOG_I("[PAYLOAD-MSB] %02X %02X %02X %02X %s", 
                      payload_msb[0], payload_msb[1], payload_msb[2], payload_msb[3],
                      match_msb ? "<== MATCH!" : "");
                LOG_I("[PAYLOAD-LSB] %02X %02X %02X %02X %s", 
                      payload_lsb[0], payload_lsb[1], payload_lsb[2], payload_lsb[3],
                      match_lsb ? "<== MATCH!" : "");
                LOG_I("============================");
            }
        }
    }
    
    if (!found)
    {
        LOG_I("[SCAN] No match found for expected payload 01 02 03 04");
        LOG_I("[SCAN] Raw RX data (first 16 bytes):");
        LOG_HEX(rx, 16);
        
        // 额外打印：显示所有 len=4 的候选供调试
        LOG_I("[DEBUG] All candidates with len=4:");
        for (int mode = 0; mode < 2; mode++)
        {
            uint8_t *bits = bit_streams[mode];
            int bit_cnt = bit_counts[mode];
            
            for (int pcf_start = 0; pcf_start <= 32 && pcf_start + 9 <= bit_cnt; pcf_start++)
            {
                uint8_t len = 0;
                for (int i = 0; i < 6; i++)
                {
                    len |= (bits[pcf_start + i] << i);
                }
                
                if (len == expected_len)
                {
                    uint8_t pid = 0;
                    for (int i = 0; i < 2; i++)
                    {
                        pid |= (bits[pcf_start + 6 + i] << i);
                    }
                    uint8_t no_ack = bits[pcf_start + 8];
                    
                    int payload_start = pcf_start + 9;
                    uint8_t payload_lsb[4] = {0};
                    for (int i = 0; i < expected_len; i++)
                    {
                        for (int b = 0; b < 8; b++)
                        {
                            int idx = payload_start + i * 8 + b;
                            if (idx < bit_cnt)
                            {
                                payload_lsb[i] |= (bits[idx] << b);
                            }
                        }
                    }
                    
                    LOG_I("  [%s] pcf_start=%d pid=%d no_ack=%d payload=%02X %02X %02X %02X",
                          mode_names[mode], pcf_start, pid, no_ack,
                          payload_lsb[0], payload_lsb[1], payload_lsb[2], payload_lsb[3]);
                }
            }
        }
    }
    
    LOG_I("========== THREE-LAYER SCAN END ==========");
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
    LOG_I("nRF24L01 RX Validation");
    LOG_I("TS8001 works as RX only, Nordic side should transmit with matching settings");
    LOG_I("========================================");
    
    RF_2G4_PARAM_ts tmp_rf_cfg = {
        .channel = NRF24L01_RF_CHANNEL,
        .phyMode = RF_2G4_RATE_1M,
        .preambleLen = 1,
        .accessCode = {0xC3, 0x3C, 0x5A, 0xA5},
        .accessLen = 4,
        .whiteEn = 0,
        .whiteInit = 0,
        .crcEn = 0,
        .crcLen = 2,
        .crcInit = 0xFFFF,
        .crcPoly = 0x1021,
        .autoLen_preSize = 0,
        .autoLen_size = 6,
        .autoLen_headerSign = 0,    
        .autoLen_extSize = 3,
        .autoLen_ext = 0,
        .autoLen_maxAllow_rxBits = NRF24L01_PAYLOAD_LEN * 8,
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
    
    ts2G4_Single_Set_DataFormat(1);  // LSB first (0=LSB, 1=MSB) - nRF24L01 uses LSB first!
    
    freq = NRF24L01_RF_CHANNEL;

    RF_2G4_PrepareStart();
    
    uint8_t last_prmble = 0x96;
    ts2G4_Set_Preamble_N_Last(&last_prmble, 1);
    
    RF_2G4_UpdateDesc_TxPkt();
    RF_2G4_UpdateDesc_RxPkt();
    
    ts800x_Rf_Set_TxPower(0);
    
    LOG_I("========================================");
    LOG_I("RF Init Complete!");
    LOG_I("[CFG] RX channel=%d", freq);
    LOG_I("========================================");
    
    // 启用发送测试
    delay_ms(1000);
    nrf24l01_start_rx();
    uint32_t tx_timer = 0;

    // 直接进入RX模式
    while (1)
    {
        // 定时发送测试 - 每500000次循环发送一次
        tx_timer++;

        
        // RX完成，继续接收
        if(!rf_rx_busy && !rf_tx_busy)
        {
            nrf24l01_start_rx();
        }
    }
}

/*******************************************************************************
 End of File
 ******************************************************************************/
