//////#include "stm32f10x.h"                  						// STM32F10x.h definitions  
//////#include "global.h"

//////void charge_Initialize(void)
//////{
//////	GPIO_InitTypeDef GPIO_InitStructure;						// LED ����IO ������������
//////	
//////	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);		// IO Ŭ������
//////	
//////	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;					// PA4(charge) �ɼ���
//////	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//////	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
//////	GPIO_Init(GPIOA, &GPIO_InitStructure);
//////}

////////
////////	��� ������Ʈ ����
////////	����������  ����������  ����������
////////	��speed ���榢charge���ꦢreject��
////////	����������  ����������  ����������
////////

//////void chk_charge_pin(void)										// PA4(charge) ���о ��� ����
//////{
//////	unsigned char value;
//////	static unsigned char cnt;
//////	
//////	value = GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_4);
//////	
//////	if(value == 1)												// ���ͷ�Ʈ��ƾ���� ��������� �ŷڼ��� ������ ��
//////	{
//////		//mode = _SPEED_;											// �����⸦ �ѹ� ������ �����Ѿ߸� ������� ���ƿ�
//////		
//////		if(cnt > 0)												// ������� ���� �������� ���� ��찡 ����
//////		{
//////			cnt--;												// ������� ������ ī���͸� �ٽ� Ŭ����
//////		}
//////	}
//////	else if(value == 0)
//////	{
//////		cnt++;													// ������� ���� �������� ���� ��찡 ����
//////		
//////		if(cnt > 10)											// �̸� �����ϱ� ���� 10ȸ�̻� �����Ǿ�� ������� ����
//////		{
//////			cnt = 10;											// ī��Ʈ ���� �����÷ο� ���� �ʵ��� ��
//////			mode = _CHARGE_;
//////		}
//////	}
//////	
//////	if((mode == _CHARGE_) && (cnt == 0))						// �����Ⱑ ���ŵǾ�����
//////	{
//////		mode = _REJECT_;
//////	}
//////}
