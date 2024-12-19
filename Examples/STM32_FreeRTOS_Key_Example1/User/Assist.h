#ifndef ASSIST_H
#define ASSIST_H
#include "stm32f10x.h"
#include "FreeRTOS.h"

/*!
 *  用于初始化LED灯和蜂鸣器
 */

#define led1_Port GPIOA
#define led1_Pin GPIO_Pin_0
#define led2_Port GPIOA
#define led2_Pin GPIO_Pin_1


#define buzzer_Port GPIOA
#define buzzer_Pin GPIO_Pin_8

void Buzzer_Init(void);
void Buzzer_Ring(FunctionalState State);
void LED_Init(void);
void LED1_Light(FunctionalState State);
void LED2_Light(FunctionalState State);


#endif
