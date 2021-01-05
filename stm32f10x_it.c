/**
  ******************************************************************************
  * @file    USART/Interrupt/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and peripherals
  *          interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "global.h"
#include "usart.h"
//////#include "keyscan.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSV_Handler exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
}

/******************************************************************************/
/*            STM32F10x Peripherals Interrupt Handlers                        */
/******************************************************************************/

/**
  * @brief  This function handles USART1 global interrupt request.
  * @param  None
  * @retval None
  */
void USART1_IRQHandler(void)
{
//////	unsigned char RxData;										// 통신테스트용
//////	
//////	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
//////	{
//////		/* Read one byte from the receive data register */
//////		RxData = USART_ReceiveData(USART1);						// 데이터를 읽으면 자동으로 플래그가 클리어 되지만
//////		USART1_PutChar(RxData);									// 받은데이터 그대로 리턴
//////		USART_ClearITPendingBit(USART1, USART_IT_RXNE);			// 혹시나 넣어봄
//////	}
}

/**
  * @brief  This function handles USART2 global interrupt request.
  * @param  None
  * @retval None
  */
void USART2_IRQHandler(void)
{
	unsigned char RxData;										// 통신테스트용
	
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
	{
		/* Read one byte from the receive data register */
		RxData = USART_ReceiveData(USART2);						// 데이터를 읽으면 자동으로 플래그가 클리어 되지만
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);	
		
		USART2_PutChar(RxData);									// 받아지는지 임시 확인용
	}
}

/**
  * @brief  This function handles TIM2 global interrupt request.
  * @param  None
  * @retval None
  */
	
void TIM2_IRQHandler(void)									// 1msec
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
		
		Timer2_Counter++;									// 딜레이 함수에서 사용
		
		Timer2_10msec_Cnt++;
		
		if(Timer2_10msec_Cnt >= 10)
		{
			Timer2_10msec_Cnt = 0;
			flag_10msec = TRUE;
		}
		
		Timer2_100msec_Cnt++;
		
		if(Timer2_100msec_Cnt >= 100)
		{
			Timer2_100msec_Cnt = 0;
			flag_100msec = TRUE;
		}
		
		Timer2_500msec_Cnt++;
		
		if(Timer2_500msec_Cnt >= 500)
		{
			Timer2_500msec_Cnt = 0;
			flag_500msec = TRUE;
		}
	}
}

/**
  * @brief  This function handles TIM4 global interrupt request.
  * @param  None
  * @retval None
  */
	
void TIM4_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
		keyscan_handler();
	}
}

/**
  * @brief  This function handles External lines 9 to 5 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI9_5_IRQHandler(void)
{
//////	if(EXTI_GetITStatus(EXTI_Line9) != RESET)
//////	{
//////		EXTI_ClearITPendingBit(EXTI_Line9);					/* Clear the  EXTI line 9 pending bit */
//////	}
}

void ADC1_2_IRQHandler(void)
{
//////	if(ADC_GetITStatus(ADC1, ADC_IT_EOC) != RESET)			
//////	{
//////		ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
//////	}
}

void USB_LP_CAN1_RX0_IRQHandler(void)
{
	CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);
	can_rx_flag = 1;
}

