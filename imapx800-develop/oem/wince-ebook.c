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

uint16_t ebook_dot_offs_y;
void oem_ebook_sixdot(uint32_t y, uint32_t num, uint32_t show)
{
	uint16_t offs_y[6] = {0, 14, 28, 42, 56, 70};
	uint16_t offs_x;
	int i;
	char *r;

	r = getenv("revert");
	if(r)
	{
	  offs_x = 750;
	  ebook_dot_offs_y = 196;
	}
	else
	{
	  offs_x = 44;
	  ebook_dot_offs_y = 208;
	}

	for(i = 0; i < 6; i++)
	{
		if(show)
		  oem_set_color(offs_x, ebook_dot_offs_y + offs_y[(r?(5-i):i)], 7, 7,
			 ((i < num)?0xd945: 0x8a49));
		else
		  oem_set_color(offs_x, ebook_dot_offs_y + offs_y[i], 7, 7, 0x8a49);
	}
}

void oem_ebook_prog_init(void)
{
	/* Sixdot slide in */
#if 0
	for(i = 0; i < 20; i++)
	{
		oem_ebook_sixdot(i * 60, 0, 0);
		oem_ebook_sixdot((i+1) * 60, 0, 1);
		udelay(20000);
	}
#endif

	oem_ebook_sixdot(ebook_dot_offs_y, 1, 1);
}

void oem_ebook_prog_finish(void)
{
	lcd_base = (uint8_t *)CONFIG_PRODUCT_WINCE_FB0;

	/* Copy fb1 to wince_fb0 */
	memcpy((uint8_t *)CONFIG_PRODUCT_WINCE_FB0, (uint8_t *)CONFIG_RESV_LCD_BASE_1, (1024 * 600 *2));
	oem_ebook_sixdot(ebook_dot_offs_y, 6, 1);
	oem_fb_switch(1);
}

static int oem_load_nand_NK(uint32_t nand_addr)
{
	uint32_t l;
	image_header_t *hdr = (image_header_t *)CONFIG_SYS_PHY_NK_BASE;

	/* Load the first 4K */
	oem_read_markbad((uint8_t *)CONFIG_SYS_PHY_NK_BASE,
	   nand_addr, 0x1000);

	/* Check Magic */
	if(image_check_hcrc(hdr))
	{
		/* Load the rest OS image */
		l = image_get_size(hdr);
		l = l - (l % 0x1000) + 0x1000;
		oem_read_markbad((uint8_t *)CONFIG_SYS_PHY_NK_BASE,
		   nand_addr, l);
	}
	else
	  printf("Wrong NAND Image type!\n");

	return 0;
}

/* Load OS image to RAM, return 0 on success, not 0 on failure */
int oem_load_NK()
{
	__attribute__((noreturn)) void (*wince)(void);
	char cmd[256];

	/* 1: Try to load NK from SD */
	sprintf(cmd, "%x", CONFIG_SYS_PHY_NK_BASE);
	char *arg_load[] = {"fatload", "mmc", "0:1", cmd, CONFIG_PRODUCT_WINCE_IMAGE_NAME};

	oem_ebook_prog_init();
	oem_ebook_sixdot(ebook_dot_offs_y, 2, 1);
	printf("Loading OS image ...\n");
	if(oem_mmc_init() || do_fat_fsload(NULL, 0, 5, arg_load))
	  goto __load_from_nand;
	oem_ebook_sixdot(ebook_dot_offs_y, 3, 1);
	wince = oem_check_img(CONFIG_SYS_PHY_NK_BASE);
	/* Availiable NK img, bootup winCE */
	if(wince)
	  return 0;

__load_from_nand:
	/* 2: Try to load NK1 from NAND */
	oem_load_nand_NK(CONFIG_SYS_NAND_NK1_OFFS);
	oem_ebook_sixdot(ebook_dot_offs_y, 3, 1);
	wince = oem_check_img(CONFIG_SYS_PHY_NK_BASE);
	/* Availiable NK img, bootup winCE */
	if(wince)
	  return 0;

	//oem_below2("OS image is demaged, try to recover ...");
	oem_ebook_sixdot(ebook_dot_offs_y, 2, 1);
	/* 3: Try to load NK2 from NAND, if Load success, renew NK1 */
	oem_load_nand_NK(CONFIG_SYS_NAND_NK2_OFFS);
	oem_ebook_sixdot(ebook_dot_offs_y, 3, 1);
	wince = oem_check_img(CONFIG_SYS_PHY_NK_BASE);
	/* Availiable NK img, bootup winCE */
	if(wince)
	{
		/* TODO: Renew NK1 */
		uint32_t l;

		l = image_get_size((image_header_t *)CONFIG_SYS_PHY_NK_BASE);
		l = l - (l % 0x1000) + 0x2000;

//		oem_below2("Recovering OS ...");
		oem_erase_markbad(CONFIG_SYS_NAND_NK1_OFFS, CONFIG_SYS_NAND_NK_LEN);
		oem_write_markbad((uint8_t *)CONFIG_SYS_PHY_NK_BASE, CONFIG_SYS_NAND_NK1_OFFS,
		   l, CONFIG_SYS_NAND_NK_LEN);
		return 0;
	}

	return -1; /* Err happened while loading NK */
}

void oem_bootw(void)
{
	image_header_t *header = (image_header_t *)CONFIG_SYS_PHY_NK_BASE;
	uint8_t *data = (uint8_t *)(CONFIG_SYS_PHY_NK_BASE + 0x40);
	__attribute__((noreturn)) void (*wince)(void);
	unsigned long l;

//	printf("Enabling MMU!!\n");
//	oem_mmu_enable();
	if(image_get_comp(header) == IH_COMP_NONE)
	  goto __start_wince__;

	l = image_get_size(header);
	/* Copy Image to SWAP */
//	oem_below2("Decompressing ...");
	oem_ebook_sixdot(ebook_dot_offs_y, 4, 1);
	printf("Move zNK to swap\n");
	memcpy((uint8_t *)CONFIG_SYS_PHY_NK_SWAP, data, l);
	printf("Move zNK to swap...done!\n");
	if(gunzip(data, CONFIG_SYS_PHY_NK_MAXLEN, (uint8_t *)CONFIG_SYS_PHY_NK_SWAP, &l))
	{
		printf("Decompress Error\n");
	}

__start_wince__:
	oem_ebook_sixdot(ebook_dot_offs_y, 5, 1);
//	oem_below2("Starting Window CE ...");
	oem_ebook_prog_finish();
	printf("Starting Window CE ...\n");
	wince = (void *)data;
	/* Boot WinCE */
	(*wince)();
}

