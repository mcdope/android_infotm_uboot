/***************************************************************************** 
** common/oem_font.c
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
** 1.1  XXX 03/06/2010 XXX	Warits
*****************************************************************************/

#include <common.h>
#include <oem_func.h>
#include <lcd.h>
#include <ft_verdana_13.h>

static u_short g_oem_x, g_oem_y;

static void oem_font_draw(u_char a)
{
	if(a < 0x20 || a > 0x7e) return;

	struct ft_font *fd = ft_font_data + a -0x20;
	u_short  x_max = g_oem_x + fd->w;
	u_short  y_max = g_oem_y + FT_FONT_SIZE + fd->offset;
	u_short gray, color, p, q, i, j;

	if ((x_max > _sc_w) || (y_max > _sc_h)) 
	{
		printf("CHAR: %d, Jumped out, x %d, y %d, xmax %d, ymax %d, fbx %d, fby %d\n",
		   a,
		   g_oem_x, g_oem_y , x_max, y_max, _sc_w, _sc_h);
		return ;
	}

	for ( i = g_oem_x, p = 0; i < x_max; i++, p++ )
	{
		for ( j = g_oem_y + FT_FONT_SIZE - fd->h + fd->offset, q = 0; j < y_max; j++, q++ )
		{
			gray = (fd->bitmap[q * fd->w + p]) & 0xff;
			color = 0;
			color |= (gray & 0xf8) << 8;
			color |= (gray & 0xfc) << 3;
			color |= gray >> 3;
			*(u_short *)(lcd_base + (j * _sc_w + i) * _v_bp) = color;
		}
	}

	g_oem_x += fd->adx;
}


int oem_font_puts(unsigned short x, unsigned short y, const char * s)
{
	/* the pen position in 26.6 cartesian space coordinates; */
	/* start at (300,200) relative to the upper left corner  */
	int num_chars = strlen(s);
	int n;

	g_oem_x = x;
	g_oem_y = y;

	for ( n = 0; n < num_chars; n++ )
	  oem_font_draw(*(s+n));

	return 0;
}

int oem_font_len(const char * s)
{
	int n, l = 0;
	int num_chars = strlen(s);
	struct ft_font *fd;

	for (n = 0; n < num_chars; n++)
	{
		fd = ft_font_data + *(s + n) -0x20;
		l += fd->adx;
	}
	return l;
}

