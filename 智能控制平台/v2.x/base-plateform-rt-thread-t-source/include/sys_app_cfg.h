#ifndef SYS_APP_CFG_H_
#define SYS_APP_CFG_H_

/*
 * 用于那种方案的程序, 只能有一个为1
 */
#define USE_TO_9INCH_LCD 1
#define USE_TO_7INCH_LCD 0


/*
 * 键盘控制芯片相关定义
 *
 * 9寸屏没有键盘控制芯片
 */
#define USE_KEY_CTR_CHIP	(!USE_TO_9INCH_LCD)
#define USE_KEY_CTR_RA8875	0
#define USE_KEY_CTR_TCA8418	0


#endif
