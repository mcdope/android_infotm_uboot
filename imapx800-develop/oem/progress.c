/***************************************************************************** 
** progress.c
** 
** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
** 
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** Description: This code implant a simple progress bar.
**
** Author: warits <warits.wang@infotmic.com.cn>
**      
** Revision History: 
** ----------------- 
** 1.1  06/25/2010 
*****************************************************************************/

#include <common.h>
#include <oem_func.h>
#include <oem_graphic.h>

static uint32_t p_count, p_len;
static const uint32_t bar_len = 600;

static void oem_simpro_up(u_short n)
{
	uint32_t top, div = 1;
	if(p_len >> 10)
	  div = p_len >> 10;

	top = (p_count++) * (bar_len << 2) / div;
	top = ((top > bar_len)?bar_len:top);
	oem_set_color((_sc_w - bar_len)/2, ((_sc_h>480)?337:259) + 2,
		   top, 6, 0xffff);
}
static uint32_t len=0;
int oem_simpro_init(char *title, uint32_t size)
{
	if(!size)
	  return -1;

	p_len = size;
	len=0;
	p_count = 0;
	oem_progress_update = oem_simpro_up;

	/* Clear Screen */
	oem_clear_screen(0);
	/* Show title */
	if(title)
	  oem_mid2(title);
	/* Show progress box */
	oem_draw_box((_sc_w - bar_len)/2 - 2, ((_sc_h>480)?337:259),
		   (_sc_w + bar_len)/2 + 2, ((_sc_h>480)?337:259) + 9);

	return 0;
}

int oem_simpro_update(uint32_t n)
{
	uint32_t w=0;
	w=(bar_len*n)/p_len;
	oem_set_color((_sc_w - bar_len)/2+len,((_sc_h>480)?337:259) + 2,w,6,0xFFFF);
	len+=w;
	return 0;
}
int oem_simpro_finish(char *title)
{
	p_count = 0;
	p_len = 0;
	len=0;
	oem_progress_update = NULL;

	/* Clear Screen */
	oem_clear_screen(0);
	/* Show title */
	if(title)
	  oem_mid2(title);

	return 0;
}
