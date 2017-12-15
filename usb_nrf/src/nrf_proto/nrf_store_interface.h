#ifndef _NRF_STORE_INTERFACE
#define _NRF_STORE_INTERFACE
#include "nrf_data.h"
#define NRF_STORE_FLAG  0x11224455
#define NRF_STORE_FLASH_ADDRESS 0x0801FC00

BOOL IsHaveStoreData(void);
void ClearStroreData(void);
UINT16 NrfReadDataFromStore(UINT8 *buf,UINT16 length);
UINT16 NrfWriteDataToStore(UINT8 *buf,UINT16 length);
#endif
