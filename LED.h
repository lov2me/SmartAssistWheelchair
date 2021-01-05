extern void LED_Initialize(void);
extern void LED_ALL_ON(void);
extern void LED_ALL_OFF(void);
extern void LED_Shift_Data_Input(unsigned char cmd);
extern void LED_Storage_Register_Clock_Input(unsigned char cmd);
extern void LED_Shift_Register_Clock_Input(unsigned char cmd);
extern void LED_Master_Reset(unsigned char cmd);
extern void Refresh_All_LED(unsigned char mode, unsigned char speed, unsigned char battery, unsigned int errcode, unsigned char suslv);
