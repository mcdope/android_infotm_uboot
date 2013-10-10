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
#include <version.h>
#include CONFIG_GZIP_LOGO_FILE
#ifdef CONFIG_UPDATER_START_SHOW
#include <updater_starting_show.h>
#endif
#ifdef CONFIG_GZIP_BATT_FILE
#include CONFIG_GZIP_BATT_FILE

void oem_init_gz_batt(void)
{
	unsigned long l;

	l = sizeof(gzipped_batt);
	if(gunzip((uint8_t *)CONFIG_RESV_LOGO, 0x200000, (uint8_t *)gzipped_batt, &l))
	  printf("Get gzipd logo failed.\n");

	if(getenv("revert"))
	  oem_revert_pic((uint8_t *)CONFIG_RESV_LOGO, CONFIG_BATT_GZIP_W, CONFIG_BATT_GZIP_H);

	return ;
}

void oem_show_batt(void)
{
	oem_init_gz_batt();
	oem_fb_switch(1);
	oem_clear_screen(0);
	oem_set_pic_raw((_sc_w - CONFIG_BATT_GZIP_W) / 2,
	   (_sc_h - CONFIG_BATT_GZIP_H) / 2, CONFIG_BATT_GZIP_W, CONFIG_BATT_GZIP_H,
	   (uint8_t *)CONFIG_RESV_LOGO);

	lcd_win_alpha_set(1, 15);
	lcd_win_onoff(1, 1);
}
#endif

void oem_init_gz_logo(void)
{
	unsigned long l;

	l = sizeof(gzipped_logo);
	if(gunzip((uint8_t *)CONFIG_RESV_LOGO, 0x200000, (uint8_t *)gzipped_logo, &l))
	  printf("Get gzipd logo failed.\n");

	if(getenv("revert"))
	  oem_revert_pic((uint8_t *)CONFIG_RESV_LOGO, CONFIG_LOGO_GZIP_W, CONFIG_LOGO_GZIP_H);

	return ;
}

/* Logo for elonex ebook */
void oem_show_logo(void)
{
#ifdef CONFIG_DISPLAY_VERSION
	char vers[32];
#endif

	oem_init_gz_logo();
	oem_fb_switch(1);
	oem_clear_screen(0);
	oem_set_pic_raw((_sc_w - CONFIG_LOGO_GZIP_W) / 2,
	   (_sc_h - CONFIG_LOGO_GZIP_H) / 2, CONFIG_LOGO_GZIP_W, CONFIG_LOGO_GZIP_H,
#ifdef CONFIG_LOGO_GZIP_OFFS
	   (uint8_t *)(CONFIG_RESV_LOGO + CONFIG_LOGO_GZIP_OFFS));
#else
	   (uint8_t *)CONFIG_RESV_LOGO);
#endif

#ifdef CONFIG_DISPLAY_VERSION
	sprintf(vers, "%d.%d.%d.%d(%s)",
	   CONFIG_BOARD_OEM, (CONFIG_BOARD_HWVER >> 16),
	   (CONFIG_BOARD_HWVER >> 8) & 0xff,
	   (CONFIG_BOARD_HWVER & 0xff),
	   strstr(U_BOOT_VERSION, "svn") + 3);
	oem_font_puts(_sc_w - 155, _sc_h - 35, vers);
#endif
	lcd_win_alpha_set(1, 15);
	lcd_win_onoff(1, 1);
}

void oem_hide_logo(void)
{
	oem_fb_switch(1);
	oem_clear_screen(0);
}

void oem_show_starting(void)
{
	//unsigned long l=0;

#ifdef CONFIG_UPDATER_START_SHOW
	l = sizeof(starting_updater);
	   
	if(gunzip((uint8_t *)CONFIG_RESV_LOGO, 0x200000, (uint8_t *)starting_updater, &l))
	 //      if(gunzip((uint8_t *)lcd_base, 0x200000, (uint8_t *)gzipped_logo, &l))
		printf("Get gzipd logo failed.\n");
	oem_clear_screen(0);
  
	int xstart=(_sc_w - 800 )? (_sc_w - 800 )>>1:0;
	int ystart=(_sc_h - 300 )? (_sc_h - 300 )>>1:0;
	int dislength=(_sc_w - 800 )? 800:_sc_w;
	int dishigth=(_sc_h - 300 )? 300:_sc_h;
	oem_set_pic_raw(xstart,ystart, dislength, dishigth ,
                       (uint8_t *)CONFIG_RESV_LOGO);
	memset(CONFIG_RESV_LOGO , 0 , 0x200000 );
#endif 
       return ;
}
