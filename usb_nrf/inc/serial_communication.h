#ifndef _SERIAL_COMMUNICATION_H
#define _SERIAL_COMMUNICATION_H
#include <stm32f10x.h>
#include <string.h>

/**************数据类型***********************/
#define MY_U8_TYPE      uint8_t
#define MY_U16_TYPE     uint16_t
#define MY_U32_TYPE			uint32_t
#define MY_S8_TYPE      int8_t
#define MY_S16_TYPE     int16_t
#define MY_S32_TYPE     int32_t
/*********************************************/



#define INTERFACE_COUNT  1
#define FIFO_COUNT (2*INTERFACE_COUNT)
#define FIFO_LENGTH (2048)
#define FIFO_TYPE MY_U8_TYPE 



extern void Fifo_Init(void);
extern MY_U16_TYPE PutFifoData(MY_U16_TYPE FifoNum,FIFO_TYPE *buf,MY_U16_TYPE length);
extern MY_U16_TYPE ReadFifoData(MY_U16_TYPE FifoNum,FIFO_TYPE *buf,MY_U16_TYPE length);
extern MY_U16_TYPE GetFifoBytes(MY_U16_TYPE FifoNum);


#define MAX_INS_COUNT  20
#define MAX_AUTO_INS_COUNT 20
#define HEADER_LENGTH  16
#define MAX_DATA_LENGTH  512

#define ACK 0x80
#define DATASTART      0x10
#define DATACONTINUE   0x20
#define DATAEND        0x40
#define MAXTIMEOUT     10


typedef struct
{
  FIFO_TYPE FifoBuf[FIFO_LENGTH];
  MY_U16_TYPE FifoBytes;
  FIFO_TYPE *FifoRead;
	FIFO_TYPE *FifoWrite;
	MY_U8_TYPE SyncEvent;
}MYFIFO;

typedef struct
{
	MY_U8_TYPE header[2];
	MY_U8_TYPE type;
	MY_U8_TYPE DataTag;
	MY_U16_TYPE seq;
	MY_U16_TYPE PackageDataLength;
	MY_U32_TYPE length;
	MY_U16_TYPE SrcPort;
	MY_U16_TYPE check_sum;
}PACKAGE_HEADER,*LPPACKAGE_HEADER;

typedef struct
{
		PACKAGE_HEADER package_Header;
		MY_U8_TYPE data[MAX_DATA_LENGTH];
} PACKAGE,*LPPACKAGE;  //数据包结构

typedef MY_U8_TYPE (*INSTRUCTIONFUNC)(MY_U8_TYPE Ins,MY_U8_TYPE *pData,MY_U16_TYPE length,MY_U16_TYPE SrcPort,MY_U8_TYPE ack);
typedef MY_U16_TYPE (*SENDDATAFUNC)(MY_U8_TYPE *pData,MY_U16_TYPE length);
typedef void (*SYNCFUNC)(MY_U8_TYPE val);
typedef struct 
{
	MY_U8_TYPE Instruction;
	MY_U8_TYPE nouse[3];
	INSTRUCTIONFUNC pFunc;
}REGISTERINS;
typedef struct
{
	MY_U8_TYPE Instruction;
	MY_U8_TYPE ack;
	MY_U8_TYPE InsFlag;
	MY_U8_TYPE bWait;
	MY_U16_TYPE Interval;
	MY_U16_TYPE WaitTime;
	MY_U16_TYPE SrcPort;
	INSTRUCTIONFUNC pFailedFunc;
	INSTRUCTIONFUNC pCompleteFunc;
}INSTRUCTIONINFO;

typedef struct
{
	MY_U16_TYPE PackageDataLength;
	REGISTERINS RegisterInsFunc[MAX_INS_COUNT];
	INSTRUCTIONINFO CurInstruction;
	INSTRUCTIONINFO AutoInstruction[MAX_AUTO_INS_COUNT];
	MY_U8_TYPE InsCount;
	MY_U8_TYPE AutoInsCount;
	MY_U8_TYPE hWaitEvent;
	MY_U8_TYPE hWaitFifoEvent;
	MY_U8_TYPE SendState;
	MY_U8_TYPE STimeout;
	MY_S32_TYPE WaitLength;
	MY_U16_TYPE CurrentWaitTime;
	MY_U8_TYPE RecvState;
	MY_U8_TYPE RTimeout;
	PACKAGE_HEADER RPackHeader;
	SENDDATAFUNC SendDataFunc;
	SYNCFUNC   ReadSyncFunc;
	SYNCFUNC   WriteSyncFunc;
	SYNCFUNC   SReadSyncFunc;
	SYNCFUNC   SWriteSyncFunc;
	MY_U16_TYPE RecordDataLength;
	MY_U8_TYPE data_buf[2*MAX_DATA_LENGTH];
}COMMUNICATIONINTERFACE;

enum STATE {HEADER,WAITDATA,WAITTIMEOUT,INTERVALWAIT};

extern void SendAndRecvThread(void);
extern void SendReadSyncFunc(MY_U8_TYPE val);
extern void FiFoClear(MY_U16_TYPE FifoNum);
extern void THREAD_NVIC_Configuration(void);
extern void THREAD_TIM_Configuration(void);
extern void InitCommunicationAll(void);
extern void Delay_us(MY_U16_TYPE i);
extern void SetSendDataFunc(MY_U8_TYPE num,SENDDATAFUNC pFunc);
extern void SetSyncFunc(MY_U8_TYPE num,SYNCFUNC pRFunc,SYNCFUNC pWFunc);
extern void SetSendSyncFunc(MY_U8_TYPE num,SYNCFUNC pSRFunc,SYNCFUNC pSWFunc);
extern void RecvData(MY_U8_TYPE num);
void SendPackage(MY_U8_TYPE num,MY_U8_TYPE Ins,MY_U8_TYPE *pData,MY_U32_TYPE length,MY_U16_TYPE SrcPort,MY_U8_TYPE ack);
extern MY_U16_TYPE calc_sum(LPPACKAGE pointer);
extern MY_U8_TYPE  check_sum(LPPACKAGE pointer);
extern MY_U8_TYPE CallHandleFunc(MY_U8_TYPE num,MY_U8_TYPE Ins,MY_U8_TYPE *pData,MY_U32_TYPE length,MY_U16_TYPE SrcPort,MY_U8_TYPE ack);
extern void SendDataFifo(MY_U8_TYPE num);
extern MY_U8_TYPE RegisterCallBack(MY_U8_TYPE num,MY_U8_TYPE Ins,INSTRUCTIONFUNC pFunc);
extern void SendAutoIns(MY_U8_TYPE num);
extern MY_U8_TYPE RegisterAutoIns(MY_U8_TYPE num,MY_U8_TYPE Ins,MY_U16_TYPE SrcPort,MY_U32_TYPE Interval,MY_U8_TYPE bWait,MY_U32_TYPE WaitTime,INSTRUCTIONFUNC pFailedFunc,INSTRUCTIONFUNC pCompleteFunc);
extern MY_U8_TYPE SendData(MY_U8_TYPE num,MY_U8_TYPE Ins,MY_U8_TYPE bQueue,MY_U8_TYPE *pData,MY_U32_TYPE length,MY_U16_TYPE SrcPort,MY_U8_TYPE bWait,MY_U32_TYPE WaitTime,INSTRUCTIONFUNC pFailedFunc,INSTRUCTIONFUNC pCompleteFunc,MY_U8_TYPE ack);
#endif
