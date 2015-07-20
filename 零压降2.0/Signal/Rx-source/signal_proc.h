#ifndef SIGNAL_PROC_H__
#define SIGNAL_PROC_H__

/* 是否打印光纤调试数据 */
#define DEBUG_DATA_TRANS  0

/* 是否使用ADC的原始数据 */
#define USE_ORG_ADC_VALUE 0

/* 不进行插值吗? */
#define DONT_INSERT_VAL   0


/*  是否禁止调整某相的零位、幅值、对称度、相位 */
#if 1
#define DISENABLE_ADJ_AMPLIFY		  0
#define DISENABLE_ADJ_AMP_FACTOR_POSITIVE 0
#define DISENABLE_ADJ_PHASE_TIME	  0
#else
#define DISENABLE_ADJ_AMPLIFY		  1
#define DISENABLE_ADJ_AMP_FACTOR_POSITIVE 1
#define DISENABLE_ADJ_PHASE_TIME	  1
#endif


/*
 * delta值的更新, 以及估算
 */
/* 8, 7, 5, 6, 4, 3  */
#define NUM_OF_RECORDING_DELTA_Y (1)

#if (8==NUM_OF_RECORDING_DELTA_Y)
#define UPDATE_OLD_DELTA_Y(delta_y, delta_y_old)  do {\
    delta_y_old[0] = delta_y_old[1];\
    delta_y_old[1] = delta_y_old[2];\
    delta_y_old[2] = delta_y_old[3];\
    delta_y_old[3] = delta_y_old[4];\
    delta_y_old[4] = delta_y_old[5];\
    delta_y_old[5] = delta_y_old[6];\
    delta_y_old[6] = delta_y_old[7];\
    delta_y_old[7] = delta_y;\
}while(0)
#define ESTIMATE_DELTA_Y_VALUE(delta_y_seq) ((((delta_y_seq[7]) + (delta_y_seq[6]) + (delta_y_seq[5]) + (delta_y_seq[4]) +\
                                            delta_y_seq[3]) + (delta_y_seq[2]) + (delta_y_seq[1]) + (delta_y_seq[0])) / 8)
#elif (7==NUM_OF_RECORDING_DELTA_Y)
#define UPDATE_OLD_DELTA_Y(delta_y, delta_y_old)  do {\
    delta_y_old[0] = delta_y_old[1];\
    delta_y_old[1] = delta_y_old[2];\
    delta_y_old[2] = delta_y_old[3];\
    delta_y_old[3] = delta_y_old[4];\
    delta_y_old[4] = delta_y_old[5];\
    delta_y_old[5] = delta_y_old[6];\
    delta_y_old[6] = delta_y;\
}while(0)
#define ESTIMATE_DELTA_Y_VALUE(delta_y_seq) ((((delta_y_seq[6]) + (delta_y_seq[5]) + (delta_y_seq[4]) +\
                                            delta_y_seq[3]) + (delta_y_seq[2]) + (delta_y_seq[1]) + (delta_y_seq[0])) / 7)
#elif (6==NUM_OF_RECORDING_DELTA_Y)
#define UPDATE_OLD_DELTA_Y(delta_y, delta_y_old)  do {\
    delta_y_old[0] = delta_y_old[1];\
    delta_y_old[1] = delta_y_old[2];\
    delta_y_old[2] = delta_y_old[3];\
    delta_y_old[3] = delta_y_old[4];\
    delta_y_old[4] = delta_y_old[5];\
    delta_y_old[5] = delta_y;\
}while(0)
#define ESTIMATE_DELTA_Y_VALUE(delta_y_seq) ((((delta_y_seq[5]) + (delta_y_seq[4]) +\
                                            delta_y_seq[3]) + (delta_y_seq[2]) + (delta_y_seq[1]) + (delta_y_seq[0])) / 6)
#elif (5==NUM_OF_RECORDING_DELTA_Y)
#define UPDATE_OLD_DELTA_Y(delta_y, delta_y_old)  do {\
    delta_y_old[0] = delta_y_old[1];\
    delta_y_old[1] = delta_y_old[2];\
    delta_y_old[2] = delta_y_old[3];\
    delta_y_old[3] = delta_y_old[4];\
    delta_y_old[4] = delta_y;\
}while(0)
#define ESTIMATE_DELTA_Y_VALUE(delta_y_seq) ((((delta_y_seq[4]) +\
                                            delta_y_seq[3]) + (delta_y_seq[2]) + (delta_y_seq[1]) + (delta_y_seq[0])) / 5)
#elif (4==NUM_OF_RECORDING_DELTA_Y)
#define UPDATE_OLD_DELTA_Y(delta_y, delta_y_old)  do {\
    delta_y_old[0] = delta_y_old[1];\
    delta_y_old[1] = delta_y_old[2];\
    delta_y_old[2] = delta_y_old[3];\
    delta_y_old[3] = delta_y;\
}while(0)
#define ESTIMATE_DELTA_Y_VALUE(delta_y_seq) (((\
                                            delta_y_seq[3]) + (delta_y_seq[2]) + (delta_y_seq[1]) + (delta_y_seq[0])) / 4)
#elif (3==NUM_OF_RECORDING_DELTA_Y)
#define UPDATE_OLD_DELTA_Y(delta_y, delta_y_old)  do {\
    delta_y_old[0] = delta_y_old[1];\
    delta_y_old[1] = delta_y_old[2];\
    delta_y_old[2] = delta_y;\
}while(0)
#define ESTIMATE_DELTA_Y_VALUE(delta_y_seq) (((delta_y_seq[2]) + (delta_y_seq[1]) + (delta_y_seq[0])) / 3)

#elif (2==NUM_OF_RECORDING_DELTA_Y)
#define UPDATE_OLD_DELTA_Y(delta_y, delta_y_old)  do {\
    delta_y_old[0] = delta_y_old[1];\
    delta_y_old[1] = delta_y;\
}while(0)
#define ESTIMATE_DELTA_Y_VALUE(delta_y_seq) (((delta_y_seq[1]) + (delta_y_seq[0])) / 2)

#elif (1==NUM_OF_RECORDING_DELTA_Y)
#define UPDATE_OLD_DELTA_Y(delta_y, delta_y_old)  do {\
    delta_y_old[0] = delta_y;\
}while(0)
#define ESTIMATE_DELTA_Y_VALUE(delta_y_seq) (delta_y_seq[0])

#else
    #error "modify delta_y_old[n] = delta_y_old[n+1];"
#endif


/*
 * 对delta值以及相关数据进行处理
 */
#if 1!=NUM_OF_RECORDING_DELTA_Y
#define	proc_delta_val(px_org_val_prev, px_delta_y_old, px_estimate_delta_y)  do{\
	px_delta_y = px_org_val - (px_org_val_prev);\
	(px_org_val_prev) = px_org_val;\
	UPDATE_OLD_DELTA_Y(px_delta_y, (px_delta_y_old));\
	px_estimate_delta_y = ESTIMATE_DELTA_Y_VALUE((px_delta_y_old));\
	} while(0)
#else
#define	proc_delta_val(px_org_val_prev, px_delta_y_old, px_estimate_delta_y)  do{\
	px_estimate_delta_y = px_org_val - (px_org_val_prev);\
	(px_org_val_prev)   = px_org_val;\
	} while(0)
#endif



/*
 * 调整某相的幅值、对称度、相位
 */
#if (0==DISENABLE_ADJ_AMPLIFY && 0==DISENABLE_ADJ_AMP_FACTOR_POSITIVE && 0==DISENABLE_ADJ_PHASE_TIME)
#define adj_px_adc_val(px_zero_position_value, px_amplify_adj, px_amp_factor_positive_adj) do{\
	/* 调整幅值 */\
	px_org_val = (px_zero_position_value) + ((px_org_val - (px_zero_position_value))\
		* ((px_amplify_adj) & 0xffff) / PX_AMPLIFY_BASE);\
	/* 调整对称度-调整正半周放大倍数 */\
	if (px_org_val >= (px_zero_position_value))\
		px_org_val = (px_zero_position_value)\
				+ ((px_org_val - (px_zero_position_value))\
				* ((px_amp_factor_positive_adj) & 0xffff) / PX_AMP_FACTOR_P_BASE);\
	if (px_org_val < 0)\
		px_org_val = 0;\
	else if (px_org_val > 65535)\
		px_org_val = 65535;\
	} while (0)

#define adj_px_phase(px_phase_time_adj, px_estimate_delta_y) do {\
	/* 调整相位 */\
	px_org_val += px_estimate_delta_y * ((px_phase_time_adj & 0xffff) - PX_PHASE_TIME_BASE)\
				/ (PX_PHASE_TIME_BASE);\
	if (px_org_val < 0)\
		px_org_val = 0;\
	else if (px_org_val > 65535)\
		px_org_val = 65535;\
	} while(0)
#endif

#if (0==DISENABLE_ADJ_AMPLIFY && 0==DISENABLE_ADJ_AMP_FACTOR_POSITIVE && 0!=DISENABLE_ADJ_PHASE_TIME)
#define adj_px_adc_val(px_zero_position_value, px_amplify_adj, px_amp_factor_positive_adj, px_phase_time_adj, px_estimate_delta_y) do{\
	/* 调整幅值 */\
	px_org_val = (px_zero_position_value) + ((px_org_val - (px_zero_position_value))\
		* ((px_amplify_adj) & 0xffff) / PX_AMPLIFY_BASE);\
	/* 调整对称度-调整正半周放大倍数 */\
	if (px_org_val >= (px_zero_position_value))\
		px_org_val = (px_zero_position_value)\
				+ ((px_org_val - (px_zero_position_value))\
				* ((px_amp_factor_positive_adj) & 0xffff)\
				/ PX_AMP_FACTOR_P_BASE);\
	} while(0)
#endif

#if (0==DISENABLE_ADJ_AMPLIFY && 0!=DISENABLE_ADJ_AMP_FACTOR_POSITIVE && 0==DISENABLE_ADJ_PHASE_TIME)
#define adj_px_adc_val(px_zero_position_value, px_amplify_adj, px_amp_factor_positive_adj, px_phase_time_adj, px_estimate_delta_y) do{\
	/* 调整幅值 */\
	px_org_val = (px_zero_position_value) + ((px_org_val - (px_zero_position_value))\
		* ((px_amplify_adj) & 0xffff) / PX_AMPLIFY_BASE);\
	/* 调整相位 */\
	px_org_val += px_estimate_delta_y * ((px_phase_time_adj & 0xffff) - PX_PHASE_TIME_BASE)\
				/ (PX_PHASE_TIME_BASE);\
	} while(0)
#endif

#if (0==DISENABLE_ADJ_AMPLIFY && 0!=DISENABLE_ADJ_AMP_FACTOR_POSITIVE && 0!=DISENABLE_ADJ_PHASE_TIME)
#define adj_px_adc_val(px_zero_position_value, px_amplify_adj, px_amp_factor_positive_adj, px_phase_time_adj, px_estimate_delta_y) do{\
	/* 调整幅值 */\
	px_org_val = (px_zero_position_value) + ((px_org_val - (px_zero_position_value))\
		* ((px_amplify_adj) & 0xffff) / PX_AMPLIFY_BASE);\
	} while(0)
#endif

#if (0!=DISENABLE_ADJ_AMPLIFY && 0==DISENABLE_ADJ_AMP_FACTOR_POSITIVE && 0==DISENABLE_ADJ_PHASE_TIME)
#define adj_px_adc_val(px_zero_position_value, px_amplify_adj, px_amp_factor_positive_adj, px_phase_time_adj, px_estimate_delta_y) do{\
	/* 调整对称度-调整正半周放大倍数 */\
	if (px_org_val >= (px_zero_position_value))\
		px_org_val = (px_zero_position_value)\
				+ ((px_org_val - (px_zero_position_value))\
				* ((px_amp_factor_positive_adj) & 0xffff)\
				/ PX_AMP_FACTOR_P_BASE);\
	/* 调整相位 */\
	px_org_val += px_estimate_delta_y * ((px_phase_time_adj & 0xffff) - PX_PHASE_TIME_BASE)\
				/ (PX_PHASE_TIME_BASE);\
	} while(0)
#endif

#if (0!=DISENABLE_ADJ_AMPLIFY && 0==DISENABLE_ADJ_AMP_FACTOR_POSITIVE && 0!=DISENABLE_ADJ_PHASE_TIME)
#define adj_px_adc_val(px_zero_position_value, px_amplify_adj, px_amp_factor_positive_adj, px_phase_time_adj, px_estimate_delta_y) do{\
	/* 调整对称度-调整正半周放大倍数 */\
	if (px_org_val >= (px_zero_position_value))\
		px_org_val = (px_zero_position_value)\
				+ ((px_org_val - (px_zero_position_value))\
				* ((px_amp_factor_positive_adj) & 0xffff)\
				/ PX_AMP_FACTOR_P_BASE);\
	} while(0)
#endif

#if (0!=DISENABLE_ADJ_AMPLIFY && 0!=DISENABLE_ADJ_AMP_FACTOR_POSITIVE && 0==DISENABLE_ADJ_PHASE_TIME)
#define adj_px_adc_val(px_zero_position_value, px_amplify_adj, px_amp_factor_positive_adj, px_phase_time_adj, px_estimate_delta_y) do{\
	/* 调整相位 */\
	px_org_val += px_estimate_delta_y * ((px_phase_time_adj & 0xffff) - PX_PHASE_TIME_BASE)\
				/ (PX_PHASE_TIME_BASE);\
	} while(0)
#endif

#if (0!=DISENABLE_ADJ_AMPLIFY && 0!=DISENABLE_ADJ_AMP_FACTOR_POSITIVE && 0!=DISENABLE_ADJ_PHASE_TIME)
#define adj_px_adc_val(px_zero_position_value, px_amplify_adj, px_amp_factor_positive_adj, px_phase_time_adj, px_estimate_delta_y) 
#endif


/*
 * 计算插值
 */
#if DONT_INSERT_VAL
#define	calc_insert_val(px_insert_val_1, px_insert_val_2, px_estimate_delta_y)
#else
#define	calc_insert_val(px_insert_val_1, px_insert_val_2, px_estimate_delta_y)  do{\
	(px_insert_val_1)   = px_org_val + px_estimate_delta_y / 3;\
	(px_insert_val_2)   = px_org_val + (px_estimate_delta_y*2)/3;} while(0)
#endif


#define abs(x)	((x)>=0 ? (x) : -(x))

extern volatile s32 real_pa_zero_position_value;
extern volatile s32 real_pb_zero_position_value;
extern volatile s32 real_pc_zero_position_value;

#endif
