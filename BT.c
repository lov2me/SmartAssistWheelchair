#include "stm32f10x.h"                  						// STM32F10x.h definitions  
#include "global.h"

#define _CHECK_		0x01		// �۽������� ���۹��ۿ� ���������Ͱ� �ִ��� üũ
#define _WATING_	0x02		// ������ ������ ACK ���Ŵ��
#define _RESEND_	0x03		// 250ms�̳��� ACK�� ���ŵ��� ���� ��� ������

#define _FINDHEAD_	0x00		// ����� ã��
#define _FINDLENG_	0x01		// ���ŵ������� ���̸� ����

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
	packet[3] = cat;		// ī�װ�
	packet[4] = type;		// Ÿ��
	packet[5] = flag;		// ������Ŷ����
	packet[8] = val;		// ������
	packet[9] = packet[2] + packet[3] + packet[4] + packet[5] + packet[6] + packet[7] + packet[8];	// CRC

	for(i=0;i<11;i++)
	{
		TxBuff[TxPutPt][i] = packet[i];
	}
	
	Waiting_Sequence = packet[2];			// �� �Լ��� ���̽�ƽ�� SEND�Ҷ��� ����ϹǷ�
	
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
		
		if(Wating_Cnt > 250)				// 250msec ���� ������
		{
			TransferState = _RESEND_;
			Wating_Cnt = 0;
		}
	}
	else if(TransferState == _RESEND_)
	{
		Resend_Cnt++;
		
		if(Resend_Cnt >= 5)					// 5ȸ ����
		{
			TxGetPt++;						// ���̻� ������ ����
	
			if(TxGetPt == 3)
			{
				TxGetPt = 0;
			}
			
			TransferState = _CHECK_;		// ��� �����ʱ�ȭ
			Resend_Cnt = 0;
			Wating_Cnt = 0;
		}
		else
		{
			for(i=0;i<11;i++)				// ������
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
			else if((Rxdata == 0xff)&&(head_flag == TRUE))		// aa ff �� ���� ���
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
			
			if(Cnt == 4)										// LSB���� ����(��� �����ʹ� 1byte�̹Ƿ�)
			{
				if(USARTBuff[4] == 0x00)						// ACK or REQ
				{
					max = 7;									// ACK or REQ�� ��� �����Ͱ� �����Ƿ� 1byte �پ��
				}					
			}
			else if(Cnt == max)									// tail byte
			{
				if(Rxdata == TAIL)								// 0xed�� �� ������
				{
					acc = 0;
					
					for(Cnt=0;Cnt<=(max-2);Cnt++)				// ���������� �����ͱ���
					{
						acc += USARTBuff[Cnt];					// üũ�� ������ ����
					}
					
					if(acc == USARTBuff[max-1])					// üũ�� Ȯ��
					{
						RxPutData_buffer(USARTBuff, max);		// ���ۿ� ������ ����
					}
				}
																// ����ε� �����Ͱ� ���͵� ������ �ʱ�ȭ
				ParserState = _FINDHEAD_;						// ������ byte�� �ȸ°ų� üũ���� �ȸ����� ����
			}
			
			Cnt++;
		
			break;
	}
}

void RxPutData_buffer(unsigned char* packet, unsigned char length)		// �������� ��Ŷ�� ���ŵ� 
{
	unsigned char i;
	
	for(i=0;i<11;i++)							// ���� �ʿ������ �ϴ� ����Ŭ����
	{
		RxBuff[RxPutPt][i] = 0;
	}
	
	for(i=0;i<=length;i++)						// ��� ���ŵ� ��Ŷ�� ���ۿ� ����
	{
		RxBuff[RxPutPt][i] = packet[i];
	}
	
	if((packet[3] == F_SEND)||(packet[3] == F_REQ))		// SEND(length=8), REQ(length=7) ����
	{
		sequence = packet[0];					// ������ ����ȭ
		Response_Sequence = packet[0];			// SEND, REQ�� ���� ACK, RES�� ������ ���
		sequence++;
		
		if(sequence == 0x80)
		{
			sequence = 0;
		}
	}
	
	RxPutPt++;									// putī���� ����
	
	if(RxPutPt == 3)
	{
		RxPutPt = 0;
	}
}

void ChkRxBuffer(void)							// USART�� ���ŵ� ��Ŷ�� ������ ����
{		
	if(RxPutPt != RxGetPt)						// ó������ ���� ��Ŷ�� ����(VAILDȮ���� ��Ŷ)
	{
		if(RxBuff[RxGetPt][3] == F_ACK)			// ACK�������
		{
			if(Waiting_Sequence ==  RxBuff[RxGetPt][0])
			{
				Ack_Receive_flag = TRUE;
			}
		}
		else if(RxBuff[RxGetPt][3] == F_SEND)	// SAND�������
		{
			RxSendProcess();
		}
		else if(RxBuff[RxGetPt][3] == F_REQ)	// REQ�������
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

void RxSendProcess(void)						// SEND��Ŷ�� �޾����� ��� ���̽�
{	
	if(RxBuff[RxGetPt][1] == C_DISPLAY)
	{
		if(RxBuff[RxGetPt][2] == T_BATTERY)
		{
			// �ش����̽� ����
		}
		else if(RxBuff[RxGetPt][2] == T_SPEED)
		{
			// �ش����̽� ����
		}
	}
	else if(RxBuff[RxGetPt][1] == C_CONTROL)
	{
		if(RxBuff[RxGetPt][2] == T_HEADLIGHT)
		{
			TransferPacket(C_CONTROL,T_HEADLIGHT,F_ACK,0);
			headlight = RxBuff[RxGetPt][6];
			// ��ɻ����Ұ�
		}
		else if(RxBuff[RxGetPt][2] == T_EMERLIGHT)
		{
			TransferPacket(C_CONTROL,T_EMERLIGHT,F_ACK,0);
			emerlight = RxBuff[RxGetPt][6];
			// ��ɻ����Ұ�
		}
		else if(RxBuff[RxGetPt][2] == T_RIGHTTURN)
		{
			TransferPacket(C_CONTROL,T_RIGHTTURN,F_ACK,0);
			rightturn = RxBuff[RxGetPt][6];
			// ��ɻ����Ұ�
		}
		else if(RxBuff[RxGetPt][2] == T_LEFTTURN)
		{
			TransferPacket(C_CONTROL,T_LEFTTURN,F_ACK,0);
			leftturn = RxBuff[RxGetPt][6];
			// ��ɻ����Ұ�
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
				//TiltRecTimer = 1000;				// ��ư�Է´� 1�� ���ۿ���
				//TiltRecTimer = 100;					// ��ư�Է´� 0.1�� �������� ����
				TiltRecTimer = 150;					// ���η����� �ǽð��� �ƴ϶� 0.15������ �÷���� �������� �ʰ� ������ ���۵�
			}
			else if(RxBuff[RxGetPt][6] == 0x02)		// recline up
			{
				recline = _INCREASE_;
				tilt = _STOP_;
				//TiltRecTimer = 1000;
				//TiltRecTimer = 100;					// ��ư�Է´� 0.1�� �������� ����
				TiltRecTimer = 150;					// ���η����� �ǽð��� �ƴ϶� 0.15������ �÷���� �������� �ʰ� ������ ���۵�
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
				//TiltRecTimer = 100;					// ��ư�Է´� 0.1�� �������� ����
				TiltRecTimer = 150;					// ���η����� �ǽð��� �ƴ϶� 0.15������ �÷���� �������� �ʰ� ������ ���۵�
			}
			else if(RxBuff[RxGetPt][6] == 0x02)		// recline down
			{
				recline = _DECREASE_;
				tilt = _STOP_;
				//TiltRecTimer = 1000;
				//TiltRecTimer = 100;					// ��ư�Է´� 0.1�� �������� ����
				TiltRecTimer = 150;					// ���η����� �ǽð��� �ƴ϶� 0.15������ �÷���� �������� �ʰ� ������ ���۵�
			}
			else if(RxBuff[RxGetPt][6] == 0x00)		// speed down
			{
				if(speed > 0)
				{
					speed--;
				}
			}
		}
		else if(RxBuff[RxGetPt][2] == T_TILT)		// �º����� 100ms�������� ���ƿ�
		{
			TransferPacket(C_CONTROL,T_TILT,F_ACK,0);
			
			TiltAngle = RxBuff[RxGetPt][6];
			//AngleTimer = 100;						// ������Ŷ�� �ȳ������ ���� ����Ͽ� 0.1�ʸ� ī��Ʈ�Ͽ� �ڵ����� �������
			AngleTimer = 150;						// ���η����� �ǽð��� �ƴ϶� 0.15������ �÷���� �������� �ʰ� ������ ���۵�
		}
		else if(RxBuff[RxGetPt][2] == T_RECLINE)	// �º����� 100ms�������� ���ƿ�
		{
			TransferPacket(C_CONTROL,T_RECLINE,F_ACK,0);
			
			ReclineAngle = RxBuff[RxGetPt][6];
			//AngleTimer = 100;						// ������Ŷ�� �ȳ������ ���� ����Ͽ� 0.1�ʸ� ī��Ʈ�Ͽ� �ڵ����� �������
			AngleTimer = 150;						// ���η����� �ǽð��� �ƴ϶� 0.15������ �÷���� �������� �ʰ� ������ ���۵�
		}
		else if(RxBuff[RxGetPt][2] == T_EMERSTOP)	// ƿƮ��Ŭ���� �������
		{
			TransferPacket(C_CONTROL,T_EMERSTOP,F_ACK,0);
			
			AngleTimer = 0;
			TiltAngle = 0xff;
			ReclineAngle = 0xff;
		}
	}
	else if(RxBuff[RxGetPt][1] == C_SETTING)		// ���� �������� �˰��� ������� ����
	{
		if(RxBuff[RxGetPt][2] == T_FMAXSPEED)
		{
			TransferPacket(C_SETTING,T_FMAXSPEED,F_ACK,0);
			fmaxspeed = RxBuff[RxGetPt][6];
			// ��ɻ����Ұ�
		}
		else if(RxBuff[RxGetPt][2] == T_FMINSPEED)
		{
			TransferPacket(C_SETTING,T_FMINSPEED,F_ACK,0);
			fminspeed = RxBuff[RxGetPt][6];
			// ��ɻ����Ұ�
		}
		else if(RxBuff[RxGetPt][2] == T_FACC)
		{
			TransferPacket(C_SETTING,T_FACC,F_ACK,0);
			facc = RxBuff[RxGetPt][6];
			// ��ɻ����Ұ�
		}
		else if(RxBuff[RxGetPt][2] == T_FDEC)
		{
			TransferPacket(C_SETTING,T_FDEC,F_ACK,0);
			fdec = RxBuff[RxGetPt][6];
			// ��ɻ����Ұ�
		}
		else if(RxBuff[RxGetPt][2] == T_RMAXSPEED)
		{
			TransferPacket(C_SETTING,T_RMAXSPEED,F_ACK,0);
			rmaxspeed = RxBuff[RxGetPt][6];
			// ��ɻ����Ұ�
		}
		else if(RxBuff[RxGetPt][2] == T_RMINSPEED)
		{
			TransferPacket(C_SETTING,T_RMINSPEED,F_ACK,0);
			rminspeed = RxBuff[RxGetPt][6];
			// ��ɻ����Ұ�
		}
		else if(RxBuff[RxGetPt][2] == T_RACC)
		{
			TransferPacket(C_SETTING,T_RACC,F_ACK,0);
			racc = RxBuff[RxGetPt][6];
			// ��ɻ����Ұ�
		}
		else if(RxBuff[RxGetPt][2] == T_RDEC)
		{
			TransferPacket(C_SETTING,T_RDEC,F_ACK,0);
			rdec = RxBuff[RxGetPt][6];
			// ��ɻ����Ұ�
		}
		else if(RxBuff[RxGetPt][2] == T_TMAXSPEED)
		{
			TransferPacket(C_SETTING,T_TMAXSPEED,F_ACK,0);
			tmaxspeed = RxBuff[RxGetPt][6];
			// ��ɻ����Ұ�
		}
		else if(RxBuff[RxGetPt][2] == T_TMINSPEED)
		{
			TransferPacket(C_SETTING,T_TMINSPEED,F_ACK,0);
			tminspeed = RxBuff[RxGetPt][6];
			// ��ɻ����Ұ�
		}
		else if(RxBuff[RxGetPt][2] == T_TACC)
		{
			TransferPacket(C_SETTING,T_TACC,F_ACK,0);
			tacc = RxBuff[RxGetPt][6];
			// ��ɻ����Ұ�
		}
		else if(RxBuff[RxGetPt][2] == T_TDEC)
		{
			TransferPacket(C_SETTING,T_TDEC,F_ACK,0);
			tdec = RxBuff[RxGetPt][6];
			// ��ɻ����Ұ�
		}
	}
	else if(RxBuff[RxGetPt][1] == C_ALERT)
	{
		if(RxBuff[RxGetPt][2] == T_LOWBATT)
		{
			// �ش����̽� ����
		}
	}
	else if(RxBuff[RxGetPt][1] == C_ERROR)
	{
		if(RxBuff[RxGetPt][2] == T_OVERCURR)
		{
			// �ش����̽� ����
		}
		else if(RxBuff[RxGetPt][2] == T_OVERVOLT)
		{
			// �ش����̽� ����
		}
		else if(RxBuff[RxGetPt][2] == T_OVERTEMP)
		{
			// �ش����̽� ����
		}
	}
}
	
void RxReqProcess(void)							// REQ��Ŷ�� �޾����� ��� ���̽�
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
			// �ش���׾���
		}
		else if(RxBuff[RxGetPt][2] == T_UP)
		{
			// �ش���׾���
		}
		else if(RxBuff[RxGetPt][2] == T_DOWN)
		{
			// �ش���׾���
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
			// ��ɱ����ʿ�
		}
	}
	else if(RxBuff[RxGetPt][1] == C_ERROR)
	{
		if(RxBuff[RxGetPt][2] == T_OVERCURR)
		{
			TransferPacket(C_ERROR,T_OVERCURR,F_RES,0x00);
			// ��ɱ����ʿ�
		}
		else if(RxBuff[RxGetPt][2] == T_OVERVOLT)
		{
			TransferPacket(C_ERROR,T_OVERVOLT,F_RES,0x00);
			// ��ɱ����ʿ�
		}
		else if(RxBuff[RxGetPt][2] == T_OVERTEMP)
		{
			TransferPacket(C_ERROR,T_OVERTEMP,F_RES,0x00);
			// ��ɱ����ʿ�
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
		packet[3] = cat;		// ī�װ�
		packet[4] = type;		// Ÿ��
		packet[5] = flag;		// ������Ŷ����
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
		packet[3] = cat;		// ī�װ�
		packet[4] = type;		// Ÿ��
		packet[5] = flag;		// ������Ŷ����
// 		packet[6] = 0x01;		// LSB(��������� ���� ��� ����)
// 		packet[7] = 0x00;		// MSB(��������� ���� ��� ����)
		packet[8] = val;
		packet[9] = packet[2] + packet[3] + packet[4] + packet[5] + packet[6] + packet[7] + packet[8];		// CHECKSUM
		packet[10] = TAIL;
		
		for(i=0;i<11;i++)
		{
			USART2_PutChar(packet[i]);
		}
	}
}
