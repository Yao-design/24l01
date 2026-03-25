#ifndef __TS800X_HAL_RF_H__
#define __TS800X_HAL_RF_H__
#include <stdint.h>

void ts800x_Rf_Init(void);
uint8_t ts800x_Rf_Sync_Is_Ok(void);
uint8_t ts800x_Rf_Crc_Is_Error(void);

void ts800x_Rf_Set_TxPower(int dbm);
int ts800x_Rf_Get_Rssi(void);

void ts800x_Start_Carrier_Mode(uint8_t ch);
void ts800x_Stop_Carrier_Mode(void);

#endif // __TS800X_HAL_RF_H__