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

/* This option enable wince features, such as bootup, and bootw */
#define CONFIG_WINCE_FEATURE	

/* Boot image */
#define CONFIG_WINCE_COMMON
#define CONFIG_LINUX_COMMON
#define CONFIG_LOGO_COMMON

/* Memory configuration */
#define CONFIG_SYS_DENALI_FILE	"asm/arch/denali_data/denali_sam_2x128_270M.h"
#define PHYS_SDRAM_1_SIZE		0x20000000		/* 512 MB in Bank #1 */
#define CONFIG_SYS_SDRAM_END	0x60000000		/* 512 MB total */
#define CONFIG_DENALI_CS		0x00000300

/* Board string */

/* Boot Env */
#define CONFIG_LINUX_DEFAULT_BOOTARGS "console=ttySAC0,115200 rdinit=/init mem=512M"
#define CONFIG_EXTRA_ENV_SETTINGS "stdin=serial\0"  \
                                  "stdout=serial\0"  \
                                  "stderr=serial\0"  \
                                  "bootcmd=bootw\0"  \

#define CONFIG_CLK_800_266
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
#define CONFIG_SERIAL1          1	/* we use uart0(SERIAL1) on iMAPx200 FPGA	*/

/* keybd */
//#define CONFIG_KEYBOARD	1

/* Gmac */
//#define CONFIG_DRIVER_GMAC		1	/* we have a gmac on imapx200 */

/* MMC */
#define CONFIG_MMC			1
#define CONFIG_IMAP_MMC			1
#define CONFIG_GENERIC_MMC		1

/* LCD */
#define CONFIG_LCD
#define CONFIG_LCD_IMAPX200
#define IMAPFB_HFP		12		/* front porch */
#define IMAPFB_HSW		68		/* hsync width */
#define IMAPFB_HBP		81		/* back porch */
#define IMAPFB_VFP		10		/* front porch */
#define IMAPFB_VSW		4		/* vsync width */
#define IMAPFB_VBP		14		/* back porch */
#define IMAPFB_HRES		1024		/* horizon pixel x resolition */
#define IMAPFB_VRES		600		/* line cnt y resolution */
#define CONFIG_LCD_PANEL_GPIO		0x20e10080	
#define CONFIG_LCD_PANEL_GPIO_Bit	0
#define CONFIG_LCD_BL_GPIO			0x20e10080
#define CONFIG_LCD_BL_GPIO_Bit		1
#define CONFIG_LCD_TOUT_PIN			2
#define CONFIG_LCDCON5				0x06000308
//#define CONFIG_LCD_ENBLCTRL

/* USB */
#define CONFIG_OTG_GPIO		0x20e10080
#define CONFIG_OTG_GPIO_NUM	6

#define CONFIG_SYS_NAND_4K_PAGE
#undef CONFIG_SYS_NAND_2K_PAGE
#define CONFIG_SYS_NAND_MLC	/* 4bit ECC, 9bytes per 512 */
#undef CONFIG_SYS_NAND_SLC	/* 1bit ECC, 4bytes per 2048 */

/* Include the common config file */
#include <configs/imapx.h>
#define CONFIG_BOARD_MAGIC	0x11dd0000

#endif
