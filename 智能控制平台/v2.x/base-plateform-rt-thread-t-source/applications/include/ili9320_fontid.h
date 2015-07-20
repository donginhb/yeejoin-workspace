#ifndef ILI9320_FONTID_H__
#define ILI9320_FONTID_H__

#define CN_FONT_ID_BASE 0X100

enum font_modid {
	FONT_ID_CN_BEI = CN_FONT_ID_BASE,	/* 北 */
	FONT_ID_CN_JING,  			/* 京 */
	FONT_ID_CN_YI, 				/* 亿 */
	FONT_ID_CN_JIANG,			/* 江 */
	FONT_ID_CN_KE, 				/* 科 */
	FONT_ID_CN_JI,				/* 技 */
	FONT_ID_CN_FA, 				/* 发 */
	FONT_ID_CN_ZHAN, 			/* 展 */
	FONT_ID_CN_YOU,				/* 有 */
	FONT_ID_CN_XIAN, 			/* 限 */
	FONT_ID_CN_GONG,			/* 公 */
	FONT_ID_CN_SI,				/* 司 */
	FONT_ID_CN_ZHUANG, 			/* 状 */
	FONT_ID_CN_TAI, 			/* 态 */
	FONT_ID_CN_ZHENG, 			/* 正 */
	FONT_ID_CN_CHANG, 			/* 常 */
	FONT_ID_CN_YI1, 			/* 异 */
	FONT_ID_CN_SUO,				/* 锁 */
	FONT_ID_CN_WEN,				/* 温 */
	FONT_ID_CN_SHI,				/* 湿 */
	FONT_ID_CN_DU,				/* 度 */
	FONT_ID_CN_DU_SYMBOL,			/* ℃ */ 
	FONT_ID_CN_NING,			/* 宁 */  
	FONT_ID_CN_XIA,				/* 夏 */ 
	FONT_ID_CN_DIAN,			/* 电 */ 
	FONT_ID_CN_LI,				/* 力 */
	FONT_ID_CN_WU,				/* 吴 */
	FONT_ID_CN_ZHONG,			/* 忠 */
	FONT_ID_CN_GONG1,			/* 供 */
	FONT_ID_CN_JU,				/* 局 */
	FONT_ID_CN_JIAN,			/* 监 */ 
	FONT_ID_CN_CE,				/* 测 */ 
	FONT_ID_CN_XIANG, 			/* 箱 */
	FONT_ID_CN_YONG	,			/* 用 */
	FONT_ID_CN_HU,				/* 户 */
	FONT_ID_CN_XUAN,			/* 选 */
	FONT_ID_CN_XIANG1,			/* 项 */
	FONT_ID_CN_ZHI,				/* 智 */
	FONT_ID_CN_NENG,			/* 能 */
	FONT_ID_CN_MEN,				/* 门 */
	FONT_ID_CN_KONG,			/* 控 */
	FONT_ID_CN_ZHI1,			/* 制 */
	FONT_ID_CN_XIN,				/* 信 */
	FONT_ID_CN_XI,				/* 息 */
	FONT_ID_CN_ZI,				/* 自 */
	FONT_ID_CN_ZHU,				/* 助 */
	FONT_ID_CN_CHA,				/* 查 */
	FONT_ID_CN_XUN,				/* 询 */
	FONT_ID_CN_XIAN1,			/* 线 */
	FONT_ID_CN_LU,				/* 路 */
	FONT_ID_CN_YUN,				/* 运 */
	FONT_ID_CN_XING,			/* 行 */
	FONT_ID_CN_QUE,				/* 确 */
	FONT_ID_CN_REN,				/* 认 */
	FONT_ID_CN_QU,				/* 取 */
	FONT_ID_CN_XIAO,			/* 消 */
	FONT_ID_CN_GUO,				/* 国 */
	FONT_ID_CN_JIA,				/* 家 */
	FONT_ID_CN_WANG,			/* 网 */ 
	FONT_ID_CN_GONG2,			/* 功 */
	FONT_ID_CN_HUAN,			/* 环 */
	FONT_ID_CN_JING1,			/* 境 */
	FONT_ID_CN_CAN,				/* 参 */
	FONT_ID_CN_SHU,				/* 数 */
	FONT_ID_CN_XUAN1,			/* 宣 */
	FONT_ID_CN_CHUAN,			/* 传 */
	FONT_ID_CN_DA,				/* 打 */
	FONT_ID_CN_KAI,				/* 开 */
	FONT_ID_CN_GUAN,			/* 关 */ 
	FONT_ID_CN_BI,				/* 闭 */
	FONT_ID_CN_MING,			/* 名 */
	FONT_ID_CN_MI,				/* 密 */
	FONT_ID_CN_MA,				/* 码 */
	FONT_ID_CN_TI,				/* 提 */ 
	FONT_ID_CN_SHI1,			/* 示 */ 
	FONT_ID_CN_JIAN1,			/* 键 */ 
	FONT_ID_CN_PAN,				/* 盘 */ 
	FONT_ID_CN_SHANG,			/* 上 */ 
	FONT_ID_CN_DE,				/* 的 */ 
	FONT_ID_CN_TONG,			/* 同 */ 
	FONT_ID_CN_YANG,			/* 样 */ 
	FONT_ID_CN_YE,				/* 也 */ 
	FONT_ID_CN_YI2,				/* 以 */ 
	FONT_ID_CN_JIN,				/* 进 */ 
	FONT_ID_CN_ZE,				/* 择 */ 
	FONT_ID_CN_HE,				/* 和 */ 
	FONT_ID_CN_DUI,				/* 对 */ 
	FONT_ID_CN_YING,			/* 应 */  
	FONT_ID_CN_GAO,				/* 告 */ /* 2012-05-03 15:14:55 */
	FONT_ID_CN_JING2,			/* 警 */
	FONT_ID_CN_JU1,				/* 居 */ /* 2012-05-04 10:38:07 */
	FONT_ID_CN_MIN,				/* 民 */
	FONT_ID_CN_DANG,			/* 当 */
	FONT_ID_CN_TIAN, 			/* 天 */
	FONT_ID_CN_SHI2,			/* 时 */
	FONT_ID_CN_LIANG, 			/* 量 */
	FONT_ID_CN_YUE,				/* 月 */
	FONT_ID_CN_WEI,				/* 未 */ /* 2012-05-06 12:01:06 */
	FONT_ID_CN_ZHI2,			/* 知 */
	FONT_ID_CN_SHE,				/* 设 */
	FONT_ID_CN_BEI1,			/* 备 */
	FONT_ID_CN_GUO1,			/* 过 */
	
	FONT_ID_CN_INVALID,			/* 非法 */
};

#endif
