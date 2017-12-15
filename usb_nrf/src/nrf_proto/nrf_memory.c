#include "nrf_memory.h"

void nrfmemset(BYTE * p,BYTE val,UINT32 length)
{
	UINT32 i=0;
	for(i=0;i<length;i++)
	{
		p[i]=val;
	}
}

void nrfmemcopy(BYTE *dst,BYTE *src,UINT32 length)
{
	UINT32 i=0;
	for(i=0;i<length;i++)
	{
		dst[i]=src[i];
	}
}
