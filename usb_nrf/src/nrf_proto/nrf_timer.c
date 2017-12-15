#include "nrf_timer.h"
extern void Delay_us(UINT16 i);
//static UINT16 freq_divider;
void Timer_Init(UINT16 Time_ARR)
{
  /* ---------------------------------------------------------------
    TIM2 Configuration: Timing Mode:
    TIM2CLK = 72 MHz, Prescaler = 23, TIM2 counter clock = 3MHz
    TIM2 ARR Register = 14999 => TIM2 Frequency = TIM2 counter clock/(ARR + 1)
    TIM2 Frequency = 200Hz.(5ms)
    TIM2 ARR Register = 29999 => TIM2 Frequency = TIM2 counter clock/(ARR + 1)
    TIM2 Frequency = 100Hz.(10ms)
    TIM2 ARR Register = 2999 => TIM2 Frequency = TIM2 counter clock/(ARR + 1)
    TIM2 Frequency = 1000Hz.(1ms)
  --------------------------------------------------------------- */
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_OCInitTypeDef  TIM_OCInitStructure; 
  NVIC_InitTypeDef NVIC_InitStructure; 
  RCC_ClocksTypeDef RCC_Clocks;
  RCC_APB1PeriphClockCmd(THREAD_TIMER_CLK, ENABLE); //??RCC,??TIMx
  RCC_GetClocksFreq(&RCC_Clocks);
  TIM_DeInit(THREAD_TIMER);         /* deinitiate */
  TIM_TimeBaseStructure.TIM_Period = Time_ARR;   /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Prescaler = RCC_Clocks.PCLK1_Frequency / 1000000;	//1MHz
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(THREAD_TIMER, &TIM_TimeBaseStructure);

  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;   /* Timing Mode :Channel1 */
  TIM_OC1Init(THREAD_TIMER, &TIM_OCInitStructure);
  TIM_OC1PreloadConfig(THREAD_TIMER, TIM_OCPreload_Enable);
  TIM_ARRPreloadConfig(THREAD_TIMER, ENABLE);  

  TIM_ITConfig(THREAD_TIMER, TIM_IT_CC1, ENABLE);   /* TIM IT enable */


  NVIC_InitStructure.NVIC_IRQChannel = THREAD_TIMER_IRQChannel;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStructure); 

  TIM_Cmd(THREAD_TIMER, ENABLE);  /* TIM3 enable counter */
}

void DelayTimerInit(void)                  
	{

// 		TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
// 		RCC_ClocksTypeDef RCC_Clocks;
// 		 RCC_GetClocksFreq(&RCC_Clocks);
// 		RCC_APB2PeriphClockCmd(DELAY_TIMER_CLK, ENABLE); 
// 		freq_divider=RCC_Clocks.PCLK1_Frequency / 1000000;
// 		TIM_TimeBaseStructure.TIM_Period = 1;                 
// 		TIM_TimeBaseStructure.TIM_Prescaler = 0;       //72M
// 		TIM_TimeBaseStructure.TIM_ClockDivision = 0;
// 		TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down; 
// 		TIM_TimeBaseInit(DELAY_TIMER, &TIM_TimeBaseStructure);
	}

// 	void Delay_us(UINT16 i)			 //1~65535us??	  
// 	{
// 		UINT16 TIMCounter = i*freq_divider;
// 		TIM_SetCounter(DELAY_TIMER, TIMCounter);
// 		TIM_Cmd(DELAY_TIMER, ENABLE);
// 		while (TIMCounter>1)
// 		{
// 			TIMCounter = TIM_GetCounter(DELAY_TIMER);
// 		}
// 		TIM_Cmd(DELAY_TIMER, DISABLE);
// 	}


void NrfDelay_us(UINT16 i)
{
  Delay_us(i);
} 
