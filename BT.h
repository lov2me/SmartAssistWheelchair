// header
#define HEAD1			0xaa
#define HEAD2			0xff

// category
#define C_DISPLAY		0x11
#define C_CONTROL		0x12
#define C_SETTING		0x13
#define C_ALERT			0x14
#define C_ERROR			0x15

// type - C_DISPLAY
#define T_BATTERY		0x01
#define T_SPEED			0x02
#define T_TILTANGLE		0x03
#define T_RECLINEANGLE	0x04

// type - C_CONTROL
#define T_HEADLIGHT		0x01
#define T_EMERLIGHT		0x02	// emergency light
#define T_RIGHTTURN		0x03
#define T_LEFTTURN		0x04
#define T_BUZZER		0x05
#define T_UP			0x06
#define T_DOWN			0x07
#define T_TILT			0x08	// 틸트 각도값
#define T_RECLINE		0x09	// 리클라인 각도값
#define T_EMERSTOP		0x0a	// 틸트리클라인 정지

// type - C_SETTING
#define T_FMAXSPEED		0x01	// forward max 
#define T_FMINSPEED		0x02	// forward min
#define T_FACC			0x03	// forward accelleration
#define T_FDEC			0x04	// forward decelleration
#define T_RMAXSPEED		0x05	// reverse max 
#define T_RMINSPEED		0x06	// reverse min
#define T_RACC			0x07	// reverse accelleration
#define T_RDEC			0x08	// reverse decelleration
#define T_TMAXSPEED		0x09	// turn max 
#define T_TMINSPEED		0x0a	// turn min
#define T_TACC			0x0b	// turn accelleration
#define T_TDEC			0x0c	// turn decelleration

// type - C_ALERT
#define T_LOWBATT		0x01

// type - C_ERROR
#define T_OVERCURR		0x01	// over current
#define T_OVERVOLT		0x02	// over voltage
#define T_OVERTEMP		0x03	// over temperature

// flag 
#define F_SEND			0x01
#define F_ACK			0x02
#define F_REQ			0x03
#define F_RES			0x04

// tail
#define TAIL			0xed

extern unsigned char sequence;

extern void TxPutData_buffer(unsigned char cat, unsigned char type, unsigned char flag, unsigned char val);
extern void ChkTxBuffer(void);
extern void packet_parser(unsigned char Rxdata);
extern void RxPutData_buffer(unsigned char* packet, unsigned char length);
extern void ChkRxBuffer(void);
extern void RxSendProcess(void);
extern void RxReqProcess(void);
extern void TransferPacket(unsigned char cat, unsigned char type, unsigned char flag, unsigned char val);
