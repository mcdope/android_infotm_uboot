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

#if 0
static void oem_prog_update_ce(u_short val)
{
	static u_short x = 0;
	static u_long uc = 0;
	u_short bw;
	
	if(uc++ % 10 != 0) return;
	/* Show prog bar */
	oem_set_pic(((_sc_w - oem_pic_bar.width)/2) - 2,
	   ((_sc_h - oem_pic_bar.height)*72/100), &oem_pic_bar);

	bw = oem_pic_bar.width - 2;
	/* Show spark */
	if(x < oem_pic_spark.width)
	{
		oem_set_pic_raw_offset(((_sc_w - bw)/2) - 2, ((_sc_h - oem_pic_bar.height)*72/100),
		   oem_pic_spark.width, oem_pic_spark.height, (oem_pic_spark.width - x), 0,
		   x, oem_pic_spark.height, oem_pic_spark.pic);
	}
	else if(x > bw)
	{
		oem_set_pic_raw_offset(((_sc_w - bw)/2) - 2 + x - oem_pic_spark.width,
		   ((_sc_h - oem_pic_bar.height)*72/100), oem_pic_spark.width, oem_pic_spark.height,
		   0, 0, bw + oem_pic_spark.width - x, oem_pic_spark.height, oem_pic_spark.pic);
	}
	else
	{
		oem_set_pic(((_sc_w - bw)/2) + x - oem_pic_spark.width - 2,
		   ((_sc_h - oem_pic_bar.height)*72/100), &oem_pic_spark);
	}

	x = (x + 5) % (bw + oem_pic_spark.width - 2);	
}

static void oem_prog_update_ce1(u_short val)
{
	static int a1 = 0, a2;
	if(val % 70 != 0) return;

	a2 = a1 - 8;
	if(a2 < 0) a2 = -a2;

	lcd_win_alpha_set(1, a2 + 7);
	a1 = (a1 + 1) % 16;
}
#endif

void oem_progress_init(void)
{
//	char *s;

	oem_hide_window(1);
	oem_clear_screen(0);

	oem_fb_switch(1);

	/* Show WinCE logo */
	oem_set_pic(((_sc_w - oem_pic_celogo.width)/2),
	   ((_sc_h - oem_pic_celogo.height)*33/100), &oem_pic_celogo);
#if 0
	/* Show Starting winCE info */
	oem_set_pic(((_sc_w - oem_pic_cetext.width)/2) - 2,
	   ((_sc_h - oem_pic_cetext.height)*65/100), &oem_pic_cetext);
#endif
	/* Show microsoft */
	oem_set_pic(((_sc_w - oem_pic_mstext.width)/2) - 7,
	   ((_sc_h - oem_pic_mstext.height)*88/100), &oem_pic_mstext);

#if 0
	s = getenv("bootstyle");
	if(simple_strtoul(s, NULL, 16) == 0)
	{
		/* Show prog bar */
		oem_set_pic(((_sc_w - oem_pic_bar.width)/2) - 2,
		   ((_sc_h - oem_pic_bar.height)*72/100), &oem_pic_bar);
		oem_progress_update = oem_prog_update_ce;
	}
	else
	{
		oem_progress_update = oem_prog_update_ce1;
	}
#endif

	oem_show_window(1);
}

void oem_progress_finish(void)
{	
	oem_hide_window(1);
	oem_progress_update = NULL;
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
		l = (l + 0x2000) & (~0xfff);
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

	/* 1: Try to load NK from SD, XXX REMOVED XXX */
	oem_progress_init();
	oem_below2("Loading OS image ...");
	printf("Loading OS image ...\n");

	/* 2: Try to load NK1 from NAND */
	oem_load_nand_NK(CONFIG_SYS_NAND_NK1_OFFS);
	oem_below2("CRC32 Checking ...");
	wince = oem_check_img(CONFIG_SYS_PHY_NK_BASE);
	/* Availiable NK img, bootup winCE */
	if(wince)
	  return 0;

	oem_below2("OS image is demaged, try to recover ...");
	/* 3: Try to load NK2 from NAND, if Load success, renew NK1 */
	oem_load_nand_NK(CONFIG_SYS_NAND_NK2_OFFS);
	oem_below2("CRC32 Checking ...");
	wince = oem_check_img(CONFIG_SYS_PHY_NK_BASE);
	/* Availiable NK img, bootup winCE */
	if(wince)
	{
		/* TODO: Renew NK1 */
		uint32_t l;

		l = image_get_size((image_header_t *)CONFIG_SYS_PHY_NK_BASE);
		l = (l + 0x2000) & (~0xfff);

		oem_below2("Recovering OS ...");
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
	oem_below2("Decompressing ...");
	printf("Move zNK to swap\n");
	memcpy((uint8_t *)CONFIG_SYS_PHY_NK_SWAP, data, l);
	printf("Move zNK to swap...done!\n");
	if(gunzip(data, CONFIG_SYS_PHY_NK_MAXLEN, (uint8_t *)CONFIG_SYS_PHY_NK_SWAP, &l))
	{
		printf("Decompress Error\n");
	}

__start_wince__:
	oem_below2("Starting Window CE ...");
	printf("Starting Window CE ...\n");
	wince = (void *)data;
	/* Copy fb1 to wince_fb0 */
	memcpy((uint8_t *)CONFIG_PRODUCT_WINCE_FB0, (uint8_t *)CONFIG_RESV_LCD_BASE_1, (1024 * 600 *2));
	/* Boot WinCE */
	(*wince)();
}

