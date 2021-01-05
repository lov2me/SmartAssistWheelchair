#include "stm32f10x.h"                  						// STM32F10x.h definitions  
#include "global.h"

unsigned char Lignting_mode = _ALLOFF_;

void light_Initialize(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;						// LED 관련IO 설정구조선언
		
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);		// IO 클럭공급
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;					// PA11(right light) 핀설정
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_ResetBits(GPIOA,GPIO_Pin_11);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;					// PA12(left light) 핀설정
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_ResetBits(GPIOA,GPIO_Pin_12);			
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);		// IO 클럭공급
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;					// PB1(head light) 핀설정
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_ResetBits(GPIOB,GPIO_Pin_1);		
}

void Head_Light_ON(void)
{
	GPIO_SetBits(GPIOB,GPIO_Pin_1);
}

void Head_Light_OFF(void)
{
	GPIO_ResetBits(GPIOB,GPIO_Pin_1);		
}

void Right_Light_ON(void)
{
	GPIO_SetBits(GPIOA,GPIO_Pin_11);
}

void Right_Light_OFF(void)
{
	GPIO_ResetBits(GPIOA,GPIO_Pin_11);
}

void Left_Light_ON(void)
{
	GPIO_SetBits(GPIOA,GPIO_Pin_12);	
}

void Left_Light_OFF(void)
{
	GPIO_ResetBits(GPIOA,GPIO_Pin_12);	
}

void LighrCtr_Thread(unsigned char Lignting_mode)
{
	static unsigned char blink = 2;
	
	// 2 -> 1 -> 0(1)
	
	if(Lignting_mode != _ALLOFF_)			// 조명버튼이 눌러졌음
	{
		if(blink <= 1)						// 1, 0
		{
			blink ^= 1;
			
			if(blink)
			{
				BuzzerTimer = 100;
				
				if(Lignting_mode == _RIGHT_)
				{
					Right_Light_ON();
				}
				else if(Lignting_mode == _LEFT_)
				{
					Left_Light_ON();
				}
				else if(Lignting_mode == _EMERG_)
				{
					Right_Light_ON();
					Left_Light_ON();
				}
			}
			else
			{
				BuzzerTimer = 100;
				
				if(Lignting_mode == _RIGHT_)
				{
					Right_Light_OFF();
				}
				else if(Lignting_mode == _LEFT_)
				{
					Left_Light_OFF();
				}
				else if(Lignting_mode == _EMERG_)
				{
					Right_Light_OFF();
					Left_Light_OFF();
				}
			}
		}
		else								// 2 (0.5 ~ 1초정도 딜레이)
		{
			blink--;
		}
	}
	else									// 모든 버튼이 꺼짐
	{
		blink = 2;
	}
}
