#include "stm32f10x.h"                  						// STM32F10x.h definitions  
#include "global.h"

void MotorDrv_Initialize(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;						// IO ������������
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);		// GPIOA, GPIOB Ŭ������
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;					// RESET_R(PA4) �ɼ���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;					// RESET_L(PA5) �ɼ���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	MotorDrv_Reset(_HIGH_);										// ���͵���̹� enable
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;					// PWMH_R(PA6) �ɼ���(�����Ұ�)
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_ResetBits(GPIOA, GPIO_Pin_6);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;					// PWMH_L(PA7) �ɼ���(�����Ұ�)
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;					// PWML_R(PB0) �ɼ���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;					// PWML_L(PB1) �ɼ���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	MotorDrv_PWML(_HIGH_);										// drive-brake mode
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;					// DIR_R(PB10) �ɼ���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;					// DIR_L(PB11) �ɼ���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	MotorDrv_DIR(_FORWARD_,_FORWARD_);							// ���͹��� ������/����
}

void MotorDrv_Reset(unsigned char cmd)							// ���͵���̹� �����Լ�(��Ƽ�� LOW)
{
	if(cmd == _HIGH_)
	{
		GPIO_SetBits(GPIOA, GPIO_Pin_4);
		GPIO_SetBits(GPIOA, GPIO_Pin_5);
	}
	else
	{
		GPIO_ResetBits(GPIOA, GPIO_Pin_4);
		GPIO_ResetBits(GPIOA, GPIO_Pin_5);
	}
}

void MotorDrv_PWML(unsigned char cmd)							// ���͵���̹� ���۸�弳��
{
	if(cmd == _HIGH_)											// drive-brake mode
	{
		GPIO_SetBits(GPIOB, GPIO_Pin_0);
		GPIO_SetBits(GPIOB, GPIO_Pin_1);
	}
	else														// ??
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_0);
		GPIO_ResetBits(GPIOB, GPIO_Pin_1);
	}
}

void MotorDrv_DIR(unsigned char right, unsigned char left)		// ���͵��۹���
{
	if(right == _BACKWARD_)											
	{
		GPIO_SetBits(GPIOB, GPIO_Pin_10);						// backward
	}
	else														
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_10);						// forward
	}
	
	if(left == _BACKWARD_)											
	{
		GPIO_SetBits(GPIOB, GPIO_Pin_11);						// backward
	}
	else														
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_11);						// forward
	}
}
