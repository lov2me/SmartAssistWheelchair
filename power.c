#include "stm32f10x.h"                  						// STM32F10x.h definitions  
#include "global.h"

void power_Initialize(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;						// LED 관련IO 설정구조선언
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);		// IO 클럭공급
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;					// PB10(on/off_Event_Key) 핀설정
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;					// PB11(DC/DC_On/Off) 핀설정
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOB,GPIO_Pin_11);							// joystick ON
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;					// PB7(Relay_On/Off) 핀설정
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOB,GPIO_Pin_7);								// motor driver ON
}

//	모드 스테이트 변경
//  poweron -> speed -> poweroff

void Chk_power_pin(void)										// PB10(on/off_Event_Key) 핀읽어서 모드 설정
{
	unsigned char value;
	static unsigned char cnt = 10;
	
	value = GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_10);			// 인터럽트루틴보다 폴링방법이 신뢰성이 높을듯 함
	
	if(value == 1)												// 버튼이 눌러짐
	{
		if(mode != _POWERON_)									// 주행모드에서 30번이상 눌러지면(약 30ms)
		{
			if((data[0] == 0)&&(data[1] == 0)&&(data2[0] == 0)&&(data2[1] == 0))	
			{
				cnt++;													
			
				if(cnt > 30)										// 이를 방지하기 위해 30회이상 감지되어야 충전모드 진입
				{
					cnt = 30;										// 카운트 값이 오버플로우 되지 않도록 함
					mode = _POWEROFF_;
					
					shutdown_Sequence();							// 종료
				}
			}
		}
	}
	else if(value == 0)											// 버튼이 안눌러짐
	{
		if(mode == _POWERON_)									// 처음 전원이 켜졌을때 전원버튼을 떼어야 주행모드로 변경
		{
			mode = _SPEED_;
		}
		else
		{
			if(cnt > 0)											// 노이즈로 인해 충전모드로 들어가는 경우가 있음
			{
				cnt--;											// 노이즈로 증가된 카운터를 다시 클리어
			}
		}
	}
}

void Chk_AutoPowerOff(void)
{
	static unsigned int cnt = 0;
	
	if((data[1] != 0)&&(data2[1] != 0))							// 조향데이터가 전송되면 클리어
	{
		cnt = 0;
	}
	else
	{
		cnt++;
	}
	
	if(cnt > 300000)											// 대충 5분 30초
	{
		shutdown_Sequence();									// 종료
	}
}

void shutdown_Sequence(void)
{
	LED_ALL_ON();
	Buzzer_ON();
	delay_ms(100);
	
	Buzzer_OFF();
	
	delay_ms(100);
	
	Buzzer_ON();
	delay_ms(100);
	
	Buzzer_OFF();
	
	delay_ms(1000);
	LED_ALL_OFF();
	
	GPIO_ResetBits(GPIOB,GPIO_Pin_7);					// motor driver OFF
	GPIO_ResetBits(GPIOB,GPIO_Pin_11);					// joystick OFF
}
