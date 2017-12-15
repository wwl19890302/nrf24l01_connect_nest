#include "fifo.h"
MYFIFO pFifo[FIFO_COUNT];
void Fifo_Init(void)
{
	 MY_U16_TYPE i=0;
	 for(i=0;i<FIFO_COUNT;i++)
	 	{
			pFifo[i].FifoBytes=0;
			pFifo[i].FifoRead=pFifo[i].FifoBuf;
			pFifo[i].FifoWrite=pFifo[i].FifoBuf;
			pFifo[i].SyncEvent=0;
		}
}

MY_U16_TYPE PutFifoData(MY_U16_TYPE FifoNum,FIFO_TYPE *buf,MY_U16_TYPE length)
{
	MY_U16_TYPE i=0;
	if(FifoNum>=FIFO_COUNT)
		return 0;
	for(i=0;i<length;i++)
		{
			if(pFifo[FifoNum].FifoWrite==pFifo[FifoNum].FifoBuf+FIFO_LENGTH)
				pFifo[FifoNum].FifoWrite=pFifo[FifoNum].FifoBuf;
			if(pFifo[FifoNum].FifoBytes>=FIFO_LENGTH)
				return i;
			*(pFifo[FifoNum].FifoWrite++)=*(buf+i);
			pFifo[FifoNum].FifoBytes++;
		}
	return length;
}
MY_U16_TYPE ReadFifoData(MY_U16_TYPE FifoNum,FIFO_TYPE *buf,MY_U16_TYPE length)
{
	MY_U16_TYPE i=0;
	if(FifoNum>=FIFO_COUNT)
		return 0;
	 NVIC->ICER[USB_LP_CAN1_RX0_IRQn >> 0x05] =
      (uint32_t)0x01 << (USB_LP_CAN1_RX0_IRQn & (uint8_t)0x1F);
	for(i=0;i<length;i++)
	{
		if(pFifo[FifoNum].FifoRead>=pFifo[FifoNum].FifoBuf+FIFO_LENGTH)
				pFifo[FifoNum].FifoRead=pFifo[FifoNum].FifoBuf;
		if(pFifo[FifoNum].FifoBytes==0)
				return i;
			*(buf+i)=*(pFifo[FifoNum].FifoRead++);
			pFifo[FifoNum].FifoBytes--;
	}
	 NVIC->ISER[USB_LP_CAN1_RX0_IRQn >> 0x05] =
      (uint32_t)0x01 << (USB_LP_CAN1_RX0_IRQn & (uint8_t)0x1F);
	return length;
}

MY_U16_TYPE GetFifoBytes(MY_U16_TYPE FifoNum)
{
	return 	 pFifo[FifoNum].FifoBytes;
}
