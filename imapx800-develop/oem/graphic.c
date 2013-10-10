/***************************************************************************** 
** common/oem_graphic.c
** 
** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
** 
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** Description: OEM graphic lib.
**
** Author:
**     Warits   <warits.wang@infotm.com>
**      
** Revision History: 
** ----------------- 
** 1.1  XXX 03/17/2010 XXX	Warits
*****************************************************************************/

#include <common.h>
#include <oem_font.h>
#include <oem_graphic.h>
#include <oem_pic.h>

void oem_fb_switch(u_short win)
{
	if(win == 0)
		lcd_base = (void *)CONFIG_RESV_LCD_BASE;
	else
		lcd_base = (void *)CONFIG_RESV_LCD_BASE_1;
}
void oem_show_window(u_short win)
{
	u_short i;
	lcd_win_alpha_set(win, 0);
	lcd_win_onoff(win, 1);
	for(i = 0; i < 16; i++)
	{
		lcd_win_alpha_set(win, i);
		udelay(40000);
	}
}

void oem_hide_window(u_short win)
{
	u_short i;
	for(i = 15;;i--)
	{
		lcd_win_alpha_set(win, i);
		if(!i) break;
		udelay(40000);
	}
	
//	lcd_win_onoff(win, 0);
}

void oem_clear_screen(u_short c)
{
	int i;
	u_long fill_patten;
	
	fill_patten = (c << 16) | c;
	for(i = 0; i < (_sc_w * _sc_h * _v_bp) >> 2; i++)
	  *(u_long *)(lcd_base + i * 4) = fill_patten;
}

void oem_set_fg(u_short x, u_short y, u_short w, u_short h, u_short c)
{
	u_short i, j;
	u_short *p;

	if((x + w) > _sc_w || (y + h) > _sc_h) return ;
	for (i = 0; i < h; i++)
	{
	  p =(u_short *)(lcd_base + _v_bp*(_sc_w*(y+i)+x));
	  for (j = 0; j < w; j++)
	  {
		  *p = c | *p;
		  p++;
	  }
	}
	return ;
}

void oem_set_color(u_short x, u_short y, u_short w, u_short h, u_short c)
{
	u_short i, j;
	u_short *p;

	if((x + w) > _sc_w || (y + h) > _sc_h) return ;
	for (i = 0; i < h; i++)
	{
	  p =(u_short *)(lcd_base + _v_bp*(_sc_w*(y+i)+x));
	  for (j = 0; j < w; j++)
	  {
		  *p = c;
		  p++;
	  }
	}
	return ;
}

void oem_set_pic_raw(u_short x, u_short y, u_short w, u_short h, const u_char *src)
{
	u_short i;

	if((x + w) > _sc_w || (y + h) > _sc_h)
		return ;
	for (i = 0; i < h; i++)
	{
		if(src)
		{
			memcpy(lcd_base + _v_bp*(_sc_w*(y+i)+x), src + _v_bp*i*w, _v_bp*w);
		}
		else
		{
			memset(lcd_base + _v_bp*(_sc_w*(y+i)+x), 0xff, _v_bp*w);
		}
	}
	return ;
}

void oem_set_pic_raw_offset(u_short x, u_short y, u_short w, u_short h,
   u_short ox, u_short oy, u_short ow, u_short oh, const u_char *src)
{
	u_short i;

	if((x + ow) > _sc_w || (y + oh) > _sc_h)
		return ;
	for (i = 0; i < oh; i++)
	{
		if(src)
		{
			memcpy(lcd_base + _v_bp*(_sc_w*(y+i)+x), src + _v_bp*((i + oy)*w + ox), _v_bp*ow);
		}
		else
		{
			memset(lcd_base + _v_bp*(_sc_w*(y+i)+x), 0xff, _v_bp*w);
		}
	}
	return ;
}

void oem_set_pic(short x, short y, struct oem_picture *picture)
{
	oem_set_pic_raw(picture->x + x, picture->y + y, picture->width,
	picture->height, picture->pic);
//	printf("Showing picture: %s\r\n", picture->desc);
}

int oem_draw_box(u_short x1, u_short y1, u_short x2, u_short y2)
{
	int i;

	memset(lcd_base + (y1*_sc_w + x1)*_v_bp, 0xff, (x2 - x1)*_v_bp);
	memset(lcd_base + (y2*_sc_w + x1)*_v_bp, 0xff, (x2 - x1)*_v_bp);
	for(i = 0; i < (y2 - y1); i++)
	{
		*(u_short *)(lcd_base + ((y1 + i)*_sc_w + x1)*_v_bp) = 0xffff;
		*(u_short *)(lcd_base + ((y1 + i)*_sc_w + x2)*_v_bp) = 0xffff;
	}

	return 0;
}   

int oem_draw_box_color(u_short x1, u_short y1, u_short x2, u_short y2, u_short color)
{
	int i, j;

	for(i = x1; i <= x2; i++)
	  for(j = y1; j<= y2; j++)
		*(u_short *)(lcd_base + (j * _sc_w + i)*_v_bp) = color;

	return 0;
}

void oem_revert_pic(uint8_t *buf, short w, short h)
{
	uint16_t dot, i, j;

	for(j = 0; j < h; j++)
	{
		for(i = 0; i < w / 2; i++)
		{
			dot = *(uint16_t *)(buf + i*2 + j*w*2);
			*(uint16_t *)(buf + i*2 + j*w*2) = *(uint16_t *)(buf + (w-i-1)*2 + (h-j-1)*w*2);
			*(uint16_t *)(buf + (w-i-1)*2 + (h-j-1)*w*2) = dot;
		}
	}
}

void oem_lcd_msg(char *s)
{
	oem_clear_screen(0);
	oem_font_puts(10, 10, s);
	printf("LCD MSG: %s\n", s);
	return ;
}

void oem_mid(char * s)
{
	int l = oem_font_len(s);
	oem_set_color((_sc_w - 600)/2, ((_sc_h>480)?204:126), 600, FBFONTH, 0);
	oem_font_puts((_sc_w - l)/2, ((_sc_h>480)?204:126), s);
}

void oem_mid2(char * s)
{
	int l = oem_font_len(s);
	oem_set_color((_sc_w - 600)/2, ((_sc_h>480)?237:159), 600, FBFONTH, 0);
	oem_font_puts((_sc_w - l)/2, ((_sc_h>480)?237:159), s);
}

void oem_below(char * s)
{
	int l = oem_font_len(s);
	oem_set_color((_sc_w - 512)/2, ((_sc_h>480)?445:420), 512, FBFONTH, 0);
	oem_font_puts((_sc_w - l)/2, ((_sc_h>480)?445:420), s);
}

void oem_below2(char * s)
{
	int l = oem_font_len(s);
	oem_set_color((_sc_w - 512)/2, ((_sc_h>480)?350:330), 512, FBFONTH, 0);
	oem_font_puts((_sc_w - l)/2, ((_sc_h>480)?350:330), s);
}

#if 0
void oem_p_ef(char * s)
{
	int l = oem_font_len(s) - 20;
	oem_mid("");
	oem_mid2("");
	oem_below("");
	oem_set_color((_sc_w - 512)/2, ((_sc_h>480)?204:159), 512, FBFONTH, 0);
	oem_font_puts((_sc_w - l)/2, ((_sc_h>480)?204:159), s);
	oem_set_pic((_sc_w - l)/2 - 30, ((_sc_h>480)?204:159), &oem_pic_cross);

	oem_below("Press ESC to return.");
}

void oem_p_done(void)
{
	oem_mid("");
	oem_mid2("");
	oem_below("");
	oem_mid("OS updates finished!");
	oem_mid2("Enjoy your newly installed OS :)");
	oem_below("Press ENTER to restart.");
}

void oem_p_bg(void)
{
	int l = oem_font_len("Windows CE update") + 6;

	oem_clear_screen(0);
	oem_set_pic((_sc_w - oem_pic_smwd.width)/2 - 70,
	   (_sc_h - oem_pic_smwd.height)*88/100, &oem_pic_smwd);
	oem_font_puts((_sc_w - oem_pic_smwd.width)/2 - 30,
	   (_sc_h - oem_pic_smwd.height)*88/100 + 5, "Windows CE");
	oem_set_color((_sc_w - 512)/2, ((_sc_h>480)?169:159), 512, FBFONTH, 0);
	oem_font_puts((_sc_w - l)/2, ((_sc_h>480)?169:159), "Windows CE update");

}

void oem_p_pro(void)
{
	oem_mid("");
	oem_mid2("");
	oem_below("");
	oem_set_pic((_sc_w - oem_pic_bar.width)/2,
	   (_sc_h - oem_pic_bar.height)*41/100, &oem_pic_bar);
}

int oem_p_spark(int a)
{
	if (a > 1000) return 1;

	int x = a * (oem_pic_bar.width - 3) / 1000;
	u_short bw = oem_pic_bar.width - 3;
	u_short bw2 = oem_pic_spark2.width / 2;

	if(x < oem_pic_spark2.width / 2)
	{
		oem_set_pic_raw_offset(((_sc_w - bw)/2) - 1, ((_sc_h - oem_pic_bar.height)*41/100),
		   oem_pic_spark2.width, oem_pic_spark2.height, (oem_pic_spark2.width - x - bw2), 0,
		   (x + bw2), oem_pic_spark2.height, oem_pic_spark2.pic);
	}
	else if(x > bw - bw2)
	{
		oem_set_pic_raw_offset(((_sc_w - bw)/2) + x + bw2 - oem_pic_spark2.width,
		   ((_sc_h - oem_pic_bar.height)*41/100), oem_pic_spark2.width, oem_pic_spark2.height,
		   0, 0, bw + oem_pic_spark2.width - x - bw2, oem_pic_spark2.height, oem_pic_spark2.pic);
	}
	else
	{
		oem_set_pic(((_sc_w - bw)/2) + x + bw2 - oem_pic_spark2.width,
		   ((_sc_h - oem_pic_bar.height)*41/100), &oem_pic_spark2);
	}
	return 0;
}
#endif
