/*
 * stm32_cpu_comm.h
 *
 *  Created on: 2013-10-20
 *      Author: David, zhaoshaowei@yeejoin.com
 */

#ifndef STM32_CPU_COMM_H_
#define STM32_CPU_COMM_H_

#include <stm32f10x.h>

/*
 * operate i/o pin
 */
#define clr_port_pin(port, pin)          (port)->BRR  = (pin)
#define set_port_pin(port, pin)          (port)->BSRR = (pin)
#define rolling_over_port_pin(port, pin) (port)->ODR ^= (pin)
#define pin_in_is_set(port, pin)         ((port)->IDR & (pin))
#define pin_out_is_set(port, pin)        ((port)->ODR & (pin))

/* bit[31:16]--BRy(bit reset), bit[15:0]--BSy(bit set) */
#define gpio_bits_reset(port, bits) ((port)->BSRR = (bits << 16))
#define gpio_bits_set(port, bits)   ((port)->BSRR = bits)


/*
 * usart
 */
#define disable_usartx_send_int(usartx)   do {(usartx)->CR1 &= ~(1<<7);} while (0)
#define enable_usartx_send_int(usartx)    do {(usartx)->CR1 |=  (1<<7);} while (0)
#define disable_usartx_recv_int(usartx)   do {(usartx)->CR1 &= ~(1<<5);} while (0)
#define enable_usartx_recv_int(usartx)    do {(usartx)->CR1 |=  (1<<5);} while (0)
/* 1 -- enable_usartx_send_int */
#define get_usartx_send_int_state(usartx) (!!((usartx)->CR1 & (1<<7)))

#define is_usartx_overrun_err(usartx)   ((usartx)->SR & (1<<3))
#define is_usartx_rxd_not_empty(usartx) ((usartx)->SR & (1<<5))
#define is_usartx_txd_empty(usartx)     ((usartx)->SR & (1<<7))
#define is_usartx_tc(usartx)		((usartx)->SR & (1<<6))
#define wait_usartx_send_over(usartx)	do{\
	while(!is_usartx_txd_empty(usartx));\
	while(!is_usartx_tc(usartx));\
}while(0)

#define clear_usartx_rxne_flag(usartx)  do{((usartx)->SR &= ~(1<<5));}while(0)
#define clear_usartx_tc_flag(usartx)  do{((usartx)->SR &= ~(1<<6));}while(0)

#define get_usartx_data(usartx)    ((usartx)->DR & 0x01ff)
#define put_usartx_data(usartx, data)    do { (usartx)->DR = (data) & 0x01ff;} while (0)



/*
 * timer
 */
#define enable_timerx(t)	(t)->CR1 |= TIM_CR1_CEN
#define disable_timerx(t)	(t)->CR1 &= (uint16_t)(~((uint16_t)TIM_CR1_CEN))
#define get_timer_cnt(t)	(t)->CNT

/* ITStatus TIM_GetITStatus(TIM_TypeDef* TIMx, uint16_t TIM_IT) */
#define is_timer_update_it_set(t)	(((t)->DIER & TIM_FLAG_Update) && ((t)->SR & TIM_FLAG_Update))


/*
 * exti
 */
/* void EXTI_ClearITPendingBit(uint32_t EXTI_Line) */
#define clear_exti_it_pending_bit(exti_line)	EXTI->PR = exti_line

/* ITStatus EXTI_GetITStatus(uint32_t EXTI_Line) */
#define is_exti_it_set(exti_line) ((0!=(EXTI->IMR & exti_line)) && (0!=(EXTI->PR & exti_line)))


enum i2c_err_e {
	IEE_OK	= 0,
	IEE_BUSY			= 1,
	IEE_TIMEOUT_MASTER_MODE		= 2, /* wait master mode select event timeout */
	IEE_TIMEOUT_MASTER_TRM_MODE	= 3, /* I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED */
	IEE_TIMEOUT_BTF			= 4, /* wait Byte transfer finished timeout */
	IEE_TIMEOUT_ADDR		= 5, /* Address sent (master mode)/matched (slave mode) */
	IEE_TIMEOUT_RXNE		= 6, /* Data register not empty (receivers) */
	IEE_TIMEOUT_STOP		= 7, /* Stop generation */
	IEE_TIMEOUT_BYTE_TRANSING	= 8, /* I2C_EVENT_MASTER_BYTE_TRANSMITTING */
	IEE_FAIL
};


extern struct rt_semaphore spi_sem_share_by_ade7880;

extern uint8_t spix_send_byte(SPI_TypeDef* SPIx, unsigned char writedat);
extern uint8_t spix_rec_byte(SPI_TypeDef* SPIx, unsigned dummy);
extern void spix_send_data(SPI_TypeDef* SPIx, const uint8_t* data, uint32_t data_len);
extern void spix_rec_data(SPI_TypeDef* SPIx, uint8_t* buffer, uint32_t buffer_len, unsigned dummy);

extern FunctionalState set_timx_int_enable_bit(TIM_TypeDef* TIMx, uint16_t TIM_IT, FunctionalState new_state);
extern FunctionalState set_exti_enable_bit(uint32_t exti_line, FunctionalState new_state);

extern enum i2c_err_e i2cx_read_byte(I2C_TypeDef *i2c_dev, unsigned char i2c_read_addr, unsigned char i2c_write_addr,
		unsigned reg_addr, int reg_addr_bits_num, unsigned char *reg_data);
extern enum i2c_err_e i2cx_write_byte(I2C_TypeDef *i2c_dev, unsigned char i2c_write_addr,
		int reg_addr, int reg_addr_bits_num, unsigned char byte);

extern enum i2c_err_e i2cx_read_bytes(I2C_TypeDef *i2c_dev, unsigned char i2c_read_addr, unsigned char i2c_write_addr,
		unsigned reg_addr, int reg_addr_bits_num, unsigned char *bytes, int byte_cnt);
extern enum i2c_err_e i2cx_write_bytes(I2C_TypeDef *i2c_dev, unsigned char i2c_write_addr,
		int reg_addr, int reg_addr_bits_num, unsigned char *bytes, int byte_cnt)
;


#endif /* STM32_CPU_COMM_H_ */
