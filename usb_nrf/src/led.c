#include "led.h"

void LED_Control(uint8_t num,uint8_t status)
{
	
	switch(status)
	{
		case MYLED_ON:
			GPIO_SetBits(GPIOA,GPIO_Pin_0<<num);
			break;
		case MYLED_OFF:
			GPIO_ResetBits(GPIOA,GPIO_Pin_0<<num);
			break;
		case MYLED_TOGGLE:
			if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0<<num))
			{
				GPIO_ResetBits(GPIOA,GPIO_Pin_0<<num);
			}
			else
			{
				GPIO_SetBits(GPIOA,GPIO_Pin_0<<num);
			}
			break;
		default:
			break;
	}
}

void Led_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO,
				ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}