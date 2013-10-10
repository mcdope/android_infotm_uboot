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


#define CONFIG_HW_PLATFORM_P8088A
/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_SYS_SDRAM     512 //MBytes
#define CONFIG_SYS_DISK_iNAND

#define SOURCE_CHANNEL 0
#ifdef CONFIG_SYS_DISK_iNAND
#define iNAND_CHANNEL  1
#define CONFIG_HIBERNATE
#endif

//#define CONFIG_DISABLE_BOOT_OTG
/* This option enable android recovery */
//#define CONFIG_DISPLAY_VERSION
//#define CONFIG_ANDROID_RECOVERY
//#define CONFIG_BOOT_AUTO_ADJUST
/* Android recovery needs the following gpio keys */
#define CONFIG_MENU_KEY_GPIO GPADAT
#define CONFIG_MENU_KEY_GPIO_NUM 3


//#define CONFIG_POWER_KEY_GPIO
//#define CONFIG_POWER_KEY_GPIO_NUM
/* (home) */
#define CONFIG_RECOVERY_KEY1_GPIO GPADAT
#define CONFIG_RECOVERY_KEY1_GPIO_NUM 3

/* (back) */
#define CONFIG_RECOVERY_KEY2_GPIO GPADAT
#define CONFIG_RECOVERY_KEY2_GPIO_NUM 3

#define CONFIG_MINUS_KEY_GPIO GPBDAT
#define CONFIG_MINUS_KEY_GPIO_NUM 1
#define CONFIG_PLUS_KEY_GPIO GPBDAT
#define CONFIG_PLUS_KEY_GPIO_NUM 2

#define CONFIG_MOTOR_GPIO		GPADAT
#define CONFIG_MOTOR_GPIO_NUM	1

/* This option enable wince features, such as bootup, and bootw */
//#define CONFIG_WINCE_FEATURE	

/* Boot image */
//#define DISABLE_LOGO
//#define CONFIG_WINCE_COMMON
#define CONFIG_LINUX_COMMON
//#define CONFIG_LOGO_COMMON
#define CONFIG_LOGO_GZIP
//#define CONFIG_LOGO_GZIP_W		552
//#define CONFIG_LOGO_GZIP_H		96
//#define CONFIG_GZIP_LOGO_FILE	<handpad.bin.gz.h>
#define CONFIG_LOGO_GZIP_W              1024
#define CONFIG_LOGO_GZIP_H              600
#define CONFIG_GZIP_LOGO_FILE   <hp.bin.gz.h>


/* Memory configuration */
#define CONFIG_SYS_DENALI_FILE	"asm/arch/denali_data/denali_sam_4x128_270Mau.h"

#define PHYS_SDRAM_1_SIZE		0x10000000		/* 256 MB in Bank #1 */
//#define CONFIG_DENALI_RFRATE	0x06420612
#define CONFIG_IMAPX200_INSP	0x46

/* Board string */

/* Boot Env */
#define CONFIG_LINUX_DEFAULT_BOOTARGS "console=ttySAC1,115200 androidboot.mode=normal"
#define CONFIG_LINUX_RECOVER_BOOTARGS "console=ttySAC1,115200 androidboot.mode=recovery"
#define CONFIG_EXTRA_ENV_SETTINGS "stdin=serial\0"  \
                                  "stdout=serial\0"  \
                                  "stderr=serial\0"  \
                                   "bootcmd=bootl x\0"
#define CONFIG_CLK_800_200
/*
#define CONFIG_CLK_533_133_66
#define CONFIG_CLK_400_100_50
#define CONFIG_CLK_1008_252
#define CONFIG_CLK_400_133_66
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
#define CONFIG_SERIAL2          1	/* we use uart0(SERIAL1) on iMAPx200 FPGA	*/

/* keybd */
//#define CONFIG_KEYBOARD	1

/* Gmac */
//#define CONFIG_DRIVER_GMAC		1	/* we have a gmac on imapx200 */

/* MMC */
#define CONFIG_MMC			1
#define CONFIG_IMAP_MMC			1
#define CONFIG_GENERIC_MMC		1

/* LCD */
#define CONFIG_DIV_CFG4_VAL	0x1212
#define CONFIG_LCD
#define CONFIG_LCD_IMAPX200
#define IMAPFB_HFP		134		/* front porch */
#define IMAPFB_HSW		32		/* hsync width */
#define IMAPFB_HBP		89		/* back porch */
#define IMAPFB_VFP		10		/* front porch */
#define IMAPFB_VSW		4		/* vsync width */
#define IMAPFB_VBP		14		/* back porch */
#define IMAPFB_HRES		1024		/* horizon pixel x resolition */
#define IMAPFB_VRES		600		/* line cnt y resolution */
#define CONFIG_LCD_PANEL_GPIO		GPEDAT	
#define CONFIG_LCD_PANEL_GPIO_Bit	6	
#define CONFIG_LCD_BL_GPIO			GPADAT
#define CONFIG_LCD_BL_GPIO_Bit		7
#define CONFIG_LCD_TOUT_PIN			2
#define CONFIG_LCDCON5				0x06000708
//#define CONFIG_LCD_ENBLCTRL

/* SPI */
//#define CONFIG_IMAPX200_SPI
//#define CONFIG_LOW_BATTERY_VAL		550
//#define CONFIG_LOGO_BATTERY_VAL		510

//#define CONFIG_POWER_GPIO       GPBDAT
//#define CONFIG_POWER_GPIO_NUM   0x2

/* USB */
#define CONFIG_OTG_GPIO
#define CONFIG_OTG_EX_GPIO	GPADAT
#define CONFIG_OTG_EX_GPIO_NUM	3

 /* NOR */
#define CONFIG_NOR
#define CONFIG_SYS_BOOT_NOR
#define CONFIG_SYS_NOR_RC 0xa
#define CONFIG_P8088A
//#define CONFIG_NOR_256K

#if 1
/* Init GPIO values */
#define CONFIG_SYS_INIT_GPIO0 GPACON
#define CONFIG_SYS_INIT_GPIO0_VAL 0x10
#define CONFIG_SYS_INIT_GPIO1 GPAPUD
#define CONFIG_SYS_INIT_GPIO1_VAL 0xF7
#else
#define CONFIG_SYS_INIT_GPIO1 GPADAT
#define CONFIG_SYS_INIT_GPIO1_VAL 0x4
#define CONFIG_SYS_INIT_GPIO2 GPOCON
#define CONFIG_SYS_INIT_GPIO2_VAL 0x4000000
#define CONFIG_SYS_INIT_GPIO3 GPODAT
#define CONFIG_SYS_INIT_GPIO3_VAL 0x2000

#endif

/* GPIO KEYS */

/* Disable settings */

#define CONFIG_SYS_NAND_4K_PAGE
#undef CONFIG_SYS_NAND_2K_PAGE
#define CONFIG_SYS_NAND_MLC	/* 4bit ECC, 9bytes per 512 */
#undef CONFIG_SYS_NAND_SLC	/* 1bit ECC, 4bytes per 2048 */

/* Include the common config file */
#include <configs/imapx.h>
#define CONFIG_BOARD_MAGIC	0xdb6db608
#define CONFIG_BOARD_HWVER	0x00090001
#define CONFIG_BOARD_OEM	0x20

#endif
