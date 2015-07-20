#ifndef SYS_COMM_H_
#define SYS_COMM_H_ 1


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

#define is_spix_txbuf_empty(spix) (!(spix->SR & 0x02))
#define is_spix_busy(spix)        (spix->SR & 0x80)


/*
 * dma
 */
/* DMA ENABLE mask */
#define CCR_ENABLE_Set          ((uint32_t)0x00000001)
#define CCR_ENABLE_Reset        ((uint32_t)0xFFFFFFFE)

/* gif -- global interrupt flag, tcif -- transfer complete interrupt flag, htif -- half transfer, teif -- transfer error */
#define DMA_INT_FLAG_BIT_GIFx(channel_num)   (0x01 << ((channel_num-1) * 4))
#define DMA_INT_FLAG_BIT_TCIFx(channel_num)  (0x02 << ((channel_num-1) * 4)) 
#define DMA_INT_FLAG_BIT_HTIFx(channel_num)  (0x04 << ((channel_num-1) * 4)) 
#define DMA_INT_FLAG_BIT_TEIFx(channel_num)  (0x08 << ((channel_num-1) * 4)) 

#define is_dmax_channely_gif_set(dmax, channel)  (dmax->ISR & DMA_INT_FLAG_BIT_GIFx(channel))
#define is_dmax_channely_tcif_set(dmax, channel) (dmax->ISR & DMA_INT_FLAG_BIT_TCIFx(channel))
#define is_dmax_channely_htif_set(dmax, channel) (dmax->ISR & DMA_INT_FLAG_BIT_HTIFx(channel))
#define is_dmax_channely_teif_set(dmax, channel) (dmax->ISR & DMA_INT_FLAG_BIT_TEIFx(channel))

#define clear_dmax_channely_gif(dmax, channel)   (dmax->IFCR = DMA_INT_FLAG_BIT_GIFx(channel))
#define clear_dmax_channely_tcif(dmax, channel)  (dmax->IFCR = DMA_INT_FLAG_BIT_TCIFx(channel))
#define clear_dmax_channely_htif(dmax, channel)  (dmax->IFCR = DMA_INT_FLAG_BIT_HTIFx(channel))
#define clear_dmax_channely_teif(dmax, channel)  (dmax->IFCR = DMA_INT_FLAG_BIT_TEIFx(channel))


#define enable_dmax_y(dmax_y)  do {dmax_y->CCR |= CCR_ENABLE_Set;} while (0)
#define disable_dmax_y(dmax_y) do {dmax_y->CCR &= CCR_ENABLE_Reset;} while (0)


/*
 *********************************************************************
 */

#define ENTER_SET_PARAM_MODE_TIME_CNT (300)


#define send_data_to_pc(data, cnt)     usart1_use_buf_tx(data, cnt)
#define send_data_to_pc_lf(data, cnt)  usart1_use_buf_tx_lf(data, cnt)


#endif
