#include "stm32f10x.h"                  						// STM32F10x.h definitions
#include "global.h"

void SPI_NSS_Init(void)					// PA8을 NSS핀으로 사용하기위해 GPIO로 초기화 
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void SPI_NSS_SET(void) 					// PA8을 NSS핀으로 사용, high로 설정
{
	GPIO_WriteBit(GPIOA, GPIO_Pin_8, Bit_SET);
}

void SPI_NSS_RESET(void) 				// PA8을 NSS핀으로 사용, low로 설정
{
	GPIO_WriteBit(GPIOA, GPIO_Pin_8, Bit_RESET);
}

void SPI1_Initialize(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;						// SPI 관련IO 설정구조선언
	SPI_InitTypeDef SPI_InitStructure;							// SPI 설정구조선언 
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);		// IO, AF 클럭공급
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);							// SPI1 클럭공급
   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7; 		// PA5: SPI1_SCK, PA7: SPI1_MOSI
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;					// PA6: SPI1_MISO
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1, &SPI_InitStructure);

	SPI_Cmd(SPI1, ENABLE);
	
	SPI_NSS_Init();						// PA8을 NSS핀으로 사용하기위해 GPIO로 초기화
	SPI_NSS_SET();						// PA8을 NSS핀으로 사용, high로 설정
}

unsigned char SPI1_Send_Byte(unsigned char data)						// SPI1으로 데이터 전송하면서 읽은데이터 리턴, 8bits
{
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);		// Wait for SPIy Tx buffer empty
	SPI_I2S_SendData(SPI1, data);
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);		// Wait for SPIy data reception
	data = SPI_I2S_ReceiveData(SPI1);
		
	return data;
}
