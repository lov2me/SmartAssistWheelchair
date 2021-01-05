//----------------------------------------------------------------------------
// Name:    main.c
// Purpose: SmartWheelchair
// Note(s): V0.1
//----------------------------------------------------------------------------

#include <stdio.h>
#include "stm32f10x.h"                  		// STM32F10x.h definitions  
#include <stm32f10x.h>
#include "global.h"

unsigned char mode = _POWERON_;					
unsigned char speed = 3;								// ����Ʈ�� 3
unsigned char battery = 0;								// ����Ʈ�� 0(���͸��� ���͵���̹����� ���� �޾ƾ߸� ������Ʈ�ȴ�)
//unsigned char data[8] = {0,0,0,0,0,0,0x03,0x20};		// ���͵���̹� �����Ӽ��� 800
//unsigned char data2[8] = {0,0,0,0,0,0,0x03,0x20};		// ���͵���̹� �����Ӽ��� 800
//unsigned char data[8] = {0,0,0,0,0,0,0x02,0xbc};		// 20180816 ���͵���̹� �����Ӽ��� 700
//unsigned char data2[8] = {0,0,0,0,0,0,0x02,0xbc};		// 20180816 ���͵���̹� �����Ӽ��� 700
//unsigned char data[8] = {0,0,0,0,0,0,0x02,0x8a};		// 20180816 ���͵���̹� �����Ӽ��� 650
//unsigned char data2[8] = {0,0,0,0,0,0,0x02,0x8a};		// 20180816 ���͵���̹� �����Ӽ��� 650
//unsigned char data[8] = {0,0,0,0,0,0,0x02,0x58};		// 20180816 ���͵���̹� �����Ӽ��� 600
//unsigned char data2[8] = {0,0,0,0,0,0,0x02,0x58};		// 20180816 ���͵���̹� �����Ӽ��� 600
//unsigned char data[8] = {0,0,0,0,0,0,0x02,0x26};		// 20180816 ���͵���̹� �����Ӽ��� 550
//unsigned char data2[8] = {0,0,0,0,0,0,0x02,0x26};		// 20180816 ���͵���̹� �����Ӽ��� 550
//unsigned char data[8] = {0,0,0,0,0,0,0x01,0x90};		// 20190108 ���͵���̹� ����������ݺ��� 400(�ּҰ�)
//unsigned char data2[8] = {0,0,0,0,0,0,0x01,0x90};		// 20190108 ���͵���̹� ����������ݺ��� 400(�ּҰ�)
unsigned char data[8] = {0,0,0,0,0,0,0,0x28};			// 20190108 ���͵���̹� ����������ݺ��� 400(�ּҰ�) ���� �����ϸ� ��� ���۵�
unsigned char data2[8] = {0,0,0,0,0,0,0,0x28};			// 20190108 ���͵���̹� ����������ݺ��� 400(�ּҰ�) ���� �����ϸ� ��� ���۵�
unsigned char data3[8] = {0,0x02,0x02,0,0,0,0,0};		// ������� �ʱ� ������ (����Ʈ)		// (���ֵ���0x0 - ����0x1 -  ����Ʈ0x2 - ���ּ���Ʈ0x3)

//unsigned char data[8] = {0,0,0,0,0,0,0x07,0xd0};		// 20180816 ���͵���̹� �����Ӽ��� 2000(�ִ밪)
//unsigned char data2[8] = {0,0,0,0,0,0,0x07,0xd0};		// 20180816 ���͵���̹� �����Ӽ��� 2000(�ִ밪)
unsigned int errcode = 0;
unsigned char CAN_flag_L = _OFF_;						// CAN�����Ͱ� �� ���ŵǴ��� Ȯ���ϴ� �÷���
unsigned char CAN_flag_R = _OFF_;						// CAN�����Ͱ� �� ���ŵǴ��� Ȯ���ϴ� �÷���
unsigned char suslv = 3;								// ������Ƿ���3 (����Ʈ)
unsigned char sus_flag_L = _OFF_;						// ��������� �����ĵǾ����� Ȯ��
unsigned char sus_flag_R = _OFF_;						// ��������� �����ĵǾ����� Ȯ��

//----------------------------------------------------------------------------
//  Main Program
//----------------------------------------------------------------------------
int main (void) 
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);			// ���ͷ�Ʈ�켱��������, 2 bits for pre-emption priority, 2 bits for subpriority
	
	TIMER2_Initialize();									// delay init		// 1KHz interrupt(for delay function)
	TIMER4_Initialize();									// keyscan timer 	// 30 msec
	USART2_Initialize();									// USART2 init		// for debug
	CAN_Initialize();										// CAN init			// 100kbps, �������ͷ�Ʈ, 2.0A��B
	LED_Initialize();										// LED init			// �ӵ�, ���͸� ǥ��
	Buzzer_Initialize();									// buzzer init		// pwmȸ�γ�������� ON/OFF��� �ϸ��
	power_Initialize();										// power init		// ������ư���� �� FET����
	light_Initialize();										// light init		// ������Ʈ & �¿��������
	KeyInit();												// keyscan init
	ADC1_Initialize();										// ADC init			// joystick x1, x2, y1, y2, Vs/2 channel(5ch)
	
	Boot_Sequence();										// LED, ���� ����Ȯ��
	
	while (1)
	{	
		if((mode == _POWERON_)||(mode == _SPEED_))
		{
			delay_ms(1);
		
			KeyScan_Thread();							// �� ��ư �Է��� üũ�ϰ� ���������� �����ϴ� ������
			
			Chk_BuzzerTimer();							// ��ư���� ������û�� ó��
			Chk_power_pin();							// ������ư�� ���������� ����
			Chk_AutoPowerOff();							// �����⸦ �������� �ʰ� 5���� ������ �ڵ�����
			Chk_CANActive();							// �����ð��̻� CAN�����Ͱ� ������ üũ
			Chk_ManAutoStatus();						// ���������� üũ
			Chk_GoDown();								// ������ ������ ���� üũ
			CANdataRx_Thread();
			
			if(flag_10msec == TRUE)						// 0.01�ʸ��� ADC�о ���̽�ƽ�� CAN����
			{
				flag_10msec = FALSE;
				SpeedRefCalc();
			}
			
			if(flag_100msec == TRUE)					// 0.1�ʸ��� LED ��������
			{
				flag_100msec = FALSE;
				Refresh_All_LED(mode,speed,battery,errcode,suslv);
			}
			
			if(flag_500msec == TRUE)					// 0.5�ʸ��� ����
			{
				flag_500msec = FALSE;
				LighrCtr_Thread(Lignting_mode);
				Chk_BatteryGage();
			}
		}
		else if(mode == _SETSUS_)
		{
			delay_ms(1);
			
			KeyScan_Thread();							// �� ��ư �Է��� üũ�ϰ� ���������� �����ϴ� ������
			
			Chk_BuzzerTimer();							// ��ư���� ������û�� ó��
			Chk_power_pin();							// ������ư�� ���������� ����
			Chk_AutoPowerOff();							// �����⸦ �������� �ʰ� 5���� ������ �ڵ�����
			Chk_CANActive();							// �����ð��̻� CAN�����Ͱ� ������ üũ
			Chk_ManAutoStatus();						// ���������� üũ
			CANdataRx_Thread();
			
			if(flag_10msec == TRUE)						// 0.01�ʸ��� ADC�о ���̽�ƽ�� CAN����
			{
				flag_10msec = FALSE;
			}
			
			if(flag_100msec == TRUE)					// 0.1�ʸ��� LED ��������
			{
				flag_100msec = FALSE;
				Refresh_All_LED(mode,speed,battery,errcode,suslv);
			}
			
			if(flag_500msec == TRUE)					// 0.5�ʸ��� ����
			{
				flag_500msec = FALSE;
			}
		}
		else if(mode == _ERROR_)
		{
			delay_ms(1);
			
			Chk_BuzzerTimer();							// ��ư���� ������û�� ó��
			Chk_power_pin();							// ������ư�� ���������� ����
			Chk_AutoPowerOff();							// �����⸦ �������� �ʰ� 5���� ������ �ڵ�����
			
			if(flag_10msec == TRUE)						// 0.01�ʸ��� ADC�о ���̽�ƽ�� CAN����
			{
				flag_10msec = FALSE;
				SpeedRefCalc();
			}
			if(flag_100msec == TRUE)					// 0.1�ʸ��� LED ��������
			{
				flag_100msec = FALSE;
				Refresh_All_LED(mode,speed,battery,errcode,suslv);
			}
			if(flag_500msec == TRUE)					// 0.5�ʸ��� ����
			{
				flag_500msec = FALSE;
				BuzzerTimer = 20;
			}
		}
		else if(mode == _CANERR_L_)
		{
			delay_ms(1);
			
			Chk_BuzzerTimer();							// ��ư���� ������û�� ó��
			Chk_power_pin();							// ������ư�� ���������� ����
			Chk_AutoPowerOff();							// �����⸦ �������� �ʰ� 5���� ������ �ڵ�����
			Chk_CANRecover();							// CAN�����Ͱ� ���ŵǸ� ������� ����
			CANdataRx_Thread();
			
			if(flag_10msec == TRUE)						// 0.01�ʸ��� ADC�о ���̽�ƽ�� CAN����
			{
				flag_10msec = FALSE;
				SpeedRefCalc();
			}
			if(flag_100msec == TRUE)					// 0.1�ʸ��� LED ��������
			{
				flag_100msec = FALSE;
				Refresh_All_LED(mode,speed,battery,errcode,suslv);
			}
			if(flag_500msec == TRUE)					// 0.5�ʸ��� ����
			{
				flag_500msec = FALSE;
				BuzzerTimer = 20;
			}
		}
		else if(mode == _CANERR_R_)
		{
			delay_ms(1);
			
			Chk_BuzzerTimer();							// ��ư���� ������û�� ó��
			Chk_power_pin();							// ������ư�� ���������� ����
			Chk_AutoPowerOff();							// �����⸦ �������� �ʰ� 5���� ������ �ڵ�����
			Chk_CANRecover();							// CAN�����Ͱ� ���ŵǸ� ������� ����
			CANdataRx_Thread();
			
			if(flag_10msec == TRUE)						// 0.01�ʸ��� ADC�о ���̽�ƽ�� CAN����
			{
				flag_10msec = FALSE;
				SpeedRefCalc();
			}
			if(flag_100msec == TRUE)					// 0.1�ʸ��� LED ��������
			{
				flag_100msec = FALSE;
				Refresh_All_LED(mode,speed,battery,errcode,suslv);
			}
			if(flag_500msec == TRUE)					// 0.5�ʸ��� ����
			{
				flag_500msec = FALSE;
				BuzzerTimer = 20;
			}
		}
		else if(mode == _MANAUTO_L_)
		{
			delay_ms(1);
			
			Chk_BuzzerTimer();							// ��ư���� ������û�� ó��
			Chk_power_pin();							// ������ư�� ���������� ����
			Chk_AutoPowerOff();							// �����⸦ �������� �ʰ� 5���� ������ �ڵ�����
			CANdataRx_Thread();
			Chk_ManAuto_L();							// ������������ �������� ���ƿԴ��� Ȯ��
			
			if(flag_10msec == TRUE)						// 0.01�ʸ��� ADC�о ���̽�ƽ�� CAN����
			{
				flag_10msec = FALSE;
				SpeedRefCalc();
			}
			if(flag_100msec == TRUE)					// 0.1�ʸ��� LED ��������
			{
				flag_100msec = FALSE;
				Refresh_All_LED(mode,speed,battery,errcode,suslv);
			}
			if(flag_500msec == TRUE)					// 0.5�ʸ��� ����
			{
				flag_500msec = FALSE;
				BuzzerTimer = 20;
			}
		}
		else if(mode == _MANAUTO_R_)
		{
			delay_ms(1);
			
			Chk_BuzzerTimer();							// ��ư���� ������û�� ó��
			Chk_power_pin();							// ������ư�� ���������� ����
			Chk_AutoPowerOff();							// �����⸦ �������� �ʰ� 5���� ������ �ڵ�����
			CANdataRx_Thread();
			Chk_ManAuto_R();							// ������������ �������� ���ƿԴ��� Ȯ��
			
			if(flag_10msec == TRUE)						// 0.01�ʸ��� ADC�о ���̽�ƽ�� CAN����
			{
				flag_10msec = FALSE;
				SpeedRefCalc();
			}
			if(flag_100msec == TRUE)					// 0.1�ʸ��� LED ��������
			{
				flag_100msec = FALSE;
				Refresh_All_LED(mode,speed,battery,errcode,suslv);
			}
			if(flag_500msec == TRUE)					// 0.5�ʸ��� ����
			{
				flag_500msec = FALSE;
				BuzzerTimer = 20;
			}
		}		
	}
}

void Boot_Sequence(void)
{
	LED_ALL_ON();
	Buzzer_ON();
	delay_ms(100);
	
	Buzzer_OFF();
	
	delay_ms(1000);
	LED_ALL_OFF();
	
	CAN_Tx_data(0x320, data3);			// ������� �ʱ⼳��
}

// for degugging 
		//USART2_PutChar(0xc6);
		//USART2_PutChar(speed);
		//USART2_PutChar(tilt);
		//USART2_PutChar(recline);
		//USART2_PutWord(ADC_DualConvertedValueTab[0]);		// x��
		//USART2_PutWord(ADC_DualConvertedValueTab[1]);		// y��
		//USART2_PutWord(ADC_DualConvertedValueTab[2]);
		//USART2_PutWord(ADC_DualConvertedValueTab[3]);
		//USART2_PutWord((unsigned short)(ADC_DualConvertedValueTab[4]*0.66));
		//CAN_Tx_data(0x100, data);	
		//GPIO_SetBits(GPIOA, GPIO_Pin_6);	
		//Buzzer_ON();
		//Buzzer_OFF();
		//LED_ALL_ON();
		//LED_ALL_OFF();
