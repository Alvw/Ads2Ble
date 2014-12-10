#include "io430.h"
#include <stdbool.h>
#include "subroutine.h"
#include "ascold.h"
#include "delay.h"
#include "define.h"
#include "ads1292.h"
#include "ADC10.h"
#include "RF.h"

//private
unsigned int timerDivider;
unsigned char mainLoopCounter;
unsigned char* charBuff;
unsigned int packetCntr;
void on_RF_Message_Received();
void onTimerEvent();
void onRF_Host_Wakeup();
void assemblePacketAndSend();
void deepHibernate();
unsigned char rts;
const int INT_MAX_VAL = 32767;
const int INT_MIN_VAL = -32768;
unsigned char DRDYFG = 0;


void onTimerEvent(){
//  timerDivider++;
//  if(timerDivider > 1){
//    timerDivider = 0;
//    RF_On_Timer_Handler();
//  }
 
}

//void onRF_Host_Wakeup(){
    //do nothing
//}

//unsigned char checkAllIFG(){
//  return (TACTL & TAIFG)|| (P1IFG & BIT1)|| RF_RX_DR || RF_Event || (P1IFG & AFE_DRDY_PIN) || (IFG2 & UCA0RXIFG) || ((IFG2 & UCA0TXIFG) && (IE2 & UCA0TXIE));
//}

/* ---------------------------------- main ---------------------------------- */
void main( void )
{
  _CLI(); // Запрет прерываний на время инициализации
  Sys_Init();
  ADC10_Init();
  RF_Init();
  timerDivider = 0;
  _SEI(); // Разрешить прерывания
  while(1) 
  { 
        if(TACTL & TAIFG){//прерывание от таймера
          TACTL &= ~TAIFG;//сбросить флаг прерывания таймера
          onTimerEvent();
        }
//        if(IFG2 & UCA0RXIFG){
//          IFG2 &= ~UCA0RXIFG;
//          RF_UART_RX_Int_Handler();
//        }
//        if((IFG2 & UCA0TXIFG) && (IE2 & UCA0TXIE)){
//          IFG2 &= ~UCA0TXIFG;
//          RF_UART_TX_Int_Handler();
//        }
        //Не факт что сие событие надо обрабатывать. 
//        if (P1IFG & BIT1){ //прерывание радиомодуля
//          P1IFG &= ~BIT1;//сбросить флаг прерывания
//          onRF_Host_Wakeup();
//        }
        if(RF_RX_DR){//RF RX Data Ready
          on_RF_RX_Data_Ready();
          RF_RX_DR = 0;
        }
        if(RF_Event){//RF Message Received
          on_RF_Message_Received();
          RF_Event = 0;
        }
//        if (P1IFG & AFE_DRDY_PIN) {       // if DRDY fired
//          P1IFG &= ~AFE_DRDY_PIN;      // Clear DRDY flag
        if (DRDYFG) {       // if DRDY fired
          DRDYFG = 0;      // Clear DRDY flag
          if(rts > 0){
            rts = 0;
            RF_Send_Packet();
          }
          ADC10_Measure();              //start ADC10 conversion
          onAFE_DRDY();
        }
        if (AFE_Data_Buf_Ready) {       // AFE buffer ready to send
          //RF_On_Timer_Handler();
          if(RF_Connected){
            assemblePacketAndSend();
            AFE_Data_Buf_Ready = 0;
          }
        }
        mainLoopCounter++;
       // if(checkAllIFG() == 0){
          __bis_SR_register(CPUOFF + GIE); // Уходим в спящий режим 
       // }
  } // end of while()
} // end of main()
/* -------------------------------------------------------------------------- */
void on_RF_Message_Received(){
  switch (RF_Event){
  case 0x01: 
    if(!AFE_isRecording){
      AFE_Init();
      AFE_StartRecording();
    }
    //TIMER_A_START;
    break; 
//  case 0x02: 
//    RF_Connected = 0;
//    RF_Start_Adv_Command();
//    break; 
  default:
    break;
  }
}

void assemblePacketAndSend(){
  //Packer counter
  charBuff = (unsigned char *)&packetCntr;
  RF_Data_To_Send[0] = charBuff[0];
  RF_Data_To_Send[1] = charBuff[1];
  //данные ADS1292
   
  for(int i = 4; i > 0; i--){
    AFE_Data[i] = AFE_Data[i] - AFE_Data[i-1];
    if(AFE_Data[i] > INT_MAX_VAL) {
      AFE_Data[i] = INT_MAX_VAL; //ограничиваем значение, чтобы не выйти за 16 бит.
    }else if(AFE_Data[i] < INT_MIN_VAL) {
      AFE_Data[i] = -INT_MAX_VAL;
    }
  }
  charBuff = (unsigned char *) AFE_Data;
  //берем 3 байта для 1-й точки и по 2 для остальных 4-рех.
    RF_Data_To_Send[2] = charBuff[0];
    RF_Data_To_Send[3] = charBuff[1];
    RF_Data_To_Send[4] = charBuff[2];
  for(int i = 0; i < 4; i ++){
    RF_Data_To_Send[i*2 + 5] = charBuff[4 + i*4];
    RF_Data_To_Send[i*2 + 6] = charBuff[5 + i*4];
  }
  //6 байт результат ADC10. 3 оси акселерометра и напряжение батарейки.
    charBuff = (unsigned char *) ADC10_Data;
    //сначала младшие байты для 2-х значений
    RF_Data_To_Send[13] = charBuff[0];
    RF_Data_To_Send[14] = charBuff[2];
    //старшие байты склеенные в один байт
    RF_Data_To_Send[15] = (charBuff[1]<<4) + charBuff[3];
    //следующие 2 значения
    RF_Data_To_Send[16] = charBuff[4];
    RF_Data_To_Send[17] = charBuff[6];
    //старшие байты склеенные в один байт
    RF_Data_To_Send[18] = (charBuff[5]<<4) + charBuff[7];
   // Start RF sending 
    on(RF_Wakeup);
    rts = 2;
    packetCntr++;
}


void deepHibernate(){
  RF_UART_GoToSleep();
  off(RF_Reset);// выключаем радиомодуль
  //------------------
  
  AFE_RESET_OUT &= ~AFE_RESET_PIN;// выключаем ads1292
  ADC10CTL0 &= ~ADC10ON; 
  //-------------------------
  P1SEL = 0x00; // GPIO 
  P1DIR = 0xFF;                             // All P1.x outputs
  P1OUT = BIT5;   // выключаем акселерометр                             // All P1.x reset
  
  P2SEL = 0x00; // GPIO 
  P2DIR = 0xFF;                             // All P2.x outputs
  P2OUT = 0;                                // All P2.x reset
  
  P3SEL = 0x00; // GPIO 
  P3DIR = 0xFF;                             // All P3.x outputs
  P3OUT = 0;                                // All P3.x reset
  
  P4SEL = 0x00; // GPIO 
  P4DIR = 0xFF;                             // All P4.x outputs
  P4OUT = 0;                                // All P4.x reset 
  
  
 // P1OUT |= BIT5; // выключаем акселерометр
  //-------------------------
  __bis_SR_register(LPM4_bits); // Уходим в спящий режим 
}
/* -------------------------------------------------------------------------- */
/* ------------------------- Прерывание от таймера -------------------------- */
/* -------------------------------------------------------------------------- */
#pragma vector = TIMERA1_VECTOR
__interrupt void TimerA_ISR(void)
{    
  __bic_SR_register_on_exit(CPUOFF); // Не возвращаемся в сон при выходе
}
/* -------------------------------------------------------------------------- */ 



/* ------------------------ Прерывание от P1 ----------------------- */

#pragma vector = PORT1_VECTOR
__interrupt void Port1_ISR(void)
{
  P1IFG &= ~AFE_DRDY_PIN;      // Clear DRDY flag
  DRDYFG = 1;
  __bic_SR_register_on_exit(CPUOFF); // Не возвращаемся в сон при выходе
}
/* -------------------------------------------------------------------------- */