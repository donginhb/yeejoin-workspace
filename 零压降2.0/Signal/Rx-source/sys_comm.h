#ifndef SYS_COMM_H_
#define SYS_COMM_H_ 1

#include ".\fwlib\stm32f10x.h"
#include "common.h"

#define set_bit(bit_vector, mask)     ((bit_vector) |=  (mask))
#define clr_bit(bit_vector, mask)     ((bit_vector) &= ~(mask))
#define reverse_bit(bit_vector, mask) ((bit_vector) ^=  (mask))
#define is_bit_set(bit_vector, mask)  (((bit_vector) & (mask)))
#define is_bit_clr(bit_vector, mask)  (!((bit_vector) & (mask)))

#define sub_abs(x, y) ((x)>(y) ? (x)-(y) : (y)-(x))

#ifndef MIN
#define MIN(a, b) ((a)>(b) ? (b) : (a))
#endif

#ifndef MAX
#define MAX(a, b) ((a)>(b) ? (a) : (b))
#endif


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
#define enable_usartx(usartx)		do {(usartx)->CR1 |=  (1<<13);} while(0)
#define disable_usartx(usartx)		do {(usartx)->CR1 &= ~(1<<13);} while(0)

#define disable_usartx_send_int(usartx)   do {(usartx)->CR1 &= ~(1<<7);} while (0)
#define enable_usartx_send_int(usartx)    do {(usartx)->CR1 |=  (1<<7);} while (0)
#define disable_usartx_recv_int(usartx)   do {(usartx)->CR1 &= ~(1<<5);} while (0)
#define enable_usartx_recv_int(usartx)    do {(usartx)->CR1 |=  (1<<5);} while (0)
/* 1 -- enable_usartx_send_int */
#define get_usartx_send_int_state(usartx) (!!((usartx)->CR1 & (1<<7)))


#define is_usartx_overrun_err(usartx)   ((usartx)->SR & (1<<3))
#define is_usartx_rxd_not_empty(usartx) ((usartx)->SR & (1<<5))
#define is_usartx_txd_empty(usartx)     ((usartx)->SR & (1<<7))
#define clear_usartx_rxne_flag(usartx)  ((usartx)->SR & ~(1<<5))


#define get_usartx_data(usartx)    ((usartx)->DR & 0x01ff)
#define put_usartx_data(usartx, data)    do { (usartx)->DR = (data) & 0x01ff;} while (0)


/*
 * watch dog
 */
#define iwdg_reloadcounter  do {IWDG->KR = KR_KEY_Reload;} while (0)


/*
 * timer
 */

/* One Pulse Mode */
#define set_timer_to_opm(timer) do {timer->CR1 |=  1<<3;} while (0)

#define enable_timer(timer)     do {timer->CR1 |=  1;} while (0)
#define disable_timer(timer)    do {timer->CR1 &= ~1;} while (0)


/*
 * SPI
 */
/* SPI SPE mask */
#define CR1_SPE_Set          ((uint16_t)0x0040)
#define CR1_SPE_Reset        ((uint16_t)0xFFBF)

#define enable_spix(spix)  do {spix->CR1 |= CR1_SPE_Set;} while (0)
#define disable_spix(spix) do {spix->CR1 &= CR1_SPE_Reset;} while (0)

#define is_spix_txbuf_empty(spix) (spix->SR & 0x02)
#define is_spix_txbuf_not_empty(spix) (!(spix->SR & 0x02))
#define is_spix_busy(spix)        (spix->SR & 0x80)


/*
 * dma
 */
/* DMA ENABLE mask */
#define CCR_ENABLE_Set          ((uint32_t)0x00000001)
#define CCR_ENABLE_Reset        ((uint32_t)0xFFFFFFFE)

/* gif -- global interrupt flag, tcif -- transfer complete interrupt flag, htif -- half transfer, teif -- transfer error
 * These bits are set by hardware and cleared by software writing 1 to the corresponding bit in the DMA_IFCR register.
 *
 * TEIFx:Channel x transfer error flag (x = 1 ..7)
 * 0: No transfer error (TE) on channel x
 * 1: A transfer error (TE) occurred on channel x
 * 
 * HTIFx:Channel x half transfer flag (x = 1 ..7)
 * 0: No half transfer (HT) event on channel x
 * 1: A half transfer (HT) event occurred on channel x
 * 
 * TCIFx:Channel x transfer complete flag (x = 1 ..7)
 * 0: No transfer complete (TC) event on channel x
 * 1: A transfer complete (TC) event occurred on channel x
 * 
 * GIFx:Channel x global interrupt flag (x = 1 ..7)
 * 0: No TE, HT or TC event on channel x
 * 1: A TE, HT or TC event occurred on channel x
 */
#define dma_int_flag_bit_gifx(channel_num)   (0x01 << ((channel_num-1) * 4))
#define dma_int_flag_bit_tcifx(channel_num)  (0x02 << ((channel_num-1) * 4))
#define dma_int_flag_bit_htifx(channel_num)  (0x04 << ((channel_num-1) * 4))
#define dma_int_flag_bit_teifx(channel_num)  (0x08 << ((channel_num-1) * 4))

#define is_dmax_channely_gif_set(dmax,  channel_num) (dmax->ISR & dma_int_flag_bit_gifx(channel_num))
#define is_dmax_channely_tcif_set(dmax, channel_num) (dmax->ISR & dma_int_flag_bit_tcifx(channel_num))
#define is_dmax_channely_htif_set(dmax, channel_num) (dmax->ISR & dma_int_flag_bit_htifx(channel_num))
#define is_dmax_channely_teif_set(dmax, channel_num) (dmax->ISR & dma_int_flag_bit_teifx(channel_num))

#define clear_dmax_channely_gif(dmax,  channel_num)  (dmax->IFCR = dma_int_flag_bit_gifx(channel_num))
#define clear_dmax_channely_tcif(dmax, channel_num)  (dmax->IFCR = dma_int_flag_bit_tcifx(channel_num))
#define clear_dmax_channely_htif(dmax, channel_num)  (dmax->IFCR = dma_int_flag_bit_htifx(channel_num))
#define clear_dmax_channely_teif(dmax, channel_num)  (dmax->IFCR = dma_int_flag_bit_teifx(channel_num))


#define enable_dmax_y(dmax_y)  dmax_y->CCR |= CCR_ENABLE_Set
#define disable_dmax_y(dmax_y) dmax_y->CCR &= CCR_ENABLE_Reset


/*
 * ****************************************************************************
 */
struct adj_zvd_param_st {
        s32 pa_amplify_adj;             /* default value: 0x8000 */
        s32 pb_amplify_adj;
        s32 pc_amplify_adj;

        s32 pa_phase_time_adj;          /* default value: 0x8000 */
        s32 pb_phase_time_adj;
        s32 pc_phase_time_adj;

        s32 pa_zeropos_adj_val;       /* default value: 0x00 */
        s32 pb_zeropos_adj_val;
        s32 pc_zeropos_adj_val;

        s32 pa_zeropos_real_val;       /* default value: 0x00 */
        s32 pb_zeropos_real_val;
        s32 pc_zeropos_real_val;

        s32 pa_amp_factor_positive_adj;   /* amplification_factor default value: 0x8000 */
        s32 pb_amp_factor_positive_adj;
        s32 pc_amp_factor_positive_adj;

        s32 pa_amp_factor_negtive_adj;    /* default value: 0x8000 */
        s32 pb_amp_factor_negtive_adj;
        s32 pc_amp_factor_negtive_adj;
};

#if USE_OPTICX_200S_VERSION
#define FIBER_CHANNEL_NUM_MAX 2
#else
#define FIBER_CHANNEL_NUM_MAX 1
#endif


struct signal_cfg_param_tbl {
        u32 magic_num;

        struct adj_zvd_param_st px_adj_param[FIBER_CHANNEL_NUM_MAX];

        /*
         * 使用通道指示信号
         *
         * 假设继电器输出类似与开漏极结构，并且有效时为低电平
         * 我们的硬件把该信号通过反相器后链接到cpu，也就是说cpu引脚读到0，表示未使用
         *
         * channel_valid_indication_level
         * 	0 -- 表示低电平有效(外部硬件接口电平)，是channel_valid_indication_level的默认值
         * 	1 -- 表示高电平有效(外部硬件接口电平)
         * */
        u8 channel_valid_indication_level;
        u8 pad[3];
};

#define SUCC 0
#define FAIL 1

extern int i2str(char *str, int n);
extern int strlen_(char *str);

extern char *strncpy(char *dst, const char *src, int n);
extern int strncmp(const char *cs, const char *ct, int count);

/*
 * independent watchdog
 */
#define USE_STM32_IWDG 1

/*
 * window watchdog
 *
 * tWWDG = tPCLK1 * 4096 * 2^WDGTB * (t[5:0]+1)  (ms)
 *
 * PCLK1 -- 36MHz max
 * tWWDG = 1/36 * 10^-3 * 4096 * 2^3 * (t[5:0]+1)
 *       = 1/36 * 32768 * 10^-3 * (t[5:0]+1)
 *	 = 910.22 * 10^-3  * (t[5:0]+1) (ms)
 *
 * if t[5:0]=0x3f then tWWDG = 58.25ms
 * reset system when t[6] change from 1 to 0
 */
#define USE_STM32_WWDG 0
#define WWDG_RELOAD_VALUE (0X7f)


extern void usr_cmd_analysis(void);
extern int is_cmd_valid(int cmd);
extern void usr_cmd_proc(int cmd, s32 param);


typedef u8 (*pf_putdata2buf)(u8 dat);
typedef u8 (*pf_getbufdata)(void);

void usartx_use_buf_tx(USART_TypeDef *usartx, unsigned long data, int cnt,
			pf_putdata2buf put_data, pf_getbufdata get_data);

void usartx_use_buf_tx_lf(USART_TypeDef *usartx, unsigned long data, int cnt,
			pf_putdata2buf put_data, pf_getbufdata get_data);

#define UART4PC USART2
#define get_uart4pc_buf_data_fn USART2_GetRxBufferData
#define put_data2uart4pc_buf_fn USART2_PutDatatoTxBuffer
#define get_uart4pc_buf_len_fn  USART2_GetRxBufferCurrentSize
#define flush_uart4pc_buf_fn    USART2_FlushRxBuffer

#define send_data_to_pc(data, cnt)     usartx_use_buf_tx(UART4PC ,data, cnt, put_data2uart4pc_buf_fn, get_uart4pc_buf_data_fn)
#define send_data_to_pc_lf(data, cnt)  usartx_use_buf_tx_lf(UART4PC, data, cnt, put_data2uart4pc_buf_fn, get_uart4pc_buf_data_fn)


#define creat_log_ind(n) (((0xa5UL)<<24) | ((n)<<16) | ('\r'<<8) | ('\n'))

#endif

