#include "nrf_interface.h"


void Interface_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
  SPI_InitTypeDef   SPI_InitStructure;

  /*!< RF_SPI_CS_GPIO, RF_SPI_MOSI_GPIO, RF_SPI_MISO_GPIO, RF_SPI_DETECT_GPIO 
       and RF_SPI_SCK_GPIO Periph clock enable */
  RCC_APB2PeriphClockCmd(RF_CE_GPIO_CLK|RF_CS_GPIO_CLK | RF_SPI_MOSI_GPIO_CLK | RF_SPI_MISO_GPIO_CLK |
                        RF_SPI_SCK_GPIO_CLK |RCC_APB2Periph_AFIO, ENABLE);

  /*!< RF_SPI Periph clock enable */
  RCC_APB2PeriphClockCmd(RF_SPI_CLK, ENABLE); 

	

	
  /*!< Configure RF_SPI pins: SCK */
  GPIO_InitStructure.GPIO_Pin = RF_SPI_SCK_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(RF_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

  /*!< Configure RF_SPI pins: MISO */
  GPIO_InitStructure.GPIO_Pin = RF_SPI_MISO_PIN;
  GPIO_Init(RF_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);

  /*!< Configure RF_SPI pins: MOSI */
  GPIO_InitStructure.GPIO_Pin = RF_SPI_MOSI_PIN;
  GPIO_Init(RF_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

  /*!< Configure RF_SPI_CS_PIN pin: RF CS pin */
  GPIO_InitStructure.GPIO_Pin = RF_CS_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(RF_CS_GPIO_PORT, &GPIO_InitStructure);

  /*!< Configure RF_SPI_CS_PIN pin: RF CE pin */
  GPIO_InitStructure.GPIO_Pin = RF_CE_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(RF_CE_GPIO_PORT, &GPIO_InitStructure);


  
  /*!< RF_SPI Config */
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;

  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;

  SPI_Init(RF_SPI, &SPI_InitStructure);
  
  
  RF_CS_HIGH();
  SPI_Cmd(RF_SPI, ENABLE); /*!< RF_SPI enable */



 }
 void NRF_EXTI_Init(void)
 {
	EXTI_InitTypeDef EXTI_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef  GPIO_InitStructure;
	 
	   RCC_APB2PeriphClockCmd(RF_IRQ_GPIO_CLK|RCC_APB2Periph_AFIO, ENABLE);
	 	/*!< Configure RF_SPI_IRQ_PIN pin: RF IRQ pin */
  GPIO_InitStructure.GPIO_Pin = RF_IRQ_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	  //??????
  GPIO_Init(RF_IRQ_GPIO_PORT, &GPIO_InitStructure);
	
	GPIO_EXTILineConfig(RF_IRQ_EXTI_PORT_SOURCE, RF_IRQ_EXTI_PIN_SOURCE);
//   /* Configure Button EXTI line */
  EXTI_InitStructure.EXTI_Line = RF_IRQ_EXTI_LINE;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
	
    /* Enable and set Button EXTI Interrupt to the lowest priority */
  NVIC_InitStructure.NVIC_IRQChannel = RF_IRQ_EXTI_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure); 
 }
 
 UINT8 NRF24L01_WriteBytes(UINT8 addr,UINT8 *buf,UINT8 count)
{
	UINT8 tmp,i=0;
	RF_CE_LOW();
	RF_CS_HIGH();
	 RF_CS_LOW();
  /*!< Wait until the transmit buffer is empty */
  while(SPI_I2S_GetFlagStatus(RF_SPI, SPI_I2S_FLAG_TXE) == RESET)
  {
  }
  
  /*!< Send the byte */
  SPI_I2S_SendData(RF_SPI, 0x20|addr);
  
  /*!< Wait to receive a byte*/
  while(SPI_I2S_GetFlagStatus(RF_SPI, SPI_I2S_FLAG_RXNE) == RESET)
  {
  }
  
  /*!< Return the byte read from the SPI bus */
  tmp=SPI_I2S_ReceiveData(RF_SPI); 

  for(i=0;i<count;i++)
  {
  	while (SPI_I2S_GetFlagStatus(RF_SPI, SPI_I2S_FLAG_TXE) == RESET)
  	{
  	}
  	/*!< Send the byte */
  	SPI_I2S_SendData(RF_SPI, buf[i]);
	while(SPI_I2S_GetFlagStatus(RF_SPI, SPI_I2S_FLAG_RXNE) == RESET)
  {
  }
  
  /*!< Return the byte read from the SPI bus */
    SPI_I2S_ReceiveData(RF_SPI); 

  }
  RF_CS_HIGH();
  return tmp;
}

/**
  * @brief  Read a byte from the NRF24L01.
  * @param  None
  * @retval The received byte.
  */
UINT8 NRF24L01_ReadBytes(UINT8 addr,UINT8 *buf,UINT8 count)
{
  UINT8 Data = 0,i=0;
  RF_CE_LOW();
  RF_CS_HIGH();
  RF_CS_LOW();
  while (SPI_I2S_GetFlagStatus(RF_SPI, SPI_I2S_FLAG_TXE) == RESET)
  {
  }
  /*!< Send the byte */
  SPI_I2S_SendData(RF_SPI, 0x00|addr);
   while (SPI_I2S_GetFlagStatus(RF_SPI, SPI_I2S_FLAG_RXNE) == RESET)
  {
  }
  /*!< Get the received data */
  Data= SPI_I2S_ReceiveData(RF_SPI);
  /*!< Wait until the transmit buffer is empty */
  for(i=0;i<count;i++)
  {
  	while (SPI_I2S_GetFlagStatus(RF_SPI, SPI_I2S_FLAG_TXE) == RESET)
  	{
  	}
  	/*!< Send the byte */
  	SPI_I2S_SendData(RF_SPI, 0x00);

  	/*!< Wait until a data is received */
 	 while (SPI_I2S_GetFlagStatus(RF_SPI, SPI_I2S_FLAG_RXNE) == RESET)
  	{
 	 }
  	/*!< Get the received data */
  	buf[i] = SPI_I2S_ReceiveData(RF_SPI);
  }
  RF_CS_HIGH();
  /*!< Return the shifted data */
  return Data;
}

