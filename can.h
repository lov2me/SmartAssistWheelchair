extern CanTxMsg TxMessage;
extern CanRxMsg RxMessage;
extern unsigned char can_rx_flag;
extern signed short int rpm_left, rpm_right;

extern void CAN_Initialize(void);
extern void CANdataRx_Thread(void);
extern void Chk_GoDown(void);
extern void Chk_ManAutoStatus(void);
extern void Chk_ManAuto_L(void);
extern void Chk_ManAuto_R(void);
extern void Chk_CANRecover(void);
extern void Chk_CANActive(void);
extern void Chk_BatteryGage(void);
extern void CAN_Tx_data(unsigned short int id, unsigned char *data);
