#ifndef _NRF_TIMER_H
#define _NRF_TIMER_H
#include "nrf_timer.h"
#include "nrf_data.h"
#define THREAD_TIMER  TIM4
#define THREAD_TIMER_CLK RCC_APB1Periph_TIM4
#define THREAD_TIMER_IRQChannel TIM4_IRQn
#define DELAY_TIMER   TIM1
#define DELAY_TIMER_CLK RCC_APB2Periph_TIM1
void Timer_Init(UINT16 Time_ARR);
void DelayTimerInit(void);
void NrfDelay_us(UINT16 i);
#endif
