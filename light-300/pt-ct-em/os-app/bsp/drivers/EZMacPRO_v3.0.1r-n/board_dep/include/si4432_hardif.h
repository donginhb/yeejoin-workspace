/*
 * si4432_hardif.h
 *
 *  Created on: 2013-11-27
 *      Author: David, zhaoshaowei@yeejoin.com
 */

#ifndef HARD_IF_H_
#define HARD_IF_H_

#include <rtdef.h>
#include <stm32f10x.h>

#include <timer4ezmacpro.h>
#include <compiler_defs.h>


/*
 * NOTE:
 * 	EZMacPRO要使用cpu的spi/timer/exti
 *
 * 	在EZMacPRO_v3.0.1r中, EZmac使用的是, TIMER3/EXT0
 *
 * */

/* ======================================= *
 *   C O M M O N   D E F I N I T I O N S   *
 * ======================================= */
#ifndef TRUE
#define TRUE                            1
#endif

#ifndef FALSE
#define FALSE                           0
#endif



#if 0
#if defined (SYSCLK__FREQ_24_500MHZ)            /* Int. Osc. @ 24.5MHz */
#define SYSCLK_HZ                     (24500000)
#define SYSCLK_KHZ                    (SYSCLK_HZ/1000)
#define SYSCLK_DIV                    (0x00)
#elif defined (SYSCLK__FREQ_16_000MHZ)          /* Xtal @ 16MHz */
#define SYSCLK_HZ                     (16000000)
#define SYSCLK_KHZ                    (SYSCLK_HZ/1000)
#define SYSCLK_DIV                    (0x00)
#ifndef SOFTWARE_DEVELOPMENT_BOARD
#error "No XT2 on Si10xx testcard!"
#endif //SOFTWARE_DEVELOPMENT_BOARD
#elif defined (SYSCLK__FREQ_8_000MHZ)           /* Xtal @ 8MHz */
#define SYSCLK_HZ                     (8000000)
#define SYSCLK_KHZ                    (SYSCLK_HZ/1000)
#define SYSCLK_DIV                    (0x10)
#ifndef SOFTWARE_DEVELOPMENT_BOARD
#error "No XT2 on Si10xx testcard!"
#endif //SOFTWARE_DEVELOPMENT_BOARD
#elif defined (SYSCLK__FREQ_4_000MHZ)           /* Xtal @ 4MHz */
#define SYSCLK_HZ                     (4000000)
#define SYSCLK_KHZ                    (SYSCLK_HZ/1000)
#define SYSCLK_DIV                    (0x20)
#ifndef SOFTWARE_DEVELOPMENT_BOARD
#error "No XT2 on Si10xx testcard!"
#endif //SOFTWARE_DEVELOPMENT_BOARD
#else
#error "System Clock not defined!"
#endif
#endif

#define SYSCLK_HZ                     (72000000)
#define SYSCLK_KHZ                    (SYSCLK_HZ/1000)



#if C51_SYNTAX_
/*!
 * Disable watchdog.
 */
#define DISABLE_WATCHDOG()              PCA0MD &= ~0x40


/*!
 * Interrupt macros.
 */
#define ENABLE_GLOBAL_INTERRUPTS()      EA=1
#define DISABLE_GLOBAL_INTERRUPTS()     EA=0

#define ENABLE_EXT0_INTERRUPT()         EX0=1
#define DISABLE_EXT0_INTERRUPT()        EX0=0
#define CLEAR_EXT0_INTERRUPT()          IE0=0

#define ENABLE_MAC_INTERRUPTS()         ENABLE_EXT0_INTERRUPT(); \
                                        ENABLE_TIMER3_INTERRUPT()
#define DISABLE_MAC_INTERRUPTS()        DISABLE_EXT0_INTERRUPT(); \
                                        DISABLE_TIMER3_INTERRUPT()

#define ENABLE_MAC_EXT_INTERRUPT()      ENABLE_EXT0_INTERRUPT()
#define DISABLE_MAC_EXT_INTERRUPT()     DISABLE_EXT0_INTERRUPT()
#define CLEAR_MAC_EXT_INTERRUPT()       CLEAR_EXT0_INTERRUPT()

#define ENABLE_UART_INTERRUPT()         ENABLE_UART0_INTERRUPT()
#define SET_UART_INTERRUPT_FLAG()       SET_UART0_INTERRUPT_FLAG()
#define CLEAR_UART_INTERRUPT_FLAG()     CLEAR_UART0_INTERRUPT_FLAG()


/*!
 * Timer macros.
 */

#define TIMER0_LOW_BYTE                 TL0
#define TIMER0_HIGH_BYTE                TH0
#define TIMER3_LOW_BYTE                 TMR3L
#define TIMER3_HIGH_BYTE                TMR3H

#define ENABLE_TIMER0_INTERRUPT()       ET0=1
#define DISABLE_TIMER0_INTERRUPT()      ET0=0
#define CLEAR_TIMER0_INTERRUPT()        TF0=0
#define START_TIMER0()                  TR0=1
#define STOP_TIMER0()                   TR0=0

#define ENABLE_TIMER2_INTERRUPT()       ET2=1
#define DISABLE_TIMER2_INTERRUPT()      ET2=0
#define CLEAR_TIMER2_INTERRUPT()        TMR2CN &= ~0x80
#define START_TIMER2()                  TMR2CN |= 0x04
#define STOP_TIMER2()                   TMR2CN &= ~0x04

#define ENABLE_TIMER3_INTERRUPT()       EIE1 |= 0x80
#define DISABLE_TIMER3_INTERRUPT()      EIE1 &= ~0x80
#define CLEAR_TIMER3_INTERRUPT()        TMR3CN &= ~0x80
#define START_TIMER3()                  TMR3CN |= 0x04
#define STOP_TIMER3()                   TMR3CN &= ~0x04


#define TIMER_LOW_BYTE                  TIMER3_LOW_BYTE
#define TIMER_HIGH_BYTE                 TIMER3_HIGH_BYTE

#else

/*!
 * Disable watchdog.
 */
//#define DISABLE_WATCHDOG()


/*!
 * Interrupt macros.
 */

//#define ENABLE_GLOBAL_INTERRUPTS()
//#define DISABLE_GLOBAL_INTERRUPTS()

#if 1
/* 在cpu中禁止si4432的外部中断 */
#define disable_si4432_ext_int()	disable_si4432_exti()
#define restore_si4432_ext_int(flag)	restore_si4432_exti(flag)
#define clr_si4432_ext_int()		clr_si4432_exti()

/* 在cpu中禁止si4432的超时时钟中断 */
#define disable_si4432_timer_int()	disable_si4432_mac_timer_int()
#define restore_si4432_timer_int(flag)	restore_si4432_mac_timer_int(flag)
#define clr_si4432_timer_int()		clr_si4432_mac_timer_int()

#define ENABLE_MAC_INTERRUPTS()         enable_si4432_mac_int()
#define DISABLE_MAC_INTERRUPTS()        disable_si4432_mac_int()

#define ENABLE_MAC_EXT_INTERRUPT()      enable_si4432_exti()
#define DISABLE_MAC_EXT_INTERRUPT()     disable_si4432_exti()
#define CLEAR_MAC_EXT_INTERRUPT()       clr_si4432_exti()

#define ENABLE_MAC_TIMER_INTERRUPT()    enable_si4432_mac_timer_int()
#define DISABLE_MAC_TIMER_INTERRUPT()   disable_si4432_mac_timer_int()
#define CLEAR_MAC_TIMER_INTERRUPT()     clr_si4432_mac_timer_int()

#define START_MAC_TIMER()               start_si4432_mac_timer()
#define STOP_MAC_TIMER()                stop_si4432_mac_timer()
#else
/* 在cpu中禁止si4432的外部中断 */
#define disable_si4432_ext_int()	0
#define restore_si4432_ext_int(flag)
#define clr_si4432_ext_int()

/* 在cpu中禁止si4432的超时时钟中断 */
#define disable_si4432_timer_int()	0
#define restore_si4432_timer_int(flag)
#define clr_si4432_timer_int()

#define ENABLE_MAC_INTERRUPTS()
#define DISABLE_MAC_INTERRUPTS()

#define ENABLE_MAC_EXT_INTERRUPT()
#define DISABLE_MAC_EXT_INTERRUPT()
#define CLEAR_MAC_EXT_INTERRUPT()

#define ENABLE_MAC_TIMER_INTERRUPT()
#define DISABLE_MAC_TIMER_INTERRUPT()
#define CLEAR_MAC_TIMER_INTERRUPT()
#define START_MAC_TIMER()
#define STOP_MAC_TIMER()

#endif

#endif

#define TIMER_PRESCALER                 SI4432_MAC_TIMER_PRESCALER
#define TIMEOUT_US(n)                   (((U32)(n)*SYSCLK_KHZ)/(TIMER_PRESCALER*1000L))
/* 用于计算无线传输一个字节(8-bits)所需要的时间 */
#define BYTE_TIME(n)                    ((SYSCLK_HZ/n)*8/TIMER_PRESCALER)

//#define DELAY_2MS                       (U16)((SYSCLK_HZ/TIMER2_PRESCALER)/450)
//#define DELAY_1MS_TIMER2                (U16)((SYSCLK_HZ/TIMER2_PRESCALER)/900)
//#define DELAY_2MS_TIMER2                (U16)((SYSCLK_HZ/TIMER2_PRESCALER)/450)
//#define DELAY_5MS_TIMER2                (U16)((SYSCLK_HZ/TIMER2_PRESCALER)/180)
//#define DELAY_15MS_TIMER2               (U16)((SYSCLK_HZ/TIMER2_PRESCALER)/60)
//#define DELAY_100MS_TIMER0              (U32)((SYSCLK_HZ/TIMER0_PRESCALER)/10)
//#define DELAY_1000MS_TIMER0             (U32)((SYSCLK_HZ/TIMER0_PRESCALER))
//#define DELAY_3000MS_TIMER0             (U32)((SYSCLK_HZ/TIMER0_PRESCALER)*3)


/*!
 * UART baud rate.
 */
#define UART0_BAUDRATE                   (115200L)
//#define UART0_BAUDRATE                   (19200L)



extern void init_si4432_spi_pin(void);
extern void cfg_si4432_spi_param(void);
extern void init_si4432_mac_timer(uint16_t tim_period);
extern void start_si4432_mac_timer(void);
extern void stop_si4432_mac_timer(void);

extern void cfg_si4432_mac_timer_nvic(void);
extern void enable_si4432_mac_timer_int(void);
extern FunctionalState disable_si4432_mac_timer_int(void);
extern void restore_si4432_mac_timer_int(FunctionalState new_state);
extern void clr_si4432_mac_timer_int(void);

extern void init_si4432_exti(void);
extern void cfg_si4432_exti_nvic(void);
extern void enable_si4432_exti(void);
extern FunctionalState disable_si4432_exti(void);
extern void restore_si4432_exti(FunctionalState new_state);
extern void clr_si4432_exti(void);

extern void enable_si4432_mac_int(void);
extern ubase_t disable_si4432_mac_int(void);
extern void restore_si4432_mac_int(ubase_t flag);

#endif /* HARD_IF_H_ */
