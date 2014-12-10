//P4.4 - CS, P1.2 - DRDY, P4.5 - reset, P4.6 - start, P4.7 ClkSel
// CS
#define AFE_CS_DIR P4DIR
#define AFE_CS_OUT P4OUT
#define AFE_CS_PIN  BIT4
//Reset
#define AFE_RESET_DIR P4DIR
#define AFE_RESET_OUT P4OUT
#define AFE_RESET_PIN BIT5
//Start
#define AFE_START_DIR P4DIR
#define AFE_START_OUT P4OUT
#define AFE_START_PIN BIT6
//Clock Select
#define AFE_CLOCK_SELECT_DIR P4DIR
#define AFE_CLOCK_SELECT_OUT P4OUT
#define AFE_CLOCK_SELECT_PIN BIT7
//Data ready pin
#define AFE_DRDY_PIN BIT2

// SPI
#define TI_SPI_USCIB0_PxSEL  P3SEL
#define TI_SPI_USCIB0_PxDIR  P3DIR
#define TI_SPI_USCIB0_SIMO   BIT1
#define TI_SPI_USCIB0_SOMI   BIT2
#define TI_SPI_USCIB0_UCLK   BIT3

void AFE_Init();
void AFE_Cmd(unsigned char cmd) ;
void AFE_Write_Reg(unsigned char addr, unsigned char value);
unsigned char AFE_Read_Reg(unsigned char addr);
void onAFE_DRDY();//This function should be called in main cycle after DRDY
void AFE_StartRecording();
void AFE_StopRecording();
extern long AFE_Data[5];
extern unsigned char AFE_Data_Buf_Ready;
extern unsigned char AFE_isRecording;

#define AFE_CS_DELAY    __delay_cycles(160);//deleay before CS high
////-------------------------------------------------------
//// CS
//#define AFE_CS_DIR P4DIR
//#define AFE_CS_OUT P4OUT
//#define AFE_CS_PIN  BIT4
//
//AFE_Read_Data(*pBuf, length);
//AFE_Read_Data(&spiRxBuf[0], 9);
//#typedef uchar unsigned char 