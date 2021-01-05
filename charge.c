//////#include "stm32f10x.h"                  						// STM32F10x.h definitions  
//////#include "global.h"

//////void charge_Initialize(void)
//////{
//////	GPIO_InitTypeDef GPIO_InitStructure;						// LED 관련IO 설정구조선언
//////	
//////	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);		// IO 클럭공급
//////	
//////	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;					// PA4(charge) 핀설정
//////	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//////	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
//////	GPIO_Init(GPIOA, &GPIO_InitStructure);
//////}

////////
////////	모드 스테이트 변경
////////	┌───┐  ┌───┐  ┌───┐
////////	│speed │→│charge│↔│reject│
////////	└───┘  └───┘  └───┘
////////

//////void chk_charge_pin(void)										// PA4(charge) 핀읽어서 모드 설정
//////{
//////	unsigned char value;
//////	static unsigned char cnt;
//////	
//////	value = GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_4);
//////	
//////	if(value == 1)												// 인터럽트루틴보다 폴링방법이 신뢰성이 높을듯 함
//////	{
//////		//mode = _SPEED_;											// 충전기를 한번 꼽으면 껏다켜야만 주행모드로 돌아옴
//////		
//////		if(cnt > 0)												// 노이즈로 인해 충전모드로 들어가는 경우가 있음
//////		{
//////			cnt--;												// 노이즈로 증가된 카운터를 다시 클리어
//////		}
//////	}
//////	else if(value == 0)
//////	{
//////		cnt++;													// 노이즈로 인해 충전모드로 들어가는 경우가 있음
//////		
//////		if(cnt > 10)											// 이를 방지하기 위해 10회이상 감지되어야 충전모드 진입
//////		{
//////			cnt = 10;											// 카운트 값이 오버플로우 되지 않도록 함
//////			mode = _CHARGE_;
//////		}
//////	}
//////	
//////	if((mode == _CHARGE_) && (cnt == 0))						// 충전기가 제거되었을때
//////	{
//////		mode = _REJECT_;
//////	}
//////}
