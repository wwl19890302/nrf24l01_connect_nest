#ifndef _LED_H
#define _LED_H
#include <stm32f10x.h>
enum LED_STATUS{MYLED_OFF,MYLED_ON,MYLED_TOGGLE};

void LED_Control(uint8_t num,uint8_t status);
void Led_Init(void);
#endif
