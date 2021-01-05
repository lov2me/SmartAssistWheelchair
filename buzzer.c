#include "stm32f10x.h"                  						// STM32F10x.h definitions  
#include "global.h"

unsigned int BuzzerTimer = 0;
unsigned char BuzzerOccupation = _OFF_;

void Buzzer_Initialize(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;						// Buzzer ����IO ������������
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);		// IO Ŭ������
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;					// PA8(buzzer) �ɼ���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	Buzzer_OFF();												// buzzer off
}

void Buzzer_ON(void)
{
	GPIO_SetBits(GPIOA, GPIO_Pin_8);							// IO����
	//TIM_Cmd(TIM3, ENABLE);									// PWM����
}

void Buzzer_OFF(void)
{
	if(BuzzerOccupation == _OFF_)
	{
		GPIO_ResetBits(GPIOA, GPIO_Pin_8);							// IO����
		//TIM_Cmd(TIM3, DISABLE);									// PWM����
	}
}

void Chk_BuzzerTimer(void)
{
	if(BuzzerTimer > 0)
	{
		Buzzer_ON();
		BuzzerTimer--;
		
		if(BuzzerTimer == 1)
		{
			Buzzer_OFF();
			BuzzerTimer = 0;
		}
	}
}
