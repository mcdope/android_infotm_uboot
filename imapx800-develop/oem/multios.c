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

static struct oem_kernel oem_kernel_list[] = {
	{"Linux", &oem_pic_pungin, oem_bootl},
	{"Windows", &oem_pic_window, oem_bootw},
};

static int cur_kd = 0;

#define _oem_box_size 160
#define _oem_box_x ((_sc_w - _oem_box_size)/2)
#define _oem_box_y ((_sc_h - _oem_box_size)*50/100)

static void __oem_box_show(u_short x, u_short y)
{
	int i;

	memset(lcd_base + (y*_sc_w + x)*_v_bp, 0xff, _oem_box_size*_v_bp);
	memset(lcd_base + ((y + _oem_box_size)*_sc_w + x)*_v_bp, 0xff, _oem_box_size*_v_bp);
//	memset(lcd_base + ((y + 2)*_sc_w + x)*_v_bp, 0xff, _oem_box_size*_v_bp);
//	memset(lcd_base + ((y + (_oem_box_size+2))*_sc_w + x)*_v_bp, 0xff, _oem_box_size*_v_bp);

	for(i = 0; i < _oem_box_size; i++)
	{
		*(u_short *)(lcd_base + ((y + i)*_sc_w + x)*_v_bp) = 0xffff;
//		*(u_short *)(lcd_base + ((y + i)*_sc_w + x + 2)*_v_bp) = 0xffff;
		*(u_short *)(lcd_base + ((y + i)*_sc_w + x + _oem_box_size)*_v_bp) = 0xffff;
//		*(u_short *)(lcd_base + ((y + i)*_sc_w + x + (_oem_box_size+2))*_v_bp) = 0xffff;
	}
}

static void __oem_box_clear(u_short x, u_short y)
{
	int i;
	for(i = 0; i < (_oem_box_size - 16); i++)
	  memset(lcd_base + ((y + i + 8)*_sc_w + x + 8)*_v_bp, 0, (_oem_box_size - 16)*_v_bp);

//	__oem_box_show(x, y);
}


static void __oem_box_set_pic(u_short bx, u_short by,
   short x, short y, struct oem_picture *picture)
{
	u_short i, w, h, xpos, l;
	u_char *src = picture->pic;
	x += picture->x;
	y += picture->y;
	w = picture->width;
	h = picture->height;

	if((x + w) > _sc_w || (y + h) > _sc_h)
		return ;

	if( x + w > bx + 10 && x < bx + _oem_box_size - 10)
	{
		xpos = ((x > bx + 10)? x : (bx + 10));
		if(x < bx + 10)
		  l = x + w - bx - 10;
		else if(x > bx + _oem_box_size - 10 - w)
		  l = bx + _oem_box_size - 10 - x;
		else
		  l = w;
	}
	else
	  return ;

	for (i = 0; i < h; i++)
	{
		if(y + i < by + 10 || y + i > by + _oem_box_size - 10)
		  continue;

		if(src)
		  memcpy(lcd_base + _v_bp*(_sc_w*(y+i)+xpos), src + _v_bp*(i*w + xpos - x), _v_bp*l);
	}
	return ;
}

static void __oem_box_switch(u_char dir)
{
	/* up 0, down 1, left 2, right 3 */
	u_short i;
	struct oem_picture *cp, *np;
	u_short kcount = ARRAY_SIZE(oem_kernel_list);

	cp = oem_kernel_list[cur_kd].logo;
	if(dir % 2 == 0)
	{
		cur_kd = (cur_kd + kcount - 1) % kcount;
		np = oem_kernel_list[cur_kd].logo;
	}
	else
	{
		cur_kd = (cur_kd + 1) % kcount;
		np = oem_kernel_list[cur_kd].logo;
	}



//	__oem_box_show(_oem_box_x, _oem_box_y);

	for(i = 0; i < 121; i+=8)
	{
		__oem_box_clear(_oem_box_x, _oem_box_y);
		switch(dir)
		{
			case 0:
				__oem_box_set_pic(_oem_box_x, _oem_box_y,
				   _oem_box_x + 40, _oem_box_y + 40 - i, cp);
				__oem_box_set_pic(_oem_box_x, _oem_box_y,
				   _oem_box_x + 40, _oem_box_y + 160 - i, np);
				break;
			case 1:
				__oem_box_set_pic(_oem_box_x, _oem_box_y,
				   _oem_box_x + 40, _oem_box_y + 40 + i, cp);
				__oem_box_set_pic(_oem_box_x, _oem_box_y,
				   _oem_box_x + 40, _oem_box_y - 80 + i, np);
				break;
			case 2:
				__oem_box_set_pic(_oem_box_x, _oem_box_y,
				   _oem_box_x + 40 - i, _oem_box_y + 40, cp);
				__oem_box_set_pic(_oem_box_x, _oem_box_y,
				   _oem_box_x + 160 - i, _oem_box_y + 40, np);
				break;
			case 3:
				__oem_box_set_pic(_oem_box_x, _oem_box_y,
				   _oem_box_x + 40 + i, _oem_box_y + 40, cp);
				__oem_box_set_pic(_oem_box_x, _oem_box_y,
				   _oem_box_x - 80 + i, _oem_box_y + 40, np);
				break;
			default:
				;
		}
		udelay(5000);
	}
}

void oem_choose_kernel(void)
{
	u_char opt;

	oem_hide_window(1);
	oem_hide_logo();
	__oem_box_show(_oem_box_x, _oem_box_y);
	__oem_box_switch(0);
	oem_font_puts(358, 155, "Please select an operation system:");

	oem_show_window(1);
	printf("in %s\n", __func__);

	while(1)
	{
		printf("before getc\n");
		opt = getc();
		printf("we got %d\n", opt);
		switch(opt)
		{
			case 'j':
			case 40:
				__oem_box_switch(1);
				break;
			case 'k':
			case 38:
				__oem_box_switch(0);
				break;
			case 'l':
			case 39:
				__oem_box_switch(3);
				break;
			case 'h':
			case 37:
				__oem_box_switch(2);
				break;
			case 13:
			case 10:
				oem_kernel_list[cur_kd].bootfunc();
				while(1);
			case 27:
				break;
			default:
				;
		}
	}
}



