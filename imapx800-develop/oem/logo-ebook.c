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
#include <livepool_logo.gz.h>

extern void oem_ebook_sixdot(uint32_t y, uint32_t num, uint32_t show);
extern uint16_t ebook_dot_offs_y;
void oem_init_gz_logo(void)
{
	unsigned long l;

	l = sizeof(elonex_ebook_logo);
	if(gunzip((uint8_t *)CONFIG_RESV_LOGO, 0x200000, (uint8_t *)elonex_ebook_logo, &l))
	  printf("Get gzipd logo failed.\n");

	if(!getenv("revert"))
	  oem_revert_pic((uint8_t *)CONFIG_RESV_LOGO, 800, 480);

	return ;
}

/* Logo for elonex ebook */
void oem_show_logo(void)
{
	oem_init_gz_logo();
	oem_fb_switch(1);
	oem_clear_screen(0);
	oem_set_pic_raw(0, 0, 800, 480, (uint8_t *)CONFIG_RESV_LOGO);
	oem_ebook_sixdot(ebook_dot_offs_y, 1, 1);
	lcd_win_alpha_set(1, 15);
	lcd_win_onoff(1, 1);
}

void oem_hide_logo(void)
{
	oem_fb_switch(1);
	oem_clear_screen(0);
}

