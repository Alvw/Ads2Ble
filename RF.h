#define RF_Wakeup  P1OUT, 4, H
#define RF_Reset P2OUT, 4, H

extern unsigned char RF_RX_DR;
extern unsigned char RF_Event; //0x01 - start recording
extern unsigned char RF_Data_To_Send[20];
extern unsigned char RF_Connected;
extern unsigned char RF_Advertizing;
void RF_Init();
void on_RF_RX_Data_Ready();
void RF_Send_Packet();
void RF_UART_GoToSleep();
void RF_Start_Adv_Command();
void RF_On_Timer_Handler();
void RF_UART_TX_Int_Handler();
