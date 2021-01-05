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
unsigned char speed = 3;								// 디폴트값 3
unsigned char battery = 0;								// 디폴트값 0(배터리는 모터드라이버에서 값을 받아야만 업데이트된다)
//unsigned char data[8] = {0,0,0,0,0,0,0x03,0x20};		// 모터드라이버 가감속설정 800
//unsigned char data2[8] = {0,0,0,0,0,0,0x03,0x20};		// 모터드라이버 가감속설정 800
//unsigned char data[8] = {0,0,0,0,0,0,0x02,0xbc};		// 20180816 모터드라이버 가감속설정 700
//unsigned char data2[8] = {0,0,0,0,0,0,0x02,0xbc};		// 20180816 모터드라이버 가감속설정 700
//unsigned char data[8] = {0,0,0,0,0,0,0x02,0x8a};		// 20180816 모터드라이버 가감속설정 650
//unsigned char data2[8] = {0,0,0,0,0,0,0x02,0x8a};		// 20180816 모터드라이버 가감속설정 650
//unsigned char data[8] = {0,0,0,0,0,0,0x02,0x58};		// 20180816 모터드라이버 가감속설정 600
//unsigned char data2[8] = {0,0,0,0,0,0,0x02,0x58};		// 20180816 모터드라이버 가감속설정 600
//unsigned char data[8] = {0,0,0,0,0,0,0x02,0x26};		// 20180816 모터드라이버 가감속설정 550
//unsigned char data2[8] = {0,0,0,0,0,0,0x02,0x26};		// 20180816 모터드라이버 가감속설정 550
//unsigned char data[8] = {0,0,0,0,0,0,0x01,0x90};		// 20190108 모터드라이버 통신프로토콜변경 400(최소값)
//unsigned char data2[8] = {0,0,0,0,0,0,0x01,0x90};		// 20190108 모터드라이버 통신프로토콜변경 400(최소값)
unsigned char data[8] = {0,0,0,0,0,0,0,0x28};			// 20190108 모터드라이버 통신프로토콜변경 400(최소값) 값만 변경하면 계속 전송됨
unsigned char data2[8] = {0,0,0,0,0,0,0,0x28};			// 20190108 모터드라이버 통신프로토콜변경 400(최소값) 값만 변경하면 계속 전송됨
unsigned char data3[8] = {0,0x02,0x02,0,0,0,0,0};		// 서스펜션 초기 설정값 (소프트)		// (아주딱딱0x0 - 딱딱0x1 -  소프트0x2 - 아주소프트0x3)

//unsigned char data[8] = {0,0,0,0,0,0,0x07,0xd0};		// 20180816 모터드라이버 가감속설정 2000(최대값)
//unsigned char data2[8] = {0,0,0,0,0,0,0x07,0xd0};		// 20180816 모터드라이버 가감속설정 2000(최대값)
unsigned int errcode = 0;
unsigned char CAN_flag_L = _OFF_;						// CAN데이터가 잘 수신되는지 확인하는 플래그
unsigned char CAN_flag_R = _OFF_;						// CAN데이터가 잘 수신되는지 확인하는 플래그
unsigned char suslv = 3;								// 서스펜션레벨3 (소프트)
unsigned char sus_flag_L = _OFF_;						// 서스펜션이 잘정렬되었는지 확인
unsigned char sus_flag_R = _OFF_;						// 서스펜션이 잘정렬되었는지 확인

//----------------------------------------------------------------------------
//  Main Program
//----------------------------------------------------------------------------
int main (void) 
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);			// 인터럽트우선순위설정, 2 bits for pre-emption priority, 2 bits for subpriority
	
	TIMER2_Initialize();									// delay init		// 1KHz interrupt(for delay function)
	TIMER4_Initialize();									// keyscan timer 	// 30 msec
	USART2_Initialize();									// USART2 init		// for debug
	CAN_Initialize();										// CAN init			// 100kbps, 수신인터럽트, 2.0A및B
	LED_Initialize();										// LED init			// 속도, 배터리 표시
	Buzzer_Initialize();									// buzzer init		// pwm회로내장부져라 ON/OFF제어만 하면됨
	power_Initialize();										// power init		// 전원버튼감지 및 FET제어
	light_Initialize();										// light init		// 헤드라이트 & 좌우측방향등
	KeyInit();												// keyscan init
	ADC1_Initialize();										// ADC init			// joystick x1, x2, y1, y2, Vs/2 channel(5ch)
	
	Boot_Sequence();										// LED, 부져 동작확인
	
	while (1)
	{	
		if((mode == _POWERON_)||(mode == _SPEED_))
		{
			delay_ms(1);
		
			KeyScan_Thread();							// 각 버튼 입력을 체크하고 눌러졌으면 상응하는 동작함
			
			Chk_BuzzerTimer();							// 버튼에서 부저요청시 처리
			Chk_power_pin();							// 전원버튼이 눌러졌는지 감지
			Chk_AutoPowerOff();							// 조종기를 움직이지 않고 5분이 지나면 자동종료
			Chk_CANActive();							// 일정시간이상 CAN데이터가 없는지 체크
			Chk_ManAutoStatus();						// 수전동레버 체크
			Chk_GoDown();								// 저전압 고전류 상태 체크
			CANdataRx_Thread();
			
			if(flag_10msec == TRUE)						// 0.01초마다 ADC읽어서 조이스틱값 CAN전송
			{
				flag_10msec = FALSE;
				SpeedRefCalc();
			}
			
			if(flag_100msec == TRUE)					// 0.1초마다 LED 리프레쉬
			{
				flag_100msec = FALSE;
				Refresh_All_LED(mode,speed,battery,errcode,suslv);
			}
			
			if(flag_500msec == TRUE)					// 0.5초마다 동작
			{
				flag_500msec = FALSE;
				LighrCtr_Thread(Lignting_mode);
				Chk_BatteryGage();
			}
		}
		else if(mode == _SETSUS_)
		{
			delay_ms(1);
			
			KeyScan_Thread();							// 각 버튼 입력을 체크하고 눌러졌으면 상응하는 동작함
			
			Chk_BuzzerTimer();							// 버튼에서 부저요청시 처리
			Chk_power_pin();							// 전원버튼이 눌러졌는지 감지
			Chk_AutoPowerOff();							// 조종기를 움직이지 않고 5분이 지나면 자동종료
			Chk_CANActive();							// 일정시간이상 CAN데이터가 없는지 체크
			Chk_ManAutoStatus();						// 수전동레버 체크
			CANdataRx_Thread();
			
			if(flag_10msec == TRUE)						// 0.01초마다 ADC읽어서 조이스틱값 CAN전송
			{
				flag_10msec = FALSE;
			}
			
			if(flag_100msec == TRUE)					// 0.1초마다 LED 리프레쉬
			{
				flag_100msec = FALSE;
				Refresh_All_LED(mode,speed,battery,errcode,suslv);
			}
			
			if(flag_500msec == TRUE)					// 0.5초마다 동작
			{
				flag_500msec = FALSE;
			}
		}
		else if(mode == _ERROR_)
		{
			delay_ms(1);
			
			Chk_BuzzerTimer();							// 버튼에서 부저요청시 처리
			Chk_power_pin();							// 전원버튼이 눌러졌는지 감지
			Chk_AutoPowerOff();							// 조종기를 움직이지 않고 5분이 지나면 자동종료
			
			if(flag_10msec == TRUE)						// 0.01초마다 ADC읽어서 조이스틱값 CAN전송
			{
				flag_10msec = FALSE;
				SpeedRefCalc();
			}
			if(flag_100msec == TRUE)					// 0.1초마다 LED 리프레쉬
			{
				flag_100msec = FALSE;
				Refresh_All_LED(mode,speed,battery,errcode,suslv);
			}
			if(flag_500msec == TRUE)					// 0.5초마다 동작
			{
				flag_500msec = FALSE;
				BuzzerTimer = 20;
			}
		}
		else if(mode == _CANERR_L_)
		{
			delay_ms(1);
			
			Chk_BuzzerTimer();							// 버튼에서 부저요청시 처리
			Chk_power_pin();							// 전원버튼이 눌러졌는지 감지
			Chk_AutoPowerOff();							// 조종기를 움직이지 않고 5분이 지나면 자동종료
			Chk_CANRecover();							// CAN데이터가 수신되면 주행모드로 복귀
			CANdataRx_Thread();
			
			if(flag_10msec == TRUE)						// 0.01초마다 ADC읽어서 조이스틱값 CAN전송
			{
				flag_10msec = FALSE;
				SpeedRefCalc();
			}
			if(flag_100msec == TRUE)					// 0.1초마다 LED 리프레쉬
			{
				flag_100msec = FALSE;
				Refresh_All_LED(mode,speed,battery,errcode,suslv);
			}
			if(flag_500msec == TRUE)					// 0.5초마다 동작
			{
				flag_500msec = FALSE;
				BuzzerTimer = 20;
			}
		}
		else if(mode == _CANERR_R_)
		{
			delay_ms(1);
			
			Chk_BuzzerTimer();							// 버튼에서 부저요청시 처리
			Chk_power_pin();							// 전원버튼이 눌러졌는지 감지
			Chk_AutoPowerOff();							// 조종기를 움직이지 않고 5분이 지나면 자동종료
			Chk_CANRecover();							// CAN데이터가 수신되면 주행모드로 복귀
			CANdataRx_Thread();
			
			if(flag_10msec == TRUE)						// 0.01초마다 ADC읽어서 조이스틱값 CAN전송
			{
				flag_10msec = FALSE;
				SpeedRefCalc();
			}
			if(flag_100msec == TRUE)					// 0.1초마다 LED 리프레쉬
			{
				flag_100msec = FALSE;
				Refresh_All_LED(mode,speed,battery,errcode,suslv);
			}
			if(flag_500msec == TRUE)					// 0.5초마다 동작
			{
				flag_500msec = FALSE;
				BuzzerTimer = 20;
			}
		}
		else if(mode == _MANAUTO_L_)
		{
			delay_ms(1);
			
			Chk_BuzzerTimer();							// 버튼에서 부저요청시 처리
			Chk_power_pin();							// 전원버튼이 눌러졌는지 감지
			Chk_AutoPowerOff();							// 조종기를 움직이지 않고 5분이 지나면 자동종료
			CANdataRx_Thread();
			Chk_ManAuto_L();							// 수전동레버가 정상으로 돌아왔는지 확인
			
			if(flag_10msec == TRUE)						// 0.01초마다 ADC읽어서 조이스틱값 CAN전송
			{
				flag_10msec = FALSE;
				SpeedRefCalc();
			}
			if(flag_100msec == TRUE)					// 0.1초마다 LED 리프레쉬
			{
				flag_100msec = FALSE;
				Refresh_All_LED(mode,speed,battery,errcode,suslv);
			}
			if(flag_500msec == TRUE)					// 0.5초마다 동작
			{
				flag_500msec = FALSE;
				BuzzerTimer = 20;
			}
		}
		else if(mode == _MANAUTO_R_)
		{
			delay_ms(1);
			
			Chk_BuzzerTimer();							// 버튼에서 부저요청시 처리
			Chk_power_pin();							// 전원버튼이 눌러졌는지 감지
			Chk_AutoPowerOff();							// 조종기를 움직이지 않고 5분이 지나면 자동종료
			CANdataRx_Thread();
			Chk_ManAuto_R();							// 수전동레버가 정상으로 돌아왔는지 확인
			
			if(flag_10msec == TRUE)						// 0.01초마다 ADC읽어서 조이스틱값 CAN전송
			{
				flag_10msec = FALSE;
				SpeedRefCalc();
			}
			if(flag_100msec == TRUE)					// 0.1초마다 LED 리프레쉬
			{
				flag_100msec = FALSE;
				Refresh_All_LED(mode,speed,battery,errcode,suslv);
			}
			if(flag_500msec == TRUE)					// 0.5초마다 동작
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
	
	CAN_Tx_data(0x320, data3);			// 서스펜션 초기설정
}

// for degugging 
		//USART2_PutChar(0xc6);
		//USART2_PutChar(speed);
		//USART2_PutChar(tilt);
		//USART2_PutChar(recline);
		//USART2_PutWord(ADC_DualConvertedValueTab[0]);		// x축
		//USART2_PutWord(ADC_DualConvertedValueTab[1]);		// y축
		//USART2_PutWord(ADC_DualConvertedValueTab[2]);
		//USART2_PutWord(ADC_DualConvertedValueTab[3]);
		//USART2_PutWord((unsigned short)(ADC_DualConvertedValueTab[4]*0.66));
		//CAN_Tx_data(0x100, data);	
		//GPIO_SetBits(GPIOA, GPIO_Pin_6);	
		//Buzzer_ON();
		//Buzzer_OFF();
		//LED_ALL_ON();
		//LED_ALL_OFF();
