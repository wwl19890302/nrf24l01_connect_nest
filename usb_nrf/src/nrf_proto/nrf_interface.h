#ifndef _NRF_INTERFACE_H
#define _NRF_INTERFACE_H
#include "nrf_data.h"

#define RF_SPI                           SPI1
#define RF_SPI_CLK                       RCC_APB2Periph_SPI1

#define RF_SPI_SCK_PIN                   GPIO_Pin_5                  /* PA.05 */
#define RF_SPI_SCK_GPIO_PORT             GPIOA                       /* GPIOA */
#define RF_SPI_SCK_GPIO_CLK              RCC_APB2Periph_GPIOA
#define RF_SPI_SCK_SOURCE                GPIO_PinSource5



 #define RF_SPI_MISO_PIN                 GPIO_Pin_6                  /* PA.06 */
 #define RF_SPI_MISO_GPIO_PORT           GPIOA                       /* GPIOA */
 #define RF_SPI_MISO_GPIO_CLK            RCC_APB2Periph_GPIOA
 #define RF_SPI_MISO_SOURCE              GPIO_PinSource6



#define RF_SPI_MOSI_PIN                  GPIO_Pin_7                  /* PA.07 */
#define RF_SPI_MOSI_GPIO_PORT            GPIOA                       /* GPIOA */
#define RF_SPI_MOSI_GPIO_CLK             RCC_APB2Periph_GPIOA
#define RF_SPI_MOSI_SOURCE               GPIO_PinSource7


#define RF_CS_PIN                        GPIO_Pin_4                  /* PA.4 */
#define RF_CS_GPIO_PORT                  GPIOA                       /* GPIOA */
#define RF_CS_GPIO_CLK                   RCC_APB2Periph_GPIOA

#define RF_CE_PIN                        GPIO_Pin_1                  /* PA.1 */
#define RF_CE_GPIO_PORT                  GPIOA                       /* GPIOA */
#define RF_CE_GPIO_CLK                   RCC_APB2Periph_GPIOA


#define RF_IRQ_PIN                    GPIO_Pin_8                 /* PA.8 */
#define RF_IRQ_EXTI_LINE              EXTI_Line8
#define RF_IRQ_EXTI_PIN_SOURCE        GPIO_PinSource8
#define RF_IRQ_GPIO_PORT              GPIOA                       /* GPIOA */
#define RF_IRQ_GPIO_CLK               RCC_APB2Periph_GPIOA
#define RF_IRQ_EXTI_PORT_SOURCE       GPIO_PortSourceGPIOA
#define RF_IRQ_EXTI_IRQn              EXTI9_5_IRQn

/* 片选口线置低选中  */
#define RF_CS_LOW()       GPIO_ResetBits(RF_CS_GPIO_PORT, RF_CS_PIN)

/* 片选口线置高不选中 */
#define RF_CS_HIGH()      GPIO_SetBits(RF_CS_GPIO_PORT, RF_CS_PIN)

/*接收模式  */
#define RF_CE_LOW()       GPIO_ResetBits(RF_CE_GPIO_PORT, RF_CE_PIN)

/* 发送模式 */
#define RF_CE_HIGH()      GPIO_SetBits(RF_CE_GPIO_PORT, RF_CE_PIN)


void Interface_Init(void); 
 void NRF_EXTI_Init(void);
UINT8 NRF24L01_WriteBytes(UINT8 addr,UINT8 *buf,UINT8 count);
UINT8 NRF24L01_ReadBytes(UINT8 addr,UINT8 *buf,UINT8 count);
#endif
