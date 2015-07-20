#ifndef __ADS8329_H__
#define __ADS8329_H__

void spi1_cfg4ads8329(void);
void init_ads8329(void);

u16 ads8329_converter_channelx(unsigned int pin_mask);

#endif

