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

#define _UP_			0			// 속도업
#define _HLIGHT_		1			// 헤드라이트
#define _LLIGHT_		2			// 좌측깜박이
#define _ELIGHT_		3			// 비상등
#define _DOWN_			4			// 속도다운
#define _BUZZER_		5			// 부져
#define _RLIGHT_		6			// 우측깜박이

static unsigned char KeyBuf[KEY_BUF_SIZE];
static unsigned char KeyBufInIx;
static unsigned char KeyBufOutIx;
static unsigned short int KeyDownTmr;
static unsigned char KeyNRead;
static unsigned char KeyScanState;
static unsigned char KeyRptStartDlyCtr;

void KeyInitPort(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;						// key 관련IO 설정구조선언
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);		// IO 클럭공급
	
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
	if(row == KEY_ALL_ROWS)										// 모든행을 0으로 출력함
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_5);
		GPIO_ResetBits(GPIOB, GPIO_Pin_6);
	}
	else if(row == 0)											// PB5 0으로 출력함
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_5);						
		GPIO_SetBits(GPIOB, GPIO_Pin_6);
	}
	else if(row == 1)											// PB6 0으로 출력함
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
	return (((~GPIO_ReadInputData(GPIOA)) >> 4) & 0x0f);		// PA11~12번을 읽어서 LSB로 쉬프트, 해당핀만 마스크, 반전(누르면 1되도록)
}

unsigned char KeyIsKeyDown(void)								// 키가 눌러졌는지 확인하고 키가 눌러진시간을 카운팅한다
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

unsigned char KeyDecode(void)									// 더 이상의 자세한 설명은 생략한다
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

void keyscan_handler(void)										// timer4에 의하여 30msec마다 동작함
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
				KeyScanState = KEY_STATE_RPT_START_DLY;			// 비상등버튼을 서스펜션 모드버튼으로 겸용사용하기 위해 길게 누르면 구별되도록 사용
				//KeyScanState = KEY_STATE_RPT_DLY;				// 부저반응을 빠르게 
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
				else if((code == _ELIGHT_)&(rpm_left == 0)&(rpm_right == 0))			// && 논리연산자 & 비트연산자
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
