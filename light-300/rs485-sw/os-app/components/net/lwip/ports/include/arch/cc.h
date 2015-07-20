/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#ifndef __CC_H__
#define __CC_H__

#include <stdio.h>
#include <stdlib.h>



/* #include "cpu.h" --- 只定义了byte order */
/* Define platform endianness */
#define BYTE_ORDER LITTLE_ENDIAN
/*
 * 主机序 网络序 之间的转换可以优化
 * lwip-1.4.0\src\include\lwip\def.h
 * LWIP_PLATFORM_BYTESWAP
 * #define htons(x) lwip_htons(x)
 * #define ntohs(x) lwip_ntohs(x)
 * #define htonl(x) lwip_htonl(x)
 * #define ntohl(x) lwip_ntohl(x)
 */




/* Define generic types used in lwIP */
typedef unsigned   char    u8_t;
typedef signed     char    s8_t;
typedef unsigned   short   u16_t;
typedef signed     short   s16_t;
typedef unsigned   long    u32_t;
typedef signed     long    s32_t;

typedef u32_t              mem_ptr_t;

#if 0
/*
 * These formatters are used in several lwip files, yet mainly for diagnostics
 * (LWIP_PLATFORM_DIAG) and debug output (LWIP_DEBUGF), so you might not need
 * them if you disable that output.
 */
/* Define (sn)printf formatters for these lwIP types */
#define X8_F  "02x"
#define U16_F "hu"
#define S16_F "hd"
#define X16_F "hx"
#define U32_F "u"
#define S32_F "d"
#define X32_F "x"
/* If only we could use C99 and get %zu */
#if defined(__x86_64__)
#define SZT_F "lu"
#else
#define SZT_F "u"
#endif

#endif



/* Compiler hints for packing structures */
/*
 * MEM_ALIGNMENT: should be set to the alignment of the CPU
 *    4 byte alignment -> #define MEM_ALIGNMENT 4
 *    2 byte alignment -> #define MEM_ALIGNMENT 2
 * #define ETH_PAD_SIZE  0
 *
 * In any case, make sure the system runs stable when chosing a structure packing
 * setting different to 1!
 */
#if 0
/** This is a helper struct which holds the starting
 * offset and the ending offset of this fragment to
 * easily chain the fragments.
 * It has the same packing requirements as the IP header, since it replaces
 * the IP header in memory in incoming fragments (after copying it) to keep
 * track of the various fragments. (-> If the IP header doesn't need packing,
 * this struct doesn't need packing, too.)
 */
#ifdef PACK_STRUCT_USE_INCLUDES
#  include "arch/bpstruct.h"
#endif
PACK_STRUCT_BEGIN
struct ip_reass_helper {
	PACK_STRUCT_FIELD(struct pbuf *next_pbuf);
	PACK_STRUCT_FIELD(u16_t start);
	PACK_STRUCT_FIELD(u16_t end);
} PACK_STRUCT_STRUCT;
PACK_STRUCT_END
#ifdef PACK_STRUCT_USE_INCLUDES
#  include "arch/epstruct.h"
#endif

E:
\MDK_workspace\Ethernet\source\lwip-1.3.2_mdify\ports\stellaris\include\arch\bpstruct.h
的内容如下:
#if defined(__IAR_SYSTEMS_ICC__)
#pragma pack(1)
#endif

E:
\MDK_workspace\Ethernet\source\lwip-1.3.2_mdify\ports\stellaris\include\arch\epstruct.h
的内容如下:
#if defined(__IAR_SYSTEMS_ICC__)
#pragma pack()
#endif

#endif

/*
 * #define PACK_STRUCT_BEGIN #pragma pack(1)
 * 经过预处理后#没有顶格, 编译出错
 */
#if 0
#define PACK_STRUCT_USE_INCLUDES

#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_END
#define PACK_STRUCT_FIELD(x) x
#define PACK_STRUCT_STRUCT
#else
/* __attribute__((packed)) 是GNU扩展, realview也支持这个编译指示 */
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_END
#define PACK_STRUCT_FIELD(x) x
#define PACK_STRUCT_STRUCT __attribute__((packed))

#endif



#if 0 /* checksum */
/*
 * checksum可以进行优化
 * #define LWIP_CHKSUM lwip_standard_chksum
 * #define LWIP_CHKSUM_ALGORITHM 2
 */
IP protocols use checksums (see RFC 1071). LwIP gives you a choice of 3 algorithms:
1.load byte by byte, construct 16 bits word and add:
not efficient for most platforms
2.load first byte if odd address, loop processing 16 bits words, add last byte.
	3.load first byte and word if not 4 byte aligned, loop processing 32 bits words, add last word/byte.
#endif







		/* Plaform specific diagnostic output */
		/*
		 * -LWIP_PLATFORM_DIAG(x) -- non-fatal, print a message. Uses printf formating
		 * -LWIP_PLATFORM_ASSERT(x) -- fatal, print message and abandon execution. Uses
		 *      printf formating. Unlike assert() from the standard c library, the parameter
		 *      x is the message, not a condition.
		 */
#if 1
#define LWIP_PLATFORM_DIAG(x) do {printf_syn x; } while(0)
#define LWIP_PLATFORM_ASSERT(x) do {printf_syn("Assertion \"%s\" failed at line %d in %s\n", \
                                     x, __LINE__, __FILE__); } while(0)
#else

#define LWIP_PLATFORM_DIAG(x) do {printf x; } while(0)
#define LWIP_PLATFORM_ASSERT(x) do {printf("Assertion \"%s\" failed at line %d in %s\n", \
                                     x, __LINE__, __FILE__); } while(0)

#define print_s printf
#endif




#if 0 /* lightweight synchronization */
		"lightweight" synchronization mechanisms -
		SYS_ARCH_DECL_PROTECT(x) - declare a protection state variable.
			SYS_ARCH_PROTECT(x)      - enter protection mode.
			SYS_ARCH_UNPROTECT(x)    - leave protection mode.

Implementors have three options how to implement it:

			1.Provide no implementation. This is appropriate when lwip is not used within multiple threads and/or in interrupts.
			Then do not define SYS_ARCH_PROTECT in cc.h or SYS_LIGHTWEIGHT_PROT in lwipopts.h.
			So SYS_LIGHTWEIGHT_PROT defaults to 0 in opt.h and sys.h provides empty default definitions for
					SYS_ARCH_DECL_PROTECT, SYS_ARCH_PROTECT and SYS_ARCH_UNPROTECT.

					2.	Provide an implementation in cc.h by defining the macros SYS_ARCH_DECL_PROTECT, SYS_ARCH_PROTECTand SYS_ARCH_UNPROTECT.

					3.	RECOMMENDED:
Provide an implementation in sys_arch.h and sys_arch.c. Do not define the macros in cc.h.
Instead define SYS_LIGHTWEIGHT_PROT as 1 in lwipopts.h. Then sys.h will provide default definitions (as of version 1.3.0) of
	these macros as:
#define SYS_ARCH_DECL_PROTECT(lev) sys_prot_t lev
#define SYS_ARCH_PROTECT(lev) lev = sys_arch_protect()
#define SYS_ARCH_UNPROTECT(lev) sys_arch_unprotect(lev)
Define the lowercase versions in sys_arch.h (and sys_arch.c) (see that header file section below). In most cases,
	   this is the "right" way to define them. lwIP uses only the UPPERCASE versions in its code.
#endif




#if 0 /* memset() */
	   If the compiler does not provide memset() this file must include a definition of it, or include a file which defines it.
#endif




	   /* errno */
	   /**
	    * This file must either include a system-local <errno.h> which defines the standard *nix error codes, or it
	    * should #define LWIP_PROVIDE_ERRNO to make lwip/arch.h define the codes which are used throughout.
	    */
#define LWIP_PROVIDE_ERRNO 1



#endif /* __CC_H__ */
