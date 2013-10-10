/***************************************************************************** 
** common/oem_func.c
** 
** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
** 
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** Description: OEM functions.
**
** Author:
**     Warits   <warits.wang@infotm.com>
**      
** Revision History: 
** ----------------- 
** 1.1  XXX 02/01/2010 XXX	Warits
*****************************************************************************/

#include <common.h>
#include <oem_func.h>
#include <oem_font.h>
#include <oem_graphic.h>
#include <oem_pic.h>
#include <asm/io.h>
#include <oem_inand.h>
void (* oem_progress_update)(unsigned short val) = 0;
#ifndef CONFIG_SYS_DISK_iNAND
int oem_burn_img(enum oem_image_type type,
   uint8_t *data, uint32_t ndaddr, uint32_t len, uint32_t max_len)
{
	uint32_t l;

	switch(type)
	{
		case OEM_IMAGE_RAW:
		{
			l = (len + 0xfff) & (~0xfff);
			if(max_len & 0x7ffff)
			{
				printf("RAW image can only be block aligned.\n");
				return -1;
			}
			else if(max_len < l)
			{
				printf("Invalid argument.\n");
				return -1;
			}

			oem_simpro_init("Updating U0 ...", l * 9 / 10);
			oem_erase_markbad(ndaddr, max_len);
			printf("Writing data to NAND, addr=%x, size=%x\n",
			   (uint32_t)data, max_len);
			oem_write_markbad(data, ndaddr, l, max_len);
			oem_simpro_finish("Update U0 finished.");
			break;
		}
		case OEM_IMAGE_RAW_H:
		{
			image_header_t *hdr = (image_header_t *)data;
			char title[256];
			if(!oem_check_img((uint32_t)data))
			  return -1;

			/* Erase 8MB */
			l = image_get_size(hdr);
			l = (l + 0x2000) & (~0xfff);

			if(l > max_len)
			{
				printf("Maxlen(0x%08x) does not fit image length(0x%08x)!\n",
				   max_len, l);
				return -1;
			}

			sprintf(title, "Updating image %s", image_get_name(hdr));
			oem_simpro_init(title, l * 9 / 10);
			oem_erase_markbad(ndaddr, max_len);
			printf("Writing data to NAND, addr=%x, size=%x\n",
			   (uint32_t)data, l);
			oem_write_markbad(data, ndaddr, l, max_len);
			sprintf(title, "Update image %s finished.", image_get_name(hdr));
			oem_simpro_finish(title);
			break;
		}
		case OEM_IMAGE_YAFFS:
		{
			oem_simpro_init("Burnning file system ...", len * 9 / 10);
			if(oem_write_yaffs((uint8_t *)data, len,
				   ndaddr, max_len))
			{
				oem_simpro_finish("Burn file system failed !");
				printf("Burn yaffs failed.\n");
				return -1;
			}

			printf("Burn OK.\n");
			oem_simpro_finish("Burn file system finished.");
			break;
		}
		default:
			printf("Image type %d can not be recognized.\n", type);
	}

	return 0;
}

int oem_burn_RD(uint8_t * data)
{
	int ret;
	ret  = oem_burn_img(OEM_IMAGE_RAW_H, data, CONFIG_SYS_NAND_RD1_OFFS,
	   0, 0x200000);
	ret |= oem_burn_img(OEM_IMAGE_RAW_H, data, CONFIG_SYS_NAND_RD2_OFFS,
	   0, 0x200000);
	return ret;
}

int oem_burn_RD_(uint8_t * data)
{
	int ret;
	ret  = oem_burn_img(OEM_IMAGE_RAW_H, data, CONFIG_SYS_NAND_RD__OFFS,
	   0, 0x600000);
	return ret;
}

int oem_burn_LK(uint8_t * data)
{
	int ret;

	ret  = oem_burn_img(OEM_IMAGE_RAW_H, data, CONFIG_SYS_NAND_LK1_OFFS,
	   0, 0x600000);
	ret |= oem_burn_img(OEM_IMAGE_RAW_H, data, CONFIG_SYS_NAND_LK2_OFFS,
	   0, 0x600000);
	return ret;
}

int oem_burn_zAS(uint8_t * data)
{
	int ret;

	ret  = oem_burn_img(OEM_IMAGE_RAW_H, data, CONFIG_SYS_NAND_BACK1_OFFS,
	   0, CONFIG_SYS_NAND_BACK_LEN);
	return ret;
}

int oem_burn_SYS(uint8_t *data, uint32_t size)
{
	return oem_burn_img(OEM_IMAGE_YAFFS, data,
	   CONFIG_SYS_NAND_SYSTEM_OFFS,
	   size, CONFIG_SYS_NAND_SYSTEM_LEN);
}

int oem_burn_UDAT(uint8_t *data, uint32_t size)
{
	return oem_burn_img(OEM_IMAGE_YAFFS, data,
	   CONFIG_SYS_NAND_UDATA_OFFS,
	   size, CONFIG_SYS_NAND_UDATA_LEN);
}

int oem_burn_NDISK(uint8_t *data, uint32_t size)
{
#ifndef CONFIG_NDISK_FTL
	return oem_burn_img(OEM_IMAGE_YAFFS, data,
	   CONFIG_SYS_NAND_NDISK_OFFS,
	   size, CONFIG_SYS_NAND_NDISK_LEN);
#else
	return oem_start_updater(1);
#endif
}

int oem_burn_NK(uint8_t * data)
{
	int ret;

	ret  = oem_burn_img(OEM_IMAGE_RAW_H, data, CONFIG_SYS_NAND_NK1_OFFS,
	   0, CONFIG_SYS_NAND_NK_LEN);
	ret |= oem_burn_img(OEM_IMAGE_RAW_H, data, CONFIG_SYS_NAND_NK2_OFFS,
	   0, CONFIG_SYS_NAND_NK_LEN);
	return ret;
}

int oem_burn_uboot(uint8_t * data)
{
	int ret;
	ret  = oem_burn_img(OEM_IMAGE_RAW_H, data, CONFIG_SYS_NAND_U1_OFFS,
	   0, 0x300000);
	ret |= oem_burn_img(OEM_IMAGE_RAW_H, data, CONFIG_SYS_NAND_U2_OFFS,
	   0, 0x300000);
	return ret;
}

/*
 * Check u-boot and NK validity after factory burn.
 * Return value have meaning as following:
 * [31:4]	reserved.
 * [3: 2]	00: U1 & U2 is invalid.
 *			01: U1 is invalid, U2 is valid.
 *			10: U1 is valid, U2 is invalid.
 *			11: U1 & U2 is valid.
 * [1: 0]	00: NK1 & NK2 is invalid.
 *			01: NK1 is invalid, NK2 is valid.
 *			10: NK1 is valid, NK2 is invalid.
 *			11: NK1 & NK2 is valid.
 */
uint32_t oem_factory_check(void)
{
	uint32_t ret = 0;
	
	oem_read_markbad((uint8_t *)CONFIG_SYS_PHY_UBOOT_SWAP, CONFIG_SYS_NAND_U1_OFFS,
	   CONFIG_SYS_NAND_U_BOOT_SIZE);
	if(oem_check_img(CONFIG_SYS_PHY_UBOOT_SWAP))
		ret |= (1 << 3);
	
	oem_read_markbad((uint8_t *)CONFIG_SYS_PHY_UBOOT_SWAP, CONFIG_SYS_NAND_U2_OFFS,
	   CONFIG_SYS_NAND_U_BOOT_SIZE);
	if(oem_check_img(CONFIG_SYS_PHY_UBOOT_SWAP))
		ret |= (1 << 2);
	
#if defined(CONFIG_WINCE_FEATURE)
	oem_load_nand_img(CONFIG_SYS_PHY_NK_BASE, CONFIG_SYS_NAND_NK1_OFFS);
	if(oem_check_img(CONFIG_SYS_PHY_NK_BASE))
		ret |= (1 << 1);
	
	oem_load_nand_img(CONFIG_SYS_PHY_NK_BASE, CONFIG_SYS_NAND_NK2_OFFS);
	if(oem_check_img(CONFIG_SYS_PHY_NK_BASE))
		ret |= (1 << 0);
#else
	oem_load_nand_img(CONFIG_SYS_PHY_LK_BASE, CONFIG_SYS_NAND_LK1_OFFS);
	if(oem_check_img(CONFIG_SYS_PHY_LK_BASE))
	  ret |= (1 << 5);

	oem_load_nand_img(CONFIG_SYS_PHY_LK_BASE, CONFIG_SYS_NAND_LK2_OFFS);
	if(oem_check_img(CONFIG_SYS_PHY_LK_BASE))
	  ret |= (1 << 4);
#endif

	oem_simpro_finish("Please pull out OTG line and restart.");
	return ret;
}

int oem_disk_clear(void)
{
	/* FIXME: This needs to recode to match various NAND chip */
#ifdef CONFIG_WINCE_FEATURE
	return oem_erase_markbad(0x12000000, 0x6d000000);
#else
	return oem_erase_markbad(CONFIG_SYS_NAND_CACHE_OFFS,
	   0x80000000 - CONFIG_SYS_NAND_CACHE_OFFS);
#endif
}

int oem_clear_badblock(void)
{
	oem_scrub(0x2000000, 0x7e000000);
	udelay(100000);
	/* Clean two times to make sure */
	return oem_scrub(0x2000000, 0x7e000000);
}

int oem_clear_env(void)
{
	return oem_erase_markbad(0x480000, 0x80000);
}

int oem_load_nand_img(uint32_t ram_base, uint32_t nand_addr)
{
	uint32_t l;
	image_header_t *hdr = (image_header_t *)ram_base;

	/* Load the first 4K */
	oem_read_markbad((uint8_t *)ram_base,
	   nand_addr, 0x1000);

	/* Check Magic */
	if(image_check_hcrc(hdr))
	{
		/* Load the rest OS image */
		l = image_get_size(hdr);
		l = (l + 0x2000) & (~0xfff);
		oem_read_markbad((uint8_t *)ram_base,
		   nand_addr, l);
	}
	else
	  printf("Wrong NAND Image type!\n");

	return 0;
}

int oem_load_img(uint32_t ram_base, uint32_t nand_addr,
   uint32_t back_addr, uint32_t recv_len)
{
	__attribute__((noreturn)) void (*kernel)(void);

	/* 2: Try to load NK1 from NAND */
	printf("Try load Image from NAND...\n");
	oem_load_nand_img(ram_base, nand_addr);
	kernel = oem_check_img(ram_base);
	/* Availiable NK img, bootup winCE */
	if(kernel)
	  return 0;

	if(!back_addr)
	  return -1;

	printf("Image is demaged, try to recover...\n");
	/* 3: Try to load NK2 from NAND, if Load success, renew NK1 */
	oem_load_nand_img(ram_base, back_addr);
	kernel = oem_check_img(ram_base);
	/* Availiable NK img, bootup winCE */
	if(kernel)
	{
		/* TODO: Renew NK1 */
		uint32_t l;

		l = image_get_size((image_header_t *)ram_base);
		l = (l + 0x2000) & (~0xfff);

		printf("Recovering Image ...\n");
		oem_erase_markbad(back_addr, recv_len);
		oem_write_markbad((uint8_t *)ram_base, nand_addr,
		   l, recv_len);
		return 0;
	}

	return -1; /* Err happened while loading NK */
}

#ifdef CONFIG_NDISK_FTL
/* type: 1=init, 2=reset, 3=update */
int oem_start_updater(int type)
{
	char cmd[64];
#if 0
	sprintf(cmd,"mmc rescan %d",iNAND_CHANNEL);	
	run_command(cmd,0);
#endif

	oem_load_LK();
	if(oem_load_updater()){
              printf("backup updater is invalid.\n");
              oem_simpro_finish("updater invalid. System update exited.");
	       return -1;
        }

		sprintf(cmd,"%s updater=%d.%d.%d",
		   CONFIG_LINUX_DEFAULT_BOOTARGS,
		   type,2,SOURCE_CHANNEL);

#ifdef CONFIG_CMDLINE_PARTITIONS
		strcat(cmd, CONFIG_CMDLINE_PARTITIONS);
#endif

        setenv("bootargs", cmd);
        sprintf(cmd,"bootm %x %x",CONFIG_SYS_PHY_LK_BASE,CONFIG_SYS_PHY_RD_BASE);
        run_command(cmd,0);

        printf("There is something error when boot updater.\n\r");
        oem_simpro_finish("Failed to boot updater. System update exited.");

	udelay(10000000);
	return -1;
}
#endif

#else
int oem_burn_img(enum inandimage type, uint8_t *data)
{
	int 	inand_add=0,
		back_addr=0,
		recv_len=0;

	if(!oem_check_img((unsigned int)data)) return -1;
	recv_len=image_get_data_size ( (image_header_t *)data)+0x40;
	printf("data size:0x%x\n",recv_len);
	recv_len>>=9;
	recv_len+=1;
	//int startblk= oem_get_imagebase();
	switch(type){
		case UIMAGE:
			inand_add=iNAND_START_ADDR_UIMG;
			if(!recv_len) 
			recv_len=iNAND_LEN_UIMG;
			break;
		case RAMDISK:
			inand_add=iNAND_START_ADDR_RD;
		        if(!recv_len) recv_len=iNAND_LEN_RD;
			break;
		case RAMDRE:
			inand_add=iNAND_START_ADDR_RE;
			if(!recv_len) recv_len=iNAND_LEN_RE;
			break;
		case UPDATER:
			inand_add=iNAND_START_ADDR_UPDATER;
			if(!recv_len) 
			recv_len=iNAND_LEN_UPDATER;
			break;
		default:
			return -1;
	}
	char filename[4][16]={"uImage","ramdisk.img","recovery_rd.img","updater"};
	char info[64];
	sprintf(info,"Burning %s",filename[type]);
	oem_simpro_init(info,recv_len);
	if(oem_write_inand((char *)data,inand_add,recv_len)){
		sprintf(info,"Burning %s failed",filename[type]);
		oem_simpro_finish(info);
		udelay(2000000);
	       	return -1;
	}else{
		memset(data,0, recv_len<<9);
		oem_read_inand((char *)data,inand_add,recv_len);
		 if(!oem_check_img((unsigned int)data)){
			sprintf(info,"Burn %s failed",filename[type]);
			oem_simpro_finish(info);
			udelay(2000000);
			return -1;
		 }
		sprintf(info,"Burn %s successed",filename[type]);
		oem_simpro_finish(info);
		udelay(500000);
	}

	back_addr=inand_add+iNAND_BACK_IMAGE_OFFSET;
	sprintf(info,"Backing up %s",filename[type]);
	oem_simpro_init(info,recv_len);
	if(oem_write_inand((char *)data,back_addr,recv_len)){
		sprintf(info,"Backup %s failed",filename[type]);
		oem_simpro_finish(info);
		udelay(2000000);
	       	return -1;
	}else{
		memset(data,0, recv_len<<9);
		oem_read_inand((char *)data,inand_add,recv_len);
		if(!oem_check_img((unsigned int)data)){
			sprintf(info,"Backup %s failed",filename[type]);
			oem_simpro_finish(info);
			udelay(2000000);
			return -1;
		}
		sprintf(info,"Backup %s successed",filename[type]);
		oem_simpro_finish(info);
		udelay(500000);
	}
	return 0;
}


int oem_burn_RD(uint8_t * data)
{
	return oem_burn_img( RAMDISK ,data );
}

int oem_burn_RD_(uint8_t * data )
{
 	return oem_burn_img( UPDATER ,data );
}

int oem_burn_LK(uint8_t * data)
{
	return oem_burn_img( UIMAGE ,data );

}
 int oem_burn_UPDT(uint8_t *data)
{
	return oem_burn_img( UPDATER ,data );
}

/* type: 1=init, 2=reset, 3=update */
int oem_start_updater(int type)
{
	char cmd[64];
	sprintf(cmd,"mmc rescan %d",iNAND_CHANNEL);	
	run_command(cmd,0);

	oem_load_LK();
	if(oem_load_updater()){
              printf("backup updater is invalid.\n");
              oem_simpro_finish("updater invalid. System update exited.");
	       return -1;
        }

		sprintf(cmd,"%s updater=%d.%d.%d",
		   CONFIG_LINUX_DEFAULT_BOOTARGS,
		   type,iNAND_CHANNEL,SOURCE_CHANNEL);
        setenv("bootargs", cmd);
        sprintf(cmd,"bootm %x %x",CONFIG_SYS_PHY_LK_BASE,CONFIG_SYS_PHY_RD_BASE);
        run_command(cmd,0);

        printf("There is something error when boot updater.\n\r");
        oem_simpro_finish("Failed to boot updater. System update exited.");

	udelay(10000000);
	return -1;
}
int oem_burn_zAS(uint8_t * data )
{
	//if(!oem_check_img((uint32_t)data)){
	//	oem_simpro_finish("system image invalidate.");
	//	printf("system image invalidate.\n");
	//	return -1;
	//}
	if(!oem_check_img((uint32_t)data))
	  return -1;
	uint32_t	size=image_get_data_size ( (image_header_t *)data);
	return	oem_burn_SYS(data+0x40, size);
}

int oem_burn_SYS(uint8_t *data, uint32_t size)
{
	int base=oem_get_systembase();
	if(size==0){
		size=0x4A000;
	}else{
		size>>=9;
		size+=1;
	}
	printf("burnsystem\n");

	printf("system image size:%dKbytes.\n",size/2);	
	char * buffer=(char *)data;
	oem_simpro_init("Creating file system ...",size);
	
	if(oem_write_inand(buffer,base,size)){
		printf("write failed buffer: %x remain size: %x\n",(int) buffer , size);
		oem_simpro_finish("Failed to create file system.");
		return -1;
	}
	oem_simpro_finish("File system is ready.");
	printf("system.img burn success.\n");
	
	return 0;
}
int oem_burn_UDAT(uint8_t *data ,uint32_t size)
{
	return 0;
}

int oem_burn_NDISK(uint8_t *data ,uint32_t size)
{
	return 0;
}

int oem_burn_NK(uint8_t * data )
{
	return 0;

}

int oem_burn_uboot(uint8_t * data)
{
	int ret=0;
	return ret;
}

/*
 * Check u-boot and NK validity after factory burn.
 * Return value have meaning as following:
 * [31:4]	reserved.
 * [3: 2]	00: U1 & U2 is invalid.
 *			01: U1 is invalid, U2 is valid.
 *			10: U1 is valid, U2 is invalid.
 *			11: U1 & U2 is valid.
 * [1: 0]	00: NK1 & NK2 is invalid.
 *			01: NK1 is invalid, NK2 is valid.
 *			10: NK1 is valid, NK2 is invalid.
 *			11: NK1 & NK2 is valid.
 */
uint32_t oem_factory_check(void)
{
	uint32_t ret = 0;
	
	
#if defined(CONFIG_WINCE_FEATURE)
#else
#endif

	oem_simpro_finish("Please pull out OTG line and restart.");
	return ret;
}

int oem_disk_clear(void)
{
	oem_start_updater(1);
	return 0;
}

int oem_clear_badblock(void)
{
	return 0;
}

int oem_clear_env(void)
{
	return 0;
}

int oem_load_img(uint32_t ram_base, char image)
{
	int inand_add=0, back_addr=0, readlen=0;

	switch(image){
		case UIMAGE:
			inand_add=iNAND_START_ADDR_UIMG;
			readlen=iNAND_LEN_UIMG;
			break;
		case RAMDISK:
			inand_add=iNAND_START_ADDR_RD;
		        readlen=iNAND_LEN_RD;
			break;
		case RAMDRE:
			inand_add=iNAND_START_ADDR_RE;
			readlen=iNAND_LEN_RE;
			break;
		case UPDATER:
			inand_add=iNAND_START_ADDR_UPDATER;
			readlen=iNAND_LEN_UPDATER;
			break;
		default:
			return -1;
	}
	memset((char *)ram_base,0, readlen<<9);
	if( oem_read_inand((char *)ram_base, inand_add, readlen))return Erro_READ;
	if(oem_check_img(ram_base)) return Erro_NON;

	back_addr=inand_add+iNAND_BACK_IMAGE_OFFSET;	
	memset((char *)ram_base,0, readlen<<9);
	if( oem_read_inand((char *)ram_base, back_addr, readlen))return Erro_READ;
	if(!oem_check_img(ram_base)) return -1;

	printf("Recovering Image ...\n");
	if(oem_write_inand((char *)ram_base, inand_add, readlen)) return -1;

	memset((char *)ram_base,0, readlen<<9);
    	if( oem_read_inand((char *)ram_base, inand_add, readlen))return Erro_READ;	
	if(oem_check_img(ram_base)) return 0;

	return -1;

}
#endif

/*
 * This function check the image validity in RAM,
 * and return the pointer to real image if check passed.
 * return NULL if no available image is found.
 */
void * oem_check_img(uint32_t addr)
{
	image_header_t *header = (image_header_t *)addr;

	/* First: Check Header CRC32 */
	if(!image_check_magic(header))
	  return NULL;

	if(!image_check_hcrc(header))
	{
		printf("Header CRC32 do not match!!\n");
		return NULL;
	}

#if 0
	/* Second: Check compress type */
	if(be32_to_cpu(header->ih_comp) != IH_COMP_NONE)
	  goto __exit__;
#endif

	/* Third: Check DATA CRC32 */
	if(!image_check_dcrc(header))
	{
		printf("Data CRC32 do not match!!\n");
		return NULL;
	}

	printf("Image Ready! Load address = 0x%08x\n",
	   image_get_load(header));
	return (void *)image_get_load(header);
}

#ifdef CONFIG_SYS_BOOT_NOR
int oem_burn_U0(uint8_t * data)
{
#if 0
	flush_cache(CONFIG_SYS_SDRAM_BASE, CONFIG_SYS_SDRAM_END);

	/* turn off I/D-cache */
	icache_disable();
	dcache_disable();
#endif

	oem_simpro_finish("Updating U0 ...");
	
	nor_hw_init();
	nor_erase_chip();
#ifdef CONFIG_NOR_256K
	nor_program(0, data, 0x40000);
#else
	nor_program(0, data, 0x60000);
#endif
	oem_simpro_finish("Update U0 finished !");
	return 0;
}
#else
int oem_burn_U0(uint8_t * data)
{
	return oem_burn_img(OEM_IMAGE_RAW, data, 0, 0x80000, 0x80000);
}
#endif

void oem_uboot_maintain(void)
{
	uint16_t stat = *(uint16_t *)CONFIG_SYS_PHY_BOOT_STAT;

	//printf("Uboot maintaining\n");
	if(stat == CONFIG_BOOTSTAT_U1)
	{
		printf("Boot status: U1!!\n");
		/* Normal boot */
		return;
	}
	else if(stat == CONFIG_BOOTSTAT_U2)
	{
		/* U1 is reported as invalid */
		printf("U1 is reported as invalid, checking again.\n");
#ifndef CONFIG_SYS_DISK_iNAND
		oem_read_markbad((uint8_t *)CONFIG_SYS_PHY_UBOOT_SWAP, CONFIG_SYS_NAND_U1_OFFS,
		   CONFIG_SYS_NAND_U_BOOT_SIZE);
		if(oem_check_img(CONFIG_SYS_PHY_UBOOT_SWAP))
		{
			printf("U1 is good!\n");
			return;
		}
		oem_erase_markbad(CONFIG_SYS_NAND_U1_OFFS, 0x300000);
		oem_read_markbad((uint8_t *)CONFIG_SYS_PHY_UBOOT_SWAP, CONFIG_SYS_NAND_U2_OFFS,
		   CONFIG_SYS_NAND_U_BOOT_SIZE);
		oem_write_markbad((uint8_t *)CONFIG_SYS_PHY_UBOOT_SWAP, CONFIG_SYS_NAND_U1_OFFS,
		   CONFIG_SYS_NAND_U_BOOT_SIZE, 0x300000);
#endif
#if 0
		int badblk = *(uint16_t *)(CONFIG_SYS_PHY_BOOT_STAT + 2);
		/* U1 is surely invalid, renew it */
		printf("U1 is surely invalid, mark blk %d as bad.\n", badblk);
		sprintf(cmd, "nand markbad %x", badblk * 0x80000);
		run_command(cmd, 0);
#endif
		printf("U1 rewrite OK!\n");
		return ;
	}
	else if(stat == CONFIG_BOOTSTAT_U0)
	{
		/* System is bootup through U0,
		   both u1&u2 is bad,
		   there is nothing we can do */
		printf("Boot status: U0!\n");
		return ;
	}
	else
	{
		printf("Boot status: unknown!\n");
		return ;
	}
}

