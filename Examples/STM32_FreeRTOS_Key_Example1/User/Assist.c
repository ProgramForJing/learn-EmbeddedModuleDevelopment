#include "Assist.h"

void Buzzer_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = buzzer_Pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(buzzer_Port, &GPIO_InitStructure);
}

void Buzzer_Ring(FunctionalState State)
{
	if(State == ENABLE)
	{
		GPIO_SetBits(buzzer_Port, buzzer_Pin);
	}
	else
	{
		GPIO_ResetBits(buzzer_Port, buzzer_Pin);
	}
}

void LED_Init(void)
{	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = led1_Pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(led1_Port, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = led2_Pin;
	GPIO_Init(led2_Port, &GPIO_InitStructure);
	
	GPIO_SetBits(led1_Port, led1_Pin);
	GPIO_SetBits(led2_Port, led2_Pin);

}

void LED1_Light(FunctionalState State)
{
	if(State == ENABLE)
	{
		GPIO_ResetBits(led1_Port, led1_Pin);
	}
	else
	{
		GPIO_SetBits(led1_Port, led1_Pin);
	}
}

void LED2_Light(FunctionalState State)
{
	if(State == ENABLE)
	{
		GPIO_ResetBits(led2_Port, led2_Pin);
	}
	else
	{
		GPIO_SetBits(led2_Port, led2_Pin);
	}
}
