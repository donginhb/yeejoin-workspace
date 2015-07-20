
在rtconfig.h文件中修改如下几行，即可生成相应的设备代码
//#define APP_CFG_FILENAME "app_cfg_pt_m.h"
//#include "app_cfg_pt_m.h"

//#define APP_CFG_FILENAME "app_cfg_pt_s.h"
//#include "app_cfg_pt_s.h"

//#define APP_CFG_FILENAME "app_cfg_ct_s.h"
//#include "app_cfg_ct_s.h"

#define APP_CFG_FILENAME "app_cfg_em.h"
#include "app_cfg_em.h"

上述头文件与设备的对应关系如下：
app_cfg_pt_m	-- pt设备，作为无线主节点
app_cfg_pt_s	-- pt设备，作为无线从节点
app_cfg_ct_s	-- ct设备，作为无线从节点
app_cfg_em	-- 电表设备
 
