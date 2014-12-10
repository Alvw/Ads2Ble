#include "io430.h"
#include "RF.h"
#include "ascold.h"
#include "define.h"
#include "string.h"

//public
//unsigned char RF_RX_Maxlen = 30;
unsigned char RF_TX_Buf_Ready;
unsigned char RF_RX_DR;
unsigned char RF_TX_Buf[30];
unsigned char RF_RX_Maxlen = 30;
unsigned char RF_RX_Buf[30];
unsigned char RF_RX_Debug_buf[100];
unsigned char RF_RX_Debug_buf_resp[30];
unsigned char RF_TX_length;
unsigned char RF_RX_length;
unsigned int attributeHandle; 
unsigned char RF_Event;
unsigned char RF_Data_To_Send[20];
unsigned char isUART_Awake;
unsigned char RF_TX_DataWakeUpWaiting;
unsigned char RF_Resp_Waiting;
unsigned char RF_Connected;
unsigned char RF_Advertizing;
unsigned char RF_Advertizing_Timout_Counter;
unsigned char RF_Timeout_counter = 1;

//private
unsigned char debug_collision_cntr = 0;
unsigned char debug_collision_buf[10];
unsigned char tx_cntr;
unsigned char rx_cntr;
unsigned char rx_debug_cntr;
unsigned char rx_lost_interrupts;//for debug
unsigned char rx_lost_frames;//for debug
const unsigned char startAdvertiseCommand[] = {0x06, 0x00, 0x02, 0x06, 0x01, 0x02,0x02};
const unsigned char sendPacketPreample[] = {0x1C, 0x00, 0x18, 0x02, 0x00, 0x0F,0x00, 0x00, 0x14};
const unsigned char port_1_output[] = {0x06,0x00,0x02,0x07,0x03,0x01,0x03};// configure p1.0 and p1.1 as output

//private functions
void RF_UART_GoToSleep();
void on_RF_Event_Received();
void on_RF_Responce_Received();
void on_RF_System_Event_Received();
void on_RF_Hardware_Event_Received();
void on_RF_Unexpected_Event_Received();
void on_RF_Unexpected_Responce_Received();
void on_RF_System_Boot_Event_Received();
void on_RF_Hardware_Port_Status_Event_Received();
void on_RF_Connection_Event_Received();
void on_RF_Connection_Status_Event_Received();
void on_RF_Attribute_Database_Event_Received();
void on_RF_Attribute_Database_Status_Event_Received();
void on_RF_System_Protocol_Error_Event_Received();
void on_RF_Attribute_Client_Event_Received();
void on_RF_Attribute_Client_Indicated_Event_Received();
void on_RF_Attribute_Database_Responce_Received();
void on_RF_Attribute_Database_Write_Responce_Received();
void on_RF_Hardware_Responce_Received();
void on_RF_GAP_Responce_Received();
void on_RF_Connection_Disconnected_Event_Received();
void RF_SetPort_1_output();
void RF_Start_Adv_Command();



void RF_Init(){
      //configure UART 230400
	P3SEL = 0x30;                             // P3.4,5 = USCI_A0 TXD/RXD
	UCA0CTL1 |= UCSSEL_2;                     // SMCLC
	UCA0BR0 = 69;                              // 16,000 MHz  230400
	UCA0BR1 = 0;                             
	UCA0MCTL = UCBRS2;               		 // Modulation UCBRSx = 4
	UCA0CTL1 &= ~UCSWRST;                   // **Initialize USCI state machine**
	IE2 |= UCA0RXIE;         // Enable USCI_A0 RX interrupt
        //----------------
        RF_UART_GoToSleep();             //Ble wake up hi
        __delay_cycles(100000);
        off(RF_Reset);
       __delay_cycles(100000);
       on(RF_Reset);
       __delay_cycles(100000);
       RF_TX_Buf_Ready = 1;
       RF_Resp_Waiting = 0;
}

void RF_UART_GoToSleep(){
  off(RF_Wakeup);
  off(LED1);
  isUART_Awake = 0;
}

void RF_On_Timer_Handler(){
//  RF_Timeout_counter--;
//  if(RF_Timeout_counter==0){
//    RF_Timeout_counter = 10;
//    tx_cntr = 0;
//    rx_cntr = 0;
//    RF_Resp_Waiting = 0;
//    RF_TX_Buf_Ready = 1;
//    off(RF_Wakeup);
//  }
//  
//  if((RF_Connected == 0) && (RF_Advertizing == 0)){
//    RF_Advertizing_Timout_Counter++;
//    if(RF_Advertizing_Timout_Counter == 10){
//     RF_Advertizing_Timout_Counter = 0; 
//      RF_Start_Adv_Command();
//    }
//  }
}

//  UART RX ISR
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void) {
       RF_Timeout_counter = 10;  
  RF_RX_Debug_buf[rx_debug_cntr++] = UCA0RXBUF;
  if(rx_debug_cntr ==100) rx_debug_cntr = 0;
  if(RF_RX_DR){
    rx_lost_interrupts++;
    return;
  }
  RF_RX_Buf[rx_cntr] = UCA0RXBUF;
  //--------------
  switch (rx_cntr) 
  {
  case 0x00: 
    rx_cntr++;
    if((RF_RX_Buf[0]==0x80) || ((RF_RX_Buf[0]==0x00) && RF_Resp_Waiting) ){
      //do nothing
    } else {
      rx_lost_frames++;
      RF_RX_DR = 0;
      rx_cntr = 0;
    }  
//    if(RF_RX_Buf[0] == 0x00){
//      off(RF_Wakeup);
//    }
    break; 
  case 0x01:
    RF_RX_length = RF_RX_Buf[1] + 4;
    rx_cntr++;
         if(RF_RX_length > RF_RX_Maxlen){
            rx_lost_frames++;
            RF_RX_DR = 0;
            rx_cntr = 0;
         }
    break;
  case 0x02:
    rx_cntr++;
    if(RF_RX_Buf[2] > 0x07){//invalid value for message ID
      rx_lost_frames++;
      RF_RX_DR = 0;
      rx_cntr = 0;
    }
    break;
  default: 
    rx_cntr++;
    if(rx_cntr > (RF_RX_length - 1)){
      RF_RX_DR = 1;
      rx_cntr = 0;
      __bic_SR_register_on_exit(CPUOFF); // Не возвращаемся в сон при выходе
    }
    break;
  } 
}

void startRFSending() {
//  if(RF_RX_DR){
//    debug_collision_cntr++;
//  }
        if (RF_Resp_Waiting) return;
        RF_Resp_Waiting = 1;
	tx_cntr = 0;
	while (!(IFG2 & UCA0TXIFG));
        IFG2 &= ~UCA0TXIFG;//tx flag reset!!!!!!!!!
        IE2 |= UCA0TXIE;                        // Enable USCI_A0 TX interrupt
	UCA0TXBUF = RF_TX_Buf[tx_cntr++];	
} 

// UART Transmit ISR
#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void) {
  if(RF_RX_DR){
    debug_collision_buf[debug_collision_cntr++] = tx_cntr;
  }
    IFG2 &= ~UCA0TXIFG;
    RF_UART_TX_Int_Handler();   
   //__bic_SR_register_on_exit(CPUOFF + GIE); // Не возвращаемся в сон при выходе
}

void RF_UART_TX_Int_Handler(){
  UCA0TXBUF = RF_TX_Buf[tx_cntr++];           // TX next character
  if (tx_cntr > (RF_TX_length - 1)) {                 // TX over?
    IE2 &= ~UCA0TXIE;                       // Disable USCI_A0 TX interrupt
    RF_TX_Buf_Ready = 1;
  }
}

void RF_Start_Adv_Command(){
  if(!RF_TX_Buf_Ready) return;
  RF_TX_Buf_Ready = 0;
  RF_TX_length = 7;
  memcpy(RF_TX_Buf, startAdvertiseCommand, sizeof(startAdvertiseCommand));
  if(isUART_Awake){
    startRFSending();
  }else{
    RF_TX_DataWakeUpWaiting = 1;
    on(RF_Wakeup);
  }
}

void RF_SetPort_1_output(){//todo refactor
  if(!RF_TX_Buf_Ready) return;
  RF_TX_Buf_Ready = 0;
  RF_TX_length = 7;
  memcpy(RF_TX_Buf, port_1_output, sizeof(port_1_output));
  if(isUART_Awake){
    startRFSending();
  }else{
    RF_TX_DataWakeUpWaiting = 1;
    on(RF_Wakeup);
  }
}

void on_RF_RX_Data_Ready(){
  switch (RF_RX_Buf[0]) 
  {
  case 0x80: 
      on_RF_Event_Received();
    break; 
  case 0x00:
      on_RF_Responce_Received();
    break;
  default: 
    break;
  } 
}

void on_RF_Event_Received(){
  switch (RF_RX_Buf[2]) 
  {
  case 0x00: 
      on_RF_System_Event_Received();
    break; 
  case 0x02: 
      on_RF_Attribute_Database_Event_Received();
    break; 
  case 0x03: 
      on_RF_Connection_Event_Received();
    break; 
  case 0x04: 
      on_RF_Attribute_Client_Event_Received();
    break; 
  case 0x07:
      on_RF_Hardware_Event_Received();
    break;
  default:
    on_RF_Unexpected_Event_Received();
    break;
  } 
}

void on_RF_Attribute_Client_Event_Received(){
  switch (RF_RX_Buf[3]) 
  {
  case 0x00: 
    on_RF_Attribute_Client_Indicated_Event_Received();
    break; 
  default:
    on_RF_Unexpected_Event_Received();
    break;
  } 
}

void on_RF_Attribute_Client_Indicated_Event_Received(){
  //TACCR0 =0;
  //RF_Send_Packet();
}

void on_RF_Attribute_Database_Event_Received(){
  switch (RF_RX_Buf[3]) 
  {
  case 0x02: 
    on_RF_Attribute_Database_Status_Event_Received();
    break; 
  default:
    on_RF_Unexpected_Event_Received();
    break;
  } 
}

void on_RF_Attribute_Database_Status_Event_Received(){
//  if(RF_RX_Buf[6] == 0x02){//indicate status flag
//     attributeHandle = (RF_RX_Buf[5] << 8) + RF_RX_Buf[4];
//     RF_Event = 0x01; //start recording
//  } else 
    if(RF_RX_Buf[6] == 0x01){//notify status flag
    attributeHandle = (RF_RX_Buf[5] << 8) + RF_RX_Buf[4];
    RF_Event = 0x01; //start recording
  }
}

void on_RF_Connection_Event_Received(){
  switch (RF_RX_Buf[3]) 
  {
  case 0x00: 
    on_RF_Connection_Status_Event_Received();
    break; 
  case 0x04: 
    on_RF_Connection_Disconnected_Event_Received();
    break; 
  default:
    on_RF_Unexpected_Event_Received();
    break;
  } 
}

void on_RF_Connection_Disconnected_Event_Received(){
  RF_Connected = 0;
  //RF_Advertizing = 0;
  RF_Start_Adv_Command();
}

void on_RF_Connection_Status_Event_Received(){
  if(RF_RX_Buf[5] & 0x01){//conntction status = connected
     RF_Connected = 1;
     RF_Advertizing = 0;
  } else{//conntction status = disconnected
    //todo 
  }
}

void on_RF_System_Event_Received(){
  switch (RF_RX_Buf[3]) 
  {
    case 0x00: 
      on_RF_System_Boot_Event_Received();
    break; 
    case 0x06: 
      on_RF_System_Protocol_Error_Event_Received();
    break; 
  default:
    on_RF_Unexpected_Event_Received();
    break;
  } 
}

void on_RF_System_Protocol_Error_Event_Received(){
    //TACCR0 = 0; //stop timer
}

void on_RF_Hardware_Event_Received(){
  switch (RF_RX_Buf[3]) 
  {
  case 0x00: 
    on_RF_Hardware_Port_Status_Event_Received();
    break; 
  default:
    on_RF_Unexpected_Event_Received();
    break;
  } 
}

void on_RF_Unexpected_Event_Received(){
  //for(int i = 0; i<22; i++){
    //RF_RX_Debug_buf[i] = RF_RX_Buf[i];
 // }
}

void on_RF_Responce_Received(){
  RF_Resp_Waiting = 0;
  switch (RF_RX_Buf[2]) 
  {
  case 0x02: 
    on_RF_Attribute_Database_Responce_Received();
    break; 
  case 0x06: 
    on_RF_GAP_Responce_Received();
    break; 
  case 0x07: 
    on_RF_Hardware_Responce_Received();
    break; 
  default:
    on_RF_Unexpected_Responce_Received();
    break;
  } 
}

void on_RF_GAP_Responce_Received(){
  switch (RF_RX_Buf[3]) 
  {
  case 0x01: //GAP Set Avertising command responce received
    RF_UART_GoToSleep();
    RF_Advertizing = 1;
    break; 
  default:
    on_RF_Unexpected_Responce_Received();
    break;
  } 
}

void on_RF_Hardware_Responce_Received(){
  switch (RF_RX_Buf[3]) 
  {
  case 0x03: //hardware port direction responce received
    RF_Start_Adv_Command();
    break; 
  default:
    on_RF_Unexpected_Responce_Received();
    break;
  } 
}

void on_RF_Attribute_Database_Responce_Received(){
    switch (RF_RX_Buf[3]) 
  {
  case 0x00: 
    on_RF_Attribute_Database_Write_Responce_Received();
    break; 
  
  default:
    on_RF_Unexpected_Responce_Received();
    break;
  } 
}

void on_RF_Attribute_Database_Write_Responce_Received(){
  RF_UART_GoToSleep();
}


void on_RF_Unexpected_Responce_Received(){
  for(int i = 0; i<22; i++){
    RF_RX_Debug_buf_resp[i] = RF_RX_Buf[i];
  }
}

void on_RF_System_Boot_Event_Received(){
  RF_SetPort_1_output();
    //RF_Start_Adv_Command();
}

void on_RF_Hardware_Port_Status_Event_Received(){
  if((RF_RX_Buf[9] & 0x01) && (RF_RX_Buf[10] & 0x01)){ // wake up pin enabled
     isUART_Awake = 1;
     on(LED1);
     if(RF_TX_DataWakeUpWaiting){
        startRFSending();
        RF_TX_DataWakeUpWaiting = 0;
     }
  }
}

void RF_Send_Packet(){
  if(!RF_TX_Buf_Ready) return;
  RF_TX_Buf_Ready = 0;
  RF_TX_length = 29;
  memcpy(RF_TX_Buf, sendPacketPreample, sizeof(sendPacketPreample));
  for(int i = 0; i < 20; i++){
    RF_TX_Buf[i + 9] = RF_Data_To_Send[i];
  }
  if(isUART_Awake){
    startRFSending();
  }else{
    RF_TX_DataWakeUpWaiting = 1;
    on(RF_Wakeup);
  }
}


 