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
//#define CONFIG_BOOT_AUTO_ADJUST

//#define CONFIG_MOTOR_GPIO               GPADAT
//#define CONFIG_MOTOR_GPIO_NUM   1

/* This option enable wince features, such as bootup, and bootw */
//#define CONFIG_WINCE_FEATURE	
#define CONFIG_NO_ADR_SYSRECOVER
#define CONFIG_MENU_KEY_GPIO GPIDAT
#define CONFIG_MENU_KEY_GPIO_NUM 9 
#define CONFIG_RECOVERY_KEY1_GPIO GPIDAT 
#define CONFIG_RECOVERY_KEY1_GPIO_NUM 10
#define CONFIG_RECOVERY_KEY2_GPIO GPGDAT
#define CONFIG_RECOVERY_KEY2_GPIO_NUM 1 

//#define CONFIG_RECOVERY_KEY2_GPIO		GPGDAT
//#define CONFIG_RECOVERY_KEY2_GPIO_NUM	5

/* Boot image */
//#define CONFIG_WINCE_COMMON
#define CONFIG_LINUX_COMMON
//#define CONFIG_LOGO_COBY

#define CONFIG_LOGO_GZIP
#define CONFIG_LOGO_GZIP_W  800
#define CONFIG_LOGO_GZIP_H  480
#define CONFIG_GZIP_LOGO_FILE   <BOBY.bin.gz.h>

/* Memory configuration */
#define CONFIG_SYS_DENALI_FILE	"asm/arch/denali_data/denali_sam_2x128_270M18_coby.h"
#define PHYS_SDRAM_1_SIZE		0x10000000		/* 1 GB in Bank #1 */
#define CONFIG_SYS_SDRAM_END	0x50000000		/* 1 GB total */
#define CONFIG_IMAPX200_INSP	0x46

/* Board string */
//#define CONFIG_IDENT_STRING		"(COBY)"		/* Board Specific Data */
//#define CONFIG_SYS_PROMPT		"coby@uboot:# "	/* Monitor Command Prompt */

/* Boot Env */
#define CONFIG_LINUX_DEFAULT_BOOTARGS "console=ttySAC3,115200 androidboot.mode=normal mem=256M"
#define CONFIG_LINUX_RECOVER_BOOTARGS "console=ttySAC3,115200 androidboot.mode=recovery mem=256M"
#define CONFIG_EXTRA_ENV_SETTINGS "stdin=serial\0"  \
                                  "stdout=serial\0"  \
                                  "stderr=serial\0"  \
                                  "bootcmd=bootl x\0"  \

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
#define CONFIG_SERIAL4          1	/* we use uart0(SERIAL1) on iMAPx200 FPGA	*/

/* keybd */
#define CONFIG_KEYBOARD	1

/* Gmac */
#define CONFIG_DRIVER_GMAC		1	/* we have a gmac on imapx200 */

/* MMC */
#define CONFIG_MMC			1
#define CONFIG_IMAP_MMC			1
#define CONFIG_GENERIC_MMC		1

/* LCD */
#define CONFIG_LCD
#define CONFIG_LCD_IMAPX200
#define IMAPFB_HFP		180		/* front porch */
#define IMAPFB_HSW		30		/* hsync width */
#define IMAPFB_HBP		46		/* back porch */
#define IMAPFB_VFP		17		/* front porch */
#define IMAPFB_VSW		10		/* vsync width */
#define IMAPFB_VBP		18		/* back porch */
#define IMAPFB_HRES		800		/* horizon pixel x resolition */
#define IMAPFB_VRES		480		/* line cnt y resolution */
#define CONFIG_LCD_PANEL_GPIO		0x20e100e0	
#define CONFIG_LCD_PANEL_GPIO_Bit	12
#define CONFIG_LCD_BL_GPIO			0x20e100c0
#define CONFIG_LCD_BL_GPIO_Bit		0
#define CONFIG_LCD_TOUT_PIN			0
#define CONFIG_LCDCON5				0x24000300
//#define CONFIG_LCD_ENBLCTRL

/* USB */
//#define CONFIG_OTG_GPIO		0x20e10080
//#define CONFIG_OTG_GPIO_NUM	12

//#define CONFIG_OTG_EX_GPIO		0x20e10090
//#define CONFIG_OTG_EX_GPIO_NUM	5
#define CONFIG_DISABLE_BOOT_OTG
//#define CONFIG_DISABLE_BOOT_SD

/* USB */

/* NOR */
#define CONFIG_NOR
#define CONFIG_SYS_BOOT_NOR
#define CONFIG_SYS_NOR_RC 0xa

#define CONFIG_SYS_INIT_GPIO0 GPOCON
#define CONFIG_SYS_INIT_GPIO0_VAL 0xaa96aaaa
#define CONFIG_SYS_INIT_GPIO1 GPODAT
#define CONFIG_SYS_INIT_GPIO1_VAL 0xffb2

#define CONFIG_SYS_NAND_8K_PAGE
#undef CONFIG_SYS_NAND_2K_PAGE
#define CONFIG_SYS_NAND_MLC	/* 4bit ECC, 9bytes per 512 */
#undef CONFIG_SYS_NAND_SLC	/* 1bit ECC, 4bytes per 2048 */

/* Include the common config file */
#include <configs/imapx.h>
#define CONFIG_BOARD_MAGIC	0

#endif
