#include "serial_communication.h"
#include <stdlib.h>

#define TRUE 1
#define FALSE 0

 PACKAGE my_package;
 MY_U8_TYPE buf[MAX_DATA_LENGTH+HEADER_LENGTH];
 MY_U8_TYPE SendDataFifobuf[MAX_DATA_LENGTH];
COMMUNICATIONINTERFACE interface[INTERFACE_COUNT];
MY_U8_TYPE Header[2]={0x58,0x58};
static MY_U32_TYPE SendThreadTime;
MYFIFO pFifo[FIFO_COUNT];
static MY_U16_TYPE freq_divider=0;

void Fifo_Init(void)                       //FIFO 初始化
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
void FiFoClear(MY_U16_TYPE FifoNum)        //清空FIFO
{
	if(FifoNum>=FIFO_COUNT)
		return;

		pFifo[FifoNum].FifoBytes=0;
		pFifo[FifoNum].FifoRead=pFifo[FifoNum].FifoBuf;
		pFifo[FifoNum].FifoWrite=pFifo[FifoNum].FifoBuf;
		pFifo[FifoNum].SyncEvent=0;	
		
}
MY_U16_TYPE PutFifoData(MY_U16_TYPE FifoNum,FIFO_TYPE *buf,MY_U16_TYPE length) //向FIFO中写入数据 注：在上次调用完成前，不能进行调用，否则出错
{
	MY_U16_TYPE i=0;
	MY_U8_TYPE  num=FifoNum/2;
	MY_U8_TYPE  fNum=FifoNum%2;
	if(FifoNum>=FIFO_COUNT)
		return 0;
	if(pFifo[FifoNum].FifoBytes+length>FIFO_LENGTH)
		return 0;
	if(fNum==0&&interface[num].WriteSyncFunc)
		interface[num].WriteSyncFunc(1);
		if(fNum==1&&interface[num].SWriteSyncFunc)
		interface[num].SWriteSyncFunc(1);
	for(i=0;i<length;i++)
		{
			if(pFifo[FifoNum].FifoWrite==pFifo[FifoNum].FifoBuf+FIFO_LENGTH)
				pFifo[FifoNum].FifoWrite=pFifo[FifoNum].FifoBuf;
			if(pFifo[FifoNum].FifoBytes>=FIFO_LENGTH)
				return i;
			*(pFifo[FifoNum].FifoWrite++)=*(buf+i);
			pFifo[FifoNum].FifoBytes++;
		}
	if(fNum==0&&interface[num].WriteSyncFunc)
		interface[num].WriteSyncFunc(0);
		if(fNum==1&&interface[num].SWriteSyncFunc)
		interface[num].SWriteSyncFunc(0);
	return length;
}
MY_U16_TYPE ReadFifoData(MY_U16_TYPE FifoNum,FIFO_TYPE *buf,MY_U16_TYPE length) //从FIFO中读取数据 注：在上次调用完成前，不能进行调用，否则出错
{
	MY_U16_TYPE i=0;
	MY_U8_TYPE  num=FifoNum/2;
	MY_U8_TYPE  fNum=FifoNum%2;
	if(FifoNum>=FIFO_COUNT)
		return 0;	
	if(fNum==0&&interface[num].ReadSyncFunc)
		interface[num].ReadSyncFunc(1);
	if(fNum==1&&interface[num].SReadSyncFunc)
		interface[num].SReadSyncFunc(1);
	for(i=0;i<length;i++)
	{
		if(pFifo[FifoNum].FifoRead>=pFifo[FifoNum].FifoBuf+FIFO_LENGTH)
				pFifo[FifoNum].FifoRead=pFifo[FifoNum].FifoBuf;
		if(pFifo[FifoNum].FifoBytes==0)
				return i;
			*(buf+i)=*(pFifo[FifoNum].FifoRead++);
			pFifo[FifoNum].FifoBytes--;
	}
	if(fNum==0&&interface[num].ReadSyncFunc)
		interface[num].ReadSyncFunc(0);
	if(fNum==1&&interface[num].SReadSyncFunc)
		interface[num].SReadSyncFunc(0);
	return length;
}

MY_U16_TYPE GetFifoBytes(MY_U16_TYPE FifoNum)          //获取FIFO中数据量
{
	return 	 pFifo[FifoNum].FifoBytes;
}




/**********************************************硬件相关********************************/


	void TIM3Cof(void)                  //延时定时器
	{

		TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
		RCC_ClocksTypeDef RCC_Clocks;
		 RCC_GetClocksFreq(&RCC_Clocks);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); 
		freq_divider=RCC_Clocks.PCLK1_Frequency / 1000000;
		TIM_TimeBaseStructure.TIM_Period = 1;                 
		TIM_TimeBaseStructure.TIM_Prescaler = 0;       //72M
		TIM_TimeBaseStructure.TIM_ClockDivision = 0;
		TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down; 
		TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	}

	void Delay_us(MY_U16_TYPE i)			 //1~65535us延时	  
	{
		MY_U16_TYPE TIMCounter = i*freq_divider;
		TIM_SetCounter(TIM3, TIMCounter);
		TIM_Cmd(TIM3, ENABLE);
		while (TIMCounter>1)
		{
			TIMCounter = TIM_GetCounter(TIM3);
		}
		TIM_Cmd(TIM3, DISABLE);
	}


void SendReadSyncFunc(MY_U8_TYPE val)            //同步函数 默认为关闭全部中断,也可采用屏蔽会产生冲突中断的方法
{
	//__set_PRIMASK(val);
}

	
void THREAD_NVIC_Configuration(void)            //中断初始化
	{
		NVIC_InitTypeDef NVIC_InitStructure;

		/* Enable the TIM2 gloabal Interrupt */
		NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
		
	}
void THREAD_TIM_Configuration(void)
{
		TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
		RCC_ClocksTypeDef RCC_Clocks;
		MY_U8_TYPE divider=0;
		RCC_GetClocksFreq(&RCC_Clocks);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); 
		divider=RCC_Clocks.PCLK1_Frequency / 1000000;
		TIM_TimeBaseStructure.TIM_Period = 1000/INTERFACE_COUNT-1;                 
		TIM_TimeBaseStructure.TIM_Prescaler = divider;       //72M
		TIM_TimeBaseStructure.TIM_ClockDivision = 0;
		TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; 
		TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	  /* TIM IT enable */
		TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
		/* TIM2 enable counter */
		TIM_Cmd(TIM2, ENABLE);
	  
	
}	
//????????
void TIM2_IRQHandler(void)
{
	
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
		SendAndRecvThread();
	}
}

/**********************************************************************************************/

void SendAndRecvThread(void)
{
	  MY_U8_TYPE num=0;
		SendThreadTime++;
		num=SendThreadTime%INTERFACE_COUNT;
			SendAutoIns(num);
			SendDataFifo(num);
			RecvData(num);
}

void InitCommunicationAll(void)
	{
		MY_U8_TYPE i=0;
		TIM3Cof();
		if(INTERFACE_COUNT==0)
			return;
		
		Fifo_Init();
		THREAD_NVIC_Configuration();
		THREAD_TIM_Configuration();
		for(i=0;i<INTERFACE_COUNT;i++)
		{
			memset((void *)&interface[i],0,sizeof(interface[i]));
			interface[i].PackageDataLength=MAX_DATA_LENGTH;
			interface[i].SendState=HEADER;
			interface[i].RecvState=HEADER;
			interface[i].SReadSyncFunc=SendReadSyncFunc;
			interface[i].SWriteSyncFunc=SendReadSyncFunc;
		}
		SendThreadTime=0;
	}
	
MY_U16_TYPE calc_sum(LPPACKAGE pointer)
{
	MY_U16_TYPE i=0;
	MY_U16_TYPE check_sum=0,temp=0;
	for(i=0;i<(HEADER_LENGTH-2);i++)
	{
		temp=((MY_U8_TYPE *)pointer)[i];
		temp &=0x00FF;
		check_sum +=temp;
	}
	for(i=0;i<(int)pointer->package_Header.PackageDataLength;i++)
	{
		temp=pointer->data[i];
		temp &=0x00FF;
		check_sum +=temp;
	}
	return check_sum;
}
MY_U8_TYPE  check_sum(LPPACKAGE pointer)
{
	if(pointer->package_Header.check_sum==calc_sum(pointer))
		return 1;
	else
		return 0;
}


	MY_U8_TYPE WaitForSingleObject(MY_U8_TYPE *event,MY_U16_TYPE wait_time)
	{
		MY_U16_TYPE i=0;
		MY_U16_TYPE j=0;
		for(i=0;i<wait_time&&!event[0];i++)
		{
		//	j=0xfff;
		//	while(j--);
			Delay_us(1);
		}
		return event[0];
	}

void SetSendDataFunc(MY_U8_TYPE num,SENDDATAFUNC pFunc)
{
	if(num>=INTERFACE_COUNT)
		return;
	interface[num].SendDataFunc=pFunc;
}

void SetSyncFunc(MY_U8_TYPE num,SYNCFUNC pRFunc,SYNCFUNC pWFunc)
{
	if(num>=INTERFACE_COUNT)
		return;
	interface[num].ReadSyncFunc=pRFunc;
	interface[num].WriteSyncFunc=pWFunc;
}

void SetSendSyncFunc(MY_U8_TYPE num,SYNCFUNC pSRFunc,SYNCFUNC pSWFunc)
{
	if(num>=INTERFACE_COUNT)
		return;
	interface[num].SReadSyncFunc=pSRFunc;
	interface[num].SWriteSyncFunc=pSWFunc;
}
void SendPackage(MY_U8_TYPE num,MY_U8_TYPE Ins,MY_U8_TYPE *pData,MY_U32_TYPE length,MY_U16_TYPE SrcPort,MY_U8_TYPE ack)
{
	MY_U16_TYPE lastbytes=length%interface[num].PackageDataLength;
	MY_U16_TYPE times=length/interface[num].PackageDataLength;
	MY_U16_TYPE i=0,j=0,k=0;
	if(num>=INTERFACE_COUNT)
		return;
		for(i=0;i<times;i++)
		{
			for(k=0;k<(interface[num].PackageDataLength+HEADER_LENGTH);k++)
				((MY_U8_TYPE *)&my_package)[k]=0;
			my_package.package_Header.header[0]=Header[0];
			my_package.package_Header.header[1]=Header[1];
			my_package.package_Header.SrcPort=SrcPort;
			if(ack)
				my_package.package_Header.type=Ins|ACK;
			else
				my_package.package_Header.type=Ins;
			if(i==0)
				my_package.package_Header.DataTag=DATASTART;
			else
				my_package.package_Header.DataTag=DATACONTINUE;
			my_package.package_Header.PackageDataLength=interface[num].PackageDataLength;
			if(!(lastbytes)&&i==times-1)
			{
				my_package.package_Header.DataTag=DATAEND;
			}
			my_package.package_Header.seq=i;
			my_package.package_Header.length=length;
			for(j=0;j<interface[num].PackageDataLength;j++)
			{
				my_package.data[j]=pData[j+interface[num].PackageDataLength*i];
			}
			my_package.package_Header.check_sum=0;
			my_package.package_Header.check_sum=calc_sum(&my_package);
			if(interface[num].SendDataFunc)
				interface[num].SendDataFunc((MY_U8_TYPE *)&my_package,my_package.package_Header.PackageDataLength+HEADER_LENGTH);
		}
		if(lastbytes||(length==0))
		{
			for(k=0;k<(interface[num].PackageDataLength+HEADER_LENGTH);k++)
				((MY_U8_TYPE *)&my_package)[k]=0;
			my_package.package_Header.header[0]=Header[0];
			my_package.package_Header.header[1]=Header[1];
			my_package.package_Header.SrcPort=SrcPort;
			if(ack)
				my_package.package_Header.type=Ins|ACK;
			else
				my_package.package_Header.type=Ins;
			my_package.package_Header.DataTag=DATAEND;
			my_package.package_Header.seq=i;
			my_package.package_Header.length=length;
			for(j=0;j<lastbytes;j++)
			{
				my_package.data[j]=pData[j+interface[num].PackageDataLength*i];
			}
			my_package.package_Header.PackageDataLength=lastbytes;
			my_package.package_Header.check_sum=0;
			my_package.package_Header.check_sum=calc_sum(&my_package);
			
			if(interface[num].SendDataFunc)
				interface[num].SendDataFunc((MY_U8_TYPE *)&my_package,my_package.package_Header.PackageDataLength+HEADER_LENGTH);
		}
		
}

void RecvData(MY_U8_TYPE num)
{

	MY_U8_TYPE DataTag=0,type,ack;
	MY_U8_TYPE i=0;
	MY_U8_TYPE check_flag=FALSE;
	MY_U8_TYPE bContinue=TRUE;
	LPPACKAGE package=(LPPACKAGE)buf;
	if(num>=INTERFACE_COUNT)
		return;
  memset(buf,0,MAX_DATA_LENGTH+HEADER_LENGTH);
	while(bContinue)
	{
	switch(interface[num].RecvState)
	{
		case HEADER:
	if(GetFifoBytes(2*num)>=HEADER_LENGTH)
	{
		check_flag=TRUE;

		for(i=0;i<2;i++)
		{
			ReadFifoData(2*num,buf+i,1);
			if(*(buf+i)!=Header[i])
			{
				check_flag=FALSE;
				break;
			}
		}
		if(check_flag)
		{
			ReadFifoData(2*num,buf+2,HEADER_LENGTH-2);
			if(package->package_Header.PackageDataLength<=MAX_DATA_LENGTH)
			{
				memcpy(&interface[num].RPackHeader,buf,HEADER_LENGTH);
				interface[num].RecvState=WAITDATA;
			}
		}
	}
	else
		bContinue=FALSE;
	break;
	case WAITDATA:
		if(interface[num].RTimeout++>MAXTIMEOUT)
			{
				interface[num].RTimeout=0;
				interface[num].RecvState=HEADER;
			}
			if(GetFifoBytes(2*num)>=interface[num].RPackHeader.PackageDataLength)
			{
				memcpy(buf,&interface[num].RPackHeader,HEADER_LENGTH);
				ReadFifoData(2*num,package->data,package->package_Header.PackageDataLength);
				if(check_sum(package))
					{
						DataTag =package->package_Header.DataTag;

						if(DataTag==DATASTART)
						{
							memcpy(interface[num].data_buf,package->data,package->package_Header.PackageDataLength);
							interface[num].RecordDataLength=package->package_Header.PackageDataLength;
							interface[num].RTimeout=0;
							interface[num].RecvState=HEADER;
							return;
						}
						if(DataTag==DATACONTINUE)
						{
							memcpy(interface[num].data_buf+package->package_Header.seq*interface[num].RecordDataLength,package->data,package->package_Header.PackageDataLength);
							interface[num].RTimeout=0;
							interface[num].RecvState=HEADER;
							return;
						}
						if(DataTag==DATAEND)
						{
							if(package->package_Header.seq)
								memcpy(interface[num].data_buf+package->package_Header.seq*interface[num].RecordDataLength,package->data,package->package_Header.PackageDataLength);
							else
							{
								memcpy(interface[num].data_buf,package->data,package->package_Header.PackageDataLength);
							}
							type=package->package_Header.type&0x7F;
							ack=package->package_Header.type&0x80;
						}
						if(interface[num].CurInstruction.bWait&&type==interface[num].CurInstruction.Instruction)
							interface[num].hWaitFifoEvent=1;
						CallHandleFunc(num,type,interface[num].data_buf,package->package_Header.length,package->package_Header.SrcPort,ack);
					}
					interface[num].RTimeout=0;
					interface[num].RecvState=HEADER;
		   }
				else
					bContinue=FALSE;
			 break;		
	}
 }
}

MY_U8_TYPE CallHandleFunc(MY_U8_TYPE num,MY_U8_TYPE Ins,MY_U8_TYPE *pData,MY_U32_TYPE length,MY_U16_TYPE SrcPort,MY_U8_TYPE ack)
{
	MY_U8_TYPE i=0;
	if(num>=INTERFACE_COUNT)
		return 0;
	for(i=0;i<interface[num].InsCount;i++)
	{
		if(interface[num].RegisterInsFunc[i].Instruction==Ins)
			break;
	}
	if(i<interface[num].InsCount)
	{
		interface[num].RegisterInsFunc[i].pFunc(Ins,pData,length,SrcPort,ack);
		return TRUE;
	}
	else
		return FALSE;

}

void SendDataFifo(MY_U8_TYPE num)
{
		MY_U8_TYPE bContinue=TRUE;
	if(num>=INTERFACE_COUNT)
		return;
	while(bContinue)
	{
		switch(interface[num].SendState)
		{
			case HEADER:
			if(GetFifoBytes(2*num+1)>=sizeof(INSTRUCTIONINFO)+sizeof(MY_U32_TYPE))
			{
				ReadFifoData(2*num+1,(MY_U8_TYPE *)&interface[num].CurInstruction,sizeof(INSTRUCTIONINFO));
				if(interface[num].CurInstruction.InsFlag!=(MY_U8_TYPE)0xAA)
					break;
				ReadFifoData(2*num+1,(MY_U8_TYPE *)&interface[num].WaitLength,sizeof(MY_U32_TYPE));
				if(interface[num].WaitLength>MAX_DATA_LENGTH)
					break;
				interface[num].SendState=WAITDATA;
			}
			else
				bContinue=FALSE;
			break;
			case WAITDATA:
				if(interface[num].STimeout++>MAXTIMEOUT)
				{
					interface[num].STimeout=0;
					interface[num].SendState=HEADER;
				}
				if(GetFifoBytes(2*num+1)>=interface[num].WaitLength)
				{
				if(interface[num].CurInstruction.bWait)
					interface[num].hWaitFifoEvent=0;
				ReadFifoData(2*num+1,SendDataFifobuf,interface[num].WaitLength);
				SendPackage(num,interface[num].CurInstruction.Instruction,SendDataFifobuf,interface[num].WaitLength,interface[num].CurInstruction.SrcPort,interface[num].CurInstruction.ack);
				interface[num].STimeout=0;
				interface[num].SendState=HEADER;
				if(interface[num].CurInstruction.bWait)
				{
						interface[num].SendState=WAITTIMEOUT;			
				}
				else
				{
						if(interface[num].CurInstruction.pCompleteFunc)
							interface[num].CurInstruction.pCompleteFunc(interface[num].CurInstruction.Instruction,SendDataFifobuf,interface[num].WaitLength,interface[num].CurInstruction.SrcPort,interface[num].CurInstruction.ack);
						if(interface[num].CurInstruction.Interval>0)
						{
						interface[num].SendState=INTERVALWAIT;
						}
				}
			}
				else
					bContinue=FALSE;

				break;
			case WAITTIMEOUT:
				if(interface[num].CurrentWaitTime<interface[num].CurInstruction.WaitTime&&!interface[num].hWaitFifoEvent)
					interface[num].CurrentWaitTime++;
				else if(interface[num].CurrentWaitTime>=interface[num].CurInstruction.WaitTime&&!interface[num].hWaitFifoEvent)
				{
					if(interface[num].CurInstruction.pFailedFunc)      
							interface[num].CurInstruction.pFailedFunc(interface[num].CurInstruction.Instruction,SendDataFifobuf,interface[num].WaitLength,interface[num].CurInstruction.SrcPort,interface[num].CurInstruction.ack);
					interface[num].CurrentWaitTime=0;
					if(interface[num].CurInstruction.Interval>0)
						interface[num].SendState=INTERVALWAIT;	
					else
						interface[num].SendState=HEADER;
				}
				else 
				{
				if(interface[num].CurInstruction.pCompleteFunc)
					interface[num].CurInstruction.pCompleteFunc(interface[num].CurInstruction.Instruction,SendDataFifobuf,interface[num].WaitLength,interface[num].CurInstruction.SrcPort,interface[num].CurInstruction.ack);
				interface[num].CurrentWaitTime=0;
				if(interface[num].CurInstruction.Interval>0)
					interface[num].SendState=INTERVALWAIT;	
				else
					interface[num].SendState=HEADER;
				}
				bContinue=FALSE;	
				break;
			case INTERVALWAIT:
				bContinue=FALSE;
			  if(interface[num].CurrentWaitTime++>=interface[num].CurInstruction.Interval)
				{
					interface[num].CurrentWaitTime=0;
					interface[num].SendState=HEADER;
				}
				break;
			}

 }
}


MY_U8_TYPE SendData(MY_U8_TYPE num,MY_U8_TYPE Ins,MY_U8_TYPE bQueue,MY_U8_TYPE *pData,MY_U32_TYPE length,MY_U16_TYPE SrcPort,MY_U8_TYPE bWait,MY_U32_TYPE WaitTime,INSTRUCTIONFUNC pFailedFunc,INSTRUCTIONFUNC pCompleteFunc,MY_U8_TYPE ack)
{
	
	INSTRUCTIONINFO temp;
	if(num>=INTERFACE_COUNT)
		return 0;
	memset(&temp,0,sizeof(INSTRUCTIONINFO));
	temp.Instruction=Ins;
	temp.bWait=bWait;
	temp.ack=ack;
	temp.WaitTime=WaitTime;
	temp.pFailedFunc=pFailedFunc;
	temp.pCompleteFunc=pCompleteFunc;
	temp.InsFlag=0xAA;
	temp.Interval=0;
	temp.SrcPort=SrcPort;
	if(!bQueue)
	{
	if(temp.bWait)
			interface[num].hWaitEvent=0;
	SendPackage(num,temp.Instruction,pData,length,temp.SrcPort,temp.ack);
	if(temp.bWait&&!WaitForSingleObject(&interface[num].hWaitEvent,temp.WaitTime))
		{
			if(temp.pFailedFunc)         
				temp.pFailedFunc(temp.Instruction,NULL,0,temp.SrcPort,temp.ack);
		}
  else
	{
	if(temp.pCompleteFunc)
			temp.pCompleteFunc(temp.Instruction,pData,length,temp.SrcPort,temp.ack);
	}
	}
	else
	{
		if(interface[num].SWriteSyncFunc)
			interface[num].SWriteSyncFunc(1);
		PutFifoData(2*num+1,(MY_U8_TYPE *)&temp,sizeof(INSTRUCTIONINFO));
		PutFifoData(2*num+1,(MY_U8_TYPE *)&length,4);
		PutFifoData(2*num+1,(MY_U8_TYPE *)pData,length);
		if(interface[num].SWriteSyncFunc)
			interface[num].SWriteSyncFunc(0);
	}
	return TRUE;
}
void SendAutoIns(MY_U8_TYPE num)
{
	MY_U8_TYPE i=0;
	MY_U32_TYPE length=0;
	if(num>=INTERFACE_COUNT)
		return;
	
	for(i=0;i<interface[num].AutoInsCount;i++)
	{
		memcpy(&interface[num].CurInstruction,&interface[num].AutoInstruction[i],sizeof(INSTRUCTIONINFO));
		PutFifoData(2*num+1,(MY_U8_TYPE *)&interface[num].CurInstruction,sizeof(INSTRUCTIONINFO));
		PutFifoData(2*num+1,(MY_U8_TYPE *)&length,sizeof(MY_U32_TYPE));
	
	}
}

MY_U8_TYPE RegisterAutoIns(MY_U8_TYPE num,MY_U8_TYPE Ins,MY_U16_TYPE SrcPort,MY_U32_TYPE Interval,MY_U8_TYPE bWait,MY_U32_TYPE WaitTime,INSTRUCTIONFUNC pFailedFunc,INSTRUCTIONFUNC pCompleteFunc)
{
	INSTRUCTIONINFO newAutoIns;
	MY_U8_TYPE i=0;
	if(num>=INTERFACE_COUNT)
		return 0;
	newAutoIns.Instruction=Ins;
	newAutoIns.pFailedFunc=pFailedFunc;
	newAutoIns.pCompleteFunc=pCompleteFunc;
	newAutoIns.bWait=bWait;
	newAutoIns.Interval=Interval;
	newAutoIns.WaitTime=WaitTime;
	newAutoIns.ack=0;
	newAutoIns.InsFlag=0xAA;
	newAutoIns.SrcPort=SrcPort;
	for(i=0;i<interface[num].AutoInsCount;i++)
	{
		if(interface[num].AutoInstruction[i].Instruction==Ins)
			break;
	}
	if(i>=interface[num].AutoInsCount)
		memcpy(&interface[num].AutoInstruction[interface[num].AutoInsCount++],&newAutoIns,sizeof(INSTRUCTIONINFO));
	else
	{
		interface[num].AutoInstruction[i].pFailedFunc=pFailedFunc;
		interface[num].AutoInstruction[i].pCompleteFunc=pCompleteFunc;
		interface[num].AutoInstruction[i].bWait=bWait;
		interface[num].AutoInstruction[i].Interval=Interval;
		interface[num].AutoInstruction[i].WaitTime=WaitTime;
	}
	return TRUE;
}


MY_U8_TYPE RegisterCallBack(MY_U8_TYPE num,MY_U8_TYPE Ins,INSTRUCTIONFUNC pFunc)
{
	REGISTERINS newResIns;
	int i=0;
	if(num>=INTERFACE_COUNT)
		return 0;
	newResIns.Instruction=Ins;
	newResIns.pFunc=pFunc;
	
	for(i=0;i<interface[num].InsCount;i++)
	{
		if(interface[num].RegisterInsFunc[i].Instruction==Ins)
			break;
	}
	if(i>=interface[num].InsCount)
		memcpy(&interface[num].RegisterInsFunc[interface[num].InsCount++],&newResIns,sizeof(REGISTERINS));
	else
		interface[num].RegisterInsFunc[i].pFunc=pFunc;
	return TRUE;
}
