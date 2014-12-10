#ifndef _SUBROUTINE_H_
#define _SUBROUTINE_H_

/* ---------- Прототипы функций ---------- */
void Sys_Init (void);    // Инициализация МК
void TimerA_Init (void); // Инициализация таймера
void Pwr_Indication (void); // Светодиодный индикатор "питание"



/* Макроопределения */
#define TIMER_A_START TACTL |= MC_1;   // Run timer, count to TACCR0
#define TIMER_A_STOP  TACTL &= MC_1;   // Stop timer



#endif  