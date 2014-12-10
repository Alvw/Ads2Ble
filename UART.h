#define UART_Wakeup P1OUT, 4, H
#define UART_Reset P2OUT, 4, H

extern unsigned char UART_TX_Buf_Ready;
extern unsigned char UART_TX_length;
extern unsigned char UART_RX_DR;
extern unsigned char UART_RX_length;
extern unsigned char UART_TX_Buf[22];
extern unsigned char UART_RX_Buf[22];
void UART_Start_TX();
void UART_Init();

