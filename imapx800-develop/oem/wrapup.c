/***************************************************************************** 
** common/wrap_up.c
** 
** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
** 
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** Description: Update system through a wrap form.
**
** Author:
**     Warits   <warits.wang@infotm.com>
**      
** Revision History: 
** ----------------- 
** 1.1  XXX 09/16/2010 XXX	Warits
*****************************************************************************/

#include <common.h>
#include <oem_func.h>
#include <oem_font.h>
#include <oem_graphic.h>
#include <oem_pic.h>
#include <asm/io.h>

/* The magics have been move to imapx.h by John */
#define CONFIG_IUW_BLK_SIZE	512
#define CONFIG_IUW_IMG_MAX	13
#define iuw_dbg(x...)		printf("iuw: " x)
#define to_iuw_data(x)		((uint8_t *)(x) + sizeof(struct iuw_header))

/* IuwTypes */
enum iuw_type {
	TYPE_U0 = 0,
	TYPE_UBOOT,
	TYPE_RD,
	TYPE_RD_,
	TYPE_LK,
	TYPE_NK,
	TYPE_ADR_SYS,
	TYPE_ADR_USER,
	TYPE_ADR_NDISK,
	TYPE_ADR_AS,
	TYPE_ICK,
	TYPE_ENV,
	TYPE_LK_XIP,
	TYPE_RD_XIP,
	TYPE_IMG_XIP,
#ifdef CONFIG_SYS_DISK_iNAND
	TYPE_UPDAT,
#endif
	TYPE_UNDEF,
};

struct iuw_header {
	uint32_t	IUWMagic;
	uint32_t	BoardMagic;
	uint32_t	HWVersion;
	uint32_t	ImgCount;
	uint32_t	Datalen;
	uint32_t	SWVersion;
	uint32_t	HeaderCRC;
	uint32_t	DataCRC;
};

struct iuw_desc {
	uint32_t	Magic;
	uint32_t	Type;
	uint32_t	Start;
	uint32_t	Count;
};

/* Iuw lower API */
int iuw_get_hdr(uint8_t *wrap, struct iuw_header *hdr)
{
	uint32_t hcrc, tmp;
	if(*(uint32_t *)wrap != CONFIG_IUW_MAGIC)
	{
		iuw_dbg("Not a valid infoTM wrap! Magic do not match.\n");
		return -1;
	}

	memcpy(hdr, wrap, sizeof(struct iuw_header));
	hcrc = hdr->HeaderCRC;
	hdr->HeaderCRC = 0;
	tmp = crc32(0, (uint8_t *)hdr, sizeof(struct iuw_header));

	// FIXME
#if 1
	if(tmp != hcrc)
	{
		iuw_dbg("Not a valid infoTM wrap! Hcrc do not match.\n");
		return -1;
	}
	else
	  iuw_dbg("Hcrc passed!!\n");
#endif 

	hdr->HeaderCRC = hcrc;
	return 0;
}


int iuw_img_desc(uint8_t * wrap, int num, struct iuw_desc *desc)
{
	struct iuw_desc *w_desc;

	w_desc = (struct iuw_desc *)(wrap + sizeof(struct iuw_header) + sizeof(struct iuw_desc) * num);

	if(w_desc->Magic != CONFIG_IUW_MAGIC)
	{
		iuw_dbg("Failed to get image description magic.\n");
		return -1;
	}
	else if(w_desc->Type > TYPE_UNDEF)
	{
		iuw_dbg("Unknown type of image.\n");
		return -1;
	}

	memcpy(desc, w_desc, sizeof(struct iuw_desc));
	return 0;
}

uint8_t * iuw_img_data(uint8_t * wrap, int num)
{
	struct iuw_desc desc;

	if(iuw_img_desc(wrap, num, &desc))
	  return NULL;

	return (wrap + desc.Start * CONFIG_IUW_BLK_SIZE);
}

uint32_t iuw_img_size(uint8_t * wrap, int num)
{
	struct iuw_desc desc;
	
	if(iuw_img_desc(wrap, num, &desc))
	  return 0;

	return (desc.Count * CONFIG_IUW_BLK_SIZE);
}

/* 0: not valid, 1: is valid */
int iuw_valid(uint8_t *wrap)
{
	uint32_t dcrc, i;
	uint8_t *data = to_iuw_data(wrap);
	struct iuw_header hdr;
	struct iuw_desc desc;

	/* check header */
	if(iuw_get_hdr(wrap, &hdr))
	  return -1;

	/* check board magic */
	if((hdr.HWVersion & CONFIG_HWVER_MASK)
	   == (CONFIG_BOARD_HWVER & CONFIG_HWVER_MASK))
	  goto __hwver_passed__;
	else if((hdr.BoardMagic == CONFIG_BOARD_MAGIC))
	  goto __hwver_passed__;
	else {
		iuw_dbg("Not a wrap for the current board.!\n");
		return -2;
	}

__hwver_passed__:
	/* check dcrc */
	dcrc = crc32(0, data, hdr.Datalen + 512 - 0x20);
	// FIXME
	iuw_dbg("0x%08x <==> save: 0x%08x, len=0x%x\n",
	   dcrc,  hdr.DataCRC, hdr.Datalen);
#if 1
	if(dcrc != hdr.DataCRC)
	{
		iuw_dbg("Not a valid infoTM wrap! Dcrc do not match.\n");
		return -1;
	}
#endif
	iuw_dbg("Dcrc passed.\n");

	/* check images */
	if(hdr.ImgCount <= 0 || hdr.ImgCount > CONFIG_IUW_IMG_MAX)
	{
		iuw_dbg("Not a valid infoTM wrap! Image count out of baundry.\n");
		return -1;
	}

	for(i = 0; i < hdr.ImgCount; i++)
	{
		if(iuw_img_desc(wrap, i, &desc))
		{
			iuw_dbg("Get image %d faild, maybe the update wrap is demaged.\n", i);
			return -1;
		}
	}

	/* a valid wrap */
	return 0;
}

/* ported from android.c */
#ifndef CONFIG_SYS_DISK_iNAND
int iuw_zAS_replace(int init)
{
	int ret;
	unsigned long l = CONFIG_SYS_PHY_AS_MAXLEN;
	__attribute__((noreturn)) void (*img)(void);
	uint8_t *data = (uint8_t *)(CONFIG_SYS_PHY_AS_SWAP + 0x40);

	oem_load_img(CONFIG_SYS_PHY_AS_SWAP, CONFIG_SYS_NAND_BACK1_OFFS,
	      0, CONFIG_SYS_NAND_BACK_LEN);

	img = oem_check_img(CONFIG_SYS_PHY_AS_SWAP);
	if(!img)
	{
		oem_mid2("zAS image is invalid !");
		for(;;);
	}

	oem_mid2("Extracting android data ...");

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
	oem_mid("System replace OK !");

#ifdef CONFIG_NDISK_FTL
	if(init)
	  oem_start_updater(1);
#endif

	return 0;
}

int iuw_update(uint8_t *wrap)
{
	struct iuw_header hdr;
	struct iuw_desc desc;
	int i, as = 0, nd = 0;
	uint8_t *data;

	iuw_get_hdr(wrap, &hdr);

#if 0
	/* move wrap to NK swap */
	if(wrap != (uint8_t *)CONFIG_SYS_PHY_NK_SWAP)
	{
		printf("Move to swap!\n");
		memmove((uint8_t *)CONFIG_SYS_PHY_NK_SWAP, wrap,
		   hdr.Datalen + CONFIG_IUW_BLK_SIZE); 
		wrap = (uint8_t *)CONFIG_SYS_PHY_NK_SWAP;
	}
#endif

	printf("Get wrap, image count=%d\n", hdr.ImgCount);
	for(i = 0; i < hdr.ImgCount; i++)
	{
		iuw_img_desc(wrap, i, &desc);
		printf("Get image type=%d, start=%d, count=%d\n",
		   desc.Type, desc.Start, desc.Count);
		data = iuw_img_data(wrap, i);

		switch(desc.Type)
		{
			case TYPE_U0:
				oem_burn_U0(data);
				break;
			case TYPE_UBOOT:
				oem_burn_uboot(data);
				break;
			case TYPE_RD:
				oem_burn_RD(data);
				break;
			case TYPE_RD_:
				oem_burn_RD_(data);
				break;
			case TYPE_LK:
				oem_burn_LK(data);
				break;
			case TYPE_NK:
				oem_burn_NK(data);
				break;
			case TYPE_ADR_AS:
				oem_burn_zAS(data);
				as = 1;
				break; // FIXME
			case TYPE_ADR_USER:
				oem_burn_UDAT(data, 0);
				break;
			case TYPE_ADR_NDISK:
#ifndef CONFIG_NDISK_FTL
				oem_burn_NDISK(data, 0);
#endif
				nd = 1;
				break;
			case TYPE_ICK:
			case TYPE_ADR_SYS:
			case TYPE_ENV:
			case TYPE_LK_XIP:
			case TYPE_RD_XIP:
			case TYPE_IMG_XIP:
			case TYPE_UNDEF:
			default:
				iuw_dbg("Image type %d is not supported currently.\n",
				   desc.Type);
		}
	}

	if(as)
	  /* burn zAS data to system partition */
	  iuw_zAS_replace(nd);

	return 0;
}
#else
int iuw_zAS_replace(uint8_t * data, int type)                                                                     
{
	int ret;
	unsigned long l = CONFIG_SYS_PHY_AS_MAXLEN;
	__attribute__((noreturn)) void (*img)(void);


	img = oem_check_img((uint32_t)data);
	if(!img)
	{
		oem_mid2("zAS image is invalid !");
		for(;;);
	}

	oem_mid2("Extracting android data ...");

	if(gunzip((uint8_t *)CONFIG_SYS_PHY_UBOOT_SWAP, CONFIG_SYS_PHY_AS_MAXLEN,
		   data + 0x40, &l))
	{
		printf("Decompress Error\n");
		oem_mid2("Extracting data failed, please check your zSYS.img.");
		for(;;);
	}

	/* Burn System Data */
	oem_mid2("Creating file system ...");
	ret = oem_burn_SYS((uint8_t *)CONFIG_SYS_PHY_UBOOT_SWAP, l);

	oem_start_updater(type);
	return ret;
}
int iuw_update(uint8_t *wrap)
{
	struct iuw_header hdr;
	struct iuw_desc desc;
	int i, as = 0, type = 2;
	uint8_t *data=NULL;

	iuw_get_hdr(wrap, &hdr);
	/* move wrap to NK swap */
	if(wrap != (uint8_t *)CONFIG_SYS_PHY_NK_SWAP)
	{
		printf("Move to swap!\n");
		memmove((uint8_t *)CONFIG_SYS_PHY_NK_SWAP, wrap,
		   hdr.Datalen + CONFIG_IUW_BLK_SIZE); 
		wrap = (uint8_t *)CONFIG_SYS_PHY_NK_SWAP;
	}

	printf("Get wrap, image count=%d\n", hdr.ImgCount);
	for(i = 0; i < hdr.ImgCount; i++)
	{
		iuw_img_desc(wrap, i, &desc);
		printf("Get image type=%d, start=%d, count=%d\n",
		   desc.Type, desc.Start, desc.Count);
		data = iuw_img_data(wrap, i);

		switch(desc.Type)
		{
			case TYPE_U0:
				oem_burn_U0(data);
				break;
			case TYPE_UBOOT:
				oem_burn_uboot(data );
				break;
			case TYPE_RD:
				oem_burn_RD(data );
				break;
			case TYPE_RD_:
				oem_burn_RD_(data);
				break;
			case TYPE_LK:
				oem_burn_LK(data );
				break;
			case TYPE_NK:
				oem_burn_NK(data );
				break;
			case TYPE_ADR_AS:
				printf("system.img get here\n");
				
				 as = 1;
				break; // FIXME
			case TYPE_ADR_USER:
				oem_burn_UDAT(data, 0);
				break;
			case TYPE_ADR_SYS:
				oem_burn_SYS(data,0);
				oem_simpro_finish("All user data will be cleared after 10 seconds!");
				
				char j=0;
				for(j=0;j<10;j++){
					char time[32];
					sprintf(time,"%d",j);
					oem_mid(time);
					udelay(1000000);
				} 
				oem_disk_clear();
				break;
			case TYPE_UPDAT:
				oem_burn_UPDT(data);
				break;
			case TYPE_ICK:
			case TYPE_ADR_NDISK:
				type = 1;
				break;
			case TYPE_ENV:
			case TYPE_LK_XIP:
			case TYPE_RD_XIP:
			case TYPE_IMG_XIP:
			case TYPE_UNDEF:
			default:
				iuw_dbg("Image type %d is not supported currently.\n",
				   desc.Type);
		}
	}

	if(as){
	  	/* burn zAS data to system partition */
		return iuw_zAS_replace(data, type);
	}
	return 0;
}
#endif


int iuw_check_update(void)
{
	uint32_t tmp;

	tmp = readl(INFO3);
	switch(tmp)
	{
		case	CONFIG_IUW_MAGIC:
			writel(0, INFO3);
			printf("Normal recovery flow ...\n");
			return 1;
#ifndef CONFIG_SYS_DISK_iNAND
		case CONFIG_IUW_MAGIC2:
			writel(0, INFO3);
			oem_simpro_finish("Erasing user data ...");
			oem_erase_markbad(CONFIG_SYS_NAND_UDATA_OFFS,
		   	CONFIG_SYS_NAND_UDATA_LEN);
			break;
		case CONFIG_IUW_MAGIC3:
			writel(0, INFO3);
			oem_simpro_finish("Erasing local data ...");
			oem_erase_markbad(CONFIG_SYS_NAND_CACHE_OFFS,
		   	0x80000000 - CONFIG_SYS_NAND_CACHE_OFFS);
			break;
		case CONFIG_IUW_MAGIC4:
			writel(0, INFO3);
			oem_simpro_finish("Erasing everything ...");
			oem_erase_markbad(CONFIG_SYS_NAND_CACHE_OFFS,
		   	0x80000000 - CONFIG_SYS_NAND_CACHE_OFFS);
			oem_erase_markbad(CONFIG_SYS_NAND_UDATA_OFFS,
		   	CONFIG_SYS_NAND_UDATA_LEN);
			break;
#else
			//TODO
		case CONFIG_IUW_MAGIC2:
		case CONFIG_IUW_MAGIC3:
		case CONFIG_IUW_MAGIC4:
		case CONFIG_IUW_CLEARDISK:
			writel(0, INFO3);
			oem_start_updater(2);
			break;
#endif
		default:
			break;
	}

	return 0;
}

/* 0, confirm OK, 1 confirm Failed */
int iuw_confirm(uint8_t *wrap)
{
	struct iuw_header hdr;
	char str[32];
	int res;

	res = iuw_valid(wrap);

	switch(res)
	{
		case -1:
			oem_mid("Update package is demaged !");
			oem_mid2("Press power key 7sec to reboot !");
			while(1);
		case -2:
			oem_mid("This is not a wrap for current product !");
			oem_mid2("Press power key 7sec to reboot !");
			while(1);
	}

	oem_clear_screen(0);
	iuw_get_hdr(wrap, &hdr);
	oem_mid("InfoTM update wrap found !");
	sprintf(str, "Firmware Version: %d.%d.%d.%d.%d.%d", hdr.SWVersion >> 16,
	   hdr.HWVersion >> 16, (hdr.HWVersion >> 8) & 0xff, hdr.HWVersion & 0xff,
	   (hdr.SWVersion >> 8) & 0xff, hdr.SWVersion & 0xff);
	oem_mid2(str);
#ifdef CONFIG_RECOVERY_KEY2_GPIO_NAME
	oem_below2("Press " CONFIG_RECOVERY_KEY2_GPIO_NAME " to update, or press RESET to exit.");
#else
	oem_below2("Press SCREEN to update, or press RESET to exit.");
#endif

#ifdef CONFIG_RECOVERY_KEY2_GPIO
	oem_getc_exec_gpio(CONFIG_RECOVERY_KEY2_GPIO, CONFIG_RECOVERY_KEY2_GPIO_NUM);
#endif

	oem_clear_screen(0);
	oem_below2("Updating system ...");
	return 0;
}

#ifndef CONFIG_SYS_DISK_iNAND
int iuw_upfrom_nand(void)
{
	uint8_t *data = (uint8_t *)CONFIG_SYS_PHY_LK_BASE;

	oem_mid("Reading update package ...");
	oem_read_markbad(data, CONFIG_SYS_NAND_RELOC_OFFS, CONFIG_SYS_NAND_BACK_LEN);
	oem_mid("done");
	if(iuw_confirm(data) == 0)
	  iuw_update(data);

	return 0;
}
#else
int iuw_upfrom_inand(void)
{	
        uint8_t *data = (uint8_t *)CONFIG_SYS_PHY_LK_BASE;
        char cmd[64];
	sprintf(cmd,"fatload mmc %d:%d %x %s",
			iNAND_CHANNEL,
			CONFIG_BACK_PARTITITION, 
		        (unsigned int)	data,
			UPDATE_PACKAGE_NAME);
#if 0
 	/*   
	 *   for test, load update package from source sd card.
	 */
	udelay(2000000);
	run_command("mmc rescan 0",0);
	printf("\nload update package.\n");
	sprintf(cmd,"fatload mmc %d:1 %x %s",SOURCE_CHANNEL ,(unsigned int)  data, UPDATE_PACKAGE_NAME);
	printf("%s\n",cmd);
#endif
	char ret=0;
	if(!(ret=run_command(cmd,0))){
		char *size;
		size=getenv("filesize");
		int sizet=simple_strtoul(size, NULL, 16);
		printf("Get update package filesize:%s %d\n",size,sizet);
	}else{
		printf("something error in loading %s\n",UPDATE_PACKAGE_NAME);
		return -1;
	}
      
       if(iuw_confirm(data) == 0)
         iuw_update(data);
 
        return 0;
} 
#endif
