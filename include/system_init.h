#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ts800x_config.h"

#ifdef TS_BLE_V0
#define ROM_HOOK_INDEX_DECLAR(_x) ROM_HOOK_##_x##_INDEX
#define ROM_HOOK_ITEM_FUNCTION(_x) _x##_Hook

enum ROM_HOOK_TABLE_INDEX
{
    /* ts800x_radio.c */
    ROM_HOOK_INDEX_DECLAR(RADIO_CommonDescInit_SetFreq),
    ROM_HOOK_INDEX_DECLAR(RADIO_StartDmaWithCheck),

    /* ts800x_bt.c */
    ROM_HOOK_INDEX_DECLAR(RF_BT_EventPrepare),
    ROM_HOOK_INDEX_DECLAR(RF_BT_SendWork),
    ROM_HOOK_INDEX_DECLAR(RF_BT_ReceiveWork),

    /* hci_driver.c */
    ROM_HOOK_INDEX_DECLAR(hci_driver_send_to_upstack),
    ROM_HOOK_INDEX_DECLAR(hci_driver_node_rx_process),

    /* hci.c */
    ROM_HOOK_INDEX_DECLAR(link_control_cmd_handle),
    ROM_HOOK_INDEX_DECLAR(ctrl_bb_cmd_handle),
    ROM_HOOK_INDEX_DECLAR(info_cmd_handle),
    ROM_HOOK_INDEX_DECLAR(status_cmd_handle),
    ROM_HOOK_INDEX_DECLAR(controller_cmd_handle),
    ROM_HOOK_INDEX_DECLAR(hci_vendor_cmd_handle),
    ROM_HOOK_INDEX_DECLAR(hci_cmd_handle),
    ROM_HOOK_INDEX_DECLAR(hci_acl_handle),
    ROM_HOOK_INDEX_DECLAR(hci_acl_encode),
    ROM_HOOK_INDEX_DECLAR(hci_evt_encode),
    ROM_HOOK_INDEX_DECLAR(hci_msg_polling_work),
    ROM_HOOK_INDEX_DECLAR(hci_init),

    /* ll_adv.c */
    ROM_HOOK_INDEX_DECLAR(ll_adv_enable),
    ROM_HOOK_INDEX_DECLAR(isr_adv_send_timer_cb),
    ROM_HOOK_INDEX_DECLAR(ll_adv_sched),

    /* ll_sch.c */
    ROM_HOOK_INDEX_DECLAR(ll_sch_prepare),
    ROM_HOOK_INDEX_DECLAR(ll_sch_release_work),

    /* ll_conn.c */
    ROM_HOOK_INDEX_DECLAR(ll_conn_reset),
    ROM_HOOK_INDEX_DECLAR(ll_conn_alloc),
    ROM_HOOK_INDEX_DECLAR(ll_conn_tx_enqueue),
    ROM_HOOK_INDEX_DECLAR(ll_tx_acl),
    ROM_HOOK_INDEX_DECLAR(ll_process_tx_acl_buf),
    ROM_HOOK_INDEX_DECLAR(ll_conn_process_tx_acl),
    ROM_HOOK_INDEX_DECLAR(ll_conn_tx_acl),
    ROM_HOOK_INDEX_DECLAR(ll_parse_ctrl),
    ROM_HOOK_INDEX_DECLAR(ll_conn_process_rx_pdu),
    ROM_HOOK_INDEX_DECLAR(ll_conn_polling_work),
    ROM_HOOK_INDEX_DECLAR(isr_conn_tx_process),
    ROM_HOOK_INDEX_DECLAR(isr_conn_check_seq),
    ROM_HOOK_INDEX_DECLAR(isr_conn_rxok),
    ROM_HOOK_INDEX_DECLAR(isr_conn_slave_start_timer_cb),
    ROM_HOOK_INDEX_DECLAR(isr_update_next_hop),
    ROM_HOOK_INDEX_DECLAR(isr_conn_superto),
    ROM_HOOK_INDEX_DECLAR(isr_conn_update_param),
    ROM_HOOK_INDEX_DECLAR(ll_conn_sched),

    /* ll.c */
    ROM_HOOK_INDEX_DECLAR(ll_init),
    ROM_HOOK_INDEX_DECLAR(ll_reset),
    ROM_HOOK_INDEX_DECLAR(ll_node_rx_process),
    ROM_HOOK_INDEX_DECLAR(ll_msg_common_send),
    ROM_HOOK_INDEX_DECLAR(ll_polling_work),
    ROM_HOOK_INDEX_DECLAR(ll_get_next_timestamp),

    /* att.c */
    ROM_HOOK_INDEX_DECLAR(att_send),
    ROM_HOOK_INDEX_DECLAR(att_handle_rsp),
    ROM_HOOK_INDEX_DECLAR(bt_att_recv),

    /* conn.c */
    ROM_HOOK_INDEX_DECLAR(bt_conn_connected),
    ROM_HOOK_INDEX_DECLAR(bt_conn_discconnected),
    ROM_HOOK_INDEX_DECLAR(send_acl),
    ROM_HOOK_INDEX_DECLAR(bt_conn_recv),
    ROM_HOOK_INDEX_DECLAR(bt_conn_set_state),
    ROM_HOOK_INDEX_DECLAR(bt_conn_process_tx),

    /* hci_core.c */
    ROM_HOOK_INDEX_DECLAR(bt_hci_cmd_send_sync),
    ROM_HOOK_INDEX_DECLAR(hci_acl),
    ROM_HOOK_INDEX_DECLAR(hci_event),
    ROM_HOOK_INDEX_DECLAR(bt_send),
    ROM_HOOK_INDEX_DECLAR(bt_recv),

    /* l2cap.c */
    ROM_HOOK_INDEX_DECLAR(le_sig_recv),
    ROM_HOOK_INDEX_DECLAR(bt_l2cap_recv),

    ROM_HOOK_TABLE_END,
};

#ifdef TS_HAS_SCH
#define TS_CTRL_ARG_FLAG   ((void*)0x10203040)
// #define TS_HOST_ARG_FLAG   ((void*)0x50607080)
#endif

extern void* rom_hook_table[ROM_HOOK_TABLE_END];

void rom_hook_register(int index, void* func);
#endif

#ifdef TS_BLE_V0_2_0
#include <stdbool.h>

typedef struct RadioCtrlDesc RadioCtrlDesc_t;
extern void (*RADIO_CommonDescInit_SetFreq_ptr)(RadioCtrlDesc_t* desc_ptr, uint16_t freq);

#ifdef CONFIG_BT_SMP
struct bt_conn;
struct bt_keys;
typedef struct simple_buf simple_buf_t;
typedef struct bt_addr_le bt_addr_le_t;

#ifdef CONFIG_BT_ECC
extern int  (*bt_smp_init_ptr)(void);
#endif
extern void (*bt_storage_kv_flash_init_ptr)(void);
extern int  (*bt_smp_recv_ptr)(struct bt_conn *conn, simple_buf_t *buf);
extern void (*bt_smp_connected_ptr)(struct bt_conn *conn);
extern void (*bt_smp_disconnected_ptr)(struct bt_conn *conn);
extern void (*bt_smp_encrypt_change_ptr)(struct bt_conn *conn, uint8_t hci_status);
extern int  (*bt_smp_start_security_ptr)(struct bt_conn *conn);
extern void (*bt_smp_update_keys_ptr)(struct bt_conn *conn);
extern bool (*bt_smp_request_ltk_ptr)(struct bt_conn *conn, uint64_t rand, uint16_t ediv, uint8_t *ltk);
extern struct bt_keys * (*bt_keys_find_addr_ptr)(uint8_t id, const bt_addr_le_t *addr);
#endif

#define ROM_PATCH_HOOK_CALL(_func_declar, _func, _argv)
#endif

#if defined(TS_BLE_V0) || defined(TS_BLE_V0_2_0)
// int xrand(void);
// void xsrand(unsigned int seed);
#endif

#define FLASH_DELAY_TM_US  500

#define __FLASH_CODE_SECTION            __attribute__((noinline, section(".flash_code_section")))
#define __RAM_CODE_SECTION              __attribute__((noinline, section(".ram_code_section")))
#define __RAM_CODE_SECTION_FORCE        __attribute__((noinline, section(".ram_code_section")))
#define __FLASH_RAM_CODE_SECTION        __attribute__((noinline, section(".flash_ram_code_section")))
#define __RAM_VAR_SECTION               __attribute__((section(".ram_code_section")))
#define __RAM_SLEEP_LOST_SECTION        __attribute__((section(".ram_segger_rtt_section")))

typedef void(*VECTOR_TABLE_Type)(void);

#define REG_SYS_RESET (8)
#define RESET_UART(idx) \
do \
{ \
    *(volatile uint32_t *)(SYS_CTRL_BASE + REG_SYS_RESET) = (1 << (10 + idx));  \
    for(volatile int i = 0; i < 10; i++); \
    *(volatile uint32_t *)(SYS_CTRL_BASE + REG_SYS_RESET) = 0;  \
} while(0)

#define WTD_SET_ADDR  0x100
#define FEED_WTD(val)         (*(volatile uint32_t *)(SYS_CTRL_BASE + WTD_SET_ADDR) = ((32 - val) << 3))

// #define TS_MEMCPY(dst, src, len) ({uint8_t * pdst = (uint8_t *)(dst); const uint8_t * psrc = (const uint8_t *)(src);for(int i = 0; i < len; i++){pdst[i] = psrc[i];}})

#define REG_RD(addr)        (*(volatile uint32_t *)(addr))
#define REG_RD_BYTE(addr)   (*(volatile uint8_t *)(addr))
#define REG_WRT(addr, val)  (*(volatile uint32_t *)(addr) = (val))
#define REG_WRT_BYTE(addr, val)  (*(volatile uint8_t *)(addr) = (val))
#define REG_WRT_OR(addr, val)   (*(volatile uint32_t *)(addr) |= (val))

#define REG_CLR_BIT(addr, pos)  (*(volatile uint32_t *)(addr) &= (~(1 << pos)))
#define REG_SET_BIT(addr, pos)  (*(volatile uint32_t *)(addr) |= (1 << pos))

#define REG_RD_BITS(addr, msb, lsb) \
  (((*(volatile uint32_t *)(addr)) & ((1 << (msb - lsb + 1)) - 1) << lsb) >> lsb)

#define REG_WRT_BITS(addr, msb, lsb, val)  \
do \
{ \
    uint32_t  msk =  (((1 << (msb - lsb + 1)) - 1) << lsb);  \
    uint32_t  tmp =  *(volatile uint32_t *)(addr); \
    tmp &= (~msk); \
    tmp |= ((val) << lsb); \
    *(volatile uint32_t *)(addr) = tmp;  \
} while (0)

#define REG_WRT_MSK(addr, msk, val) \
do \
{ \
    uint32_t  reg_val = REG_RD(addr); \
    reg_val &= (~(msk)); \
    reg_val |= (val);    \
    REG_WRT(addr, reg_val);  \
} while (0)

#define GET_CUR_TICK_US()       (REG_RD(0x42000104))

uint32_t system_Enter_Critical(void);
void system_Exit_Critical(uint32_t prev_stat);

#ifdef __cplusplus
}
#endif

#endif
