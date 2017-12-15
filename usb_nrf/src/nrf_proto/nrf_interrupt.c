#include "nrf_interrupt.h"


static void nrf_interrupthandler()
{
		uint8_t NRFITStatus=0;
		NRF24L01_ReadBytes(STATUS,&NRFITStatus,1);

			if(NRFITStatus&TX_DS)
			{
				TranFinish_flag=1;
				ClearIT(TX_DS);			
			}
			if(NRFITStatus&RX_DR)
			{
				ClearIT(RX_DR);
				NrfRecvData();
					
			}
			if(NRFITStatus&MAX_RT)
			{
				timeout=1;
				ClearIT(MAX_RT);
			}
}


void EXTI9_5_IRQHandler(void)
{
	
	if(EXTI_GetITStatus(RF_IRQ_EXTI_LINE) != RESET)
  {
    	
		nrf_interrupthandler();  //中断处理
    EXTI_ClearITPendingBit(RF_IRQ_EXTI_LINE);		

  } 	
}

void TIM4_IRQHandler(void)
{
  if (TIM_GetITStatus(THREAD_TIMER, TIM_IT_CC1) != RESET)
  {
		TIM_Cmd(THREAD_TIMER, DISABLE);
		proto_thread();        //协议线程
    TIM_ClearITPendingBit(THREAD_TIMER, TIM_IT_CC1);                   // clear timer overflow flag
		TIM_Cmd(THREAD_TIMER, ENABLE);
  }
}

