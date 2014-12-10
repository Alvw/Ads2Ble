#include "io430.h"
#include "UART.h"
#include "ascold.h"

//public
unsigned char UART_TX_Buf_Ready;
unsigned char UART_RX_DR;
unsigned char UART_TX_Buf[22];
unsigned char UART_RX_Buf[22];
unsigned char UART_TX_length;
unsigned char UART_RX_length;

//private
unsigned char tx_cntr;
unsigned char rx_cntr;
unsigned char rx_lost_interrupts;//for debug
unsigned char rx_lost_frames;//for debug

void UART_Init(){
      //configure UART 230400
	P3SEL = 0x30;                             // P3.4,5 = USCI_A0 TXD/RXD
	UCA0CTL1 |= UCSSEL_2;                     // SMCLC
	UCA0BR0 = 69;                              // 16,000 MHz  230400
	UCA0BR1 = 0;                             
	UCA0MCTL = UCBRS2;               		 // Modulation UCBRSx = 4
	UCA0CTL1 &= ~UCSWRST;                   // **Initialize USCI state machine**
	IE2 |= UCA0RXIE;         // Enable USCI_A0 RX interrupt
        //----------------
        off(UART_Wakeup);               //Ble wake up low
        __delay_cycles(1000000);
        off(UART_Reset);
        __delay_cycles(1000000);
        on(UART_Reset);
        __delay_cycles(100000);
        on(UART_Wakeup);
}

//  UART RX ISR
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void) {
  if(UART_RX_DR){
    rx_lost_interrupts++;
    return;
  }
  UART_RX_Buf[rx_cntr] = UCA0RXBUF;
  if((UART_RX_Buf[0]!=0x80) && (UART_RX_Buf[0]!=0x00)){
    rx_lost_frames++;
    rx_cntr = 0;
    return;
  }
  if(rx_cntr == 1){
    UART_RX_length = UART_RX_Buf[rx_cntr] + 4;
  }
  rx_cntr++;
  if(rx_cntr > (UART_RX_length - 1)){
    UART_RX_DR = 1;
    rx_cntr = 0;
    __bic_SR_register_on_exit(CPUOFF + GIE); // Не возвращаемся в сон при выходе
  }
}

void startUartSending() {
	tx_cntr = 0;
	IE2 |= UCA0TXIE;                        // Enable USCI_A0 TX interrupt
	while (!(IFG2 & UCA0TXIFG));
	UCA0TXBUF = UART_TX_Buf[tx_cntr++];		
} 

// UART Transmit ISR
#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void) {
	if (tx_cntr > (UART_TX_length - 1)) {                 // TX over?
                UART_TX_Buf_Ready = 1;
		IE2 &= ~UCA0TXIE;                       // Disable USCI_A0 TX interrupt
                return;
	}
        UCA0TXBUF = UART_TX_Buf[tx_cntr++];           // TX next character
}
 