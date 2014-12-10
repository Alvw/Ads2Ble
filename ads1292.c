#include "io430.h"
#include "ads1292.h"
#include "define.h"
#include "ascold.h"

unsigned char AFE_isRecording;
unsigned char spiRxBuf[9];
long AFE_Data[5];
unsigned char AFE_Data_Buf_Ready;
//unsigned char AFE_SPI_RX_Cntr;
unsigned char* AFE_CharBuff;
unsigned char DRDY_cntr;
long ch1_value[1]; //helper variable
/******************************************************************************/
/*                      ADS1292 initialization and start up sequence          */
/******************************************************************************/
void AFE_Init(){
  //configure connections to ADS1292
  //P4.4 - CS, P1.2 - DRDY, P4.5 - reset, P4.6 - start, P4.7 ClkSel
  //CS
  AFE_CS_DIR |= AFE_CS_PIN;
  AFE_CS_OUT |= AFE_CS_PIN; 
  //Reset
  AFE_RESET_DIR |= AFE_RESET_PIN;
  AFE_RESET_OUT |= AFE_RESET_PIN; 
  //Start
  AFE_START_DIR |= AFE_START_PIN;
  AFE_START_OUT &= ~AFE_START_PIN; //start pin low 
  //Clock select pin
  AFE_CLOCK_SELECT_DIR |= AFE_CLOCK_SELECT_PIN;
  AFE_CLOCK_SELECT_OUT |= AFE_CLOCK_SELECT_PIN;
   
  //Spi setup
  UCB0CTL1 |= UCSWRST;                      // **Disable USCI state machine**
  UCB0CTL0 |= UCMST + UCMSB + UCSYNC;       // 3-pin, 8-bit SPI master
  UCB0CTL1 |= UCSSEL_2;                     // SMCLK
  UCB0BR0 = 0x10;                           // UCLK/16
  UCB0BR1 = 0;
  TI_SPI_USCIB0_PxSEL |= TI_SPI_USCIB0_SIMO | TI_SPI_USCIB0_SOMI | TI_SPI_USCIB0_UCLK;              // SPI option select
  TI_SPI_USCIB0_PxDIR |= TI_SPI_USCIB0_SIMO | TI_SPI_USCIB0_UCLK;                      // SPI TXD out direction
  UCB0CTL1 &= ~UCSWRST;                   // **Initialize USCI state machine**
  
  //initialize DRDY pin (P1.2)
  P1REN |= AFE_DRDY_PIN; // Pull-UP/DOWN Resistors Enabled
  P1IES |= AFE_DRDY_PIN;       // Interrupt on falling edge
  P1IFG &= ~AFE_DRDY_PIN;      // Clear flag
  P1IE |= AFE_DRDY_PIN;        // Enable interrupt on DRDY
  
  //start up sequence for ADS1292
  __delay_cycles(16000000);                  //Wait after power-up until reset
  AFE_RESET_OUT &= ~AFE_RESET_PIN;
  __delay_cycles(320);                         //Reset low width
  AFE_RESET_OUT |= AFE_RESET_PIN;
  __delay_cycles(800);                         //delay before using ADS1292
  AFE_Cmd(0x11);                                 //stop continious
  AFE_Write_Reg(0x44, 0x00);                 //Set Channel 1 to test
  AFE_Write_Reg(0x45, 0x81);                 //Set Channel 2 to Input Short and disable
  AFE_Write_Reg(0x41, 0x02);                //Set sampling ratio to 500 SPS
  AFE_Write_Reg(0x42, 0xE0);      	//Set internal reference PDB_REFBUF = 1; int test enable
  AFE_Write_Reg(0x46, 0x20);         	    //Turn on Drl.
  AFE_Write_Reg(0x49, 0x02); //Set mandatory bit. RLD REF INT doesn't work without it.
  AFE_Write_Reg(0x4A, 0x03);				//Set RLDREF_INT
  AFE_Cmd(0x10);                         //start continious
  //AFE_START_OUT |= AFE_START_PIN;                           //start pin hi
  AFE_isRecording = 0;
}

void AFE_StartRecording(){
  AFE_START_OUT |= AFE_START_PIN;                           //start pin hi
  AFE_isRecording = 1;
}
void AFE_StopRecording(){
  AFE_START_OUT &= ~AFE_START_PIN;                           //start pin lo
  AFE_isRecording = 0;
}

/******************************************************************************/
/*                             Функция обмена по SPI                          */
/* Принимает:  1 байт данных                                                  */
/* Возвращает: 1 байт данных                                                  */
/******************************************************************************/
unsigned char AFE_SPI_Exchange (unsigned char tx_data)
{
  unsigned char rx_data;
  while (!(IFG2 & UCB0TXIFG)); // Wait for TXBUF ready
  UCB0TXBUF = tx_data;         // Send data
  while (UCB0STAT & UCBUSY);   // Wait for TX to complete  
  rx_data = UCB0RXBUF;         // Read received data
  return rx_data;
}

void spiReadData() {
  AFE_CS_OUT &= ~AFE_CS_PIN;                           
  unsigned char i = 0;
  for (; i < 9; i++) {
    spiRxBuf[i] = AFE_SPI_Exchange(0x00);
  }
  AFE_CS_DELAY;
  AFE_CS_OUT |= AFE_CS_PIN;                            
} 

void AFE_Cmd(unsigned char cmd) {
  AFE_CS_OUT &= ~AFE_CS_PIN;                           
  AFE_SPI_Exchange(cmd);
  AFE_CS_DELAY;
  AFE_CS_OUT |= AFE_CS_PIN;                            
}

void AFE_Write_Reg(unsigned char addr, unsigned char value) {
  AFE_CS_OUT &= ~AFE_CS_PIN;                           
  AFE_SPI_Exchange(addr);
  AFE_SPI_Exchange(0x01);                      // Send number of bytes to write
  AFE_SPI_Exchange(value);                     // Send data
  AFE_CS_DELAY;
  AFE_CS_OUT |= AFE_CS_PIN;                     
}

unsigned char AFE_Read_Reg(unsigned char addr) {
  AFE_CS_OUT &= ~AFE_CS_PIN;                    // CS enable
  AFE_SPI_Exchange(addr);                       // Send address
  AFE_SPI_Exchange(0x01);                       // Send number bytes to read
  unsigned char x = AFE_SPI_Exchange(0x00);
  AFE_CS_DELAY;
  AFE_CS_OUT |= AFE_CS_PIN;                     // CS disable
  return x;
} 

void onAFE_DRDY(){
  spiReadData();
  AFE_CharBuff = (unsigned char *) ch1_value; 
  AFE_CharBuff[3] = spiRxBuf[3];
  AFE_CharBuff[2] = spiRxBuf[4];
  AFE_CharBuff[1] = spiRxBuf[5];
  ch1_value[0] = ch1_value[0] >> 8;
//  AFE_CharBuff[3] = 0xFF;
//  AFE_CharBuff[2] = 0x00;
//  AFE_CharBuff[1] = 0x55;
  
  if(DRDY_cntr == 0){
    for(int i =0; i<5; i++){
      AFE_Data[i] = 0;
    }
  }
  unsigned char sub_cntr = DRDY_cntr / 10;
  AFE_Data[sub_cntr] += ch1_value[0];
  DRDY_cntr++;
  if(DRDY_cntr == 50){
    for(int i =0; i<5; i++){
      AFE_Data[i] /= 10;
    }
    DRDY_cntr = 0;
    AFE_Data_Buf_Ready = 1;
  }
}