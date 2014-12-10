void Ble112_Init();
void Ble_Send_Packet();
void startAdv();
void bleGetEvt();
void sendDataToHost();

extern unsigned char in_buf[22];
extern unsigned char out_buf[22];
extern unsigned char len;
extern unsigned char data_ready;