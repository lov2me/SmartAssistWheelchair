#include "stm32f10x.h"                  						// STM32F10x.h definitions  
#include "global.h"

void power_Initialize(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;						// LED ����IO ������������
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);		// IO Ŭ������
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;					// PB10(on/off_Event_Key) �ɼ���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;					// PB11(DC/DC_On/Off) �ɼ���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOB,GPIO_Pin_11);							// joystick ON
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;					// PB7(Relay_On/Off) �ɼ���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOB,GPIO_Pin_7);								// motor driver ON
}

//	��� ������Ʈ ����
//  poweron -> speed -> poweroff

void Chk_power_pin(void)										// PB10(on/off_Event_Key) ���о ��� ����
{
	unsigned char value;
	static unsigned char cnt = 10;
	
	value = GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_10);			// ���ͷ�Ʈ��ƾ���� ��������� �ŷڼ��� ������ ��
	
	if(value == 1)												// ��ư�� ������
	{
		if(mode != _POWERON_)									// �����忡�� 30���̻� ��������(�� 30ms)
		{
			if((data[0] == 0)&&(data[1] == 0)&&(data2[0] == 0)&&(data2[1] == 0))	
			{
				cnt++;													
			
				if(cnt > 30)										// �̸� �����ϱ� ���� 30ȸ�̻� �����Ǿ�� ������� ����
				{
					cnt = 30;										// ī��Ʈ ���� �����÷ο� ���� �ʵ��� ��
					mode = _POWEROFF_;
					
					shutdown_Sequence();							// ����
				}
			}
		}
	}
	else if(value == 0)											// ��ư�� �ȴ�����
	{
		if(mode == _POWERON_)									// ó�� ������ �������� ������ư�� ����� ������� ����
		{
			mode = _SPEED_;
		}
		else
		{
			if(cnt > 0)											// ������� ���� �������� ���� ��찡 ����
			{
				cnt--;											// ������� ������ ī���͸� �ٽ� Ŭ����
			}
		}
	}
}

void Chk_AutoPowerOff(void)
{
	static unsigned int cnt = 0;
	
	if((data[1] != 0)&&(data2[1] != 0))							// ���ⵥ���Ͱ� ���۵Ǹ� Ŭ����
	{
		cnt = 0;
	}
	else
	{
		cnt++;
	}
	
	if(cnt > 300000)											// ���� 5�� 30��
	{
		shutdown_Sequence();									// ����
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
