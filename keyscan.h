extern void KeyInitPort(void);
extern void KeyInit(void);
extern void KeySelRow(unsigned char row);
extern void keyscan_handler(void);
extern unsigned char KeyIsKeyDown(void);
extern unsigned char KeyGetCol(void);
extern unsigned char KeyHit(void);
extern unsigned char KeyGetKey(void);
extern void KeyScan_Thread(void);
