#include "stm32f10x.h"                  					// STM32F10x.h definitions  
#include "global.h"

CanTxMsg TxMessage;
CanRxMsg RxMessage;
unsigned char can_rx_flag = 0;
unsigned short int batt_left, batt_right;					// CAN통신을 통해 모터드라이버로 부터 좌우 배터리 값을 받음
signed short int rpm_left, rpm_right;						// CAN통신을 통해 모터드라이버로 부터 좌우 속도 값을 받음
unsigned char manauto_left, manauto_right;					// 20190108 수전동전환레버 감지추가
unsigned char curr_left, curr_right;						// 20190116 전류소모 감지추가

void CAN_Initialize(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	CAN_InitTypeDef CAN_InitStructure;
	CAN_FilterInitTypeDef CAN_FilterInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;				// CANRX
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;				// CANTX
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_PinRemapConfig(GPIO_Remap1_CAN1, ENABLE);
	
	CAN_DeInit(CAN1);
	CAN_StructInit(&CAN_InitStructure);
	
	CAN_InitStructure.CAN_TTCM = DISABLE;
	CAN_InitStructure.CAN_ABOM = DISABLE;
	CAN_InitStructure.CAN_AWUM = DISABLE;
	CAN_InitStructure.CAN_NART = DISABLE;
	CAN_InitStructure.CAN_RFLM = DISABLE;
	CAN_InitStructure.CAN_TXFP = DISABLE;
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;
	CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
	CAN_InitStructure.CAN_BS1 = CAN_BS1_12tq;
	CAN_InitStructure.CAN_BS2 = CAN_BS2_7tq;				// 100kbps
	CAN_InitStructure.CAN_Prescaler = 18;
	CAN_Init(CAN1, &CAN_InitStructure);
	
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = 0;
	CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;
	CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
	CAN_FilterInitStructure.CAN_FilterNumber = 0;
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
	CAN_FilterInit(&CAN_FilterInitStructure);
	
	CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);
	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitStructure);
}

void CANdataRx_Thread(void)
{	
	if(can_rx_flag == 1)
	{
		can_rx_flag = 0;
		
		if(RxMessage.IDE == CAN_ID_STD)
		{
			//USART2_PutWord(RxMessage.StdId);
			
			if(RxMessage.StdId == 0x0500)							// 좌측 휠
			{
				CAN_flag_L = _ON_;									// 데이터 수신
				
				rpm_left = 0;										// clear
				rpm_left = RxMessage.Data[1];						// 속도값 읽기
				rpm_left = rpm_left << 8;
				rpm_left = rpm_left | RxMessage.Data[2];
				
				batt_left = 0;										// clear
				batt_left = RxMessage.Data[3];						// 배터리값 읽기
				batt_left = batt_left << 8;
				batt_left = batt_left | RxMessage.Data[4];
				
				curr_left = RxMessage.Data[6];						// 전류 읽기
				
				errcode = RxMessage.Data[0];						// 에러코드 읽기
				manauto_left = RxMessage.Data[5];					// 수전동 상태읽기
				
				if(errcode != 0)
				{
					mode = _ERROR_;
					errcode = errcode | 0x40;
				}
			}
			else if(RxMessage.StdId == 0x0600)							// 우측 휠
			{
				CAN_flag_R = _ON_;									// 데이터 수신
				
				rpm_right = 0;										// clear
				rpm_right = RxMessage.Data[1];						// 속도값 읽기
				rpm_right = rpm_right << 8;
				rpm_right = rpm_right | RxMessage.Data[2];
				
				batt_right = 0;										// clear
				batt_right = RxMessage.Data[3];						// 배터리값 읽기
				batt_right = batt_right << 8;
				batt_right = batt_right | RxMessage.Data[4];
				
				curr_right = RxMessage.Data[6];						// 전류 읽기
				
				errcode = RxMessage.Data[0];						// 에러코드 읽기
				manauto_right = RxMessage.Data[5];					// 수전동 상태읽기
				
				if(errcode != 0)
				{
					mode = _ERROR_;
					errcode = errcode | 0x80;
				}
			}
		}
		else
		{
			//USART2_PutWord(RxMessage.ExtId);			// 32bit함수로 바꿔야 함
		}
	}
}

void Chk_GoDown(void)
{
	unsigned char curr_avg = 0;
	
	curr_avg = (curr_left >> 1) + (curr_right >> 1);
	
	//if((batt_left < 238)&&(curr_avg > 75))			//20190314 평지주행중에도 너무 많이 발생함	// 배터리 2칸 && 7.5A(15A)
	if((batt_left < 228)&&(curr_avg > 85))				// 배터리 1칸 && 8.5A(17A)
	{
		speed = 1;
		BuzzerTimer = _BTNTIME_;
	} 
}

void Chk_ManAutoStatus(void)
{
	if(manauto_left == 1)
	{
		mode = _MANAUTO_L_;
	}
	else if(manauto_right == 1)
	{
		mode = _MANAUTO_R_;
	}
}

void Chk_ManAuto_L(void)
{
	if(manauto_left == 0)								// 전동
	{
		mode = _SPEED_;
	}
}

void Chk_ManAuto_R(void)
{
	if(manauto_right == 0)								// 전동
	{
		mode = _SPEED_;
	}
}

void Chk_CANRecover(void)
{
	if((CAN_flag_L == _ON_)&&(CAN_flag_R == _ON_))
	{
		mode = _SPEED_;		
	}
}

void Chk_CANActive(void)
{
	static unsigned short int cnt_L = 0;
	static unsigned short int cnt_R = 0;
	
	if(CAN_flag_L == _ON_)			// CAN데이터를 수신하면 ON이 된다
	{
		cnt_L = 0;					// 클리어!!
	}
	else							// 수신하지 못했음
	{
		cnt_L++;
		
		if(cnt_L >= 1000)				// 대략 1초
		{
			mode = _CANERR_L_;		
			cnt_L = 1000;
		}
	}
	
	CAN_flag_L = _OFF_;				// 플래그는 계속 OFF로 된다
	
	
	
	if(CAN_flag_R == _ON_)			// CAN데이터를 수신하면 ON이 된다
	{
		cnt_R = 0;					// 클리어!!
	}
	else							// 수신하지 못했음
	{
		cnt_R++;
		
		if(cnt_R >= 1000)				// 대략 1초
		{
			mode = _CANERR_R_;		
			cnt_R = 1000;
		}
	}
	
	CAN_flag_R = _OFF_;				// 플래그는 계속 OFF로 된다
}

void Chk_BatteryGage(void)
{
	// 양쪽휠 배터리 데이터가 모두 있지만 왼쪽만 사용함 
	if(batt_left > 277)					// 실측 27.4 (게이지 9개 --> 8개)
	{
		battery = 9;
	}
	else if(batt_left > 270)			
	{
		battery = 8;
	}
	else if(batt_left > 263)			
	{
		battery = 7;
	}
	else if(batt_left > 256)			
	{
		battery = 6;
	}
	else if(batt_left > 249)			
	{
		battery = 5;
	}
	else if(batt_left > 242)			
	{
		battery = 4;
	}
	else if(batt_left > 235)			
	{
		battery = 3;
	}
	else if(batt_left > 228)			// 게이지 2개 --> 1개
	{
		battery = 2;
	}
	else								
	{
		battery = 1;
	}
}

void CAN_Tx_data(unsigned short int id, unsigned char *data)			// 표준 2.0A
{
	unsigned char i;
	
	TxMessage.StdId = id;
	TxMessage.RTR = CAN_RTR_DATA;
	TxMessage.IDE = CAN_ID_STD;
	TxMessage.DLC = 8;
	for(i=0;i<8;i++) TxMessage.Data[i] = data[i];
	CAN_Transmit(CAN1, &TxMessage);
}
