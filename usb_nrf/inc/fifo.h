#ifndef _FIFO_H
#include "stm32f10x.h"
#define _FIFO_H

/**************数据类型***********************/
#define MY_U8_TYPE      u8
#define MY_U16_TYPE     u16
#define MY_U32_TYPE			u32
#define MY_S8_TYPE      s8
#define MY_S16_TYPE     s16
#define MY_S32_TYPE     s32
/*********************************************/



#define INTERFACE_COUNT  1
#define FIFO_COUNT (2*INTERFACE_COUNT)
#define FIFO_LENGTH (2048)
#define FIFO_TYPE MY_U8_TYPE 
typedef struct
{
  FIFO_TYPE FifoBuf[FIFO_LENGTH];
  MY_U16_TYPE FifoBytes;
  FIFO_TYPE *FifoRead;
	FIFO_TYPE *FifoWrite;
	MY_U8_TYPE SyncEvent;
}MYFIFO;


extern void Fifo_Init(void);
extern MY_U16_TYPE PutFifoData(MY_U16_TYPE FifoNum,FIFO_TYPE *buf,MY_U16_TYPE length);
extern MY_U16_TYPE ReadFifoData(MY_U16_TYPE FifoNum,FIFO_TYPE *buf,MY_U16_TYPE length);
extern MY_U16_TYPE GetFifoBytes(MY_U16_TYPE FifoNum);
#endif
