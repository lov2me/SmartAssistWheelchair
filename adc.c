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
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ADC_DualConvertedValueTab;		// ������ ����
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

void SpeedRefCalc(void)			// ���̽�ƽ�� ��ǥ�κ��� �ӵ����۷����� ����ϰ� ����
{
	unsigned short x, y, center;						// ADC raw data
	long x_sign, y_sign;								// ���Ͱ��� �߽����� ��ȣȭ
	static unsigned char JoyCenterPosition = FALSE;		// ���̽�ƽ�� ���Ϳ� �;� �����ϰ� �ϱ����� ����
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
	
	x = ADC_DualConvertedValueTab[0];					// x�ప
	y = ADC_DualConvertedValueTab[1];					// y�ప
	center = ADC_DualConvertedValueTab[2];				// ���Ͱ�
			
	x_sign = (long)x - (long)center;					// x�� ��ȣȭ
	y_sign = (long)y - (long)center;					// y�� ��ȣȭ
	
	x_sign = x_sign / 2;								// �� 2�� ������? ���� Ŀ��? Ȯ���غ���...
	y_sign = y_sign / 2;
	
	if(x_sign > 1000) x_sign = 1000;					// �ִ��ּҰ� ����
	if(x_sign < -1000) x_sign = -1000;
	if(y_sign > 1000) y_sign = 1000;
	if(y_sign < -1000) y_sign = -1000;
			
	if((abs(rpm_left) < 250) && (abs(rpm_right) < 250))				// ������ ������ �������� ������ �ȵǴ� �����ذ�
	{		
		if((x_sign < 50)&&(x_sign > -50)) x_sign = 0;	// ������ ����
		if((y_sign < 50)&&(y_sign > -50)) y_sign = 0;
	}
			
	if(JoyCenterPosition == FALSE)
	{
		if((x_sign == 0) && (y_sign == 0))							// ��� �Ѱ� ó������ ���̽�ƽ�� ���ͷ� ����
		{
			JoyCenterPosition = TRUE;
		}
		else														// ��� �Ѱ� ���� ���̽�ƽ�� ���ͷ� ���� ����
		{
			x_sign = 0;
			y_sign = 0;
		}
	}
	
	if(mode != _SPEED_)												// �����尡 �ƴϸ� ���ⵥ���͸� 0���� ���� ������(������ ���۹���)
	{
		x_sign = 0;
		y_sign = 0;
	}
	
	x_sign = (long)((float)x_sign * (((float)speed + 2) / 7));		// �ӵ������� ���� x, y �����ϸ�
	y_sign = (long)((float)y_sign * (((float)speed + 2) / 7));		// speed(1~5)�� �ִ밪�� ���� �Ʒ��� �����ϸ��� �ٽ��������
																									// speed(5) + _SPEEDOFFSET_ = _DENOMINATOR_
	x_sign = (signed short)((float)x_sign * 0.2);	// 0.55  20180423	// ���򼺺� ����(����������)
	if(y_sign < 0) y_sign = (long)((float)y_sign * 0.6);			// �������� ����(����������)
	
	sub_range = (signed short)((((float)speed + 2.0) / 7.0) * 1000);		// �ӵ������� ���� �ִ�ӵ���
	sub_range = sub_range * 3;
	sub_range_x = sub_range / 10;									// *0.2 *0.25(���򼺺� * 25%) �� �� �ӵ��������� �ִ�ӵ��� 50% ���ʿ����� �ſ� ������ ������  
	sub_range_y = sub_range / 4;									// ������ 3
	
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
			if(SmoothData_Y == 0)				// ����
			{
				//SmoothData_Y = LPF(y_sign, SmoothData_Y, gamma_Y_acc1);
				SmoothData_Y = SmoothData_Y + int_Y_acc1;
			}
			else if((SmoothData_Y > 0) && (y_sign > SmoothData_Y))		// ����
			{
				//SmoothData_Y = LPF(y_sign, SmoothData_Y, gamma_Y_acc1);
				SmoothData_Y = SmoothData_Y + int_Y_acc1;
			}
			else if((SmoothData_Y > 0) && (y_sign < SmoothData_Y))		// ����
			{
				//SmoothData_Y = LPF(0, SmoothData_Y, gamma_Y_dec1);
				SmoothData_Y = SmoothData_Y - int_Y_dec1;
			}
			else if((SmoothData_Y > 0) && (y_sign == SmoothData_Y))		// ���
			{
				
			}
			else if(SmoothData_Y < 0)		// ����(��)
			{
				//SmoothData_Y = LPF(0, SmoothData_Y, gamma_Y_dec1);
				SmoothData_Y = SmoothData_Y + int_Y_dec1;
			}
		}
		else if(y_sign < 0)
		{
			if(SmoothData_Y == 0)				// ����
			{
				//SmoothData_Y = LPF(y_sign, SmoothData_Y, gamma_Y_acc1);
				SmoothData_Y = SmoothData_Y - int_Y_acc1;
			}
			else if((SmoothData_Y < 0) && (y_sign > SmoothData_Y))		// ����
			{
				//SmoothData_Y = LPF(0, SmoothData_Y, gamma_Y_dec1);
				SmoothData_Y = SmoothData_Y + int_Y_dec1;
			}
			else if((SmoothData_Y < 0) && (y_sign < SmoothData_Y))		// ����
			{
				//SmoothData_Y = LPF(y_sign, SmoothData_Y, gamma_Y_acc1);
				SmoothData_Y = SmoothData_Y - int_Y_acc1;
			}
			else if((SmoothData_Y < 0) && (y_sign == SmoothData_Y))		// ���
			{
				
			}
			else if(SmoothData_Y > 0)		// ����(��)
			{
				//SmoothData_Y = LPF(0, SmoothData_Y, gamma_Y_dec1);
				SmoothData_Y = SmoothData_Y - int_Y_dec1;
			}
		}
		else if(y_sign == 0)
		{
			if(SmoothData_Y > 0)				// ����(��)
			{
				//SmoothData_Y = LPF(0, SmoothData_Y, gamma_Y_dec1);
				SmoothData_Y = SmoothData_Y - int_Y_dec1;
			}
			else if(SmoothData_Y < 0)			// ����
			{
				//SmoothData_Y = LPF(0, SmoothData_Y, gamma_Y_dec1);
				SmoothData_Y = SmoothData_Y + int_Y_dec1;
			}
			else if(y_sign == SmoothData_Y)		// ���
			{
				
			}
		}
	}
	else
	{
		if(y_sign > 0)
		{
			if(SmoothData_Y == 0)				// ����
			{
				//SmoothData_Y = LPF(y_sign, SmoothData_Y, gamma_Y_acc2);
				SmoothData_Y = SmoothData_Y + int_Y_acc2;
			}
			else if((SmoothData_Y > 0) && (y_sign > SmoothData_Y))		// ����
			{
				//SmoothData_Y = LPF(y_sign, SmoothData_Y, gamma_Y_acc2);
				SmoothData_Y = SmoothData_Y + int_Y_acc2;
			}
			else if((SmoothData_Y > 0) && (y_sign < SmoothData_Y))		// ����
			{
				SmoothData_Y = LPF(0, SmoothData_Y, gamma_Y_dec2);
			}
			else if((SmoothData_Y > 0) && (y_sign == SmoothData_Y))		// ���
			{
				
			}
			else if(SmoothData_Y < 0)		// ����(��)
			{
				SmoothData_Y = LPF(0, SmoothData_Y, gamma_Y_dec2);
				//SmoothData_Y = SmoothData_Y + int_Y_dec2;
			}
		}
		else if(y_sign < 0)
		{
			if(SmoothData_Y == 0)				// ����
			{
				//SmoothData_Y = LPF(y_sign, SmoothData_Y, gamma_Y_acc2);
				SmoothData_Y = SmoothData_Y - int_Y_acc2;
			}
			else if((SmoothData_Y < 0) && (y_sign > SmoothData_Y))		// ����
			{
				SmoothData_Y = LPF(0, SmoothData_Y, gamma_Y_dec2);
			}
			else if((SmoothData_Y < 0) && (y_sign < SmoothData_Y))		// ����
			{
				//SmoothData_Y = LPF(y_sign, SmoothData_Y, gamma_Y_acc2);
				SmoothData_Y = SmoothData_Y - int_Y_acc2;
			}
			else if((SmoothData_Y < 0) && (y_sign == SmoothData_Y))		// ���
			{
				
			}
			else if(SmoothData_Y > 0)		// ����(��)
			{
				SmoothData_Y = LPF(0, SmoothData_Y, gamma_Y_dec2);
				//SmoothData_Y = SmoothData_Y - int_Y_dec2;
			}
		}
		else if(y_sign == 0)
		{
			if(SmoothData_Y > 0)				// ����(��)
			{
				SmoothData_Y = LPF(0, SmoothData_Y, gamma_Y_dec2);
			}
			else if(SmoothData_Y < 0)			// ����
			{
				SmoothData_Y = LPF(0, SmoothData_Y, gamma_Y_dec2);
			}
			else if(y_sign == SmoothData_Y)		// ���
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
	
	xr_sign = -x_sign;						// �¿������ �µ��� �и�
	xl_sign = -x_sign;
	yr_sign = y_sign;
	yl_sign = -y_sign;

	ref_r = xr_sign + yr_sign;				// �������� �¿���� ���۷��� ��				
	ref_l = xl_sign + yl_sign;
	
	if(ref_r > sub_range) ref_r = sub_range;				// �������а� ���򼺺��� ���������� �ִ밪�� �Ѵ°�찡 �־ ����Ʈ ����			
	if(ref_r < -sub_range) ref_r = -sub_range;
	if(ref_l > sub_range) ref_l = sub_range;
	if(ref_l < -sub_range) ref_l = -sub_range;
		
	ref_r_scaled = (signed short)((float)ref_r * 2.7);		// 20180423 ���͵���̹��� ���߾� �ٽ� �����ϸ�
	ref_l_scaled = (signed short)((float)ref_l * 2.7);		// 20180814 ��ũ���ڶ� ���ӱ⸦ 43:1 --> 57:1�� ������ �׷��Ƿ� ��ȯ����� 2.44 --> 3.2���� ����
	                                                        // 20210105 �����Ÿ��ʰ��� ����(���� 3.21 -->
															
	ref_r_scaled = -ref_r_scaled;							// 20191124 ȸ������ٲ� // 20180816 ���ӱ⸦ 57:1�� �����ϸ鼭 ��ȸ�������� �ݴ�� �ٲ�(���缱���⶧��)
	ref_l_scaled = ref_l_scaled;							// 20180816 ���ӱ⸦ 57:1�� �����ϸ鼭 ��ȸ�������� �ݴ�� �ٲ�(���缱���⶧��)
		
	data[0] = ref_l_scaled >> 8;
	data[1] = ref_l_scaled;
	
	data2[0] = ref_r_scaled >> 8;
	data2[1] = ref_r_scaled;
	
	CAN_Tx_data(0x300, data);
	CAN_Tx_data(0x310, data2);
}

