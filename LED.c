#include "stm32f10x.h"                  						// STM32F10x.h definitions  
#include "global.h"

void LED_Initialize(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;						// LED 관련IO 설정구조선언
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);		// IO 클럭공급
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;					// PB12(Shift Data Input) 핀설정
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;					// PB13(Storage Register Clock Input) 핀설정
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;					// PB14(Shift Register Clock Input) 핀설정
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;					// PB15(Master Reset) 핀설정
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	LED_Master_Reset(_HIGH_);									// 74hc595 enable
}

void LED_Shift_Data_Input(unsigned char cmd)					// 74hc595.SDI
{
	if(cmd == _HIGH_)
	{
		GPIO_SetBits(GPIOB, GPIO_Pin_12);
	}
	else
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_12);
	}
}

void LED_Storage_Register_Clock_Input(unsigned char cmd)		// 74hc595.RCLK
{
	if(cmd == _HIGH_)
	{
		GPIO_SetBits(GPIOB, GPIO_Pin_13);
	}
	else
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_13);
	}
}

void LED_Shift_Register_Clock_Input(unsigned char cmd)			// 74hc595.SRCLK
{
	if(cmd == _HIGH_)
	{
		GPIO_SetBits(GPIOB, GPIO_Pin_14);
	}
	else
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_14);
	}
}

void LED_Master_Reset(unsigned char cmd)						// 74hc595.CLR
{
	if(cmd == _HIGH_)
	{
		GPIO_SetBits(GPIOB, GPIO_Pin_15);
	}
	else
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_15);
	}
}

void Refresh_All_LED(unsigned char mode, unsigned char speed, unsigned char battery, unsigned int errcode, unsigned char suslv)		// LED전역변수 조합하여 한번에 리프레쉬
{
	int i;
	unsigned int ShiftData = 0;												// LED 표시용 변수
	unsigned char LEDspeed = 0;												// LED 표시용 변수
	unsigned int LEDbatt = 0;												// LED 표시용 변수
	
	// 전역변수 speed의 값은 1~5 범위의 값을 갖는다
	// 이 값을 LED에 표시하기위해 0x01~0x1f로 변환한다
	if(mode == _SPEED_)
	{
		if(speed == 1)LEDspeed = 0x01;
		else if(speed == 2)LEDspeed = 0x03;
		else if(speed == 3)LEDspeed = 0x07;
		else if(speed == 4)LEDspeed = 0x0f;
		else if(speed == 5)LEDspeed = 0x1f;
		
		if(battery == 1)LEDbatt = 0x01;
		else if(battery == 2)LEDbatt = 0x03;
		else if(battery == 3)LEDbatt = 0x07;
		else if(battery == 4)LEDbatt = 0x0f;
		else if(battery == 5)LEDbatt = 0x1f;
		else if(battery == 6)LEDbatt = 0x3f;
		else if(battery == 7)LEDbatt = 0x7f;
		else if(battery == 8)LEDbatt = 0xff;
		else if(battery == 9)LEDbatt = 0x1ff;
		else LEDbatt = 0x00;
	}
	else if(mode == _SETSUS_)
	{
		LEDbatt = 0x155;
		if(suslv == 1)LEDspeed = 0x01;
		else if(suslv == 2)LEDspeed = 0x03;
		else if(suslv == 3)LEDspeed = 0x07;
		else if(suslv == 4)LEDspeed = 0x0f;
		else LEDspeed = 0x00;
	}
	else if(mode == _ERROR_)
	{
		if(errcode & 0x40)				// left
		{
			LEDbatt = 0x1;
		}
		else
		{
			LEDbatt = 0x100;
		}
		
		LEDspeed = errcode & 0x0f;
	}
	else if(mode == _CANERR_L_)			// OK
	{
		LEDspeed = 0x00;
		LEDbatt = 0x1;
	}
	else if(mode == _CANERR_R_)			// OK
	{
		LEDspeed = 0x00;
		LEDbatt = 0x100;
	}
	else if(mode == _MANAUTO_L_)		// OK
	{
		LEDspeed = 0x10;
		LEDbatt = 0x1;
	}
	else if(mode == _MANAUTO_R_)		// OK
	{
		LEDspeed = 0x10;
		LEDbatt = 0x100;
	}
	else								// OK
	{
		LEDspeed = 0x00;
		LEDbatt = 0x00;
	}
	
	
	ShiftData = (LEDspeed << 9) | (LEDbatt);
	
	for(i=13;i>=0;i--)
	{
		LED_Storage_Register_Clock_Input(_LOW_);
		LED_Shift_Register_Clock_Input(_LOW_);
	
		if((ShiftData >> i) & 0x1)
		{
			LED_Shift_Data_Input(_HIGH_);
		}
		else
		{
			LED_Shift_Data_Input(_LOW_);
		}
		
		LED_Shift_Register_Clock_Input(_HIGH_);
	}
	
	LED_Storage_Register_Clock_Input(_HIGH_);
}

void LED_ALL_ON(void)
{
	int i;
	unsigned int ShiftData = 0;
	
	ShiftData = 0x3fff;
	
	for(i=13;i>=0;i--)
	{
		LED_Storage_Register_Clock_Input(_LOW_);
		LED_Shift_Register_Clock_Input(_LOW_);
	
		if((ShiftData >> i) & 0x1)
		{
			LED_Shift_Data_Input(_HIGH_);
		}
		else
		{
			LED_Shift_Data_Input(_LOW_);
		}
		
		LED_Shift_Register_Clock_Input(_HIGH_);
	}
	
	LED_Storage_Register_Clock_Input(_HIGH_);
}

void LED_ALL_OFF(void)
{
	int i;
	unsigned int ShiftData = 0;
	
	ShiftData = 0;
	
	for(i=13;i>=0;i--)
	{
		LED_Storage_Register_Clock_Input(_LOW_);
		LED_Shift_Register_Clock_Input(_LOW_);
	
		if((ShiftData >> i) & 0x1)
		{
			LED_Shift_Data_Input(_HIGH_);
		}
		else
		{
			LED_Shift_Data_Input(_LOW_);
		}
		
		LED_Shift_Register_Clock_Input(_HIGH_);
	}
	
	LED_Storage_Register_Clock_Input(_HIGH_);
}
