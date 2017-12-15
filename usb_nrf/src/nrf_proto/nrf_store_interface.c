#include "nrf_store_interface.h"

UINT16 NrfWriteDataToStore(UINT8 *buf,UINT16 length)
{
	UINT16 i=0;
	UINT16 word_count=0;
	UINT32 address;
	UINT32 *pu=NULL;
	if(length==0)
	{
		return 0;
	}
	__set_PRIMASK(1);
	word_count=length/4+(length%4!=0);
	pu=(UINT32 *)buf;
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR); 
	FLASH_ErasePage(NRF_STORE_FLASH_ADDRESS);
	address=NRF_STORE_FLASH_ADDRESS;
	FLASH_ProgramWord(address,NRF_STORE_FLAG);
	address=address+4;
	for(i=0;i<word_count;i++)
		FLASH_ProgramWord(address+i*4,pu[i]);
	FLASH_Lock();
	__set_PRIMASK(0);
	return word_count*4;
}


UINT16 NrfReadDataFromStore(UINT8 *buf,UINT16 length)
{
	UINT32 address=NRF_STORE_FLASH_ADDRESS;
	UINT8 i=0,j=0;
	UINT32 *pu=(UINT32 *)buf;	
	UINT32 tmp=0;
	UINT16 word_count=0;
	UINT8 last_count=0;
	word_count=length/4;
	last_count=length%4;
	if(IsHaveStoreData())
	{
			address=address+4;
			for(i=0;i<word_count;i++)
				pu[i]=*(u32*)(address+i*4);
			if(last_count)
			{
				tmp=*(u32 *)(address+i*4);
				for(j=0;j<last_count;j++)
				{
					buf[i*4+j]=*((UINT8 *)&tmp+j);
				}
			}
	}
	return length;
}

BOOL IsHaveStoreData(void)
{
	UINT32 address=NRF_STORE_FLASH_ADDRESS;
	if((*(u32*)address)==(u32)NRF_STORE_FLAG)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

void ClearStroreData(void)
{
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR); 
	FLASH_ErasePage(NRF_STORE_FLASH_ADDRESS);
	FLASH_Lock();
}
