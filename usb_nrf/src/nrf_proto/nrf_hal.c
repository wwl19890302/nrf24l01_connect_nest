#include "nrf_hal.h"
static UINT8 ConfigData;
volatile  UINT8 TranFinish_flag,timeout;


void Init_HardWare(void)   //初始化相关硬件
{
	Interface_Init();
	NRF_EXTI_Init();
	DelayTimerInit();

}

void NRF24L01_Config(UINT8 channel,UINT8 *mac)
{
	Init_HardWare();
	TranFinish_flag=0;
	timeout=0;
  SetNRFPowerDown();
	DataPipeEnable(DataPipe0,DataPipe0);  //Enable and Auto_ack PIPE0
	SetTimeout(0,0);	                     //ReTransmit disable timeout 250
	SetChannel(channel);		                                     //frequency channel 2
	SetRfParm(0,1,3,1);	   //No Force PLL lock Data Rate:1Mbps	0db LNA gain
	ClearIT(RX_DR|TX_DS|MAX_RT);	   //clear RX_DR,TX_DS ,MAX_RT
	SetPipeFIFO(DataPipe0,PAYLOADLEGTH);
	SetPipe0Mac(mac);	
	ClearTxFIFO();
	ClearRxFIFO();
	SetNRFStandbyI();
	NrfDelay_us(2000);
	SetNRFRxMode();
	NrfDelay_us(150);
	Timer_Init(THREADTIME);
}

void SetPipe0Mac(UINT8 *mac)
{
		SetTxOrRxAddr(TxPipe,1,MACLENGTH,mac);	   //PIPE0
}

void EnablePipe1(UINT8 *mac)
{
		DataPipeEnable(DataPipe1,0);
		SetPipeFIFO(DataPipe1,PAYLOADLEGTH);
		SetTxOrRxAddr(DataPipe1,0,MACLENGTH,mac);	   //PIPE1	
}

void SetRegData(UINT8 addr,UINT8 data)
{
	ConfigData=data;
	RF_CE_LOW();
	NRF24L01_WriteBytes(addr,&ConfigData,1);
}



void SetNRFRxMode(void)
{
	//RF_CE_LOW();
	SetRegData(CONFIG,0x0F);  //RX_DR,TX_DS,MAX_RT中断使能，CRC,POWER_UP,接收模式
	RF_CE_HIGH();
//	NrfDelay_us(10);
}
void SetNRFTxMode(void)
{
//	RF_CE_LOW();
	SetRegData(CONFIG,0x0E);  //RX_DR,TX_DS,MAX_RT中断使能，CRC:16,POWER_UP,发送模式
	RF_CE_HIGH();
}
void SetNRFStandbyI(void)
{
	RF_CE_LOW();
	SetRegData(CONFIG,0x7E);  //RX_DR,TX_DS,MAX_RT中断使能，CRC:16,POWER_UP,接收模式
}
void SetNRFPowerDown(void)
{
	RF_CE_LOW();
	SetRegData(CONFIG,0x08);  //RX_DR,TX_DS,MAX_RT中断使能，CRC,POWER_DOWN
}
void StartTransmit(void)
{
	RF_CE_LOW();
	NrfDelay_us(1);
	RF_CE_HIGH();
	NrfDelay_us(15);
//	RF_CE_LOW();
}
UINT8 IsTxFIFOFull(void)
{
	 NRF24L01_ReadBytes(FIFO_STATUS,&ConfigData,1);
	if(ConfigData&0x20)
		return 1;
	else
		return 0;
	
}
void ClearTxFIFO(void)
{
	NRF24L01_ReadBytes(FLUSH_TX,&ConfigData,0);	
}
void ClearRxFIFO(void)
{
	NRF24L01_ReadBytes(FLUSH_RX,&ConfigData,0);
}

UINT8 SetChannel(UINT8 channel)
{
	NRF24L01_ReadBytes(RF_CH,&ConfigData,1);
	SetRegData(RF_CH,channel);
	return ConfigData;
}

UINT8 ReadChannel(void)
{
	NRF24L01_ReadBytes(RF_CH,&ConfigData,1);
	return ConfigData;
}
UINT8 DataPipeEnable(UINT8 DataPipeNum,UINT8 DataPipeACK)
{
	 NRF24L01_ReadBytes(EN_RXADDR,&ConfigData,1);
	 SetRegData(EN_RXADDR,DataPipeNum|ConfigData);
	 NRF24L01_ReadBytes(EN_AA,&ConfigData,1);
	 SetRegData(EN_AA,DataPipeACK|ConfigData);
	 return ConfigData;
}


void SetTxOrRxAddr(UINT8 DataPipeNum,UINT8 AutoAck,UINT8 AddrWidth,UINT8 *addr_buf)
{
	if(AddrWidth>2)
		SetRegData(SETUP_AW,AddrWidth-2);	
	switch(DataPipeNum)
	{
		case DataPipe0:
			NRF24L01_WriteBytes(RX_ADDR_P0,addr_buf,AddrWidth);	   
			break;
		case DataPipe1:
			NRF24L01_WriteBytes(RX_ADDR_P1,addr_buf,AddrWidth);
			break;
		case DataPipe2:
			SetRegData(RX_ADDR_P2,addr_buf[0]);
			break;
		case DataPipe3:
			SetRegData(RX_ADDR_P3,addr_buf[0]);
			break;
		case DataPipe4:
			SetRegData(RX_ADDR_P4,addr_buf[0]);
			break;
		case DataPipe5:
			SetRegData(RX_ADDR_P5,addr_buf[0]);  
			break; 
		case TxPipe:
			NRF24L01_WriteBytes(TX_ADDR,addr_buf,AddrWidth);
			if(AutoAck)
			{
				NRF24L01_WriteBytes(RX_ADDR_P0,addr_buf,AddrWidth);
			}	
			break;   	
	}
}

void SetTimeout(UINT8 interval,UINT8 MaxReTransmitCount)
{
	SetRegData(SETUP_RETR,(interval<<4)|MaxReTransmitCount);
}
UINT8 ReadTxFailCount(void)
{
	NRF24L01_ReadBytes(OBSERVE_TX,&ConfigData,1);
	return ConfigData&0xf0;
}
void SetRfParm(UINT8 pll,UINT8 datarate,UINT8 power,UINT8 lna)
{
	pll &=pll&0x01;
	datarate &=datarate&0x01;
	power &=power&0x03;
	lna   &=lna&0x01;
	SetRegData(RF_SETUP,(pll<<4)|(datarate<<3)|(power<<1)|lna);
}

void SetPipeFIFO(UINT8 DataPipeNum,UINT8 length)
{
	switch(DataPipeNum)
	{
		case DataPipe0:
			SetRegData(RX_PW_P0,length);	   
			break;
		case DataPipe1:
			SetRegData(RX_PW_P1,length);
			break;
		case DataPipe2:
			SetRegData(RX_PW_P2,length);
			break;
		case DataPipe3:
			SetRegData(RX_PW_P3,length);
			break;
		case DataPipe4:
			SetRegData(RX_PW_P4,length);
			break;
		case DataPipe5:
			SetRegData(RX_PW_P5,length);  
			break;  	
	}
}

UINT8 ClearIT(UINT8 interrupt)
{
	NRF24L01_ReadBytes(STATUS,&ConfigData,1);
	SetRegData(STATUS,interrupt);
	return ConfigData&0x70;
}

UINT8 Disturbance_Detect(void)
{
	UINT8 state;
	RF_CE_LOW();
	SetNRFRxMode();
	NrfDelay_us(150);
	NRF24L01_ReadBytes(CD,&state,1);
	return state;
}

BOOL QuickDisturbanceDetect(void)  //检测当前频点的外部环境信号强度，据此判断是否可以发送数据包
{
	UINT8 state;
	RF_CE_LOW();
	NrfDelay_us(1);
	RF_CE_HIGH();
	NrfDelay_us(50);
	NRF24L01_ReadBytes(CD,&state,1);
	if(state)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}



BOOL SendPayload(UINT8 *mac,UINT8 channel,UINT8 *buf)   //发送一个payload
{
	UINT16 i=1;
	SetTxOrRxAddr(TxPipe,1,MACLENGTH,mac);
	SetChannel(channel);
	NRF24L01_WriteBytes(W_TX_PAYLOAD,buf,PAYLOADLEGTH);
	StartTransmit();
	
	while(!timeout&&i++)
	{
		if(TranFinish_flag)
				{
					TranFinish_flag=0;
					break;
				}
		NrfDelay_us(1);
	}
	if(timeout)
	{
		  timeout=0;
			ClearTxFIFO();
			return FALSE;
	}
	else
	{
		return TRUE;
	}
	
}


UINT8 ReadRevData(UINT8 *buf,UINT8 count)
{
	UINT8 bytes=count;
	UINT8 pipe_num=0;
	NRF24L01_ReadBytes(STATUS,&ConfigData,1);
	pipe_num=ConfigData&RX_P_NO_MASK;
	if(bytes>PAYLOADLEGTH)
	{
		bytes=PAYLOADLEGTH;
	}
	if(bytes)
	{
		NRF24L01_ReadBytes(R_RX_PAYLOAD,buf,bytes);
	}
	return (pipe_num>>1);
}

