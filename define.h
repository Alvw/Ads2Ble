#ifndef _DEFINE_H_
#define _DEFINE_H_



#define _CLI() __disable_interrupt()
#define _SEI() __enable_interrupt()



// Стандартные объявления
#define ON    1
#define OFF   0
#define YES   1
#define NO    0

// Светодиод
#define LED     P3OUT, 7, H // Анод светодиода
#define LED1     P3OUT, 6, H // Анод светодиода

// Акселерометр
#define ACC_PWRDWN   P1OUT, 5, H 

// Каналы АЦП
//#define ECG_CH    0
//#define BATT_CH   4
#define ACC_X_CH  0
#define ACC_Y_CH  1
#define ACC_Z_CH  2

// Управление
#define BTN_PWR   P1IN,  0, L // Кнопка
#define PWR_CTRL  P1OUT, 1, H
#define BTN_PWROFF_TIMEOUT 50

// Пороги АЦП
//#define BATT_FULL_TH  837 // 3,80В
//#define BATT_MID_TH   805 // 3,65В
//#define BATT_LOW_TH   770 // 3,50В
#define BATT_EMPTY 520 // 3,05В



#endif  //_DEFINE_H_