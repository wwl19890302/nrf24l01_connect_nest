

#ifndef __DATA_HANDLE_H
#define __DATA_HANDLE_H
#include "stm32f10x.h"
#include "hw_config.h"
#include "serial_communication.h"
#include "nrf_phy.h"
#define SETNODE  0x01
#define SETLOCALMAC 0x02
#define SETLINKMAC  0x03
#define SETUSERCHANNEL 0x04
#define SETBROADCASTCHANNEL 0x05
#define SETROUTERID  0x06
#define SETBROCADCAST 0x07
#define SETBUSERCHANNEL 0x08
#define GETPARAM   0x09 
#define RESETPROTOCAL 0x0A
#define CLEARFLASHDATA 0x0B
#define SENDDATA    0x0C
#define SENDDATAACK 0x0D
#define GETDATA     0x0E
#define GETADDRLIST 0x0F
#define UPDATEONEARP 0x10
#pragma pack(1)
typedef struct
{
	uint8_t addr;
	uint32_t id;
	uint8_t buf[100];
}PCDATA,*LPPCDATA;
typedef struct
{
	uint8_t res;
	uint32_t id;
}SENDDATARES,*LPSENDDATARES;
#pragma pack()
extern volatile uint8_t Send_Flag;
u16 UsbSendData(u8 *buf,u16 length);
void WriteData(u8 *buf,u16 length);
void InitAll(void);
u8 HandleInstruction(u8 Ins,u8 *buf,u16 length,u16 srcport,u8 ack);
#endif


