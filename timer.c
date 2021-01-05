#include "stm32f10x.h"                  						// STM32F10x.h definitions
#include "global.h"

unsigned int Timer2_Counter = 0;								// 딜레이 함수 카운터용

unsigned char Timer2_10msec_Cnt = 0;
unsigned char flag_10msec = FALSE;

unsigned char Timer2_100msec_Cnt = 0;
unsigned char flag_100msec = FALSE;

unsigned int Timer2_500msec_Cnt = 0;
unsigned char flag_500msec = FALSE;

void TIMER2_Initialize(void)									// 딜레이 함수 카운터용(다용도)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;				// TIMER 설정구조선언
	NVIC_InitTypeDef NVIC_InitStructure;						// TIMER 인터럽트 설정구조선언
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);		// TIMER2 클럭공급
	
	TIM_TimeBaseStructure.TIM_Period = 1000-1;					// TIMER2 설정(1msec) : TIM_Period= Fclk * (1/Prescaler)*(1/F_tim) = 72Mhz*(1/72)*(1/1Khz)= 1000
	TIM_TimeBaseStructure.TIM_Prescaler	= 72-1;
	TIM_TimeBaseStructure.TIM_ClockDivision	= 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;				// TIMER2 인터럽트 설정
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	// 인터럽트간 우선순위로 0~3까지 설정가능, 0이면 젤 높은거
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			// 인터럽트루틴 수행중 대기우선순위로 0~3까지 설정가능, 0이면 젤 높은거
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_Cmd(TIM2, ENABLE);										// TIM2 Enable
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);					// TIM2 Interrupt Enable
}

void delay_ms(unsigned int del)									// TIMER2와 연동된 딜레이 함수
{
	Timer2_Counter = 0;
	while(Timer2_Counter < del);
}

//////void TIMER3_Initialize(void)									// Not Used		// for buzzer
//////{
//////	GPIO_InitTypeDef GPIO_InitStructure;						// buzzer 관련IO 설정구조선언
//////	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;				// TIMER 설정구조선언
//////	TIM_OCInitTypeDef TIM_OCInitStructure;						// TIMER 채널 설정구조선언
//////	
//////	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
//////	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
//////	
//////	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;					// PA6(buzzer_pwm) 핀설정
//////	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//////	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
//////	GPIO_Init(GPIOA, &GPIO_InitStructure);
//////	
//////	TIM_TimeBaseStructure.TIM_Period = 1000-1;					// TIMER3 설정(1msec) : TIM_Period= Fclk * (1/Prescaler)*(1/F_tim) = 72Mhz*(1/32)*(1/1Khz)= ??
//////	TIM_TimeBaseStructure.TIM_Prescaler	= 32-1;
//////	TIM_TimeBaseStructure.TIM_ClockDivision	= 0;
//////	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
//////	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
//////	
//////	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
//////	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
//////	TIM_OCInitStructure.TIM_Pulse = 500;	
//////	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
//////						
//////	TIM_OC1Init(TIM3, &TIM_OCInitStructure);					// CH1
//////	
//////	TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
//////	TIM_ARRPreloadConfig(TIM3, ENABLE);
//////	
//////	TIM_Cmd(TIM3, DISABLE);										// TIM3 Enable
//////}

void TIMER4_Initialize(void)									// for keyscan
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;				// TIMER 설정구조선언
	NVIC_InitTypeDef NVIC_InitStructure;						// TIMER 인터럽트 설정구조선언
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);		// TIMER4 클럭공급
	
	TIM_TimeBaseStructure.TIM_Period = 30303-1;					// TIMER4 설정(1msec) : TIM_Period= Fclk * (1/Prescaler)*(1/F_tim) = 72Mhz*(1/72)*(1/30303)= 30msec
	TIM_TimeBaseStructure.TIM_Prescaler	= 72-1;
	TIM_TimeBaseStructure.TIM_ClockDivision	= 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;				// TIMER4 인터럽트 설정
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;	// 인터럽트간 우선순위로 0~3까지 설정가능, 0이면 젤 높은거
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;			// 인터럽트루틴 수행중 대기우선순위로 0~3까지 설정가능, 0이면 젤 높은거
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_Cmd(TIM4, ENABLE);										// TIM4 Enable
	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);					// TIM4 Interrupt Enable
}
