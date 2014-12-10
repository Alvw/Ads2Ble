#ifndef DELAY_H
#define DELAY_H

#include "in430.h"

#define CPU_CLK_Hz 16000000 // Очень примерно
#define CPU_CLK_kHz (unsigned long)(CPU_CLK_Hz/1000)

#define delay_ns(x) __delay_cycles(x*CPU_CLK_kHz*0.000001)
#define delay_us(x) __delay_cycles(x*(CPU_CLK_Hz/1000000))
#define delay_ms(x) __delay_cycles(x*(CPU_CLK_Hz/1000))
#define delay_s(x)  __delay_cycles(x*CPU_CLK_Hz)

#endif
