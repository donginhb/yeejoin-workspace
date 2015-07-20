import time
import platform
import os
import sys
from SCons.Script import *

from building import *

# toolchains options
ARCH='arm'
CPU='cortex-m3'
CROSS_TOOL='gcc'

'''
Low-density devices
	are STM32F101xx, STM32F102xx and STM32F103xx microcontrollers where the Flash memory density ranges between 16 and 32 Kbytes.
Medium-density devices
	are STM32F101xx, STM32F102xx and STM32F103xx microcontrollers where the Flash memory density ranges between 64 and 128 Kbytes.
High-density devices
	are STM32F101xx and STM32F103xx microcontrollers where the Flash memory density ranges between 256 and 512 Kbytes.
XL-density devices
	are STM32F101xx and STM32F103xx microcontrollers where the Flash memory density ranges between 768 Kbytes and 1 Mbyte.
Connectivity line devices 
	are STM32F105xx and STM32F107xx microcontrollers.
'''
#device options
# STM32_TYPE = 
# 'STM32F10X_LD','STM32F10X_LD_VL',
# 'STM32F10X_MD','STM32F10X_MD_VL',
# 'STM32F10X_HD','STM32F10X_HD_VL',
# 'STM32F10X_XL','STM32F10X_CL'

STM32_TYPE	= 'STM32F10X_HD'
LD_FILE		= ' stm32_rom.ld'


#/home/zsw/dev-work/eclipse-ws/wireless-si4432/os-app
RTT_ROOT = os.path.normpath(os.getcwd() + '/../..')

Init_BuildOptions(RTT_ROOT)


def GetMacroStr(file_name, macro_name):
	global RTT_ROOT
	
	is_find_macro = False
	macro_val = ''
	macros = {}

	# parse rtconfig.h to get used component
	PreProcessor = SCons.cpp.PreProcessor()
	#f = file(RTT_ROOT+'/include/rtdef.h', 'r')
	f = file(file_name, 'r')
	contents = f.read()
	f.close()
	PreProcessor.process_contents(contents)
	macros.update(PreProcessor.cpp_namespace)

	if macros.has_key(macro_name):
		is_find_macro = True
		macro_val = macros[macro_name]

#		print 'macro val is', macros[macro_name]
	else:
		is_find_macro = False
		print "GetMacroStr() error"
		
	return (is_find_macro, macro_val)



# lcd panel options
# 'FMT0371','ILI932X', 'SSD1289'
RT_USING_LCD_TYPE = 'ILI932X'
macro_file = RTT_ROOT+'/include/rtdef.h'
(is_succ, val) = GetMacroStr(macro_file, 'RT_CUR_APP_VERSION_STR')
if is_succ:
	app_version = val
else:
	app_version = 'x.x.x'



RT_APP_VER_TYPE = '_Alpha_0.'
nowtimestr = '.' + time.strftime('%Y%m%d',time.localtime())

if GetDepend('PT_DEVICE'):
	STM32_TYPE = 'STM32F10X_HD'
	LD_FILE = ' stm32_rom_512kf_64kr.ld'

	if GetDepend('WIRELESS_MASTER_NODE'):
		TARGET_VER_NAME = 'light300-ptc-m_' + app_version[1:-1] + nowtimestr + RT_APP_VER_TYPE
	elif GetDepend('WIRELESS_SLAVE_NODE'):
		TARGET_VER_NAME = 'light300-ptc-s_' + app_version[1:-1] + nowtimestr + RT_APP_VER_TYPE
	else:
		TARGET_VER_NAME = 'light300-ptc-x_' + app_version[1:-1] + nowtimestr + RT_APP_VER_TYPE
elif GetDepend('RS485_SW_DEVICE'):
	STM32_TYPE = 'STM32F10X_HD'
	LD_FILE = ' stm32_rom_512kf_64kr.ld'

	TARGET_VER_NAME = 'lt300-485sw_' + app_version[1:-1] + nowtimestr + RT_APP_VER_TYPE
else:
	TARGET_VER_NAME = 'light300-xx_' + app_version[1:-1] + nowtimestr + RT_APP_VER_TYPE



# !!!just for my test, stm32f103vf cpu
#STM32_TYPE	= 'STM32F10X_XL'
#LD_FILE		= ' stm32_rom_768kf_96kr.ld'



# cross_tool provides the cross compiler
# EXEC_PATH is the compiler execute path, for example, CodeSourcery, Keil MDK, IAR

if  CROSS_TOOL == 'gcc':
	PLATFORM 	= 'gcc'
	if 'Linux' == platform.system():
		EXEC_PATH 	= '/home/zsw/xtools/MentorGraphics/Sourcery_CodeBench_Lite_for_ARM_EABI/bin'
	else :
		if 1:
			EXEC_PATH 	= 'D:/Program Files/CodeSourcery/Sourcery_CodeBench_Lite_for_ARM_EABI/bin'
		else:
			EXEC_PATH 	= 'C:/Program Files (x86)/CodeSourcery/Sourcery_CodeBench_Lite_for_ARM_EABI/bin'
elif CROSS_TOOL == 'keil':
	PLATFORM 	= 'armcc'
	EXEC_PATH 	= 'E:/Keil'
elif CROSS_TOOL == 'iar':
	PLATFORM 	= 'iar'
	IAR_PATH 	= 'E:/Program Files/IAR Systems/Embedded Workbench 6.0'

BUILD = 'debug'

if PLATFORM == 'gcc':
    # toolchains
    PREFIX = 'arm-none-eabi-'
    CC = PREFIX + 'gcc'
    AS = PREFIX + 'gcc'
    AR = PREFIX + 'ar'
    LINK = PREFIX + 'gcc'
    TARGET_EXT = 'axf'
    SIZE = PREFIX + 'size'
    OBJDUMP = PREFIX + 'objdump'
    OBJCPY = PREFIX + 'objcopy'

    DEVICE = ' -mcpu=cortex-m3 -mthumb -ffunction-sections -fdata-sections'  + ' -D ' + STM32_TYPE
    CFLAGS = ' -Wall ' + DEVICE
    AFLAGS = ' -c' + DEVICE + ' -x assembler-with-cpp'
    LFLAGS = DEVICE + ' -Wl,--gc-sections,-Map=rtthread-stm32.map,-cref,-u,Reset_Handler -T ' + LD_FILE

    CPATH = ''
    LPATH = ''

    if BUILD == 'debug':
        CFLAGS += ' -O0 -gdwarf-2'
        AFLAGS += ' -gdwarf-2'
    else:
        CFLAGS += ' -O2'

    POST_ACTION = OBJCPY + ' -O binary $TARGET '+ TARGET_VER_NAME +'bin\n' + SIZE + ' $TARGET \n' 

elif PLATFORM == 'armcc':
    # toolchains
    CC = 'armcc'
    AS = 'armasm'
    AR = 'armar'
    LINK = 'armlink'
    TARGET_EXT = 'axf'

    DEVICE = ' --device DARMSTM'
    CFLAGS = DEVICE + ' --apcs=interwork'
    AFLAGS = DEVICE
    LFLAGS = DEVICE + ' --info sizes --info totals --info unused --info veneers --list rtthread-stm32.map --scatter stm32_rom.sct'

    CFLAGS += ' -I' + EXEC_PATH + '/ARM/RV31/INC'
    LFLAGS += ' --libpath ' + EXEC_PATH + '/ARM/RV31/LIB'

    EXEC_PATH += '/arm/bin40/'

    if BUILD == 'debug':
        CFLAGS += ' -g -O0'
        AFLAGS += ' -g'
    else:
        CFLAGS += ' -O2'

    POST_ACTION = 'fromelf --bin $TARGET --output rtthread.bin \nfromelf -z $TARGET'

elif PLATFORM == 'iar':
    # toolchains
    CC = 'iccarm'
    AS = 'iasmarm'
    AR = 'iarchive'
    LINK = 'ilinkarm'
    TARGET_EXT = 'out'

    DEVICE = ' -D USE_STDPERIPH_DRIVER' + ' -D ' + STM32_TYPE

    CFLAGS = DEVICE
    CFLAGS += ' --diag_suppress Pa050'
    CFLAGS += ' --no_cse' 
    CFLAGS += ' --no_unroll' 
    CFLAGS += ' --no_inline' 
    CFLAGS += ' --no_code_motion' 
    CFLAGS += ' --no_tbaa' 
    CFLAGS += ' --no_clustering' 
    CFLAGS += ' --no_scheduling' 
    CFLAGS += ' --debug' 
    CFLAGS += ' --endian=little' 
    CFLAGS += ' --cpu=Cortex-M3' 
    CFLAGS += ' -e' 
    CFLAGS += ' --fpu=None'
    CFLAGS += ' --dlib_config "' + IAR_PATH + '/arm/INC/c/DLib_Config_Normal.h"'    
    CFLAGS += ' -Ol'    
    CFLAGS += ' --use_c++_inline'
        
    AFLAGS = ''
    AFLAGS += ' -s+' 
    AFLAGS += ' -w+' 
    AFLAGS += ' -r' 
    AFLAGS += ' --cpu Cortex-M3' 
    AFLAGS += ' --fpu None' 

    LFLAGS = ' --config stm32f10x_flash.icf'
    LFLAGS += ' --redirect _Printf=_PrintfTiny' 
    LFLAGS += ' --redirect _Scanf=_ScanfSmall' 
    LFLAGS += ' --entry __iar_program_start'    

    EXEC_PATH = IAR_PATH + '/arm/bin/'
    POST_ACTION = ''
