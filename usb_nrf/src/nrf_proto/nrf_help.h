#ifndef _NRF_HELP_H
#define _NRF_HELP_H
#include "nrf_data.h"
void copyMac(UINT8 *dstMac,const UINT8 *srcMac);
BOOL cmpMac(const UINT8 *dstMac,const UINT8 *srcMac);
#endif
