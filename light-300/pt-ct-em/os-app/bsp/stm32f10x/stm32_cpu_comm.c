/*
 * stm32_cpu_comm.c
 *
 *  Created on: 2013-10-20
 *      Author: David, zhaoshaowei@yeejoin.com
 */

#include <stm32f10x.h>
#include <stm32_cpu_comm.h>

#define is_spix_sr_flag_set(spix, flag)	(0 != ((spix)->SR & (flag)))

#define spi_i2s_send_byte(spix, data)	((spix)->DR = (unsigned char)data)
#define spi_i2s_recv_byte(spix)		((spix)->DR)

struct rt_semaphore spi_sem_share_by_ade7880;

/**
 * Sends a byte over the SPI bus.
 *
 * \param[in] b The byte to send.
 */
uint8_t spix_send_byte(SPI_TypeDef* SPIx, unsigned char writedat)
{
	//Wait until the transmit buffer is empty
	while(!is_spix_sr_flag_set(SPIx, SPI_I2S_FLAG_TXE));
	spi_i2s_send_byte(SPIx, writedat);

	//Wait until a data is received
	while(!is_spix_sr_flag_set(SPIx, SPI_I2S_FLAG_RXNE));

	return spi_i2s_recv_byte(SPIx);
}

/**
 * Receives a byte from the SPI bus.
 *
 * \returns The received byte.
 */
uint8_t spix_rec_byte(SPI_TypeDef* SPIx, unsigned dummy)
{
	while(!is_spix_sr_flag_set(SPIx, SPI_I2S_FLAG_TXE));
	spi_i2s_send_byte(SPIx, dummy); /* Ϊ��ʹclock�ܹ�ʹ�� */

	//Wait until a data is received
	while(!is_spix_sr_flag_set(SPIx, SPI_I2S_FLAG_RXNE));

	return spi_i2s_recv_byte(SPIx);
}

/**
 * Sends data contained in a buffer over the SPI bus.
 *
 * \param[in] data A pointer to the buffer which contains the data to send.
 * \param[in] data_len The number of bytes to send.
 */
void spix_send_data(SPI_TypeDef* SPIx, const uint8_t* data, uint32_t data_len)
{
	uint8_t b;
	while (data_len--) {
		b = *data++;

		while(!is_spix_sr_flag_set(SPIx, SPI_I2S_FLAG_TXE));
		spi_i2s_send_byte(SPIx, b);
		//Wait until a data is received
		while(!is_spix_sr_flag_set(SPIx, SPI_I2S_FLAG_RXNE));
		spi_i2s_recv_byte(SPIx);
	}

	return;
}

/**
 * Receives multiple bytes from the SPI bus and writes them to a buffer.
 *
 * \param[out] buffer A pointer to the buffer into which the data gets written.
 * \param[in] buffer_len The number of bytes to read.
 */
void spix_rec_data(SPI_TypeDef* SPIx, uint8_t* buffer, uint32_t buffer_len, unsigned dummy)
{
	while (buffer_len--) {
		while(!is_spix_sr_flag_set(SPIx, SPI_I2S_FLAG_TXE));
		spi_i2s_send_byte(SPIx, dummy); /* Ϊ��ʹclock�ܹ�ʹ�� */

		while(!is_spix_sr_flag_set(SPIx, SPI_I2S_FLAG_RXNE));
		*buffer++ = spi_i2s_recv_byte(SPIx);
	}

	return;
}


/*
 * @brief  set the specified TIM interrupts enable bit and return old state.
 * @param  TIMx: where x can be 1 to 17 to select the TIMx peripheral.
 * @param  TIM_IT: specifies the TIM interrupts sources to be enabled or disabled.
 *   This parameter can be any combination of the following values:
 *     @arg TIM_IT_Update: TIM update Interrupt source
 *     @arg TIM_IT_CC1: TIM Capture Compare 1 Interrupt source
 *     @arg TIM_IT_CC2: TIM Capture Compare 2 Interrupt source
 *     @arg TIM_IT_CC3: TIM Capture Compare 3 Interrupt source
 *     @arg TIM_IT_CC4: TIM Capture Compare 4 Interrupt source
 *     @arg TIM_IT_COM: TIM Commutation Interrupt source
 *     @arg TIM_IT_Trigger: TIM Trigger Interrupt source
 *     @arg TIM_IT_Break: TIM Break Interrupt source
 * @note
 *   - TIM6 and TIM7 can only generate an update interrupt.
 *   - TIM9, TIM12 and TIM15 can have only TIM_IT_Update, TIM_IT_CC1,
 *      TIM_IT_CC2 or TIM_IT_Trigger.
 *   - TIM10, TIM11, TIM13, TIM14, TIM16 and TIM17 can have TIM_IT_Update or TIM_IT_CC1.
 *   - TIM_IT_Break is used only with TIM1, TIM8 and TIM15.
 *   - TIM_IT_COM is used only with TIM1, TIM8, TIM15, TIM16 and TIM17.
 *
 * @param  NewState: new state of the TIM interrupts.
 *   This parameter can be: ENABLE or DISABLE.
 *
 * @retval ENABLE or DISABLE.
 * */
FunctionalState set_timx_int_enable_bit(TIM_TypeDef* TIMx, uint16_t TIM_IT, FunctionalState new_state)
{
	FunctionalState ret;

	/* Check the parameters */
	assert_param(IS_TIM_ALL_PERIPH(TIMx));
	assert_param(IS_TIM_IT(TIM_IT));
	assert_param(IS_FUNCTIONAL_STATE(new_state));

	if (0 != (TIMx->DIER & TIM_IT))
		ret = ENABLE;
	else
		ret = DISABLE;

	if (new_state != DISABLE)
		TIMx->DIER |= TIM_IT; /* Enable the Interrupt sources */
	else
		TIMx->DIER &= (uint16_t)~TIM_IT; /* Disable the Interrupt sources */

	return ret;
}


/*
 *
 * Interrupt mask register (EXTI_IMR)
 * 	Bits 19:0 MRx: Interrupt Mask on line x
 *	0: Interrupt request from Line x is masked
 * 	1: Interrupt request from Line x is not masked
 * 	Note: Bit 19 is used in connectivity line devices only and is reserved otherwise.
 * */
FunctionalState set_exti_enable_bit(uint32_t exti_line, FunctionalState new_state)
{
	FunctionalState ret;

	if (0 != (EXTI->IMR & exti_line))
		ret = ENABLE;
	else
		ret = DISABLE;

	if (new_state != DISABLE) {
		EXTI->IMR |= exti_line; /* set EXTI line configuration */
	} else {
		EXTI->IMR &= ~exti_line; /* Clear EXTI line configuration */
	}

	return ret;
}


/* Maximum Timeout values for flags and events waiting loops. These timeouts are
   not based on accurate values, they just guarantee that the application will
   not remain stuck if the I2C communication is corrupted.
   You may modify these timeout values depending on CPU frequency and application
   conditions (interrupts routines ...). */
#define I2C_FLAG_TIMEOUT_TIME             ((uint32_t)0x1000*2)
#define I2C_LONG_TIMEOUT_TIME             ((uint32_t)(300 * I2C_FLAG_TIMEOUT_TIME)*2)

/*
 * 以网络序发送reg_addr
 * */
enum i2c_err_e i2cx_read_byte(I2C_TypeDef *i2c_dev, unsigned char i2c_read_addr, unsigned char i2c_write_addr,
		unsigned reg_addr, int reg_addr_bits_num, unsigned char *reg_data)
{
	uint32_t timeout;

	/*!< While the bus is busy */
	timeout = I2C_LONG_TIMEOUT_TIME;
	while(I2C_GetFlagStatus(i2c_dev, I2C_FLAG_BUSY))
		if((timeout--) == 0) return IEE_BUSY;

	/*!< Re-Enable Acknowledgment to be ready for another reception */
	I2C_AcknowledgeConfig(i2c_dev, ENABLE);
	/* Start the config sequence */
	I2C_GenerateSTART(i2c_dev, ENABLE);
	/* Test on EV5 and clear it */
	timeout = I2C_FLAG_TIMEOUT_TIME;
	while (!I2C_CheckEvent(i2c_dev, I2C_EVENT_MASTER_MODE_SELECT))
		if((timeout--) == 0) return IEE_TIMEOUT_MASTER_MODE;

	/* Transmit the slave address and enable writing operation */
	I2C_Send7bitAddress(i2c_dev, i2c_write_addr, I2C_Direction_Transmitter);
	/* Test on EV6 and clear it */
	timeout = I2C_FLAG_TIMEOUT_TIME;
	while (!I2C_CheckEvent(i2c_dev, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
		if((timeout--) == 0) return IEE_TIMEOUT_MASTER_TRM_MODE;

	if (16 == reg_addr_bits_num) {
		/* Transmit the register address to be read */
		I2C_SendData(i2c_dev, reg_addr>>8);
		/* Test on EV8 and clear it */
		timeout = I2C_FLAG_TIMEOUT_TIME;
		while (I2C_GetFlagStatus(i2c_dev, I2C_FLAG_BTF) == RESET)
			if((timeout--) == 0) return IEE_TIMEOUT_BTF;

	}

	/* Transmit the register address to be read */
	I2C_SendData(i2c_dev, reg_addr);
	/* Test on EV8 and clear it */
	timeout = I2C_FLAG_TIMEOUT_TIME;
	while (I2C_GetFlagStatus(i2c_dev, I2C_FLAG_BTF) == RESET)
		if((timeout--) == 0) return IEE_TIMEOUT_BTF;

	/*!< Send STRAT condition a second time */
	I2C_GenerateSTART(i2c_dev, ENABLE);
	/*!< Test on EV5 and clear it (cleared by reading SR1 then writing to DR) */
	timeout = I2C_FLAG_TIMEOUT_TIME;
	while(!I2C_CheckEvent(i2c_dev, I2C_EVENT_MASTER_MODE_SELECT))
		if((timeout--) == 0) return IEE_TIMEOUT_MASTER_MODE;

	/*!< Send Codec address for read */
	I2C_Send7bitAddress(i2c_dev, i2c_read_addr, I2C_Direction_Receiver);
	/* Wait on ADDR flag to be set (ADDR is still not cleared at this level */
	timeout = I2C_FLAG_TIMEOUT_TIME;
	while(I2C_GetFlagStatus(i2c_dev, I2C_FLAG_ADDR) == RESET)
		if((timeout--) == 0) return IEE_TIMEOUT_ADDR;

	/*!< Disable Acknowledgment */
	I2C_AcknowledgeConfig(i2c_dev, DISABLE);
	/* Clear ADDR register by reading SR1 then SR2 register (SR1 has already been read) */
	(void)i2c_dev->SR2;
	I2C_GenerateSTOP(i2c_dev, ENABLE); 	/*!< Send STOP Condition */

	/* Wait for the byte to be received */
	timeout = I2C_FLAG_TIMEOUT_TIME;
	while(I2C_GetFlagStatus(i2c_dev, I2C_FLAG_RXNE) == RESET)
		if((timeout--) == 0) return IEE_TIMEOUT_RXNE;

	/*!< Read the byte received from the Codec */
	*reg_data = I2C_ReceiveData(i2c_dev);

	/* End the configuration sequence */
	I2C_GenerateSTOP(i2c_dev, ENABLE);
	/* Wait to make sure that STOP flag has been cleared */
	timeout = I2C_FLAG_TIMEOUT_TIME;
	while(i2c_dev->CR1 & I2C_CR1_STOP)
		if((timeout--) == 0) return IEE_TIMEOUT_STOP;

	/* Clear AF flag for next communication */
	I2C_ClearFlag(i2c_dev, I2C_FLAG_AF);

	return IEE_OK;
}



enum i2c_err_e i2cx_write_byte(I2C_TypeDef *i2c_dev, unsigned char i2c_write_addr,
		int reg_addr, int reg_addr_bits_num, unsigned char byte)
{
	int      timeout;

	/*!< While the bus is busy */
	timeout = I2C_LONG_TIMEOUT_TIME;
	while(0 != (I2C_GetFlagStatus(i2c_dev, I2C_FLAG_BUSY)))
		if((timeout--) == 0) return IEE_BUSY;

	I2C_AcknowledgeConfig(i2c_dev, ENABLE);
	/* Start the config sequence */
	I2C_GenerateSTART(i2c_dev, ENABLE);

	/* Test on EV5 and clear it */
	timeout = I2C_FLAG_TIMEOUT_TIME;
	while (0 == (I2C_CheckEvent(i2c_dev, I2C_EVENT_MASTER_MODE_SELECT)))
		if((timeout--) == 0) return IEE_TIMEOUT_MASTER_MODE;

	/* Transmit the slave address and enable writing operation */
	I2C_Send7bitAddress(i2c_dev, i2c_write_addr,
				I2C_Direction_Transmitter);

	/* Test on EV6 and clear it */
	timeout = I2C_FLAG_TIMEOUT_TIME;
	while (0 == (I2C_CheckEvent(i2c_dev, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)))
		if((timeout--) == 0) return IEE_TIMEOUT_MASTER_TRM_MODE;


	if (16 == reg_addr_bits_num) {
		/* Transmit the register address to be read */
		I2C_SendData(i2c_dev, reg_addr>>8);
		/* Test on EV8 and clear it */
		timeout = I2C_FLAG_TIMEOUT_TIME;
		while (I2C_GetFlagStatus(i2c_dev, I2C_FLAG_BTF) == RESET)
			if((timeout--) == 0) return IEE_TIMEOUT_BTF;

	}

	/* Transmit the first address for write operation */
	I2C_SendData(i2c_dev, reg_addr);

	/* Test on EV8 and clear it */
	timeout = I2C_FLAG_TIMEOUT_TIME;
	while (0 == (I2C_CheckEvent(i2c_dev, I2C_EVENT_MASTER_BYTE_TRANSMITTING)))
		if((timeout--) == 0) return IEE_TIMEOUT_BYTE_TRANSING;

	/* Prepare the register value to be sent */
	//I2C_SendData(i2c_dev, byte);
	i2c_dev->DR = byte;
	/*!< Wait till all data have been physically transferred on the bus */
	timeout = I2C_LONG_TIMEOUT_TIME;
	while (0 == (I2C_GetFlagStatus(i2c_dev, I2C_FLAG_BTF)))
		if((timeout--) == 0) return IEE_TIMEOUT_BTF;

	I2C_AcknowledgeConfig(i2c_dev, DISABLE);
	/* End the configuration sequence */
	I2C_GenerateSTOP(i2c_dev, ENABLE);

	return IEE_OK;
}


/*
 * reg_addr	-- 按照大端序发送
 * bytes
 * */
enum i2c_err_e i2cx_read_bytes(I2C_TypeDef *i2c_dev, unsigned char i2c_read_addr, unsigned char i2c_write_addr,
		unsigned reg_addr, int reg_addr_bits_num, unsigned char *bytes, int byte_cnt)
{
	uint32_t timeout;

	/*!< While the bus is busy */
	timeout = I2C_LONG_TIMEOUT_TIME;
	while(I2C_GetFlagStatus(i2c_dev, I2C_FLAG_BUSY))
		if((timeout--) == 0) return IEE_BUSY;

	I2C_AcknowledgeConfig(i2c_dev, ENABLE);
	/* Start the config sequence */
	I2C_GenerateSTART(i2c_dev, ENABLE);
	/* Test on EV5 and clear it */
	timeout = I2C_FLAG_TIMEOUT_TIME;
	while (!I2C_CheckEvent(i2c_dev, I2C_EVENT_MASTER_MODE_SELECT))
		if((timeout--) == 0) return IEE_TIMEOUT_MASTER_MODE;

	/* Transmit the slave address and enable writing operation */
	I2C_Send7bitAddress(i2c_dev, i2c_write_addr, I2C_Direction_Transmitter);
	/* Test on EV6 and clear it */
	timeout = I2C_FLAG_TIMEOUT_TIME;
	while (!I2C_CheckEvent(i2c_dev, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
		if((timeout--) == 0) return IEE_TIMEOUT_MASTER_TRM_MODE;

	if (16 == reg_addr_bits_num) {
		/* Transmit the register address to be read */
		I2C_SendData(i2c_dev, reg_addr>>8);
		/* Test on EV8 and clear it */
		timeout = I2C_FLAG_TIMEOUT_TIME;
		while (I2C_GetFlagStatus(i2c_dev, I2C_FLAG_BTF) == RESET)
			if((timeout--) == 0) return IEE_TIMEOUT_BTF;
	}

	/* Transmit the register address to be read */
	I2C_SendData(i2c_dev, reg_addr);
	/* Test on EV8 and clear it */
	timeout = I2C_FLAG_TIMEOUT_TIME;
	while (I2C_GetFlagStatus(i2c_dev, I2C_FLAG_BTF) == RESET)
		if((timeout--) == 0) return IEE_TIMEOUT_BTF;

	/*!< Send STRAT condition a second time */
	I2C_GenerateSTART(i2c_dev, ENABLE);
	/*!< Test on EV5 and clear it (cleared by reading SR1 then writing to DR) */
	timeout = I2C_FLAG_TIMEOUT_TIME;
	while(!I2C_CheckEvent(i2c_dev, I2C_EVENT_MASTER_MODE_SELECT))
		if((timeout--) == 0) return IEE_TIMEOUT_MASTER_MODE;

	/*!< Send Codec address for read */
	I2C_Send7bitAddress(i2c_dev, i2c_read_addr, I2C_Direction_Receiver);
	/* Wait on ADDR flag to be set (ADDR is still not cleared at this level */
	timeout = I2C_FLAG_TIMEOUT_TIME;
	while(I2C_GetFlagStatus(i2c_dev, I2C_FLAG_ADDR) == RESET)
		if((timeout--) == 0) return IEE_TIMEOUT_ADDR;

	while (byte_cnt-- > 0) {
		/* Wait for the byte to be received */
		timeout = 2*I2C_FLAG_TIMEOUT_TIME;
		while(!I2C_CheckEvent(i2c_dev, I2C_EVENT_MASTER_BYTE_RECEIVED))
			if((timeout--) == 0) return IEE_TIMEOUT_RXNE;

		/*!< Read the byte received from the Codec */
		*bytes++ = I2C_ReceiveData(i2c_dev);

		if (1 == byte_cnt) {
			I2C_AcknowledgeConfig(i2c_dev, DISABLE);
		}
	}

	/* End the configuration sequence */
	I2C_GenerateSTOP(i2c_dev, ENABLE);
	/* Wait to make sure that STOP flag has been cleared */
	timeout = I2C_FLAG_TIMEOUT_TIME;
	while(i2c_dev->CR1 & I2C_CR1_STOP)
		if((timeout--) == 0) return IEE_TIMEOUT_STOP;

	/* Clear AF flag for next communication */
	I2C_ClearFlag(i2c_dev, I2C_FLAG_AF);

	return IEE_OK;
}



enum i2c_err_e i2cx_write_bytes(I2C_TypeDef *i2c_dev, unsigned char i2c_write_addr,
		int reg_addr, int reg_addr_bits_num, unsigned char *bytes, int byte_cnt)
{
	int      timeout;

	/*!< While the bus is busy */
	timeout = I2C_LONG_TIMEOUT_TIME;
	while(0 != (I2C_GetFlagStatus(i2c_dev, I2C_FLAG_BUSY)))
		if((timeout--) == 0) return IEE_BUSY;

	I2C_AcknowledgeConfig(i2c_dev, ENABLE);
	/* Start the config sequence */
	I2C_GenerateSTART(i2c_dev, ENABLE);

	/* Test on EV5 and clear it */
	timeout = I2C_FLAG_TIMEOUT_TIME;
	while (0 == (I2C_CheckEvent(i2c_dev, I2C_EVENT_MASTER_MODE_SELECT)))
		if((timeout--) == 0) return IEE_TIMEOUT_MASTER_MODE;

	/* Transmit the slave address and enable writing operation */
	I2C_Send7bitAddress(i2c_dev, i2c_write_addr,
				I2C_Direction_Transmitter);

	/* Test on EV6 and clear it */
	timeout = I2C_FLAG_TIMEOUT_TIME;
	while (0 == (I2C_CheckEvent(i2c_dev, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)))
		if((timeout--) == 0) return IEE_TIMEOUT_MASTER_TRM_MODE;


	if (16 == reg_addr_bits_num) {
		/* Transmit the register address to be read */
		I2C_SendData(i2c_dev, reg_addr>>8);
		/* Test on EV8 and clear it */
		timeout = I2C_FLAG_TIMEOUT_TIME;
		while (I2C_GetFlagStatus(i2c_dev, I2C_FLAG_BTF) == RESET)
			if((timeout--) == 0) return IEE_TIMEOUT_BTF;

	}

	/* Transmit the first address for write operation */
	I2C_SendData(i2c_dev, reg_addr);

	/* Test on EV8 and clear it */
	timeout = I2C_FLAG_TIMEOUT_TIME;
	while (0 == (I2C_CheckEvent(i2c_dev, I2C_EVENT_MASTER_BYTE_TRANSMITTING)))
		if((timeout--) == 0) return IEE_TIMEOUT_BYTE_TRANSING;

	while (byte_cnt-- > 0) {
		/* Prepare the register value to be sent */
		//I2C_SendData(i2c_dev, byte);
		i2c_dev->DR = *bytes++;
		/*!< Wait till all data have been physically transferred on the bus */
		timeout = I2C_LONG_TIMEOUT_TIME;
		while (0 == (I2C_GetFlagStatus(i2c_dev, I2C_FLAG_BTF)))
			if((timeout--) == 0) return IEE_TIMEOUT_BTF;
	}

	I2C_AcknowledgeConfig(i2c_dev, DISABLE);
	/* End the configuration sequence */
	I2C_GenerateSTOP(i2c_dev, ENABLE);

	return IEE_OK;
}
