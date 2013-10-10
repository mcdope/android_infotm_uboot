/***************************************************************************** 
** include/configs/imap_dev.h 
** 
** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
** 
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** Description: iMAPx200 develop board configuration.
**
** Author:
**     warits	<warits.wang@infotmic.com.cn>
**      
** Revision History: 
** ----------------- 
** 1.1  05/31/2010 
*****************************************************************************/

#ifndef __IMAPCFG_DEV_H__
#define __IMAPCFG_DEV_H__

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_DISPLAY_VERSION
//#define CONFIG_BOOT_AUTO_ADJUST
//#define CONFIG_ANDROID_RECOVERY
#define CONFIG_NO_ADR_SYSRECOVER
#define CONFIG_RECOVERY_KEY1_GPIO		GPGDAT
#define CONFIG_RECOVERY_KEY1_GPIO_NUM	4 
#define CONFIG_RECOVERY_KEY2_GPIO		GPGDAT
#define CONFIG_RECOVERY_KEY2_GPIO_NUM	5

/* This option enable wince features, such as bootup, and bootw */
//#define CONFIG_WINCE_FEATURE	

/* Boot image */
//#define CONFIG_WINCE_COMMON
#define CONFIG_LINUX_COMMON
#define CONFIG_LOGO_GZIP
#define CONFIG_LOGO_GZIP_W		356
#define CONFIG_LOGO_GZIP_H		101
#define CONFIG_GZIP_LOGO_FILE	<logo_enUS.bin.gz.h>
#define CONFIG_BATT_GZIP_W		300
#define CONFIG_BATT_GZIP_H		300
#define CONFIG_GZIP_BATT_FILE	<batt.enUS.bin.gz.h>

/* Memory configuration */
#define CONFIG_SYS_DENALI_FILE	"asm/arch/denali_data/denali_sam_2x128_270M.h"
#define PHYS_SDRAM_1_SIZE		0x10000000		/* 256 MB in Bank #1 */
#define CONFIG_SYS_SDRAM_END	0x50000000		/* 256 MB total */

/* Board string */

/* Boot Env */
#define CONFIG_LINUX_DEFAULT_BOOTARGS "console=ttySAC3,115200 androidboot.mode=normal"
#define CONFIG_LINUX_RECOVER_BOOTARGS "console=ttySAC3,115200 androidboot.mode=recovery"
#define CONFIG_EXTRA_ENV_SETTINGS "stdin=serial\0"  \
                                  "stdout=serial\0"  \
                                  "stderr=serial\0"  \
                                  "bootcmd=bootl x\0"  \

#define CONFIG_CLK_800_200
/*
#define CONFIG_CLK_533_133_66
#define CONFIG_CLK_400_100_50
#define CONFIG_CLK_400_133_66
#define CONFIG_CLK_800_266
#define CONFIG_CLK_800_200
#define CONFIG_CLK_800_133
#define CONFIG_CLK_840_210
#define CONFIG_CLK_MANUAL_MODE
#define CONFIG_CLK_600_200
#define CONFIG_CLK_450_150
#define CONFIG_CLK_936_234
#define CONFIG_CLK_1008_252
#define CONFIG_SYNC_MODE
*/

/* Drivers */
/* UART */
#define CONFIG_SERIAL4          1	/* we use uart0(SERIAL1) on iMAPx200 FPGA	*/

/* keybd */
//#define CONFIG_KEYBOARD	1

/* Gmac */
//#define CONFIG_DRIVER_GMAC		1	/* we have a gmac on imapx200 */

/* MMC */
#define CONFIG_MMC			1
#define CONFIG_IMAP_MMC			1
#define CONFIG_GENERIC_MMC		1
#define SOURCE_CHANNEL	1

/* LCD */
#define CONFIG_LCD
#define CONFIG_LCD_IMAPX200
#define IMAPFB_HFP		210		/* front porch */
#define IMAPFB_HSW		20		/* hsync width */
#define IMAPFB_HBP		46		/* back porch */
#define IMAPFB_VFP		22		/* front porch */
#define IMAPFB_VSW		10		/* vsync width */
#define IMAPFB_VBP		23		/* back porch */
#define IMAPFB_HRES		800		/* horizon pixel x resolition */
#define IMAPFB_VRES		480		/* line cnt y resolution */
#define CONFIG_LCD_PANEL_GPIO		GPADAT		
#define CONFIG_LCD_PANEL_GPIO_Bit	3
#define CONFIG_LCD_BL_GPIO			GPADAT
#define CONFIG_LCD_BL_GPIO_Bit		6
#define CONFIG_LCD_TOUT_PIN			1
#define CONFIG_LCDCON5				0x24000308
#define CONFIG_DIV_CFG4_VAL			0x0e0e
#define CONFIG_LCDCON1_CLKVAL		2
//#define CONFIG_LCD_ENBLCTRL

/*SPI*/
//#define CONFIG_IMAPX200_SPI
//#define CONFIG_LOW_BATTERY_VAL        520
//#define CONFIG_LOGO_BATTERY_VAL       480

/* USB */
#define CONFIG_OTG_HW_KEY		GPEDAT
#define CONFIG_OTG_HW_KEY_NUM	14

#define CONFIG_NOR
#define CONFIG_SYS_BOOT_NOR
#define CONFIG_SYS_NOR_RC 0xa

#define CONFIG_SYS_EX_INIT_GPIO {	\
	{GPODAT, 13, 0, 1, 0},	\
	{0, 0, 0, 0, 0}	\
}

/* Disable settings */
//#define CONFIG_DISABLE_BOOT_OTG
//#define CONFIG_DISABLE_BOOT_SD

#define CONFIG_SYS_NAND_4K_PAGE
#undef CONFIG_SYS_NAND_2K_PAGE
#define CONFIG_SYS_NAND_MLC	/* 4bit ECC, 9bytes per 512 */
#undef CONFIG_SYS_NAND_SLC	/* 1bit ECC, 4bytes per 2048 */

/* Include the common config file */
#include <configs/imapx.h>
#define CONFIG_BOARD_MAGIC	0xeeaa0703
#define CONFIG_BOARD_HWVER	0x00030101
#define CONFIG_BOARD_OEM	0x0

#endif
