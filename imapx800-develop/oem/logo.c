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

void oem_show_logo(void)
{
	oem_fb_switch(1);
	oem_clear_screen(0);
	oem_set_pic(((_sc_w - oem_pic_logo.width)/2),
	   ((_sc_h - oem_pic_logo.height)*4/9), &oem_pic_logo);
	lcd_win_alpha_set(1, 15);
	lcd_win_onoff(1, 1);
}

void oem_hide_logo(void)
{
	oem_fb_switch(1);
	oem_set_color(oem_pic_logo.x + (_sc_w - oem_pic_logo.width)/2,
		oem_pic_logo.y + (_sc_h - oem_pic_logo.height)*4/9, oem_pic_logo.width,
		oem_pic_logo.height, 0);
}
