#include "io430.h"
#include "ADC10.h"

unsigned int ADC10_DMA_Data[4];
unsigned int ADC10_Data[4];
unsigned char ADC10_cntr;
//unsigned char* ADC10_CharBuff;

void ADC10_Init(){
  ADC10CTL0 = SREF_1;      // VREF = VREF+ - VSS
  ADC10CTL0 |= ADC10SHT_3; // S&H time = 64 CLKs
  ADC10CTL0 |= ADC10SR;    // 50 ksps max
  ADC10CTL0 |= REFBURST;   // REF Buffer On only while sample and conversion
  ADC10CTL0 |= REF2_5V;    // VREF = 2.5V
  ADC10CTL0 |= REFON;      // REF Generator On
  ADC10CTL0 |= MSC;         //Multiply sample and conversion
  ADC10CTL0 |= ADC10ON;     //ADC10 On
  
  ADC10CTL1 = INCH_3 + CONSEQ_1;        // A3/A2/A1/A0, single sequence
  ADC10DTC1 = 0x04;                         // 4 conversions
  ADC10AE0 |= 0x0F;                         // P2.3, 2.2, 2.1, 2.0 ADC10 option select
  ADC10_cntr = 0;
}

/* --------------------- Измерение АЦП по 4-м каналам -------------------- */
void ADC10_Measure()
{  
  if(ADC10_cntr == 0){//обнуляем значения
    for(int i = 0; i<4; i++){
      ADC10_Data[i] = ADC10_DMA_Data[i];
    } 
  }
  // Прибавляем результат прошлого измерения
  for(int i = 0; i<4; i++){
      ADC10_Data[i] += ADC10_DMA_Data[i];
  } 
  ADC10_cntr++;
  if(ADC10_cntr == 50){
    for(int i = 0; i<4; i++){
      ADC10_Data[i] /= 13;// обрезаем до 12-ти бит
    } 
    ADC10_cntr = 0;
  }
  ADC10CTL0 &= ~ENC;
  while (ADC10CTL1 & BUSY);               // Wait if ADC10 core is active
  ADC10SA = (int)ADC10_DMA_Data;                        // Data buffer start
  //ADC10CTL0 &= ~ADC10IFG;
  ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start 
}
/* -------------------------------------------------------------------------- */
