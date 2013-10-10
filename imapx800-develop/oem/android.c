/***************************************************************************** 
** oem/android.c
** 
** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
** 
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** Description: Android specialized functions.
**
** Author:
**     Warits   <warits.wang@infotm.com>
**      
** Revision History: 
** ----------------- 
** 1.1  XXX 07/20/2010 XXX	Warits
*****************************************************************************/

#include <common.h>
#include <oem_func.h>
#include <oem_font.h>
#include <oem_graphic.h>
#include <oem_pic.h>
#include <asm/io.h>
#include <oem_inand.h>
/* Check if the current condition meet recovery requirement
 * 0 not meet, 1 do recovery
 */
int android_check_recovery(void)
{
	printf("Checking recovery key state ...\n");
#ifdef CONFIG_RECOVERY_KEY1_GPIO
	if(readl(CONFIG_RECOVERY_KEY1_GPIO) & (1 << CONFIG_RECOVERY_KEY1_GPIO_NUM))
	  return 0;
#ifdef CONFIG_RECOVERY_KEY2_GPIO
	if(readl(CONFIG_RECOVERY_KEY2_GPIO) & (1 << CONFIG_RECOVERY_KEY2_GPIO_NUM))
	  return 0;
#ifdef CONFIG_RECOVERY_KEY3_GPIO
	if(readl(CONFIG_RECOVERY_KEY3_GPIO) & (1 << CONFIG_RECOVERY_KEY3_GPIO_NUM))
	  return 0;
#endif
#endif
#endif

	/* All required key is pressed */
	return 1;
}

/* Get recovery confirm */
static int android_get_confirm(void)
{
	char str[256];
	int cbstat0 = 0, mnstat0 = 0;
	int cbstat1 = 0, mnstat1 = 0;
	int t;
	printf("Please press the required key again to apply a factory reset ...\n");
#ifndef CONFIG_NO_ADR_SYSRECOVER
	printf("Press MENU key to boot normally...\n");
#endif

	for(t = 0; t < 50000; t++)
	{
#ifdef CONFIG_RECOVERY_KEY1_GPIO
		cbstat1 = readl(CONFIG_RECOVERY_KEY1_GPIO) & (1 << CONFIG_RECOVERY_KEY1_GPIO_NUM);
#ifdef CONFIG_RECOVERY_KEY2_GPIO
		cbstat1 |= readl(CONFIG_RECOVERY_KEY2_GPIO) & (1 << CONFIG_RECOVERY_KEY2_GPIO_NUM);
#ifdef CONFIG_RECOVERY_KEY3_GPIO
		cbstat1 |= readl(CONFIG_RECOVERY_KEY3_GPIO) & (1 << CONFIG_RECOVERY_KEY3_GPIO_NUM);
#endif
#endif
#endif

#ifndef CONFIG_NO_ADR_SYSRECOVER
		mnstat1 = readl(CONFIG_MENU_KEY_GPIO) & (1 << CONFIG_MENU_KEY_GPIO_NUM);
#endif

		if((mnstat0 != mnstat1))
		{
			if(!(mnstat0 = mnstat1))
			  return 0;
		}
		else if((cbstat0 != cbstat1))
		{
			if(!(cbstat0 = cbstat1))
			  return 1;
		}

		if(t % 10000 == 0)
		{
			sprintf(str, "System will boot normally in %d seconds ...", 5 - t / 10000);
			oem_mid2(str);
		}

		udelay(100);
	}
	printf("Time out... boot normally!\n");

	return 0;
}

/* Do android recovery, erase userdata parititon
 * reburn system partition
 */
int android_do_recovery(void)
{
	char cmd[64];
#ifndef CONFIG_SYS_DISK_iNAND
	unsigned long l = CONFIG_SYS_PHY_AS_MAXLEN;
	int ret, i;
	__attribute__((noreturn)) void (*img)(void);
	uint8_t *data = (uint8_t *)(CONFIG_SYS_PHY_AS_SWAP + 0x40);


	printf("Doing recovery ...\n");

	oem_clear_screen(0);
	oem_mid("Press the required keys if you want to apply a factory reset.");
	if(!android_get_confirm())
	  return -1;


	oem_mid(" ");
	oem_mid2("Getting recovery image ...");
	/* 1: Try to load zSYS.img from SD */
	sprintf(cmd, "%x", CONFIG_SYS_PHY_AS_SWAP);
	char *arg_load[] = {"fatload", "mmc", "0:1", cmd, CONFIG_PRODUCT_ANDROID_SYSIMG_NAME};

	if(oem_mmc_init() || do_fat_fsload(NULL, 0, 5, arg_load))
	    goto __load_from_nand;

	img = oem_check_img(CONFIG_SYS_PHY_AS_SWAP);
	/* Availiable NK img, bootup winCE */
	if(img)
	  goto __system_repalce__;

__load_from_nand:
	/* 2: Try to load zSYS.img from SD */
	oem_load_img(CONFIG_SYS_PHY_AS_SWAP, CONFIG_SYS_NAND_BACK1_OFFS,
	      0, CONFIG_SYS_NAND_BACK_LEN);

	img = oem_check_img(CONFIG_SYS_PHY_AS_SWAP);
	if(!img)
	{
		printf("Recovery failed due to no availiable image is found.\n");
		printf("Try to recover from SD card.\n");
		oem_mid2("No recovery image is found, try recover from SD.");
		for(;;);
	}

__system_repalce__:
	oem_mid2("Extracting recovery data ...");

	if(gunzip((uint8_t *)CONFIG_SYS_PHY_UBOOT_SWAP, CONFIG_SYS_PHY_AS_MAXLEN,
		   data, &l))
	{
		printf("Decompress Error\n");
		oem_mid2("Extracting data failed, please check your zSYS.img.");
		for(;;);
	}

	/* Burn System Data */
	oem_mid2("Rewriting file system ...");
	ret = oem_burn_SYS((uint8_t *)CONFIG_SYS_PHY_UBOOT_SWAP, l);

	if(ret)
	{
		printf("Burn system data failed.\n");
		oem_mid2("Invaid file system, please check your zSYS.img.");
		for(;;);
	}

	printf("Burn system data completed.\n");
	oem_mid("Recovery OK !");

	/* Erase User partition */
	oem_erase_markbad(CONFIG_SYS_NAND_UDATA_OFFS, CONFIG_SYS_NAND_UDATA_LEN);

	for(i = 0; i < 10; i++)
	{
		sprintf(cmd, "System will boot in %d seconds ...", 10 - i);
		oem_mid2(cmd);
		udelay(1000000);
	}
	reset_cpu(0);
#else
	char str[64];
	unsigned long l = 0;
	int ret, i;
	__attribute__((noreturn)) void (*img)(void);
	uint8_t *data =( uint8_t *)(CONFIG_SYS_PHY_AS_SWAP+0x40);


	printf("Doing recovery ...\n");

	oem_clear_screen(0);
	oem_mid("Press the required keys if you want to apply a factory reset.");
	if(!android_get_confirm())
	  return -1;
#if 0

	oem_mid(" ");
	oem_mid2("Getting recovery image ...");
	/* 1: Try to load zSYS.img from SD */
	sprintf(cmd, "%x", CONFIG_SYS_PHY_AS_SWAP);
	sprintf(str, "%x:1",SOURCE_CHANNEL );
	char *arg_load[] = {"fatload", "mmc", str, cmd, CONFIG_PRODUCT_ANDROID_SYSIMG_NAME};

	if(oem_mmc_init() || do_fat_fsload(NULL, 0, 5, arg_load))

	    goto __load_from_inand;

	char *s=getenv("filesize");
	l=simple_strtoul(s, NULL, 16);
	/* Availiable NK img, bootup winCE */
	  goto __system_repalce__;
__load_from_inand:
	sprintf(cmd, "%x", CONFIG_SYS_PHY_AS_SWAP);
	sprintf(str, "%x:%d",iNAND_CHANNEL, CONFIG_BACK_PARTITITION );
	
	if(oem_mmc_init()) goto exit;
	arg_load[]= {"fatload", "mmc", str, cmd,CONFIG_PRODUCT_ANDROID_SYSIMG_NAME};
	if(!do_fat_fsload(NULL, 0, 5, arg_load))  goto __system_repalce__;
	arg_load[]= {"fatload", "mmc", str, cmd,UPDATE_PACKAGE_NAME};
	if(!do_fat_fsload(NULL, 0, 5, arg_load)) goto exit;
	
	struct iuw_header hdr;
      	struct iuw_desc desc;
       	int i, as = 0;
       	int ZASlength=0;

       	iuw_get_hdr(CONFIG_SYS_PHY_AS_SWAP, &hdr);
	/* move wrap to NK swap */
	printf("Get wrap, image count=%d\n", hdr.ImgCount);
    	for(i = 0; (i < hdr.ImgCount)&&(TYPE_ADR_AS!=desc.Type); i++){
          	iuw_img_desc(CONFIG_SYS_PHY_AS_SWAP, i, &desc);
             	printf("Get image type=%d, start=%d, count=%d\n",
             	desc.Type, desc.Start, desc.Count);
       		data = iuw_img_data(CONFIG_SYS_PHY_AS_SWAP, i);

              	if(TYPE_ADR_AS==desc.Type){
                      	printf("system.img get here\n");
                       	ZASlength=desc.Count;
                       	ZASlength<<=9;
		 	l=ZASlength;
                }
        }
__system_repalce__:
	if(oem_burn_zAS(data,l)) return -1;
#endif
	oem_disk_clear();
exit:

#endif
	/* never reach here */
	return 0;
}
