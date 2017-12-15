


#include "data_handle.h"
#include <stdlib.h>
#include "hw_config.h"
#include "usb_desc.h"
#include "usb_lib.h"

volatile uint8_t Send_Flag;
static SENDPARAMDATA param_data;
static PCDATA data;
static SENDDATARES sRes;
uint8_t bUsbOpen=0;
u16 UsbSendData(u8 *buf,u16 length)
{
	u16 lastbytes=length%VIRTUAL_COM_PORT_DATA_SIZE;
	u16 times=length/VIRTUAL_COM_PORT_DATA_SIZE;
	u16 i=0;
	if(!bUsbOpen)
		return 0;
	for(i=0;i<times;i++)
	{
		Send_Flag=0;
		UserToPMABufferCopy(buf+i*VIRTUAL_COM_PORT_DATA_SIZE, ENDP1_TXADDR, VIRTUAL_COM_PORT_DATA_SIZE);
		SetEPTxCount(ENDP1, VIRTUAL_COM_PORT_DATA_SIZE);
		SetEPTxValid(ENDP1);
		while(!Send_Flag);
	}
	UserToPMABufferCopy(buf+i*VIRTUAL_COM_PORT_DATA_SIZE, ENDP1_TXADDR, lastbytes);
	SetEPTxCount(ENDP1, lastbytes);
	SetEPTxValid(ENDP1);
	return length;
}

void WriteData(u8 *buf,u16 length)
{
	PutFifoData(0,buf,length);
}

void ReadSyncFunc(u8 val)
{
	if(val)
	{
		 NVIC->ICER[USB_LP_CAN1_RX0_IRQn >> 0x05] =
      (uint32_t)0x01 << (USB_LP_CAN1_RX0_IRQn & (uint8_t)0x1F);
	}
	else
	{
			 NVIC->ISER[USB_LP_CAN1_RX0_IRQn >> 0x05] =
      (uint32_t)0x01 << (USB_LP_CAN1_RX0_IRQn & (uint8_t)0x1F);
	}
}


void SendCallBack(UINT8 src_addr,UINT8 dst_addr,UINT8 length,UINT8 *buf,UINT8 status)
{
	sRes.res=status;
	sRes.id=*(uint32_t *)buf;
	SendData(0,SENDDATAACK,TRUE,(u8 *)&sRes,sizeof(sRes),0,FALSE,0,NULL,NULL,TRUE);
}

void RecvCallBack(UINT8 src_addr,UINT8 dst_addr,UINT8 length,UINT8 *buf,UINT8 status)
{
	data.addr=src_addr;
	nrfmemcopy((BYTE *)&data+1,(BYTE *)buf,length);
	SendData(0,GETDATA,TRUE,(uint8_t *)&data,length+1,0,FALSE,0,NULL,NULL,TRUE);
}

u8 HandleInstruction(u8 Ins,u8 *buf,u16 length,u16 srcport,u8 ack)
{
	LPPCDATA pData;
	ARPINFO *pArp;
	switch(Ins)
	{
		case SETNODE:	
			SetNode(buf[0],buf[1]);
			break;
		case SETLOCALMAC:
			SetLocalMac(buf);
			break;
		case SETLINKMAC:
			SetLinkMac(buf);
			break;
		case SETUSERCHANNEL:
			SetLinkChannel(buf[0]);	
			break;
		case SETROUTERID:
			SetRouterId(*((UINT16 *)buf));
			break;
		case SETBROCADCAST:
			SetBroadcastSwitch(buf[0]);
			break;
		case SETBUSERCHANNEL:
			SetUserChannelSwitch(buf[0]);
			break;
		case SETBROADCASTCHANNEL:
			SetBroadcastChannel(buf[0]);
			break;
		case GETPARAM:
			GetParamData(&param_data);
			SendData(0,GETPARAM,TRUE,(u8 *)&param_data,sizeof(SENDPARAMDATA),0,FALSE,0,NULL,NULL,TRUE);
			break;
		case RESETPROTOCAL:
			InitNrfProto();
			break;
		case CLEARFLASHDATA:
			ClearStroreData();
			break;
		case SENDDATA:
			pData=(LPPCDATA)buf;
			nrf_senddata(pData->addr,length-1,buf+1,SendCallBack);
			break;
		case GETADDRLIST:
			SendData(0,GETADDRLIST,TRUE,(u8 *)ArpInfo,ARPINFOCOUNT*sizeof(ARPINFO),0,FALSE,0,NULL,NULL,TRUE);
			break;
		case UPDATEONEARP:
			pArp=(ARPINFO *)buf;
			UpdateOneArp(pArp->status,pArp->mac,pArp->bStatic);
			break;
	}
	return length;
}


void InitAll(void)
{

	InitCommunicationAll();
	SetSendDataFunc(0,UsbSendData);
	SetSyncFunc(0,ReadSyncFunc,NULL);
	RegisterCallBack(0,SETNODE,HandleInstruction);
	RegisterCallBack(0,SETLOCALMAC,HandleInstruction);
	RegisterCallBack(0,SETLINKMAC,HandleInstruction);
	RegisterCallBack(0,SETUSERCHANNEL,HandleInstruction);
	RegisterCallBack(0,SETROUTERID,HandleInstruction);
	RegisterCallBack(0,SETBROCADCAST,HandleInstruction);
	RegisterCallBack(0,SETBUSERCHANNEL,HandleInstruction);
	RegisterCallBack(0,GETPARAM,HandleInstruction);
	RegisterCallBack(0,SETBROADCASTCHANNEL,HandleInstruction);
	RegisterCallBack(0,RESETPROTOCAL,HandleInstruction);
	RegisterCallBack(0,CLEARFLASHDATA,HandleInstruction);
	RegisterCallBack(0,SENDDATA,HandleInstruction);
	RegisterCallBack(0,GETADDRLIST,HandleInstruction);
	RegisterCallBack(0,UPDATEONEARP,HandleInstruction);
	SetDefaultRecvHandler(RecvCallBack);
	SetBroadcastRecvHandler(RecvCallBack);
}

