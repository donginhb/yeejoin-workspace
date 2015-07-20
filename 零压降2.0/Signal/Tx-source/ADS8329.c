/*
  修改内容:此为加入的ADS8329的初始化程序，与RX端进行联调。
  2012/11/14  malooei

  SPI1_ADS8329初始化函数 spi1_cfg4ads8329()
  转换调用函数(16bit)    ADS8329_OneChannel_Converter();
*/
#include ".\fwlib\stm32f10x.h"
#include ".\fwlib\stm32f10x_spi.h"

#include "ads8329.h"
#include "board.h"

#define USE_CONVERTION_DONE_AND_NEXT_START_GAP 0

/* Pulse duration, CONVST low 40ns(min) */
#define start_adc_conversion_falling_edge() 	clr_port_pin(ADS8329_NCONVST_PORT, ADS8329_NCONVST_PIN)
#define start_adc_conversion_rising_edge()  	set_port_pin(ADS8329_NCONVST_PORT, ADS8329_NCONVST_PIN)

#define is_conversion_doing()  	(!pin_in_is_set(ADS8329_EOC_PORT, ADS8329_EOC_PIN))

#define enable_ads8329_cs()	clr_port_pin(ADS8329_CS_PORT, ADS8329_CS_PIN)
#define disable_ads8329_cs()	set_port_pin(ADS8329_CS_PORT, ADS8329_CS_PIN)

#define is_spix_sr_flag_set(spix, flag)	(0 != ((spix)->SR & (flag)))

#define spi_i2s_send_hword(spix, data)	((spix)->DR = (u16)data)
#define spi_i2s_recv_hword(spix)		((spix)->DR)

static u16 spix_send_hword(SPI_TypeDef* SPIx, u16 writedat);
static void start_ads8329_sampling(void);

#if USE_CONVERTION_DONE_AND_NEXT_START_GAP
static void convertion_done_and_next_start_gap(void);
#endif

void spi1_cfg4ads8329(void)
{
	SPI_InitTypeDef SPI_InitStructure;

	/* 在时钟CLK下降沿采集数据 */
	SPI_Cmd(SPI1, DISABLE);
	SPI_InitStructure.SPI_Direction	= SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode	= SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize	= SPI_DataSize_16b;  /* 16位帧结构 */

	SPI_InitStructure.SPI_CPOL	= SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA	= SPI_CPHA_1Edge; /* 数据捕获于第1个时钟沿 */
	SPI_InitStructure.SPI_NSS	= SPI_NSS_Soft;   /* 软件控制NSS信号 */
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	SPI_InitStructure.SPI_FirstBit	= SPI_FirstBit_MSB;   /* 数据大头传输 */
	SPI_InitStructure.SPI_CRCPolynomial	= 7;  /* 定义用于CRC值计算的多项式7 */
	SPI_Init(SPI1,&SPI_InitStructure);

	SPI_Cmd(SPI1,ENABLE);

	return;
}

/*
 * CFR-D[11-0]:0xffd(default)
 *
 * D11 default = 1, Channel select mode
 * 0: Manual channel select enabled. Use channel select commands to access a different channel.
 * 1: Auto channel select enabled. All channels are sampled and converted sequentially until the cycle after this bit is set to 0.
 *
 * D10 default = 1, Conversion clock (CCLK) source select
 * 0: Conversion clock (CCLK) = SCLK/2
 * 1: Conversion clock (CCLK) = Internal OSC
 *
 * D9 default = 1, Trigger (conversion start) select: start conversion at the end of sampling (EOS). If D9 = 0, the D4 setting is ignored.
 * 0: Auto trigger automatically starts (4 internal clocks after EOC inactive)
 * 1: Manual trigger manually started by falling edge of CONVST
 *
 * D8 default = 1 Don't care Don't care
 *
 * D7 default = 1, Pin 10 polarity select when used as an output (EOC/INT)
 * 0: EOC Active high / INT active high
 * 1: EOC active low / INT active low
 *
 * D6 default = 1, Pin 10 function select when used as an output (EOC/INT)
 * 0: Pin used as INT
 * 1: Pin used as EOC
 *
 * D5 default = 1, Pin 10 I/O select for chain mode operation
 * 0: Pin 10 is used as CDI input (chain mode enabled)
 * 1: Pin 10 is used as EOC/INT output
 *
 * D4 default = 1, Auto nap power-down enable/disable (mid voltage and comparator shut down between cycles). This bit setting is ignored if D9 = 0.
 * 0: Auto nap power-down enabled (not activated)
 * 1: Auto nap power-down disabled
 *
 * D3 default = 1, Nap power-down (mid voltage and comparator shut down between cycles). This bit is set to 1 automatically by wake-up command.
 * 0: Enable/activate device in nap power-down
 * 1: Remove device from nap power-down (resume)
 *
 * D2 default = 1, Deep power-down. This bit is set to 1 automatically by wake-up command.
 * 0: Enable/activate device in deep power-down
 * 1: Remove device from deep power-down (resume)
 *
 * D1 default = 0: ADS8329 1: ADS8330, TAG bit enable. This bit is ignored by the ADS8329 and is always read 0.
 * 0: TAG bit disabled.
 * 1: TAG bit output enabled. TAG bit appears at the 17th SCLK.
 *
 * D0 default = 1, Reset
 * 0: System reset
 * 1: Normal operation
 */
void init_ads8329(void)
{
	int i;

	/* soft-reset ads8329 */
	enable_ads8329_cs();
	spix_send_hword(SPI1, 0xE000);
	disable_ads8329_cs();

	for (i=10; i>0; --i)
		;
	/*  */
	enable_ads8329_cs();
	spix_send_hword(SPI1, 0xEffd);
	disable_ads8329_cs();

	return;
}

/*
 * Read while sampling, Read while converting
 *
 * 参见手册中的Figure 1. Timing for Conversion and Acquisition Cycles for Manual Trigger (Read while sampling)
 * 等待转换完成后, 读取数据
 */
u16 ads8329_converter_channelx(unsigned int pin_mask)
{
#if 0 /* 用于测试硬件 */
	return 55536;
#else
	int i;
	u32 value;

	/* 初始化时, 打开a、关闭bc */
	start_ads8329_sampling();

	i = 72;
	while (is_conversion_doing() && i--)
		;

	if (0 == i) /* 作为保护, 防止ads8329工作异常时, 导致cpu死机, David */
		init_ads8329();

#if USE_CONVERTION_DONE_AND_NEXT_START_GAP
	convertion_done_and_next_start_gap();
#endif

	enable_ads8329_cs();
	value = spix_send_hword(SPI1, 0); /* 0对ads8329来说是无效命令, David */
	disable_ads8329_cs();

	/* adc完成后为下次做准备 */
	switch (pin_mask) {
	case PA_ADS8329_CH_PIN:
		close_pa_switch();
		open_pb_switch();
		break;

	case PB_ADS8329_CH_PIN:
		close_pb_switch();
		open_pc_switch();
		break;

	case PC_ADS8329_CH_PIN:
		close_pc_switch();
		open_pa_switch();
		break;

	default:
		break;
	}

	return value;
#endif
}

/*
 * Sends a half-word over the SPI bus.
 */
static u16 spix_send_hword(SPI_TypeDef* SPIx, u16 writedat)
{
	//Wait until the transmit buffer is empty
	while(!is_spix_sr_flag_set(SPIx, SPI_I2S_FLAG_TXE));
	spi_i2s_send_hword(SPIx, writedat);

	//Wait until a data is received
	while(!is_spix_sr_flag_set(SPIx, SPI_I2S_FLAG_RXNE));

	return spi_i2s_recv_hword(SPIx);
}


/*
 * twL(CONVST)Pulse duration, CONVST low, 40ns(min)
 *
 * ads8329内部时钟的频率(MHz)为(20, 22.3, 23.5)
 *
 * stm32@72MHz时, 一条汇编指令的时间不小于 1000/72 ns
 * 3*1000/72 = 41.67 (ns)
 */
static void start_ads8329_sampling(void)
{
	volatile int delay_var;

	start_adc_conversion_falling_edge();
	delay_var = 0;
	delay_var = 1;
	delay_var = 2;
	delay_var = 3;
	start_adc_conversion_rising_edge();

	return;
}

#if USE_CONVERTION_DONE_AND_NEXT_START_GAP
/*
 * min -- 3 cclk, 3000/20 = 150(ns)
 *
 * 11 * 1000/72 = 152.78 (ns)
 */
static void convertion_done_and_next_start_gap(void)
{
	volatile int delay_var;

	delay_var = 0;
	delay_var = 1;
#if 0
	delay_var = 2;
	delay_var = 3;
	delay_var = 4;
	delay_var = 5;
	delay_var = 6;
	delay_var = 7;

	delay_var = 8;
	delay_var = 9;
#endif
	return;
}
#endif

