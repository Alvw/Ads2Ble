#ifndef _SUBROUTINE_H_
#define _SUBROUTINE_H_

/* ---------- ��������� ������� ---------- */
void Sys_Init (void);    // ������������� ��
void TimerA_Init (void); // ������������� �������
void Pwr_Indication (void); // ������������ ��������� "�������"



/* ���������������� */
#define TIMER_A_START TACTL |= MC_1;   // Run timer, count to TACCR0
#define TIMER_A_STOP  TACTL &= MC_1;   // Stop timer



#endif  