#ifndef _NRF_HAL_H
#define _NRF_HAL_H
#include "nrf_data.h"
#include "nrf_interface.h"
#include "nrf_store_interface.h"
#include "nrf_timer.h"
/*NRF24L01 INSTRUCTION*/
#define R_RX_PAYLOAD 0x61
#define W_TX_PAYLOAD 0xA0
#define FLUSH_TX     0xE1
#define FLUSH_RX     0xE2
#define RESUE_TX_RL  0xE3
#define NOP          0xFF
/*NRF24L01 REG ADDRESS*/
#define CONFIG       0x00
#define EN_AA        0x01
#define EN_RXADDR    0x02
#define SETUP_AW     0x03
#define SETUP_RETR   0x04
#define RF_CH        0x05
#define RF_SETUP     0x06
#define STATUS       0x07
#define OBSERVE_TX   0x08
#define CD           0x09
#define RX_ADDR_P0   0x0A
#define RX_ADDR_P1   0x0B
#define RX_ADDR_P2   0x0C
#define RX_ADDR_P3   0x0D
#define RX_ADDR_P4   0x0E
#define RX_ADDR_P5   0x0F
#define TX_ADDR      0x10
#define RX_PW_P0     0x11
#define RX_PW_P1     0x12
#define RX_PW_P2     0x13
#define RX_PW_P3     0x14
#define RX_PW_P4     0x15
#define RX_PW_P5     0x16
#define FIFO_STATUS  0x17 

/******NRF24L01 Interrupt****/
#define RX_DR		0x40
#define TX_DS 		0x20
#define MAX_RT		0x10

#define RX_P_NO_MASK 0x0E
#define RX_P_NO0 0x00
#define RX_P_NO1 0x02
#define RX_P_NO2 0x04
#define RX_P_NO3 0x06
#define RX_P_NO4 0x08
#define RX_P_NO5 0x0A
#define RX_FIFO_EMPTY 0x0E


#define DataPipe0    0x01
#define DataPipe1    0x02
#define DataPipe2    0x04
#define DataPipe3    0x08
#define DataPipe4    0x10
#define DataPipe5    0x20
#define TxPipe       0x40


/*******************************同步锁***************************/
#define THREAD_LOCK(x)  __set_PRIMASK(x)
/***************************************************************/


extern volatile  UINT8 TranFinish_flag,Rev_flag,timeout;

void NRF24L01_Config(UINT8 channel,UINT8 *mac);
void SetRegData(UINT8 addr,UINT8 buf);
void SetNRFPowerDown(void);
void SetNRFStandbyI(void);
void SetNRFRxMode(void);
void SetNRFTxMode(void);
void StartTransmit(void);
UINT8 ReadRevData(UINT8 *buf,UINT8 count);
void ClearTxFIFO(void);
void ClearRxFIFO(void);
UINT8 IsTxFIFOFull(void);
UINT8 DataPipeEnable(UINT8 DataPipeNum,UINT8 DataPipeACK);
void SetTxOrRxAddr(UINT8 DataPipeNum,UINT8 AutoAck,UINT8 AddrWidth,UINT8 *addr_buf);
void SetTimeout(UINT8 interval,UINT8 MaxReTransmitCount);
void SetRfParm(UINT8 pll,UINT8 datarate,UINT8 power,UINT8 lna);
UINT8 ClearIT(UINT8 interrupt);
void SetPipeFIFO(UINT8 DataPipeNum,UINT8 length);
UINT8 ReadTxFailCount(void);
UINT8 Disturbance_Detect(void);

UINT8 SetChannel(UINT8 channel);
UINT8 ReadChannel(void);
void SetPipe0Mac(UINT8 *mac);
void EnablePipe1(UINT8 *mac);
BOOL SendPayload(UINT8 *mac,UINT8 channel,UINT8 *buf);
BOOL QuickDisturbanceDetect(void);
void Init_HardWare(void);   //初始化相关硬件
#endif
