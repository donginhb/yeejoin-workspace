/*!\file EZMacPro_TimerInt.c
 * \brief Timer interrupt triggered state machine and functions.
 *
 * \n EZMacPRO version: 3.0.1r
 *
 * \n This software must be used in accordance with the End User License
 * \n Agreement.
 *
 * \b COPYRIGHT
 * \n Copyright 2012 Silicon Laboratories, Inc.
 * \n http://www.silabs.com
 */

/* ======================================= *
 *              I N C L U D E              *
 * ======================================= */

#include "stack.h"

#if 0
#define ezmacprof_debug(x) rt_kprintf x
#else
#define ezmacprof_debug(x)
#endif


/* ======================================= *
 *     G L O B A L   V A R I A B L E S     *
 * ======================================= */

SEGMENT_VARIABLE(ChannelOccupiedInStartPeriod, U8, EZMAC_PRO_GLOBAL_MSPACE);
SEGMENT_VARIABLE(ChannelOccupiedCounter, U8, EZMAC_PRO_GLOBAL_MSPACE);


static void timerIntWakeUp (void);

#ifndef RECEIVER_ONLY_OPERATION
static void timerIntTX_StateMachine (U8 state);
#endif
#ifndef TRANSMITTER_ONLY_OPERATION
static void timerIntRX_StateMachine (U8 state);

#ifndef MORE_CHANNEL_IS_USED
static void timerIntNextRX_Channel (void);
#endif

static void timerIntSetFunction1(U8 value);
#endif

#ifdef TRANSCEIVER_OPERATION
static U8 timerIntRandom (void);
#endif

static void timerIntDisableInterrupts (void);
static void timerIntGotoNextStateUsingSECR( U8 tx);

#ifdef FOUR_CHANNEL_IS_USED
#ifndef TRANSMITTER_ONLY_OPERATION
static void timerIntIncrementError (U8 mask);
static void timerIntIncrementError (U8 mask);
#endif
#endif

static void timerIntGotoNextStateUsingSECR( U8 tx);
static void timerIntSetEnable2(U8 value);


/* ======================================= *
 *      L O C A L   F U N C T I O N S      *
 * ======================================= */

/*------------------------------------------------------------------------------------------------
 * Function Name
 *    timerIntT0_ISR()
 *
 * Return Value : None
 * Parameters   : None
 *
 * This is the Interrupt Service Routing for the T0 timer. The T0 timer is used for all MAC
 * time outs and MAC timing events. The T0 time base uses SYSCLK/4 for all supported SYSCLK
 * frequencies. The time outs are calculated using macros or calculated by the initialization
 * or register write API functions. Since some time outs require long periods a 24-bit timer
 * has been implemented using a global variable for the most significant byte. If the MSB of the
 * timer (EZMacProTimerMSB) is non-zero, it will be decrement and the ISR will be called again
 * when the timer overflows.
 *
 * This function disables the timer interrupts before executing the state machines. If a
 * timeout event is to initiate another timeout event, the timerIntTimeout() function should
 * be used.
 *
 * The Basic States (Wake-up, Receive, and Transmit) are implemented using if..else if bit
 * tests for the corresponding bit in the master control register. The detailed TX and RX
 * state machines are implemented in separate functions.
 *
 * Conditional compile time switches remove the TX or RX state machines for the RX only and
 * and TX only builds.
 *
 * The timer interrupt should not call functions from other modules. This would create cause
 * a multiple call to segment warning and result in poor RAM usage.
 *
 *-----------------------------------------------------------------------------------------------
 */
INTERRUPT(timerIntT3_ISR, INTERRUPT_TIMER3)
{
	U8 state;

	if (EZMacProTimerMSB == 0) {
ezmac_timeout:
		DISABLE_MAC_TIMER_INTERRUPT();
		STOP_MAC_TIMER();
		CLEAR_MAC_TIMER_INTERRUPT();
		is_mac_timer_inuse = 0; /* add by David */

		state = EZMacProReg.name.MSR & 0x0F;

		ezmacprof_debug(("mac timer timeout, msr:0x%02x, tcr:0x%02x, mcr:0x%02x,\n",
				EZMacProReg.name.MSR, EZMacProReg.name.TCR, EZMacProReg.name.MCR)); /* mark by David */

		if (EZMacProReg.name.MSR == EZMAC_PRO_WAKE_UP) {
			//if the MAC is in Wake up state call the WakeUp function
			timerIntWakeUp();
		}
#ifndef RECEIVER_ONLY_OPERATION
		else if ((EZMacProReg.name.MSR & TX_STATE_BIT) == TX_STATE_BIT) {
			//if the MAC is in transmit state then call the transmit state machine
			timerIntTX_StateMachine(state);
		}
#endif
#ifndef TRANSMITTER_ONLY_OPERATION
		else if ((EZMacProReg.name.MSR & RX_STATE_BIT) == RX_STATE_BIT) {
			//if the MAC is in receive state then call the receiver
			timerIntRX_StateMachine(state);
		}
#endif
		else {
			/* mark by David */
		}
	} else {
		CLEAR_MAC_TIMER_INTERRUPT();
		EZMacProTimerMSB--;

		/* add by David */
		if (0 == EZMacProTimerMSB) {
			if (0 != EZMacProTimerMSB) {
				init_si4432_mac_timer(EZMacProTimerLSB);
				START_MAC_TIMER();
			} else {
				goto ezmac_timeout;
			}
		}
	}
}


/*------------------------------------------------------------------------------------------------
 * Function Name
 *    timerIntWakeUp()
 *
 * Return Value : None
 * Parameters   : None
 *
 * The EZMAC_PRO_WAKE_UP state is used when starting the radio XTAL after sleep mode or when
 * resetting the radio using a SW Reset SPI command.
 *
 * If this function is called, the crystal has failed to start with the specified time. This
 * indicates are hardware problem. Either the crystal is not connect, the SDN/ is asserted,
 * or the radio is not powered.
 *-----------------------------------------------------------------------------------------------
 */
static void timerIntWakeUp (void)
{
	// SWRESET timeout error
	DISABLE_MAC_INTERRUPTS();
	//go to WAKE UP ERROR state
	set_ezmacpro_msr(WAKE_UP_ERROR);
	//call the error state entered callback function
	EZMacPRO_StateErrorEntered();
}


/*------------------------------------------------------------------------------------------------
 * Function Name
 *    timerIntTX_StateMachine()
 *
 * Return Value : None
 * Parameters   : U8 state - TX state, least significant nibble of MSR
 *
 * This function implements the detailed TX state machine. The state machine is implemented
 * using switch...case... statements. For efficient compilation, the states are a series
 * of sequential continuous integers (no missing values). The states are enumerated in the
 * EZMacPro.h header file.
 *
 * Conditional compile time switches remove the unused TX states if LBT of ACK is not supported.
 * These states are also removed from the enumeration at the begriming or end of the list.
 *
 * Conditional compile time switches remove this entire function for the Receiver only build.
 *
 *-----------------------------------------------------------------------------------------------
 */
#ifndef RECEIVER_ONLY_OPERATION
static void timerIntTX_StateMachine (U8 state)
{

#ifdef TRANSCEIVER_OPERATION
	U32 timeout;
#endif//TRANSCEIVER_OPERATION


#ifdef ANTENNA_DIVERSITY_ENABLED
#ifndef B1_ONLY
	U8 temp8;
#endif//B1_ONLY
#endif//ANTENNA_DIVERSITY_ENABLED

	switch (state) {
#ifdef TRANSCEIVER_OPERATION
	case TX_STATE_LBT_START_LISTEN:
		// disable the reciever
		timerIntSetFunction1(SI4432_XTON);

		//go to the next state
		set_ezmacpro_msr(TX_STATE_BIT | TX_STATE_LBT_LISTEN);
		// enable the receiver again
		timerIntSetFunction1( SI4432_RXON|SI4432_XTON);

		if(BusyLBT == 0) {
			//the channel was free during the first 0.5ms
			ChannelOccupiedInStartPeriod = 0;
			// start timer with fix 4.5ms timeout
			timerIntTimeout(LBT_FIXED_TIME_4500US);
		} else {
			//the channnel was busy during the first 0.5ms
			BusyLBT = 0;
			ChannelOccupiedInStartPeriod = 1;
			ChannelOccupiedCounter = 0;
			// start timer with fix 1ms timeout
			timerIntTimeout(LBT_FIXED_TIME_1000US);
		}
		ENABLE_MAC_TIMER_INTERRUPT();
		break;

	case TX_STATE_LBT_LISTEN:
		if (ChannelOccupiedInStartPeriod == 0) {
			//the channel was free during the 0.5ms start period
			if(BusyLBT == 0) {
				// LBT passed, channel should be clear in the fix 4.5ms period
				//disable the receiver
				timerIntSetFunction1(SI4432_XTON);
				// clear enable 2 interrupt
				timerIntSetEnable2(0x00);
				// enable ENPKSENT bit
				timerIntSpiWriteReg(SI4432_INTERRUPT_ENABLE_1, SI4432_ENPKSENT);
				//go to the next state
				set_ezmacpro_msr(TX_STATE_BIT | TX_STATE_WAIT_FOR_TX);
				// enable TX
				timerIntSetFunction1( SI4432_TXON|SI4432_XTON);
				// start timer with transmit packet timeout
				timerIntTimeout(TimeoutTX_Packet);
			} else {
				//the channel was busy during the 4.5ms
				// disable the reciever
				timerIntSetFunction1(SI4432_XTON);

				BusyLBT = 0;
				//go to the next state
				set_ezmacpro_msr(TX_STATE_BIT | TX_STATE_LBT_RANDOM_LISTEN);
				// enable the receiver again
				timerIntSetFunction1( SI4432_RXON|SI4432_XTON);
				// multiple by fixed plus random number
				timeout = LBT_FIXED_TIME_5000US + TimeoutLBTI * (U32)(timerIntRandom());
				// start timer with timeout
				timerIntTimeout(timeout);
			}
			ENABLE_MAC_TIMER_INTERRUPT();
		} else {
			//the channel was busy during the 0.5ms start period
			if(BusyLBT == 0) {
				//go to the next state
				set_ezmacpro_msr(TX_STATE_BIT | TX_STATE_LBT_RANDOM_LISTEN);

				// LBT passed, channel should be clear in the fix 1ms period
				// multiple by fixed plus random number
				timeout = LBT_FIXED_TIME_5000US + TimeoutLBTI * (U32)(timerIntRandom());
				// start timer with timeout
				timerIntTimeout(timeout);
				ENABLE_MAC_TIMER_INTERRUPT();
			} else {
				BusyLBT = 0;
				//the channel was busy in the fix 1ms period
				if (ChannelOccupiedCounter < 9) {
					// disable the reciever
					timerIntSetFunction1(SI4432_XTON);

					ChannelOccupiedCounter++;
					// go to the next state
					set_ezmacpro_msr(TX_STATE_BIT | TX_STATE_LBT_LISTEN);
					// enable the receiver again
					timerIntSetFunction1( SI4432_RXON|SI4432_XTON);
					// start timer with fix 1ms timeout
					timerIntTimeout(LBT_FIXED_TIME_1000US);

					ENABLE_MAC_TIMER_INTERRUPT();
				} else {
					//the channel was busy during the 10*1ms
					EZMacProLBT_Retrys++;
					if(EZMacProLBT_Retrys < MAX_LBT_RETRIES) {
						//the channel was busy and the retries didn't reach the maximum value
						// disable the reciever
						timerIntSetFunction1(SI4432_XTON);
						// go to the next state
						set_ezmacpro_msr(TX_STATE_BIT | TX_STATE_LBT_START_LISTEN);
						// enable the receiver again
						timerIntSetFunction1( SI4432_RXON|SI4432_XTON);

						// start timer with fix ETSI timeout
						timerIntTimeout(TIMEOUT_LBTI_ETSI);
						ENABLE_MAC_TIMER_INTERRUPT();
					} else {
						//the channel was busy and the retries reach the maximum value
						//DISABLE_MAC_INTERRUPTS();
						//disable the receiver
						timerIntSetFunction1(SI4432_XTON);
						//go to the next state
						set_ezmacpro_msr(TX_STATE_BIT | TX_ERROR_CHANNEL_BUSY);
#ifdef FOUR_CHANNEL_IS_USED
						//increment error counter
						timerIntIncrementError(EZMAC_PRO_ERROR_CHANNEL_BUSY);
#endif //FOUR_CHANNEL_IS_USED
						//call the LBT error callback function
						EZMacPRO_LBTTimeout();
					}
				}
			}

		}
		break;

	case TX_STATE_LBT_RANDOM_LISTEN:
		if(BusyLBT == 0) {
			// the channel was free during the 5ms + random period
			//disable the receiver
			timerIntSetFunction1(SI4432_XTON);
			// clear enable 2 interrupt
			timerIntSetEnable2(0x00);
			// enable ENPKSENT bit
			timerIntSpiWriteReg(SI4432_INTERRUPT_ENABLE_1, SI4432_ENPKSENT);
			//go to the next state
			set_ezmacpro_msr(TX_STATE_BIT | TX_STATE_WAIT_FOR_TX);
			// enable TX
			timerIntSetFunction1( SI4432_TXON|SI4432_XTON);
			// start timer with transmit packet timeout
			timerIntTimeout(TimeoutTX_Packet);
			ENABLE_MAC_TIMER_INTERRUPT();
		} else {
			//the channel was busy during the 5ms + random period
			BusyLBT = 0;
			EZMacProLBT_Retrys++;
			if(EZMacProLBT_Retrys < MAX_LBT_RETRIES) {
				//the channel was busy and the retries didn't reach the maximum value
				// disable the reciever
				timerIntSetFunction1(SI4432_XTON);
				// go to the next state
				set_ezmacpro_msr(TX_STATE_BIT | TX_STATE_LBT_START_LISTEN);
				// enable the receiver again
				timerIntSetFunction1( SI4432_RXON|SI4432_XTON);

				// start timer with fix ETSI timeout
				timerIntTimeout(TIMEOUT_LBTI_ETSI);
				ENABLE_MAC_TIMER_INTERRUPT();
			} else {
				//the channel was busy and the retries reach the maximum value
				//disable the receiver
				timerIntSetFunction1(SI4432_XTON);
				//go to the next state
				set_ezmacpro_msr(TX_STATE_BIT | TX_ERROR_CHANNEL_BUSY);
#ifdef FOUR_CHANNEL_IS_USED
				//increment error counter
				timerIntIncrementError(EZMAC_PRO_ERROR_CHANNEL_BUSY);
#endif //FOUR_CHANNEL_IS_USED
				//call LBT error callback function
				EZMacPRO_LBTTimeout();
			}

		}

		break;

#endif//TRANSCEIVER_OPERATION

	case TX_STATE_WAIT_FOR_TX:
		// TX transmit error - no ipksent interrupt before timeout
		// This indicates a problem with the hardware or timeout
		//if there is a TX error then switch back the internal antenna diversity algorthm
#ifdef ANTENNA_DIVERSITY_ENABLED
#ifndef B1_ONLY
		//if revision V2 or A0 chip is used
		if ((EZMacProReg.name.DTR == 0x00) || (EZMacProReg.name.DTR == 0x01)) {

			//switch ON the internal algorithm
			temp8 = timerIntSpiReadReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_2);
			timerIntSpiWriteReg (SI4432_OPERATING_AND_FUNCTION_CONTROL_2, temp8 | 0x80);
			//the gpios control the rf chip automatically
			timerIntSpiWriteReg(SI4432_GPIO1_CONFIGURATION, 0x17);
			timerIntSpiWriteReg(SI4432_GPIO2_CONFIGURATION, 0x18);
		}
#endif//B1_ONLY
#endif//ANTENNA_DIVERSITY_ENABLED
//		DISABLE_MAC_INTERRUPTS();        // clear EX0 & ET0
//		STOP_MAC_TIMER();                // stop Timer
//		CLEAR_MAC_TIMER_INTERRUPT();     // clear flag
//		CLEAR_MAC_EXT_INTERRUPT();

		//go to the TX ERROR STATE
		set_ezmacpro_msr(TX_STATE_BIT | TX_ERROR_STATE);
		//Call the Error state entered callback.
		EZMacPRO_StateErrorEntered();
		break;

#ifdef EXTENDED_PACKET_FORMAT
//#ifndef TRANSMITTER_ONLY_OPERATION
#ifdef TRANSCEIVER_OPERATION
	case TX_STATE_WAIT_FOR_ACK:
		DISABLE_MAC_INTERRUPTS();        // clear EX0 & ET0
		//disbale the interrupts
		timerIntSpiWriteReg( SI4432_INTERRUPT_ENABLE_1, 0x00);
		timerIntSetEnable2(0x00);
		//clear interrupts
		timerIntSpiReadReg(SI4432_INTERRUPT_STATUS_1);
		timerIntSpiReadReg(SI4432_INTERRUPT_STATUS_2);
		//disable the receiver
		timerIntSetFunction1(SI4432_XTON);
//		//go to TX ERROR NO ACK state
//		set_ezmacpro_msr(TX_STATE_BIT | TX_ERROR_NO_ACK);
		//call the no ack callback function
		EZMacPRO_AckTimeout();
		// all done use SECR to determine next state
		timerIntGotoNextStateUsingSECR(1);
		break;
//#endif//TRANSMITTER_ONLY_OPERATION
#endif//TRANSCEIVER_OPERATION
#endif//EXTENDED_PACKET_FORMAT
	case TX_ERROR_STATE:
		DISABLE_MAC_INTERRUPTS();        // clear EX0 & ET0
		// disable all Si443x interrupt sources
		timerIntSpiWriteReg(SI4432_INTERRUPT_ENABLE_1, 0x00);
		timerIntSetEnable2(0x00);
		// clear interrupts
		timerIntSpiReadReg(SI4432_INTERRUPT_STATUS_1);
		timerIntSpiReadReg(SI4432_INTERRUPT_STATUS_2);
		STOP_MAC_TIMER();                // stop Timer
		CLEAR_MAC_TIMER_INTERRUPT();     // clear flag
		CLEAR_MAC_EXT_INTERRUPT();
#ifndef B1_ONLY
		if(EZMacProReg.name.DTR == 0)//if rev V2 chip is used
			// this register setting is need for good current consumption in Idle mode (only rev V2)
			timerIntSpiWriteReg (SI4432_CRYSTAL_OSCILLATOR_CONTROL_TEST, SI4432_BUFOVR);
#endif
		break;
#ifdef TRANSCEIVER_OPERATION
	case TX_ERROR_CHANNEL_BUSY:
		break;
#endif //TRANSCEIVER_OPERATION
	} // end switch
}
#endif // RECEIVER_ONLY_OPERATION not defined


/*------------------------------------------------------------------------------------------------
 * Function Name
 *    timerIntRX_StateMachine()
 *
 * Return Value : None
 * Parameters   : U8 state - RX state, least significant nibbble of MSR
 *
 * This function implements the detailed RX state machine. The state machine is implemented
 * using switch...case... statements. For efficient compilation, the states are a series
 * of sequential continuous integers (no missing values). The states are enumerated in the
 * EZMacPro.h header file.
 *
 * Conditional compile time switches remove the unused RX states if LBT, ACK, or packet
 * forwarding is not supported. These states are also removed from the enumeration at the
 * begriming or end of the list.
 *
 * Conditional compile time switches remove this entire function for the Transmitter only build.
 *
 *-----------------------------------------------------------------------------------------------
 */
#ifndef TRANSMITTER_ONLY_OPERATION
static void timerIntRX_StateMachine (U8 state)
{
	U8 temp8;
#ifdef   PACKET_FORWARDING_SUPPORTED
	U32 timeout;
#endif//PACKET_FORWARDING_SUPPORTED
	switch (state) {
#ifdef FOUR_CHANNEL_IS_USED
	case RX_STATE_FREQUENCY_SEARCH:
		// jump to the next channel if search mechanism is enabled
		if((EZMacProReg.name.RCR & 0x04) == 0x04) {
			timerIntNextRX_Channel();
		}
		// start timer with channel search timeout
		timerIntTimeout(TimeoutChannelSearch);
		ENABLE_MAC_TIMER_INTERRUPT();
		break;
#endif //FOUR_CHANNEL_IS_USED

#ifdef MORE_CHANNEL_IS_USED
	case RX_STATE_FREQUENCY_SEARCH:
		//check the channel number
		if (SelectedChannel < (maxChannelNumber - 1)) {
			//jump to the next channel
			SelectedChannel++;
			//switch off the receiver
			timerIntSetFunction1(SI4432_XTON);
			timerIntSpiWriteReg (SI4432_FREQUENCY_HOPPING_CHANNEL_SELECT,EZMacProReg.array[FR0+SelectedChannel]);
			//switch on the receiver
			timerIntSetFunction1(SI4432_XTON|SI4432_RXON);
			// start timer with channel search timeout
			timerIntTimeout(TimeoutChannelSearch);
			ENABLE_MAC_TIMER_INTERRUPT();
		} else {
			//jump to the first channel
			SelectedChannel = 0;
			//switch off the receiver
			timerIntSetFunction1(SI4432_XTON);
			timerIntSpiWriteReg (SI4432_FREQUENCY_HOPPING_CHANNEL_SELECT,EZMacProReg.array[FR0+SelectedChannel]);
			//switch on the receiver
			timerIntSetFunction1(SI4432_XTON|SI4432_RXON);
			// start timer with channel search timeout
			timerIntTimeout(TimeoutChannelSearch);
			ENABLE_MAC_TIMER_INTERRUPT();
		}
		break;
#endif//MORE_CHANNEL_IS_USED


	case RX_STATE_WAIT_FOR_PACKET:
		//set the dinamic plength if it is needed otherwise set the fix length
		if ((EZMacProReg.name.MCR & 0x04) == 0x04) {
			temp8 = timerIntSpiReadReg(SI4432_HEADER_CONTROL_2);
			temp8 &= ~0x08;
			timerIntSpiWriteReg(SI4432_HEADER_CONTROL_2, temp8);
		} else {
			timerIntSpiWriteReg(SI4432_TRANSMIT_PACKET_LENGTH, EZMacProReg.name.PLEN);
		}

#ifdef MORE_CHANNEL_IS_USED
	case RX_STATE_WAIT_FOR_PREAMBLE:
#endif//MORE_CHANNEL_IS_USED

	case RX_STATE_WAIT_FOR_SYNC:
		// RX error - HW error or bad timeout calculation
		//switch off the receiver
		timerIntSetFunction1(SI4432_XTON);
		// clear interrupt enable 1
		timerIntSpiWriteReg(SI4432_INTERRUPT_ENABLE_1, 0x00);

		//go to the next channel
		set_ezmacpro_msr(RX_STATE_BIT | RX_STATE_FREQUENCY_SEARCH);

#ifdef FOUR_CHANNEL_IS_USED
		//Enable the Preamble valid interrupt
		timerIntSetEnable2(SI4432_ENPREAVAL);
		// jump to the next channel if search mechanism is enabled
		if((EZMacProReg.name.RCR & 0x04) == 0x04) {
			timerIntNextRX_Channel();

			//enable the receiver
			timerIntSetFunction1(SI4432_RXON|SI4432_XTON);

			// start timer with channel search timeout
			timerIntTimeout(TimeoutChannelSearch); /* mark by David, 搜索多个通道时才需要设置超时 */
			ENABLE_MAC_TIMER_INTERRUPT();

		} else {
			//enable the receiver
			timerIntSetFunction1(SI4432_RXON|SI4432_XTON);
		}
#endif//FOUR_CHANNEL_IS_USED

#ifdef MORE_CHANNEL_IS_USED
		//enable the preamble valid interrupt
		timerIntSetEnable2(SI4432_ENPREAVAL);
		//determine the next channel number
		if (SelectedChannel < (maxChannelNumber - 1)) {
			SelectedChannel++;
		} else {
			SelectedChannel = 0;
		}
		//jump to the next channel
		timerIntSpiWriteReg (SI4432_FREQUENCY_HOPPING_CHANNEL_SELECT,EZMacProReg.array[FR0+SelectedChannel]);

		//enable the receiver
		timerIntSetFunction1(SI4432_RXON|SI4432_XTON);

		// start timer with channel search timeout
		timerIntTimeout(TimeoutChannelSearch);
		ENABLE_MAC_TIMER_INTERRUPT();
#endif//MORE_CHANNEL_IS_USED
		break;

#ifdef EXTENDED_PACKET_FORMAT
#ifndef RECEIVER_ONLY_OPERATION
	case RX_STATE_WAIT_FOR_SEND_ACK:
		// SW error
#ifdef ANTENNA_DIVERSITY_ENABLED
#ifndef B1_ONLY
		//if revision V2 or A0 chip is used
		if ((EZMacProReg.name.DTR == 0x00) || (EZMacProReg.name.DTR == 0x01)) {
			//switch ON the internal algorithm
			temp8 = timerIntSpiReadReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_2);
			timerIntSpiWriteReg (SI4432_OPERATING_AND_FUNCTION_CONTROL_2, temp8 | 0x80);
			//the gpios control the rf chip automatically
			timerIntSpiWriteReg(SI4432_GPIO1_CONFIGURATION, 0x17);
			timerIntSpiWriteReg(SI4432_GPIO2_CONFIGURATION, 0x18);
		}
#endif//B1_ONLY
#endif//ANTENNA_DIVERSITY_ENABLED
#ifndef PACKET_FORWARDING_SUPPORTED
		//set back the preamble length
#ifdef FOUR_CHANNEL_IS_USED
		timerIntSpiWriteReg(SI4432_PREAMBLE_LENGTH, PreamRegValue);
#endif//FOUR_CHANNEL_IS_USED
#ifdef MORE_CHANNEL_IS_USED
		temp8 = timerIntSpiReadReg(SI4432_HEADER_CONTROL_2);
		timerIntSpiWriteReg(SI4432_HEADER_CONTROL_2, (temp8 | 0x01));
		timerIntSpiWriteReg(SI4432_PREAMBLE_LENGTH, PreamRegValue);
#endif//MORE_CHANNEL_IS_USED
#endif//PACKET_FORWARDING_SUPPORTED
		//go to the RX ERROR STATE
		set_ezmacpro_msr((RX_STATE_BIT | RX_ERROR_STATE));
		//Call the Error state entered callback.
		EZMacPRO_StateErrorEntered();
		break;
#endif//RECEIVER_ONLY_OPERATION
#endif//EXTENDED_PACKET_FORMAT

#ifdef   PACKET_FORWARDING_SUPPORTED
#ifdef   TRANSCEIVER_OPERATION
	case RX_STATE_FORWARDING_LBT_START_LISTEN:
		if(BusyLBT == 0) {
			//the channel was free during the first 0.5ms
			// disable the receiver
			timerIntSetFunction1(SI4432_XTON);

			ChannelOccupiedInStartPeriod = 0;

			//go to the next state
			set_ezmacpro_msr(RX_STATE_BIT | RX_STATE_FORWARDING_LBT_LISTEN);
			// enable the receiver again
			timerIntSetFunction1( SI4432_RXON|SI4432_XTON);

			// start timer with fix 4.5ms timeout
			timerIntTimeout(LBT_FIXED_TIME_4500US);
			ENABLE_MAC_TIMER_INTERRUPT();
		} else {
			//the channnel was busy during the first 0.5ms
			// disable the receiver
			timerIntSetFunction1(SI4432_XTON);

			BusyLBT = 0;
			ChannelOccupiedInStartPeriod = 1;
			ChannelOccupiedCounter = 0;

			// go to the next state
			set_ezmacpro_msr(RX_STATE_BIT | RX_STATE_FORWARDING_LBT_LISTEN);
			// enable the receiver again
			timerIntSetFunction1( SI4432_RXON|SI4432_XTON);

			// start timer with fix 1ms timeout
			timerIntTimeout(LBT_FIXED_TIME_1000US);
			ENABLE_MAC_TIMER_INTERRUPT();
		}
		break;


	case RX_STATE_FORWARDING_LBT_LISTEN:
		if (ChannelOccupiedInStartPeriod == 0) {
			//the channel was free during the 0.5ms start period

			if(BusyLBT == 0) {
				// LBT passed, channel should be clear in the fix 4.5ms period
				//disable the receiver
				timerIntSetFunction1(SI4432_XTON);
				// clear enable 2 interrupt
				timerIntSetEnable2(0x00);
				// enable ENPKSENT bit
				timerIntSpiWriteReg(SI4432_INTERRUPT_ENABLE_1, SI4432_ENPKSENT);

				//go to the next state
				set_ezmacpro_msr(RX_STATE_BIT | RX_STATE_FORWARDING_WAIT_FOR_TX);
				// enable TX
				timerIntSetFunction1( SI4432_TXON|SI4432_XTON);

				// start timer with packet transmit timeout
				timerIntTimeout(TimeoutTX_Packet);
				ENABLE_MAC_TIMER_INTERRUPT();
			} else {
				//the channel was busy during the 4.5ms
				BusyLBT = 0;

				// disable the receiver
				timerIntSetFunction1(SI4432_XTON);


				//got to the next state
				set_ezmacpro_msr(RX_STATE_BIT | RX_STATE_FORWARDING_LBT_RANDOM_LISTEN);

				// multiple by fixed plus random number
				timeout = LBT_FIXED_TIME_5000US + TimeoutLBTI * (U32)(timerIntRandom());
				// enable the receiver again
				timerIntSetFunction1( SI4432_RXON|SI4432_XTON);
				// start timer with timeout
				timerIntTimeout(timeout);
				ENABLE_MAC_TIMER_INTERRUPT();
			}
		} else {
			//the channel was busy during the 0.5ms start period
			if(BusyLBT == 0) {
				// LBT passed, channel should be clear in the fix 1ms period
				// multiple by fixed plus random number
				timeout = LBT_FIXED_TIME_5000US + TimeoutLBTI * (U32)(timerIntRandom());

				//go to the next state
				set_ezmacpro_msr(RX_STATE_BIT | RX_STATE_FORWARDING_LBT_RANDOM_LISTEN);

				// start timer with timeout
				timerIntTimeout(timeout);
				ENABLE_MAC_TIMER_INTERRUPT();
			} else {
				//the channel was busy in the fix 1ms period
				if (ChannelOccupiedCounter < 9) {
					BusyLBT = 0;
					ChannelOccupiedCounter++;
					//go to the next state
					set_ezmacpro_msr(RX_STATE_BIT | RX_STATE_FORWARDING_LBT_LISTEN);
					// disable the receiver
					timerIntSetFunction1(SI4432_XTON);
					// enable the receiver again
					timerIntSetFunction1( SI4432_RXON|SI4432_XTON);
					// start timer with fix 1ms timeout
					timerIntTimeout(LBT_FIXED_TIME_1000US);
					ENABLE_MAC_TIMER_INTERRUPT();
				} else {
					//the channel was busy during the 10*1ms
					BusyLBT = 0;
					EZMacProLBT_Retrys++;
					if(EZMacProLBT_Retrys < MAX_LBT_RETRIES) {
						//the channel was busy and the retries didn't reach the maximum value
						// disable the reciever
						timerIntSetFunction1(SI4432_XTON);
						//go to the next state
						set_ezmacpro_msr(RX_STATE_BIT | RX_STATE_FORWARDING_LBT_START_LISTEN);

						// enable the receiver again
						timerIntSetFunction1( SI4432_RXON|SI4432_XTON);

						// start timer with fix ETSI timeout
						timerIntTimeout(TIMEOUT_LBTI_ETSI);
						ENABLE_MAC_TIMER_INTERRUPT();
					} else {
						//the channel was busy and the retries reach the maximum value
						//disable the receiver
						timerIntSetFunction1(SI4432_XTON);
						//go to the next state
						set_ezmacpro_msr(RX_STATE_BIT | RX_ERROR_FORWARDING_WAIT_FOR_TX);
#ifdef  FOUR_CHANNEL_IS_USED
						//increment error counter
						timerIntIncrementError(EZMAC_PRO_ERROR_CHANNEL_BUSY);
#endif//FOUR_CHANNEL_IS_USED
						//call the LBT error callback function
						EZMacPRO_LBTTimeout();
					}
				}
			}

		}
		break;

	case RX_STATE_FORWARDING_LBT_RANDOM_LISTEN:
		if(BusyLBT == 0) {
			// the channel was free during the 5ms + random period
			//disable the receiver
			timerIntSetFunction1(SI4432_XTON);
			// clear enable 2 interrupt
			timerIntSetEnable2(0x00);
			// enable ENPKSENT bit
			timerIntSpiWriteReg(SI4432_INTERRUPT_ENABLE_1, SI4432_ENPKSENT);
			//go to the next state
			set_ezmacpro_msr(RX_STATE_BIT | RX_STATE_FORWARDING_WAIT_FOR_TX);
			// enable TX
			timerIntSetFunction1( SI4432_TXON|SI4432_XTON);
			// start timer with fix transmit packet timeout
			timerIntTimeout(TimeoutTX_Packet);
			ENABLE_MAC_TIMER_INTERRUPT();
		} else {
			//the channel was busy during the 5ms + random period
			BusyLBT = 0;
			EZMacProLBT_Retrys++;
			if(EZMacProLBT_Retrys < MAX_LBT_RETRIES) {
				//the channel was busy and the retries didn't reach the maximum value
				// disable the reciever
				timerIntSetFunction1(SI4432_XTON);
				// go to the next state
				set_ezmacpro_msr(RX_STATE_BIT | RX_STATE_FORWARDING_LBT_START_LISTEN);
				// enable the receiver again
				timerIntSetFunction1( SI4432_RXON|SI4432_XTON);

				// start timer with fix ETSI timeout
				timerIntTimeout(TIMEOUT_LBTI_ETSI);
				ENABLE_MAC_TIMER_INTERRUPT();
			} else {
				//the channel was busy and the retries reach the maximum value
				//disable the receiver
				timerIntSetFunction1(SI4432_XTON);
				//go to the next state
				set_ezmacpro_msr(RX_STATE_BIT | RX_ERROR_FORWARDING_WAIT_FOR_TX);
#ifdef FOUR_CHANNEL_IS_USED
				//increment the error counter
				timerIntIncrementError(EZMAC_PRO_ERROR_CHANNEL_BUSY);
#endif//FOUR_CHANNEL_IS_USED
				//cll the LBT error callback function
				EZMacPRO_LBTTimeout();
			}

		}

		break;

#endif   // TRANSCEIVER_OPERATION
	case RX_STATE_FORWARDING_WAIT_FOR_TX:
		// TX timeout - HW error or bad timeout calculation
		//if there is a TX error then switch back the internal antenna diversity algorthm
#ifdef ANTENNA_DIVERSITY_ENABLED
#ifndef B1_ONLY
		//if revision V2 or A0 chip is used
		if ((EZMacProReg.name.DTR == 0x00) || (EZMacProReg.name.DTR == 0x01)) {

			//switch ON the internal algorithm
			temp8 = timerIntSpiReadReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_2);
			timerIntSpiWriteReg (SI4432_OPERATING_AND_FUNCTION_CONTROL_2, temp8 | 0x80);
			//the gpios control the rf chip automatically
			timerIntSpiWriteReg(SI4432_GPIO1_CONFIGURATION, 0x17);
			timerIntSpiWriteReg(SI4432_GPIO2_CONFIGURATION, 0x18);
		}
#endif//B1_ONLY
#endif//ANTENNA_DIVERSITY_ENABLED
		set_ezmacpro_msr(RX_STATE_BIT | RX_ERROR_STATE);
		//Call the Error state entered callback.
		EZMacPRO_StateErrorEntered();
		break;
#endif   // PACKET_FORWARDING_SUPPORTED

	case RX_ERROR_STATE:
		DISABLE_MAC_INTERRUPTS();        // clear EX0 & ET0
		// disable all Si443x interrupt sources
		timerIntSpiWriteReg(SI4432_INTERRUPT_ENABLE_1, 0x00);
		timerIntSetEnable2(0x00);
		// clear interrupts
		timerIntSpiReadReg(SI4432_INTERRUPT_STATUS_1);
		timerIntSpiReadReg(SI4432_INTERRUPT_STATUS_2);
		STOP_MAC_TIMER();                // stop Timer
		CLEAR_MAC_TIMER_INTERRUPT();     // clear flag
		CLEAR_MAC_EXT_INTERRUPT();
#ifndef B1_ONLY
		if(EZMacProReg.name.DTR == 0)//if rev V2 chip is used
			// this register setting is need for good current consumption in Idle mode (only rev V2)
			timerIntSpiWriteReg (SI4432_CRYSTAL_OSCILLATOR_CONTROL_TEST, SI4432_BUFOVR);
#endif//B1_ONLY
		break;
	default:
		break;
	}  // end switch
}
#endif   // TRANSMITTER_ONLY_OPERATION not defined


/*-----------------------------------------------------------------------------------------------
 * Function Name
 *    timerIntRandom()
 *
 * Return Value : U8 4-bit random number
 * Parameters   : None
 *
 * Notes:
 *
 * This function provides a pseudo random number.
 * It uses 8-bit multiply and shift to generate the next random number.
 *
 * The constants used are based on the largest possible primes that will
 * satisfy the linear congruent criteria.
 *
 * The pseudo random sequence will repeat every 256 times.
 * The sequence always starts at the same point. If the application
 * requires a different sequence each time on reset, a truly random
 * seed may be required.
 *
 * This function is only included if LISTEN_BEFORE_TALK_SUPPORTED is defined.
 *
 *-----------------------------------------------------------------------------------------------
 */
#ifdef TRANSCEIVER_OPERATION
static U8 timerIntRandom (void)
{
	U8 temp8;
	// 61 is largest prime less than 256/4
	EZMacProRandomNumber *= 245;        // 4 * 61 + 1 = 245
	EZMacProRandomNumber += 251;        // 251 is the largest prime < 256

	temp8 = EZMacProRandomNumber>>4;
	if (temp8 == 0)
		return 1;
	else
		return temp8;
}
#endif //TRANSCEIVER_OPERATION


/*------------------------------------------------------------------------------------------------
 * Function Name
 *    timerIntDisableInterrupts()
 *
 * Return Value : None
 * Parameters   : None
 *
 * This function is used to disable all interrupts at the completion of a state. Using it saves
 * a few bytes of code.
 *-----------------------------------------------------------------------------------------------
 */
static void timerIntDisableInterrupts (void)
{
	// disable timer since no subsequent timeout is used
	DISABLE_MAC_TIMER_INTERRUPT();
	// clear Interrupt Enable 1 register
	timerIntSpiWriteReg(SI4432_INTERRUPT_ENABLE_1, 0x00);
	// clear Interrupt Enable 2 register
	timerIntSetEnable2 (0x00);
}


/*------------------------------------------------------------------------------------------------
 * Function Name
 *    timerIntNextRX_Channel()
 *
 * Return Value : None
 * Parameters   : None
 *
 * Notes:
 *
 * This function will advance to the next channel. The channel is incremented and then checked
 * against the frequency mask in the MAC RCR register. Setting all mask bits is not permitted.
 *
 * This function is not included for the Transmitter only configuration.
 *
 *-----------------------------------------------------------------------------------------------
 */
#ifndef TRANSMITTER_ONLY_OPERATION
#ifndef MORE_CHANNEL_IS_USED
static void timerIntNextRX_Channel (void)
{
	U8  mask;
	U8  n;

	n = EZMacProCurrentChannel;

	//initialize mask
	mask = 0x08;
	mask <<= n;

	do {
		n++;                             // increment n
		n &= 0x03;                       // wrap modulo 4
		mask <<=1 ;                      // shift mask left
		if(mask == 0x80) mask = 0x08;    // wrap mask to bits 3-6

		// continue until unmasked bit is found
	} while ((EZMacProReg.name.RCR & mask) == mask);

	//switch off the receiver
	timerIntSetFunction1(SI4432_XTON);
	timerIntSpiWriteReg (SI4432_FREQUENCY_HOPPING_CHANNEL_SELECT,EZMacProReg.array[FR0+n]);
	//switch on the receiver
	timerIntSetFunction1(SI4432_XTON|SI4432_RXON);

	EZMacProCurrentChannel = n;
}
#endif//MORE_CHANNEL_IS_USED
#endif // TRANSMITTER_ONLY_OPERATION not defined


/*------------------------------------------------------------------------------------------------
 * Function Name
 *    timerIntGotoNextStateUsingSECR()
 *
 * Return Value : None
 * Parameters   : U8 tx - use SATX bits if non-zero
 *
 * Notes:
 *
 * This function is used to put the radio into a programmable state depending on the contents
 * of the MAC SECR register. After transmit the SATX bits are used. After RX the SARX bits are
 * used.
 *
 * A U8 (unsigned char) is used for the boolean tx parameter instead of a bit, because a U8 is
 * passed in a register.
 *
 *-----------------------------------------------------------------------------------------------
 */
static void timerIntGotoNextStateUsingSECR( U8 tx)
{
	U8 next;

#ifndef TRANSMITTER_ONLY_OPERATION
	U8 temp8;
#endif//TRANSMITTER_ONLY_OPERATION

	//Disable All interrupts
	timerIntDisableInterrupts();
#ifndef TRANSMITTER_ONLY_OPERATION
#ifndef B1_ONLY
	if(EZMacProReg.name.DTR == 0) //if rev V2 chip is used
		//set the RX deviation in all case (only rev V2)
		timerIntSpiWriteReg (SI4432_FREQUENCY_DEVIATION, RX_Freq_dev);
#endif//B1_ONLY
#endif//TRANSMITTER_ONLY_OPERATON
	//determine the next state
	if (tx) {
		//the next state after transmit
		next = EZMacProReg.name.SECR >> 6;
	} else {
		//the next state after receive
		next =  (EZMacProReg.name.SECR >> 4) & 0x03;
	}

	if(next == 0) {
		//go to sleep mode
		// stop XTAL ( TX, RX, PLLON)
		timerIntSetFunction1 (0x00);
		set_ezmacpro_msr(EZMAC_PRO_SLEEP);
		//Call the Sleep state entered callback.
		EZMacPRO_StateSleepEntered();
	}
#ifndef TRANSMITTER_ONLY_OPERATION
	else if(next == 2) {
		//go to RX mode

		// clear RX FIFO
		temp8 = timerIntSpiReadReg (SI4432_OPERATING_AND_FUNCTION_CONTROL_2);
		temp8 |= SI4432_FFCLRRX;
		timerIntSpiWriteReg (SI4432_OPERATING_AND_FUNCTION_CONTROL_2, temp8);
		temp8 &= ~SI4432_FFCLRRX;
		timerIntSpiWriteReg (SI4432_OPERATING_AND_FUNCTION_CONTROL_2, temp8);

		//go to the next state
		set_ezmacpro_msr(RX_STATE_BIT | RX_STATE_FREQUENCY_SEARCH);

#ifdef FOUR_CHANNEL_IS_USED
		// enable preamble detect interrupt
		timerIntSetEnable2 (SI4432_ENPREAVAL);

		if((EZMacProReg.name.RCR & 0x04) == 0x04) {
			//jump to the next channel
			timerIntNextRX_Channel();

			//start receiver
			timerIntSetFunction1 (SI4432_RXON|SI4432_XTON);
			ENABLE_MAC_EXT_INTERRUPT();

			// start timer with channel search timeout
			timerIntTimeout(TimeoutChannelSearch);	/* mark by David, 搜索多个通道时需要设置超时 */
			ENABLE_MAC_TIMER_INTERRUPT();
		} else {
			//start receiver
			timerIntSetFunction1 (SI4432_RXON|SI4432_XTON);
			ENABLE_MAC_EXT_INTERRUPT();
		}
#endif//FOUR_CHANNEL_IS_USED

#ifdef MORE_CHANNEL_IS_USED
		//enable the preamble detect interrupt
		timerIntSetEnable2(SI4432_ENPREAVAL);
		//select the nex channel
		SelectedChannel = 0;
		timerIntSpiWriteReg (SI4432_FREQUENCY_HOPPING_CHANNEL_SELECT,EZMacProReg.array[FR0+SelectedChannel]);

		//start receiver
		timerIntSetFunction1 (SI4432_RXON|SI4432_XTON);
		ENABLE_MAC_EXT_INTERRUPT();
		// start timer with channel search timeout
		timerIntTimeout(TimeoutChannelSearch);
		ENABLE_MAC_TIMER_INTERRUPT();
#endif//MORE_CHANNEL_IS_USED

	}
#endif // TRANSMITTER_ONLY_OPERATION
	else { //  default
		// go to Idle mode
		// disable RX & TX
		timerIntSetFunction1 (SI4432_XTON);
#ifndef B1_ONLY
		if(EZMacProReg.name.DTR == 0)//if rev V2 chip is used
			//this register setting is need for good current consumption in Idle mode (only rev V2)
			timerIntSpiWriteReg (SI4432_CRYSTAL_OSCILLATOR_CONTROL_TEST, SI4432_BUFOVR);
#endif//B1_ONLY
#ifdef ANTENNA_DIVERSITY_ENABLED
#ifndef B1_ONLY
		//if revision V2 or A0 chip is used
		if ((EZMacProReg.name.DTR == 0x00) || (EZMacProReg.name.DTR == 0x01)) {

			//switch BACK the internal algorithm
			temp8 = timerIntSpiReadReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_2);
			timerIntSpiWriteReg (SI4432_OPERATING_AND_FUNCTION_CONTROL_2, temp8 | 0x80);
			//the gpios control the rf chip automatically
			timerIntSpiWriteReg(SI4432_GPIO1_CONFIGURATION, 0x17);
			timerIntSpiWriteReg(SI4432_GPIO2_CONFIGURATION, 0x18);
		}
#endif//B1_ONLY
#endif//ANTENNA_DIVERSITY_ENABLED

		set_ezmacpro_msr(EZMAC_PRO_IDLE);
		//Call the Idle state entered callback function.
		EZMacPRO_StateIdleEntered();
	}
}


/*------------------------------------------------------------------------------------------------
 * Function Name
 *    timerIntIncrementError()
 *
 * Return Value : None
 * Parameters   : U8 mask - SECR error mask
 *
 * Notes:
 *
 * This function will increment the appropriate error counter register if the error type is
 * enabled in the MAC SECR register. The error codes for the SECR register are defined in the
 * EZMacPro.h header file. First the error code is compared to the error mask in the SECR
 * register. The counting is enabled for the particular type of error, the error count is
 * incremented. The error count corresponding to the current frequency is incremented.
 *
 * This function is not included for the Transmitter only configuration.
 *------------------------------------------------------------------------------------------------
 */
#ifdef FOUR_CHANNEL_IS_USED
#ifndef TRANSMITTER_ONLY_OPERATION
static void timerIntIncrementError (U8 mask)
{
	// mask is SECR error bit mask
	mask &= 0x0F;                          // ignore upper nibble

	if((EZMacProReg.name.SECR & mask) == mask ) {
		if(EZMacProReg.array[EC0+EZMacProCurrentChannel] < 255)
			EZMacProReg.array[EC0+EZMacProCurrentChannel]++;
	}

	return;
}
#endif // TRANSMITTER_ONLY_OPERATION not defined
#endif //FOUR_CHANNEL_IS_USED


/*------------------------------------------------------------------------------------------------
 * Function Name
 *    timerIntSetEnable2()
 *
 * Return Value : None
 * Parameters   : U8 value
 *
 * Notes:
 *
 * This function is used instead of timerIntSpiWriteReg() when writing to the
 * SI4432_INTERRUPT_ENABLE_2 register. This function adds support for the low frequency timer
 * and low battery detector, if the build options are defined. If not, the register is written
 * directly.
 *
 * This function is not included for the Transmitter only configuration.
 *-----------------------------------------------------------------------------------------------
 */
static void timerIntSetEnable2(U8 value)
{
	if((EZMacProReg.name.LFTMR2 & 0x80)==0x80)
		value |= SI4432_ENWUT;

	if((EZMacProReg.name.LBDR & 0x80)==0x80)
		value |= SI4432_ENLBDI;

	timerIntSpiWriteReg(SI4432_INTERRUPT_ENABLE_2, value);

	return;
}


/*------------------------------------------------------------------------------------------------
 * Function Name
 *    timerIntSetFunction1()
 *
 * Return Value : None
 * Parameters   : U8 value
 *
 * This function is used instead of timerIntSpiWriteReg() when writing to the
 * SI4432_OPERATING_AND_FUNCTION_CONTROL_1 register. This function adds support for the
 * low frequency timer and low battery detector, if the build options are defined. If not,
 * the register is written directly.
 *
 * This function is not included for the Transmitter only configuration.
 *------------------------------------------------------------------------------------------------
 */
#ifndef TRANSMITTER_ONLY_OPERATION
static void timerIntSetFunction1(U8 value)
{
	if((EZMacProReg.name.LFTMR2 & 0x80)==0x80) {
		value |= SI4432_ENWT;
		ENABLE_MAC_EXT_INTERRUPT();
	}

	if((EZMacProReg.name.LFTMR2 & 0x40)==0x00)
		value |= SI4432_X32KSEL;

	if((EZMacProReg.name.LBDR & 0x80)==0x80)
		value |= SI4432_ENLBD;

	ezmacprof_debug(("func:%s(), line:%d, SI4432_OPERATING_AND_FUNCTION_CONTROL_1:0x%2x --> 0x%2x\n"
			,__FUNCTION__, __LINE__, macSpiReadReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_1)
			,value));

	timerIntSpiWriteReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_1, value);

	return;
}
#endif//TRANSMITTER_ONLY_OPERATION


