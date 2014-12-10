#include "io430.h"
#include <stdbool.h>
#include "subroutine.h"
#include "define.h"
#include "ascold.h"
#include "delay.h"

/* -------------------------- Инициализация таймера ------------------------- */
void TimerA_Init (void)
{
  // Таймер  
  TACTL_bit.TACLR  = 1; // Reset TAR, divider and count dir
  TACTL = TASSEL_2;     // SMCLK
  TACTL |= ID_2 + ID_1; // 1:8  
   TACCR0 = 0xFFFF;
  TACTL_bit.TAIE = 1;   // INT enable  
  TACTL &= ~TAIFG;      // Сброс прерывания
}
/* -------------------------------------------------------------------------- */



/* ----------------------------- Инициализация ------------------------------ */
void Sys_Init (void)
{
 // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;
  
 // CLOCK
  BCSCTL1 = CALBC1_16MHZ + DIVA_2;
  //BCSCTL3 = LFXT1S_2;
  DCOCTL = CALDCO_16MHZ;
  
  // GPIO & INT
  P1REN = BIT1 + BIT2 + BIT3 + BIT6; // Pull-UP/DOWN Resistors Disabled
  P1SEL = 0x00; // GPIO  
  P1DIR = BIT0 + BIT4 + BIT5 + BIT7; // OUTPUTs
  P1OUT = 0x00; // All LOW
 // P1IES |= BIT1;  // INT on rising edge
  //P1IFG &= ~BIT1; // Clear flag

  //P1IE |= BIT1;   // INT Enable
  
  P2REN = 0x00; // Pull-UP/DOWN Resistors Disabled
  P2SEL = BIT0 + BIT1 + BIT2 + BIT3; // ADC
  P2DIR = BIT4; //Outputs
  P2OUT = 0x00; // Подтяжка?  
  P2IE = 0x00;  // INT OFF
  
  P3REN = 0x00; // Pull-UP/DOWN Resistors Disabled
  //P3SEL = BIT0; // ADC 
  P3DIR |= BIT0 + BIT6 + BIT7; // OUTPUT
  P3OUT |= BIT6 + BIT7;
  
  P4REN = 0x00; // Pull-UP/DOWN Resistors Disabled
  P4SEL = 0x00;  
  P4DIR = BIT1 + BIT2 + BIT3 + BIT4 + BIT5 + BIT7; // OUTPUT
  //P4OUT = BIT1; // Set High
 
  // Таймер  
  TimerA_Init();
 // TIMER_A_START;
  
  //Светодиоты
  off(LED);
  off(LED1);
}
/* -------------------------------------------------------------------------- */



/* --------------------- Индикатор "питание" -------------------- */
void Pwr_Indication (void)
{
  off(LED);
  for (unsigned char cntr = 0; cntr < 6; cntr++) // Мигаем 3 раза
     {
       toggle(LED);
       delay_ms(100);
     } 
}
/* -------------------------------------------------------------------------- */