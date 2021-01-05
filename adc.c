#include "stm32f10x.h"                  						// STM32F10x.h definitions  
#include "global.h"

//#define _MAXRANGE_	1000 
//#define _MINRANGE_	-1000
//#define _MAXDEAD_	50
//#define _MINDEAD_	-50
//#define _VCOFF_		0.66
//#define _HCOFF_		0.20
//#define _SPEEDOFFSET_	2
//#define _DENOMINATOR_	7 


//__IO uint32_t ADC_DualConvertedValueTab[5];
__IO uint32_t ADC_DualConvertedValueTab[3];

void ADC1_Initialize(void)
{
	ADC_InitTypeDef ADC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
// 	NVIC_InitTypeDef NVIC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
//	GPIO_Init(GPIOA, &GPIO_InitStructure);
//	
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
//	GPIO_Init(GPIOA, &GPIO_InitStructure);
//	
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
//	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// DMA1 channel1 configuration ----------------------------------------------
	DMA_DeInit(DMA1_Channel1);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ADC_DualConvertedValueTab;		// 변수와 연결
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
//	DMA_InitStructure.DMA_BufferSize = 5;
	DMA_InitStructure.DMA_BufferSize = 3;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word; // 32bit
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word; // 32bit
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);

	DMA_Cmd(DMA1_Channel1, ENABLE);								// Enable DMA1 Channel1

	// configure ADC1 parameters ------------------------------------------------
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
//	ADC_InitStructure.ADC_NbrOfChannel = 5;
	ADC_InitStructure.ADC_NbrOfChannel = 3;
	ADC_Init(ADC1, &ADC_InitStructure);

	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 3, ADC_SampleTime_239Cycles5);
//	ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 3, ADC_SampleTime_239Cycles5);
//	ADC_RegularChannelConfig(ADC1, ADC_Channel_7, 4, ADC_SampleTime_239Cycles5);
//	ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 5, ADC_SampleTime_239Cycles5);

// 	NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn;
// 	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
// 	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
// 	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
// 	NVIC_Init(&NVIC_InitStructure);

	ADC_DMACmd(ADC1, ENABLE);
	ADC_Cmd(ADC1, ENABLE);
// 	ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);
// 	
	ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1));
	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1));
	
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

float LPF(float input, float prev_in, float gamma)
{
	prev_in = (1 - gamma) * prev_in + gamma * input;
	return prev_in;
}

void SpeedRefCalc(void)			// 조이스틱의 좌표로부터 속도레퍼런스를 계산하고 전송
{
	unsigned short x, y, center;						// ADC raw data
	long x_sign, y_sign;								// 센터값을 중심으로 부호화
	static unsigned char JoyCenterPosition = FALSE;		// 조이스틱이 센터에 와야 동작하게 하기위한 변수
	signed short sub_range, sub_range_x, sub_range_y;
	signed short xr_sign, xl_sign, yr_sign, yl_sign;
	signed short ref_r, ref_l, ref_r_scaled, ref_l_scaled;
	static float SmoothData_X, SmoothData_Y;
	
	float gamma_X = 0.02;
	float gamma_Y_dec1 = 0.015;
	//float gamma_Y_acc1 = 0.006;
	float gamma_Y_dec2 = 0.015;
	//float gamma_Y_acc2 = 0.012;
	float int_Y_acc1 = 2;
	float int_Y_acc2 = 2;
	float int_Y_dec1 = 2;
	//float int_Y_dec2 = 1.5;
	
	x = ADC_DualConvertedValueTab[0];					// x축값
	y = ADC_DualConvertedValueTab[1];					// y축값
	center = ADC_DualConvertedValueTab[2];				// 센터값
			
	x_sign = (long)x - (long)center;					// x값 부호화
	y_sign = (long)y - (long)center;					// y값 부호화
	
	x_sign = x_sign / 2;								// 왜 2로 나누지? 값이 커서? 확인해볼것...
	y_sign = y_sign / 2;
	
	if(x_sign > 1000) x_sign = 1000;					// 최대최소값 제한
	if(x_sign < -1000) x_sign = -1000;
	if(y_sign > 1000) y_sign = 1000;
	if(y_sign < -1000) y_sign = -1000;
			
	if((abs(rpm_left) < 250) && (abs(rpm_right) < 250))				// 주행중 데드존 범위에서 조향이 안되는 문제해결
	{		
		if((x_sign < 50)&&(x_sign > -50)) x_sign = 0;	// 데드존 설정
		if((y_sign < 50)&&(y_sign > -50)) y_sign = 0;
	}
			
	if(JoyCenterPosition == FALSE)
	{
		if((x_sign == 0) && (y_sign == 0))							// 장비를 켜고 처음으로 조이스틱이 센터로 왔음
		{
			JoyCenterPosition = TRUE;
		}
		else														// 장비를 켜고 아직 조이스틱이 센터로 오지 않음
		{
			x_sign = 0;
			y_sign = 0;
		}
	}
	
	if(mode != _SPEED_)												// 주행모드가 아니면 조향데이터를 0으로 임의 삭제함(충전중 동작방지)
	{
		x_sign = 0;
		y_sign = 0;
	}
	
	x_sign = (long)((float)x_sign * (((float)speed + 2) / 7));		// 속도설정에 따른 x, y 스케일링
	y_sign = (long)((float)y_sign * (((float)speed + 2) / 7));		// speed(1~5)의 최대값에 따라 아래서 스케일링을 다시해줘야함
																									// speed(5) + _SPEEDOFFSET_ = _DENOMINATOR_
	x_sign = (signed short)((float)x_sign * 0.2);	// 0.55  20180423	// 수평성분 조정(계란같은모양)
	if(y_sign < 0) y_sign = (long)((float)y_sign * 0.6);			// 수직성분 조정(계란같은모양)
	
	sub_range = (signed short)((((float)speed + 2.0) / 7.0) * 1000);		// 속도설정에 따른 최대속도값
	sub_range = sub_range * 3;
	sub_range_x = sub_range / 10;									// *0.2 *0.25(수평성분 * 25%) 즉 각 속도설정에서 최대속도의 50% 안쪽에서는 매우 느리게 가감속  
	sub_range_y = sub_range / 4;									// 기존값 3
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	if((abs(rpm_left) < sub_range_x) && (abs(rpm_right) < sub_range_x))
	{
		if(x_sign > SmoothData_X)
		{
			SmoothData_X = SmoothData_X + 1;
		}
		else if(x_sign < SmoothData_X)
		{
			SmoothData_X = SmoothData_X - 1;
		}
	}
	else
	{
		SmoothData_X = LPF(x_sign, SmoothData_X, gamma_X);
	}
	
	x_sign = SmoothData_X;
	///////////////////////////////////////////////////////////////////////////////////////////////	
	
	
	
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	if((abs(rpm_left) < sub_range_y) && (abs(rpm_right) < sub_range_y))
	{
		if(y_sign > 0)
		{
			if(SmoothData_Y == 0)				// 가속
			{
				//SmoothData_Y = LPF(y_sign, SmoothData_Y, gamma_Y_acc1);
				SmoothData_Y = SmoothData_Y + int_Y_acc1;
			}
			else if((SmoothData_Y > 0) && (y_sign > SmoothData_Y))		// 가속
			{
				//SmoothData_Y = LPF(y_sign, SmoothData_Y, gamma_Y_acc1);
				SmoothData_Y = SmoothData_Y + int_Y_acc1;
			}
			else if((SmoothData_Y > 0) && (y_sign < SmoothData_Y))		// 감속
			{
				//SmoothData_Y = LPF(0, SmoothData_Y, gamma_Y_dec1);
				SmoothData_Y = SmoothData_Y - int_Y_dec1;
			}
			else if((SmoothData_Y > 0) && (y_sign == SmoothData_Y))		// 등속
			{
				
			}
			else if(SmoothData_Y < 0)		// 감속(급)
			{
				//SmoothData_Y = LPF(0, SmoothData_Y, gamma_Y_dec1);
				SmoothData_Y = SmoothData_Y + int_Y_dec1;
			}
		}
		else if(y_sign < 0)
		{
			if(SmoothData_Y == 0)				// 가속
			{
				//SmoothData_Y = LPF(y_sign, SmoothData_Y, gamma_Y_acc1);
				SmoothData_Y = SmoothData_Y - int_Y_acc1;
			}
			else if((SmoothData_Y < 0) && (y_sign > SmoothData_Y))		// 감속
			{
				//SmoothData_Y = LPF(0, SmoothData_Y, gamma_Y_dec1);
				SmoothData_Y = SmoothData_Y + int_Y_dec1;
			}
			else if((SmoothData_Y < 0) && (y_sign < SmoothData_Y))		// 가속
			{
				//SmoothData_Y = LPF(y_sign, SmoothData_Y, gamma_Y_acc1);
				SmoothData_Y = SmoothData_Y - int_Y_acc1;
			}
			else if((SmoothData_Y < 0) && (y_sign == SmoothData_Y))		// 등속
			{
				
			}
			else if(SmoothData_Y > 0)		// 감속(급)
			{
				//SmoothData_Y = LPF(0, SmoothData_Y, gamma_Y_dec1);
				SmoothData_Y = SmoothData_Y - int_Y_dec1;
			}
		}
		else if(y_sign == 0)
		{
			if(SmoothData_Y > 0)				// 감속(급)
			{
				//SmoothData_Y = LPF(0, SmoothData_Y, gamma_Y_dec1);
				SmoothData_Y = SmoothData_Y - int_Y_dec1;
			}
			else if(SmoothData_Y < 0)			// 감속
			{
				//SmoothData_Y = LPF(0, SmoothData_Y, gamma_Y_dec1);
				SmoothData_Y = SmoothData_Y + int_Y_dec1;
			}
			else if(y_sign == SmoothData_Y)		// 등속
			{
				
			}
		}
	}
	else
	{
		if(y_sign > 0)
		{
			if(SmoothData_Y == 0)				// 가속
			{
				//SmoothData_Y = LPF(y_sign, SmoothData_Y, gamma_Y_acc2);
				SmoothData_Y = SmoothData_Y + int_Y_acc2;
			}
			else if((SmoothData_Y > 0) && (y_sign > SmoothData_Y))		// 가속
			{
				//SmoothData_Y = LPF(y_sign, SmoothData_Y, gamma_Y_acc2);
				SmoothData_Y = SmoothData_Y + int_Y_acc2;
			}
			else if((SmoothData_Y > 0) && (y_sign < SmoothData_Y))		// 감속
			{
				SmoothData_Y = LPF(0, SmoothData_Y, gamma_Y_dec2);
			}
			else if((SmoothData_Y > 0) && (y_sign == SmoothData_Y))		// 등속
			{
				
			}
			else if(SmoothData_Y < 0)		// 감속(급)
			{
				SmoothData_Y = LPF(0, SmoothData_Y, gamma_Y_dec2);
				//SmoothData_Y = SmoothData_Y + int_Y_dec2;
			}
		}
		else if(y_sign < 0)
		{
			if(SmoothData_Y == 0)				// 가속
			{
				//SmoothData_Y = LPF(y_sign, SmoothData_Y, gamma_Y_acc2);
				SmoothData_Y = SmoothData_Y - int_Y_acc2;
			}
			else if((SmoothData_Y < 0) && (y_sign > SmoothData_Y))		// 감속
			{
				SmoothData_Y = LPF(0, SmoothData_Y, gamma_Y_dec2);
			}
			else if((SmoothData_Y < 0) && (y_sign < SmoothData_Y))		// 가속
			{
				//SmoothData_Y = LPF(y_sign, SmoothData_Y, gamma_Y_acc2);
				SmoothData_Y = SmoothData_Y - int_Y_acc2;
			}
			else if((SmoothData_Y < 0) && (y_sign == SmoothData_Y))		// 등속
			{
				
			}
			else if(SmoothData_Y > 0)		// 감속(급)
			{
				SmoothData_Y = LPF(0, SmoothData_Y, gamma_Y_dec2);
				//SmoothData_Y = SmoothData_Y - int_Y_dec2;
			}
		}
		else if(y_sign == 0)
		{
			if(SmoothData_Y > 0)				// 감속(급)
			{
				SmoothData_Y = LPF(0, SmoothData_Y, gamma_Y_dec2);
			}
			else if(SmoothData_Y < 0)			// 감속
			{
				SmoothData_Y = LPF(0, SmoothData_Y, gamma_Y_dec2);
			}
			else if(y_sign == SmoothData_Y)		// 등속
			{
				
			}
		}
	}
	
	if((SmoothData_Y < int_Y_acc1)&&(SmoothData_Y > -int_Y_acc1))
	{
		SmoothData_Y = 0;
	}
	
	y_sign = SmoothData_Y;
	///////////////////////////////////////////////////////////////////////////////////////////////
	
	xr_sign = -x_sign;						// 좌우바퀴에 맞도록 분리
	xl_sign = -x_sign;
	yr_sign = y_sign;
	yl_sign = -y_sign;

	ref_r = xr_sign + yr_sign;				// 최종적인 좌우바퀴 레퍼런스 값				
	ref_l = xl_sign + yl_sign;
	
	if(ref_r > sub_range) ref_r = sub_range;				// 수직성분과 수평성분이 합쳐졌을때 최대값을 넘는경우가 있어서 리미트 해줌			
	if(ref_r < -sub_range) ref_r = -sub_range;
	if(ref_l > sub_range) ref_l = sub_range;
	if(ref_l < -sub_range) ref_l = -sub_range;
		
	ref_r_scaled = (signed short)((float)ref_r * 2.7);		// 20180423 모터드라이버에 맞추어 다시 스케일링
	ref_l_scaled = (signed short)((float)ref_l * 2.7);		// 20180814 토크모자라서 감속기를 43:1 --> 57:1로 변경함 그러므로 변환계수도 2.44 --> 3.2으로 변경
	                                                        // 20210105 정지거리초과로 변경(기존 3.21 -->
															
	ref_r_scaled = -ref_r_scaled;							// 20191124 회전방향바꿈 // 20180816 감속기를 57:1로 변경하면서 휠회전방향이 반대로 바뀜(나사선방향때문)
	ref_l_scaled = ref_l_scaled;							// 20180816 감속기를 57:1로 변경하면서 휠회전방향이 반대로 바뀜(나사선방향때문)
		
	data[0] = ref_l_scaled >> 8;
	data[1] = ref_l_scaled;
	
	data2[0] = ref_r_scaled >> 8;
	data2[1] = ref_r_scaled;
	
	CAN_Tx_data(0x300, data);
	CAN_Tx_data(0x310, data2);
}

