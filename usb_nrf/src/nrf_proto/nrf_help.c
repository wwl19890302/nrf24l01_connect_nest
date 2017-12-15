#include "nrf_help.h"

void copyMac(UINT8 *dstMac,const UINT8 *srcMac)
{
	UINT8 i=0;
	for(i=0;i<MACLENGTH;i++)
	{
		dstMac[i]=srcMac[i];
	}
}

BOOL cmpMac(const UINT8 *dstMac,const UINT8 *srcMac)
{
	UINT8 i=0;
	for(i=0;i<MACLENGTH;i++)
	{
		if(dstMac[i]!=srcMac[i])
		{
			return FALSE;
		}
	}
	return TRUE;
}