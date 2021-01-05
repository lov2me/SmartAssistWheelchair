#include "stm32f10x.h"                  						// STM32F10x.h definitions  
#include "global.h"

#define _CHECK_		0x01		// 송신측에서 전송버퍼에 보낼데이터가 있는지 체크
#define _WATING_	0x02		// 데이터 전송후 ACK 수신대기
#define _RESEND_	0x03		// 250ms이내에 ACK가 수신되지 않을 경우 재전송

#define _FINDHEAD_	0x00		// 헤더를 찾음
#define _FINDLENG_	0x01		// 수신데이터의 길이를 읽음

unsigned char sequence = 0;
unsigned char TxBuff[3][11];
unsigned char TxPutPt = 0, TxGetPt = 0;
unsigned char TransferState = _CHECK_;
unsigned int Wating_Cnt = 0;
unsigned char Resend_Cnt = 0;
unsigned char Ack_Receive_flag = FALSE;
unsigned char ParserState = _FINDHEAD_;
unsigned char RxBuff[3][11];
unsigned char RxPutPt = 0, RxGetPt = 0;
unsigned char Waiting_Sequence = 0;
unsigned char Response_Sequence = 0;

void TxPutData_buffer(unsigned char cat, unsigned char type, unsigned char flag, unsigned char val)
{
	unsigned char i;
	unsigned char packet[11] = {HEAD1,HEAD2,0,0,0,0,0x01,0,0,0,TAIL};
	
	packet[2] = sequence;
	packet[3] = cat;		// 카테고리
	packet[4] = type;		// 타입
	packet[5] = flag;		// 전송패킷종류
	packet[8] = val;		// 데이터
	packet[9] = packet[2] + packet[3] + packet[4] + packet[5] + packet[6] + packet[7] + packet[8];	// CRC

	for(i=0;i<11;i++)
	{
		TxBuff[TxPutPt][i] = packet[i];
	}
	
	Waiting_Sequence = packet[2];			// 이 함수는 조이스틱이 SEND할때만 사용하므로
	
	sequence++;
	
	if(sequence == 0x80)
	{
		sequence = 0;
	}
	
	TxPutPt++;
	
	if(TxPutPt == 3)
	{
		TxPutPt = 0;
	}
}

void ChkTxBuffer(void)
{
	unsigned char i;
	
	if(TransferState == _CHECK_)
	{
		if(TxPutPt != TxGetPt)
		{
			for(i=0;i<11;i++)
			{
				USART2_PutChar(TxBuff[TxGetPt][i]);
			}
			
			TransferState = _WATING_;
			Wating_Cnt = 0;
		}
	}
	else if(TransferState == _WATING_)
	{
		if(Ack_Receive_flag == TRUE)
		{
			TxGetPt++;
	
			if(TxGetPt == 3)
			{
				TxGetPt = 0;
			}
			
			TransferState = _CHECK_;
			Ack_Receive_flag = FALSE;
			Resend_Cnt = 0;
			Wating_Cnt = 0;
		}
		else
		{
			Wating_Cnt++;
		}
		
		if(Wating_Cnt > 250)				// 250msec 마다 재전송
		{
			TransferState = _RESEND_;
			Wating_Cnt = 0;
		}
	}
	else if(TransferState == _RESEND_)
	{
		Resend_Cnt++;
		
		if(Resend_Cnt >= 5)					// 5회 전송
		{
			TxGetPt++;						// 더이상 전송은 포기
	
			if(TxGetPt == 3)
			{
				TxGetPt = 0;
			}
			
			TransferState = _CHECK_;		// 모든 변수초기화
			Resend_Cnt = 0;
			Wating_Cnt = 0;
		}
		else
		{
			for(i=0;i<11;i++)				// 재전송
			{
				USART2_PutChar(TxBuff[TxGetPt][i]);
			}
			
			TransferState = _WATING_;
			Wating_Cnt = 0;
		}
	}
}

void packet_parser(unsigned char Rxdata)
{
	static unsigned char head_flag = FALSE;
	static unsigned char Cnt = 0, max = 8;
	static unsigned char USARTBuff[9] = {0,0,0,0,0,0,0,0,0};
	unsigned char acc;
	
	switch(ParserState)
	{
		case _FINDHEAD_ :
		
			if(Rxdata == 0xaa)
			{
				head_flag = TRUE;
			}
			else if((Rxdata == 0xff)&&(head_flag == TRUE))		// aa ff 가 들어올 경우
			{
				ParserState = _FINDLENG_;
				head_flag = FALSE;
				Cnt = 0;		
				max = 8;														
			}
			else
			{
				head_flag = FALSE;
			}
		
			break;
			
		case _FINDLENG_ :
			
			if(Cnt <= max)
			{
				USARTBuff[Cnt] = Rxdata;
			}
			
			if(Cnt == 4)										// LSB까지 받음(모든 데이터는 1byte이므로)
			{
				if(USARTBuff[4] == 0x00)						// ACK or REQ
				{
					max = 7;									// ACK or REQ의 경우 데이터가 없으므로 1byte 줄어듬
				}					
			}
			else if(Cnt == max)									// tail byte
			{
				if(Rxdata == TAIL)								// 0xed가 잘 들어왔음
				{
					acc = 0;
					
					for(Cnt=0;Cnt<=(max-2);Cnt++)				// 시퀀스에서 데이터까지
					{
						acc += USARTBuff[Cnt];					// 체크섬 데이터 생성
					}
					
					if(acc == USARTBuff[max-1])					// 체크섬 확인
					{
						RxPutData_buffer(USARTBuff, max);		// 버퍼에 데이터 넣음
					}
				}
																// 제대로된 데이터가 들어와도 어차피 초기화
				ParserState = _FINDHEAD_;						// 마지막 byte가 안맞거나 체크섬이 안맞으면 버림
			}
			
			Cnt++;
		
			break;
	}
}

void RxPutData_buffer(unsigned char* packet, unsigned char length)		// 정상적인 패킷이 수신됨 
{
	unsigned char i;
	
	for(i=0;i<11;i++)							// 별로 필요없지만 일단 버퍼클리어
	{
		RxBuff[RxPutPt][i] = 0;
	}
	
	for(i=0;i<=length;i++)						// 방금 수신된 패킷을 버퍼에 넣음
	{
		RxBuff[RxPutPt][i] = packet[i];
	}
	
	if((packet[3] == F_SEND)||(packet[3] == F_REQ))		// SEND(length=8), REQ(length=7) 수신
	{
		sequence = packet[0];					// 시퀀스 동기화
		Response_Sequence = packet[0];			// SEND, REQ에 따른 ACK, RES를 보낼때 사용
		sequence++;
		
		if(sequence == 0x80)
		{
			sequence = 0;
		}
	}
	
	RxPutPt++;									// put카운터 증가
	
	if(RxPutPt == 3)
	{
		RxPutPt = 0;
	}
}

void ChkRxBuffer(void)							// USART로 수신된 패킷이 있으면 동작
{		
	if(RxPutPt != RxGetPt)						// 처리되지 않은 패킷이 있음(VAILD확인한 패킷)
	{
		if(RxBuff[RxGetPt][3] == F_ACK)			// ACK받은경우
		{
			if(Waiting_Sequence ==  RxBuff[RxGetPt][0])
			{
				Ack_Receive_flag = TRUE;
			}
		}
		else if(RxBuff[RxGetPt][3] == F_SEND)	// SAND받은경우
		{
			RxSendProcess();
		}
		else if(RxBuff[RxGetPt][3] == F_REQ)	// REQ받은경우
		{
			RxReqProcess();
		}
		
		RxGetPt++;

		if(RxGetPt == 3)
		{
			RxGetPt = 0;
		}
	}
}

void RxSendProcess(void)						// SEND패킷을 받았을때 모든 케이스
{	
	if(RxBuff[RxGetPt][1] == C_DISPLAY)
	{
		if(RxBuff[RxGetPt][2] == T_BATTERY)
		{
			// 해당케이스 없음
		}
		else if(RxBuff[RxGetPt][2] == T_SPEED)
		{
			// 해당케이스 없음
		}
	}
	else if(RxBuff[RxGetPt][1] == C_CONTROL)
	{
		if(RxBuff[RxGetPt][2] == T_HEADLIGHT)
		{
			TransferPacket(C_CONTROL,T_HEADLIGHT,F_ACK,0);
			headlight = RxBuff[RxGetPt][6];
			// 기능삽입할것
		}
		else if(RxBuff[RxGetPt][2] == T_EMERLIGHT)
		{
			TransferPacket(C_CONTROL,T_EMERLIGHT,F_ACK,0);
			emerlight = RxBuff[RxGetPt][6];
			// 기능삽입할것
		}
		else if(RxBuff[RxGetPt][2] == T_RIGHTTURN)
		{
			TransferPacket(C_CONTROL,T_RIGHTTURN,F_ACK,0);
			rightturn = RxBuff[RxGetPt][6];
			// 기능삽입할것
		}
		else if(RxBuff[RxGetPt][2] == T_LEFTTURN)
		{
			TransferPacket(C_CONTROL,T_LEFTTURN,F_ACK,0);
			leftturn = RxBuff[RxGetPt][6];
			// 기능삽입할것
		}
		else if(RxBuff[RxGetPt][2] == T_BUZZER)
		{
			TransferPacket(C_CONTROL,T_BUZZER,F_ACK,0);
			
			if(RxBuff[RxGetPt][6] == 0x01)
			{
				Buzzer_ON();
				BuzzerTimer = 500;
			}
		}
		else if(RxBuff[RxGetPt][2] == T_UP)
		{
			TransferPacket(C_CONTROL,T_UP,F_ACK,0);
			
			if(RxBuff[RxGetPt][6] == 0x01)			// tilt up
			{
				tilt = _INCREASE_;
				recline = _STOP_;
				//TiltRecTimer = 1000;				// 버튼입력당 1초 동작에서
				//TiltRecTimer = 100;					// 버튼입력당 0.1초 동작으로 변경
				TiltRecTimer = 150;					// 메인루프가 실시간이 아니라서 0.15정도로 늘려줘야 끊어지지 않고 데이터 전송됨
			}
			else if(RxBuff[RxGetPt][6] == 0x02)		// recline up
			{
				recline = _INCREASE_;
				tilt = _STOP_;
				//TiltRecTimer = 1000;
				//TiltRecTimer = 100;					// 버튼입력당 0.1초 동작으로 변경
				TiltRecTimer = 150;					// 메인루프가 실시간이 아니라서 0.15정도로 늘려줘야 끊어지지 않고 데이터 전송됨
			}
			else if(RxBuff[RxGetPt][6] == 0x00)		// speed up
			{
				if(speed < 4)
				{
					speed++;
				}
			}
		}
		else if(RxBuff[RxGetPt][2] == T_DOWN)
		{
			TransferPacket(C_CONTROL,T_DOWN,F_ACK,0);
						
			if(RxBuff[RxGetPt][6] == 0x01)			// tilt down
			{
				tilt = _DECREASE_;
				recline = _STOP_;
				//TiltRecTimer = 1000;
				//TiltRecTimer = 100;					// 버튼입력당 0.1초 동작으로 변경
				TiltRecTimer = 150;					// 메인루프가 실시간이 아니라서 0.15정도로 늘려줘야 끊어지지 않고 데이터 전송됨
			}
			else if(RxBuff[RxGetPt][6] == 0x02)		// recline down
			{
				recline = _DECREASE_;
				tilt = _STOP_;
				//TiltRecTimer = 1000;
				//TiltRecTimer = 100;					// 버튼입력당 0.1초 동작으로 변경
				TiltRecTimer = 150;					// 메인루프가 실시간이 아니라서 0.15정도로 늘려줘야 끊어지지 않고 데이터 전송됨
			}
			else if(RxBuff[RxGetPt][6] == 0x00)		// speed down
			{
				if(speed > 0)
				{
					speed--;
				}
			}
		}
		else if(RxBuff[RxGetPt][2] == T_TILT)		// 태블릿에서 100ms간격으로 날아옴
		{
			TransferPacket(C_CONTROL,T_TILT,F_ACK,0);
			
			TiltAngle = RxBuff[RxGetPt][6];
			//AngleTimer = 100;						// 정지패킷이 안날라오는 것을 대비하여 0.1초를 카운트하여 자동으로 긴급정지
			AngleTimer = 150;						// 메인루프가 실시간이 아니라서 0.15정도로 늘려줘야 끊어지지 않고 데이터 전송됨
		}
		else if(RxBuff[RxGetPt][2] == T_RECLINE)	// 태블릿에서 100ms간격으로 날아옴
		{
			TransferPacket(C_CONTROL,T_RECLINE,F_ACK,0);
			
			ReclineAngle = RxBuff[RxGetPt][6];
			//AngleTimer = 100;						// 정지패킷이 안날라오는 것을 대비하여 0.1초를 카운트하여 자동으로 긴급정지
			AngleTimer = 150;						// 메인루프가 실시간이 아니라서 0.15정도로 늘려줘야 끊어지지 않고 데이터 전송됨
		}
		else if(RxBuff[RxGetPt][2] == T_EMERSTOP)	// 틸트리클라인 긴급정지
		{
			TransferPacket(C_CONTROL,T_EMERSTOP,F_ACK,0);
			
			AngleTimer = 0;
			TiltAngle = 0xff;
			ReclineAngle = 0xff;
		}
	}
	else if(RxBuff[RxGetPt][1] == C_SETTING)		// 아직 변수들이 알고리즘에 적용되지 않음
	{
		if(RxBuff[RxGetPt][2] == T_FMAXSPEED)
		{
			TransferPacket(C_SETTING,T_FMAXSPEED,F_ACK,0);
			fmaxspeed = RxBuff[RxGetPt][6];
			// 기능삽입할것
		}
		else if(RxBuff[RxGetPt][2] == T_FMINSPEED)
		{
			TransferPacket(C_SETTING,T_FMINSPEED,F_ACK,0);
			fminspeed = RxBuff[RxGetPt][6];
			// 기능삽입할것
		}
		else if(RxBuff[RxGetPt][2] == T_FACC)
		{
			TransferPacket(C_SETTING,T_FACC,F_ACK,0);
			facc = RxBuff[RxGetPt][6];
			// 기능삽입할것
		}
		else if(RxBuff[RxGetPt][2] == T_FDEC)
		{
			TransferPacket(C_SETTING,T_FDEC,F_ACK,0);
			fdec = RxBuff[RxGetPt][6];
			// 기능삽입할것
		}
		else if(RxBuff[RxGetPt][2] == T_RMAXSPEED)
		{
			TransferPacket(C_SETTING,T_RMAXSPEED,F_ACK,0);
			rmaxspeed = RxBuff[RxGetPt][6];
			// 기능삽입할것
		}
		else if(RxBuff[RxGetPt][2] == T_RMINSPEED)
		{
			TransferPacket(C_SETTING,T_RMINSPEED,F_ACK,0);
			rminspeed = RxBuff[RxGetPt][6];
			// 기능삽입할것
		}
		else if(RxBuff[RxGetPt][2] == T_RACC)
		{
			TransferPacket(C_SETTING,T_RACC,F_ACK,0);
			racc = RxBuff[RxGetPt][6];
			// 기능삽입할것
		}
		else if(RxBuff[RxGetPt][2] == T_RDEC)
		{
			TransferPacket(C_SETTING,T_RDEC,F_ACK,0);
			rdec = RxBuff[RxGetPt][6];
			// 기능삽입할것
		}
		else if(RxBuff[RxGetPt][2] == T_TMAXSPEED)
		{
			TransferPacket(C_SETTING,T_TMAXSPEED,F_ACK,0);
			tmaxspeed = RxBuff[RxGetPt][6];
			// 기능삽입할것
		}
		else if(RxBuff[RxGetPt][2] == T_TMINSPEED)
		{
			TransferPacket(C_SETTING,T_TMINSPEED,F_ACK,0);
			tminspeed = RxBuff[RxGetPt][6];
			// 기능삽입할것
		}
		else if(RxBuff[RxGetPt][2] == T_TACC)
		{
			TransferPacket(C_SETTING,T_TACC,F_ACK,0);
			tacc = RxBuff[RxGetPt][6];
			// 기능삽입할것
		}
		else if(RxBuff[RxGetPt][2] == T_TDEC)
		{
			TransferPacket(C_SETTING,T_TDEC,F_ACK,0);
			tdec = RxBuff[RxGetPt][6];
			// 기능삽입할것
		}
	}
	else if(RxBuff[RxGetPt][1] == C_ALERT)
	{
		if(RxBuff[RxGetPt][2] == T_LOWBATT)
		{
			// 해당케이스 없음
		}
	}
	else if(RxBuff[RxGetPt][1] == C_ERROR)
	{
		if(RxBuff[RxGetPt][2] == T_OVERCURR)
		{
			// 해당케이스 없음
		}
		else if(RxBuff[RxGetPt][2] == T_OVERVOLT)
		{
			// 해당케이스 없음
		}
		else if(RxBuff[RxGetPt][2] == T_OVERTEMP)
		{
			// 해당케이스 없음
		}
	}
}
	
void RxReqProcess(void)							// REQ패킷을 받았을때 모든 케이스
{	
	if(RxBuff[RxGetPt][1] == C_DISPLAY)
	{
		if(RxBuff[RxGetPt][2] == T_BATTERY)
		{
			TransferPacket(C_DISPLAY,T_BATTERY,F_RES,row_batt);
		}
		else if(RxBuff[RxGetPt][2] == T_SPEED)
		{
			TransferPacket(C_DISPLAY,T_SPEED,F_RES,speed);
		}
	}
	else if(RxBuff[RxGetPt][1] == C_CONTROL)
	{
		if(RxBuff[RxGetPt][2] == T_HEADLIGHT)
		{
			TransferPacket(C_CONTROL,T_HEADLIGHT,F_RES,headlight);
		}
		else if(RxBuff[RxGetPt][2] == T_EMERLIGHT)
		{
			TransferPacket(C_CONTROL,T_EMERLIGHT,F_RES,emerlight);
		}
		else if(RxBuff[RxGetPt][2] == T_RIGHTTURN)
		{
			TransferPacket(C_CONTROL,T_RIGHTTURN,F_RES,rightturn);
		}
		else if(RxBuff[RxGetPt][2] == T_LEFTTURN)
		{
			TransferPacket(C_CONTROL,T_LEFTTURN,F_RES,leftturn);
		}
		else if(RxBuff[RxGetPt][2] == T_BUZZER)
		{
			// 해당사항없음
		}
		else if(RxBuff[RxGetPt][2] == T_UP)
		{
			// 해당사항없음
		}
		else if(RxBuff[RxGetPt][2] == T_DOWN)
		{
			// 해당사항없음
		}
	}
	else if(RxBuff[RxGetPt][1] == C_SETTING)
	{
		if(RxBuff[RxGetPt][2] == T_FMAXSPEED)
		{
			TransferPacket(C_SETTING,T_FMAXSPEED,F_RES,fmaxspeed);
		}
		else if(RxBuff[RxGetPt][2] == T_FMINSPEED)
		{
			TransferPacket(C_SETTING,T_FMINSPEED,F_RES,fminspeed);
		}
		else if(RxBuff[RxGetPt][2] == T_FACC)
		{
			TransferPacket(C_SETTING,T_FACC,F_RES,facc);
		}
		else if(RxBuff[RxGetPt][2] == T_FDEC)
		{
			TransferPacket(C_SETTING,T_FDEC,F_RES,fdec);
		}
		else if(RxBuff[RxGetPt][2] == T_RMAXSPEED)
		{
			TransferPacket(C_SETTING,T_RMAXSPEED,F_RES,rmaxspeed);
		}
		else if(RxBuff[RxGetPt][2] == T_RMINSPEED)
		{
			TransferPacket(C_SETTING,T_RMINSPEED,F_RES,rminspeed);
		}
		else if(RxBuff[RxGetPt][2] == T_RACC)
		{
			TransferPacket(C_SETTING,T_RACC,F_RES,racc);
		}
		else if(RxBuff[RxGetPt][2] == T_RDEC)
		{
			TransferPacket(C_SETTING,T_RDEC,F_RES,rdec);
		}
		else if(RxBuff[RxGetPt][2] == T_TMAXSPEED)
		{
			TransferPacket(C_SETTING,T_TMAXSPEED,F_RES,tmaxspeed);
		}
		else if(RxBuff[RxGetPt][2] == T_TMINSPEED)
		{
			TransferPacket(C_SETTING,T_TMINSPEED,F_RES,tminspeed);
		}
		else if(RxBuff[RxGetPt][2] == T_TACC)
		{
			TransferPacket(C_SETTING,T_TACC,F_RES,tacc);
		}
		else if(RxBuff[RxGetPt][2] == T_TDEC)
		{
			TransferPacket(C_SETTING,T_TDEC,F_RES,tdec);
		}
	}
	else if(RxBuff[RxGetPt][1] == C_ALERT)
	{
		if(RxBuff[RxGetPt][2] == T_LOWBATT)
		{
			TransferPacket(C_ALERT,T_LOWBATT,F_RES,0x00);
			// 기능구현필요
		}
	}
	else if(RxBuff[RxGetPt][1] == C_ERROR)
	{
		if(RxBuff[RxGetPt][2] == T_OVERCURR)
		{
			TransferPacket(C_ERROR,T_OVERCURR,F_RES,0x00);
			// 기능구현필요
		}
		else if(RxBuff[RxGetPt][2] == T_OVERVOLT)
		{
			TransferPacket(C_ERROR,T_OVERVOLT,F_RES,0x00);
			// 기능구현필요
		}
		else if(RxBuff[RxGetPt][2] == T_OVERTEMP)
		{
			TransferPacket(C_ERROR,T_OVERTEMP,F_RES,0x00);
			// 기능구현필요
		}
	}
}

void TransferPacket(unsigned char cat, unsigned char type, unsigned char flag, unsigned char val)
{
	unsigned char i;
	unsigned char packet[11] = {HEAD1,HEAD2,0,0,0,0,0x01,0,0,0,TAIL};

	if(flag == F_ACK)
	{
		// example : TransferPacket(C_DISPLAY,T_SPEED,F_ACK,0);
		
		packet[2] = Response_Sequence;
		packet[3] = cat;		// 카테고리
		packet[4] = type;		// 타입
		packet[5] = flag;		// 전송패킷종류
		packet[6] = val;		// LSB
		packet[7] = val;		// MSB
		packet[8] = packet[2] + packet[3] + packet[4] + packet[5] + packet[6] + packet[7];		// CHECKSUM
		packet[9] = TAIL;
		
		for(i=0;i<10;i++)
		{
			USART2_PutChar(packet[i]);
		}
	}
	else if(flag == F_RES)
	{
		// example : TransferPacket(C_DISPLAY,T_BATTERY,F_RES,row_batt);
		
		packet[2] = Response_Sequence;
		packet[3] = cat;		// 카테고리
		packet[4] = type;		// 타입
		packet[5] = flag;		// 전송패킷종류
// 		packet[6] = 0x01;		// LSB(변수선언시 값이 들어 있음)
// 		packet[7] = 0x00;		// MSB(변수선언시 값이 들어 있음)
		packet[8] = val;
		packet[9] = packet[2] + packet[3] + packet[4] + packet[5] + packet[6] + packet[7] + packet[8];		// CHECKSUM
		packet[10] = TAIL;
		
		for(i=0;i<11;i++)
		{
			USART2_PutChar(packet[i]);
		}
	}
}
