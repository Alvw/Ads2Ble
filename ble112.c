#include "io430.h"
#include "ble112.h"
#include "define.h"
#include "ascold.h"
unsigned char in_cntr = 0;
unsigned char in_buf[22];
unsigned char out_buf[22];
unsigned char len;
unsigned char out_buf_len;
unsigned char data_ready;

void sendByteUart(unsigned char value) {
	while (!(IFG2 & UCA0TXIFG));
	UCA0TXBUF = value;
}

void startAdv(){
  in_cntr = 0;
  data_ready = 0;
  out_buf_len = 7;
  out_buf[0] = 0x06;
  out_buf[1] = 0x00;
  out_buf[2] = 0x02;
  out_buf[3] = 0x06;
  out_buf[4] = 0x01;
  out_buf[5] = 0x02;
  out_buf[6] = 0x02;
  on(LED);     
}

void sendDataToHost(){
  in_cntr = 0;
  data_ready = 0;
  out_buf_len = 14;
  out_buf[0] = 0x0D;
  out_buf[1] = 0x00;
  out_buf[2] = 0x09;
  out_buf[3] = 0x02;
  out_buf[4] = 0x00;
  out_buf[5] = 0x0F;
  out_buf[6] = 0x00;
  out_buf[7] = 0x00;
  out_buf[8] = 0x05;
  out_buf[9] = 0x12;
  out_buf[10] = 0x34;
  out_buf[11] = 0x56;
  out_buf[12] = 0x78;
  out_buf[13] = 0x78;
  on(LED); 
  //   sendByteUart(0x0D);
//   sendByteUart(0x00);
//   sendByteUart(0x09);
//   sendByteUart(0x02);
//   sendByteUart(0x00);
//   sendByteUart(0x0F);
//   sendByteUart(0x00);
//   sendByteUart(0x00);
//   sendByteUart(0x05);
//   sendByteUart(0x12);
//   sendByteUart(0x34);
//   sendByteUart(0x56);
//   sendByteUart(0x78);
//   sendByteUart(0x78);
}


void Ble112_Init(){
  //configure UART 230400
	P3SEL = 0x30;                             // P3.4,5 = USCI_A0 TXD/RXD
	UCA0CTL1 |= UCSSEL_2;                     // SMCLC
	UCA0BR0 = 69;                              // 16,000 MHz  230400
	UCA0BR1 = 0;                             
	UCA0MCTL = UCBRS2;               		 // Modulation UCBRSx = 4
	UCA0CTL1 &= ~UCSWRST;                   // **Initialize USCI state machine**
	IE2 |= UCA0RXIE;         // Enable USCI_A0 RX interrupt
        //----------------
//        off(LED);
        P1OUT &= ~BIT4;                 //Ble wake up low
        __delay_cycles(1000000);
       // P2OUT &= ~BIT4;                 //Ble reset low
        __delay_cycles(1000000);
       // P2OUT |= BIT4;                  //Ble reset hi
}

//void Ble_Send_Packet(){
//   sendByteUart(0x0D);
//   sendByteUart(0x00);
//   sendByteUart(0x09);
//   sendByteUart(0x02);
//   sendByteUart(0x00);
//   sendByteUart(0x0F);
//   sendByteUart(0x00);
//   sendByteUart(0x00);
//   sendByteUart(0x05);
//   sendByteUart(0x12);
//   sendByteUart(0x34);
//   sendByteUart(0x56);
//   sendByteUart(0x78);
//   sendByteUart(0x78);
//}
void Ble_Send_Packet(){
  for(int i = 0; i<out_buf_len; i++){
    sendByteUart(out_buf[i]);
  }
}

void bleGetEvt(){
  in_cntr = 0;
  data_ready = 0;
}

//  UART data from Ble112
//#pragma vector=USCIAB0RX_VECTOR
//__interrupt void USCI0RX_ISR(void) {
//    in_buf[in_cntr] = UCA0RXBUF;
//    if((in_buf[0]!=0x80) && (in_buf[0]!=0x00)){
//      return;
//    }
//    if(in_cntr == 1){
//      len = in_buf[in_cntr] + 4;
//    }
//    in_cntr++;
//    if(in_cntr == len){
//      data_ready = 1;
//    }
//    __bic_SR_register_on_exit(CPUOFF + GIE); // Не возвращаемся в сон при выходе
//}
