#include "stm32f10x.h"                  						// STM32F10x.h definitions  
#include "global.h"

//////void USART1_Initialize(void)
//////{
//////	GPIO_InitTypeDef GPIO_InitStructure;						// USART ����IO ������������
//////	USART_InitTypeDef USART_InitStructure;						// USART ������������
//////	NVIC_InitTypeDef NVIC_InitStructure;						// USART ���ͷ�Ʈ ������������
//////	
//////	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO | RCC_APB2Periph_USART1, ENABLE);		// IO, AF, USART1 Ŭ������
//////	
//////	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;					// USART1 TX �ɼ���
//////	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//////	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
//////	GPIO_Init(GPIOB, &GPIO_InitStructure);
//////	
//////	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;					// USART1 RX �ɼ���
//////	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
//////	GPIO_Init(GPIOB, &GPIO_InitStructure);
//////	
//////	GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);				// remap
//////	
//////	USART_InitStructure.USART_BaudRate = 115200;				// USART ��ż���
//////	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
//////	USART_InitStructure.USART_StopBits = USART_StopBits_1;
//////	USART_InitStructure.USART_Parity = USART_Parity_No;
//////	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
//////	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
//////	
//////	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;			// USART1 �������ͷ�Ʈ ����
//////    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
//////    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
//////    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//////    NVIC_Init(&NVIC_InitStructure);
//////	
//////	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);				// USART1 �������ͷ�Ʈ enable
//////	USART_Init(USART1, &USART_InitStructure);					// Configure USART1
//////	USART_Cmd(USART1, ENABLE);									// Enable the USART1
//////}

//////void USART1_PutChar(unsigned char c)							// USART1 ĳ���� �����Լ�
//////{
//////	USART_SendData(USART1, c);
//////	while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
//////}

//////void USART1_PutWord(unsigned short int w)						// USART1 ���� �����Լ�
//////{
//////	unsigned char c1, c2;
//////	
//////	c1 = (unsigned char)(w >> 8);
//////	c2 = (unsigned char)(w & 0x00ff);
//////	
//////	USART_SendData(USART1, c1);
//////	while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
//////	USART_SendData(USART1, c2);
//////	while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
//////}

//////void USART1_PutString(unsigned char *s)							// USART1 ��Ʈ�� �����Լ�
//////{
//////	while (*s != '\0')
//////	{
//////		USART1_PutChar(*s);
//////		s++;
//////	}
//////}

void USART2_Initialize(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;						// USART ����IO ������������
	USART_InitTypeDef USART_InitStructure;						// USART ������������
	NVIC_InitTypeDef NVIC_InitStructure;						// USART ���ͷ�Ʈ ������������
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);		// IO, AF Ŭ������
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);		// USART2 Ŭ������
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;					// USART2 TX �ɼ���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;					// USART2 RX �ɼ���
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	USART_InitStructure.USART_BaudRate = 115200;					// USART ��ż���
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;			// USART2 �������ͷ�Ʈ ����
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
	
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);				// USART2 �������ͷ�Ʈ enable
	USART_Init(USART2, &USART_InitStructure);					// Configure USART2
	USART_Cmd(USART2, ENABLE);									// Enable the USART2
}

void USART2_PutChar(unsigned char c)							// USART2 ĳ���� �����Լ�
{
	USART_SendData(USART2, c);
	while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
}

void USART2_PutWord(unsigned short int w)						// USART2 ���� �����Լ�
{
	unsigned char c1, c2;
	
	c1 = (unsigned char)(w >> 8);
	c2 = (unsigned char)(w & 0x00ff);
	
	USART_SendData(USART2, c1);
	while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
	USART_SendData(USART2, c2);
	while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
}

void USART2_PutString(unsigned char *s)							// USART2 ��Ʈ�� �����Լ�
{
	while (*s != '\0')
	{
		USART2_PutChar(*s);
		s++;
	}
}
