#include "timer.h"
#include "usart.h"
#include "LED.h"
#include "buzzer.h"
#include "power.h"
#include "keyscan.h"
#include "light.h"
#include "can.h"
#include "adc.h"

#define _HIGH_		1
#define _LOW_		0

#define TRUE		1
#define FALSE		0

#define _SPEED_			0
#define _POWEROFF_		1
#define _POWERON_		2
#define _ERROR_			3
#define _CANERR_L_		4
#define _CANERR_R_		5
#define _MANAUTO_L_		6
#define _MANAUTO_R_		7
#define _SETSUS_		8

#define _OFF_		0
#define _ON_		1

#define _ALLOFF_	0
#define _RIGHT_		1
#define _LEFT_		2
#define _EMERG_		3

#define _BTNTIME_	20

#define MAX(a,b) a>b?a:b
#define MIN(a,b) a<b?a:b
#define abs(x) ((x) < 0 ? (-(x)) : (x)) 

extern unsigned int Timer2_Counter; 

//////extern volatile unsigned char RxData;





//////extern unsigned char speed, battery;
extern __IO uint32_t ADC_DualConvertedValueTab[3];

extern unsigned char mode;
extern unsigned char speed;
extern unsigned char data[8];
extern unsigned char data2[8];
extern unsigned char data3[8];
extern unsigned char battery;
extern unsigned int errcode;
extern unsigned char CAN_flag_L;
extern unsigned char CAN_flag_R;
extern unsigned char sus_flag_L;						// 서스펜션이 잘정렬되었는지 확인
extern unsigned char sus_flag_R;						// 서스펜션이 잘정렬되었는지 확인
extern unsigned char suslv;

//////extern unsigned char sub_ref_l_scaled, sub_ref_r_scaled;

//////extern unsigned int TiltRecTimer;
//////extern unsigned char headlight, emerlight, rightturn, leftturn;
//////extern unsigned char TiltAngle;					// 임상실험용 프로토콜 추가
//////extern unsigned char ReclineAngle;				// 임상실험용 프로토콜 추가
//////extern unsigned int AngleTimer;					// 임상실험용 프로토콜 추가

extern void Boot_Sequence(void);
//////extern void Chk_TiltRecTimer(void);
//////extern void Chk_AngleTimer(void);
