#ifndef _NRF_MEMORY_H
#define _NRF_MEMORY_H
#include "nrf_data.h"

void nrfmemset(BYTE * p,BYTE val,UINT32 length);
void nrfmemcopy(BYTE *dst,BYTE *src,UINT32 length);
#endif
