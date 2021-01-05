#include "stm32f10x.h"                  						// STM32F10x.h definitions  
#include "global.h"

#define KEY_STATE_UP				1
#define KEY_STATE_DEBOUNCE			2
#define KEY_STATE_RPT_START_DLY		3
#define KEY_STATE_RPT_DLY			4

#define KEY_BUF_SIZE				10
#define KEY_MAX_ROWS				2
#define KEY_MAX_COLS				4
#define KEY_ALL_ROWS 				0xff
#define KEY_RPT_START_DLY			10				// scan period(30msec) * 10 = 300msec

#define _UP_			0			// �ӵ���
#define _HLIGHT_		1			// ������Ʈ
#define _LLIGHT_		2			// ����������
#define _ELIGHT_		3			// ����
#define _DOWN_			4			// �ӵ��ٿ�
#define _BUZZER_		5			// ����
#define _RLIGHT_		6			// ����������

static unsigned char KeyBuf[KEY_BUF_SIZE];
static unsigned char KeyBufInIx;
static unsigned char KeyBufOutIx;
static unsigned short int KeyDownTmr;
static unsigned char KeyNRead;
static unsigned char KeyScanState;
static unsigned char KeyRptStartDlyCtr;

void KeyInitPort(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;						// key ����IO ������������
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);		// IO Ŭ������
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;					// PA4, input
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;					// PA5, input
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;					// PA6, input
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;					// PA7, input
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;					// PB5, output
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;					// PB6, output
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void KeySelRow(unsigned char row)
{
	if(row == KEY_ALL_ROWS)										// ������� 0���� �����
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_5);
		GPIO_ResetBits(GPIOB, GPIO_Pin_6);
	}
	else if(row == 0)											// PB5 0���� �����
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_5);						
		GPIO_SetBits(GPIOB, GPIO_Pin_6);
	}
	else if(row == 1)											// PB6 0���� �����
	{
		GPIO_SetBits(GPIOB, GPIO_Pin_5);
		GPIO_ResetBits(GPIOB, GPIO_Pin_6);
	}
}

void KeyInit(void)
{
	KeySelRow(KEY_ALL_ROWS);
	KeyScanState = KEY_STATE_UP;
	KeyNRead = 0;
	KeyDownTmr = 0;
	KeyBufInIx = 0;
	KeyBufOutIx = 0;
	KeyInitPort();
}

unsigned char KeyGetCol(void)
{
	return (((~GPIO_ReadInputData(GPIOA)) >> 4) & 0x0f);		// PA11~12���� �о LSB�� ����Ʈ, �ش��ɸ� ����ũ, ����(������ 1�ǵ���)
}

unsigned char KeyIsKeyDown(void)								// Ű�� ���������� Ȯ���ϰ� Ű�� �������ð��� ī�����Ѵ�
{
	if(KeyGetCol() & ((1 << KEY_MAX_COLS) - 1))
	{
		KeyDownTmr++;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

unsigned char KeyDecode(void)									// �� �̻��� �ڼ��� ������ �����Ѵ�
{
	unsigned char col;
	unsigned char row;
	unsigned char done;
	unsigned char col_id;
	unsigned char msk;
	
	done = FALSE;
	row = 0;
	
	while(row < KEY_MAX_ROWS && !done)
	{
		KeySelRow(row);											
		
		if(KeyIsKeyDown())
		{
			done = TRUE;
		}
		else
		{
			row++;
		}
	}
	
	col = KeyGetCol();
	msk = 0x01;
	col_id = 0;
	done = FALSE;
	
	while(col_id < KEY_MAX_COLS && !done)
	{
		if(col & msk)
		{
			done = TRUE;
		}
		else
		{
			col_id++;
			msk <<= 1;
		}
	}
	
	return (row * KEY_MAX_COLS + col_id);
}

void KeyBufIn(unsigned char code)
{
	if(KeyNRead < KEY_BUF_SIZE)
	{
		KeyNRead++;
		KeyBuf[KeyBufInIx++] = code;
		
		if(KeyBufInIx >= KEY_BUF_SIZE)
		{
			KeyBufInIx = 0;
		}
	}
}

unsigned char KeyHit(void)
{
	unsigned char hit;
	
	hit = (KeyNRead > 0) ? TRUE : FALSE;
	return hit;
}

unsigned char KeyGetKey(void)
{
	unsigned char code;
	
	if(KeyNRead > 0)
	{
		KeyNRead--;
		code = KeyBuf[KeyBufOutIx];
		KeyBufOutIx++;
		
		if(KeyBufOutIx >= KEY_BUF_SIZE)
		{
			KeyBufOutIx = 0;
		}
		
		return code;
	}
	else
	{
		return 0xff;
	}
}

void KeyScan_Thread(void)
{
	static unsigned char hlight = 0;
	static unsigned char rlight = 0;
	static unsigned char llight = 0;
	static unsigned char elight = 0;
	
	if(KeyHit())
	{		
		switch(KeyGetKey())
		{
			
			case _UP_:					
			
				BuzzerTimer = _BTNTIME_;
				
				if(mode != _SETSUS_)
				{
					if(speed < 5)
					{
						speed++;

					}
				}
				else
				{
					if(suslv < 4)
					{
						suslv++;
						data3[1] = suslv - 1;
						data3[2] = suslv - 1;
						CAN_Tx_data(0x320, data3);
					}
				}
				
				break;
			
				
			case _HLIGHT_:					
			
				BuzzerTimer = _BTNTIME_;
			
				if(mode != _SETSUS_)
				{
					hlight ^= 1;
				
					if(hlight)
					{
						Head_Light_ON();
					}
					else
					{
						Head_Light_OFF();
					}
				}
			
				break;
			
			
			case _LLIGHT_:					
			
				BuzzerTimer = _BTNTIME_;
				
				if(mode != _SETSUS_)
				{
					llight ^= 1;
				
					if(llight)
					{
						rlight = 0;
						elight = 0;
						
						Left_Light_ON();
						Right_Light_OFF();
						Lignting_mode = _LEFT_;
					}
					else
					{
						Right_Light_OFF();
						Left_Light_OFF();
						Lignting_mode = _ALLOFF_;
					}
				}
			
				break;
			
			
			case _ELIGHT_:					
			
				BuzzerTimer = _BTNTIME_;
			
				if(mode != _SETSUS_)
				{
					elight ^= 1;
				
					if(elight)
					{
						rlight = 0;
						llight = 0;
						
						Right_Light_ON();
						Left_Light_ON();
						Lignting_mode = _EMERG_;
					}
					else
					{
						Right_Light_OFF();
						Left_Light_OFF();
						Lignting_mode = _ALLOFF_;
					}
				}
			
				break;
			
			case _DOWN_:	
				
				BuzzerTimer = _BTNTIME_;
			
				if(mode != _SETSUS_)
				{
					if(speed > 1)
					{
						speed--;
					}
				}
				else
				{
					if(suslv > 1)
					{
						suslv--;
						data3[1] = suslv - 1;
						data3[2] = suslv - 1;
						CAN_Tx_data(0x320, data3);
					}
				}

				break;
			
				
			case _BUZZER_:					
			
				break;
			
			
			case _RLIGHT_:					
			
				BuzzerTimer = _BTNTIME_;
			
				if(mode != _SETSUS_)
				{
					rlight ^= 1;
				
					if(rlight)
					{
						llight = 0;
						elight = 0;
						
						Right_Light_ON();
						Left_Light_OFF();
						Lignting_mode = _RIGHT_;
					}
					else
					{
						Right_Light_OFF();
						Left_Light_OFF();
						Lignting_mode = _ALLOFF_;
					}
				}
			
				break;
		}
	}
}

void keyscan_handler(void)										// timer4�� ���Ͽ� 30msec���� ������
{
	unsigned char code;
	static unsigned char setsus = 0, setsus_flag = _OFF_;
	
	switch(KeyScanState)
	{
		case KEY_STATE_UP:
			if(KeyIsKeyDown())
			{
				KeyScanState = KEY_STATE_DEBOUNCE;
				KeyDownTmr = 0;
			}
			break;
			
		case KEY_STATE_DEBOUNCE:
			if(KeyIsKeyDown())
			{
				code = KeyDecode();
				KeyBufIn(code);
				KeyRptStartDlyCtr = KEY_RPT_START_DLY;
				KeyScanState = KEY_STATE_RPT_START_DLY;			// �����ư�� ������� ����ư���� ������ϱ� ���� ��� ������ �����ǵ��� ���
				//KeyScanState = KEY_STATE_RPT_DLY;				// ���������� ������ 
			}
			else
			{
				KeySelRow(KEY_ALL_ROWS);
				KeyScanState = KEY_STATE_UP;
			}
			break;
			
		case KEY_STATE_RPT_START_DLY:
			if(KeyIsKeyDown())
			{
				if(KeyRptStartDlyCtr > 0)
				{
					KeyRptStartDlyCtr--;
				}
				
				if(KeyRptStartDlyCtr == 0)
				{
					KeyScanState = KEY_STATE_RPT_DLY;
				}
			}
			else
			{
				KeyScanState = KEY_STATE_DEBOUNCE;
			}
			break;
		
		case KEY_STATE_RPT_DLY:
			if(KeyIsKeyDown())
			{
				code = KeyDecode();
				
				if(code == _BUZZER_) 
				{
					BuzzerOccupation =_ON_;
					Buzzer_ON();
				}
				else if((code == _ELIGHT_)&(rpm_left == 0)&(rpm_right == 0))			// && �������� & ��Ʈ������
				{
					
					if(setsus_flag == _OFF_)
					{
						setsus ^= 1;
						setsus_flag = _ON_;
					}
					
					if(setsus)
					{
						mode = _SETSUS_;
						Right_Light_OFF();
						Left_Light_OFF();
						Lignting_mode = _ALLOFF_;
					}
					else
					{
						mode = _SPEED_;
						Right_Light_OFF();
						Left_Light_OFF();
						Lignting_mode = _ALLOFF_;
					}
				}
			}
			else
			{
				BuzzerOccupation = _OFF_;
				Buzzer_OFF();
				KeyScanState = KEY_STATE_DEBOUNCE;
				setsus_flag = _OFF_;
			}
			break;
	}
}				
