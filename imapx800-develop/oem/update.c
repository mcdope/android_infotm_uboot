/***************************************************************************** 
** XXX common/oem_update.c XXX
** 
** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
** 
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** Description: WinCE updating tools.
**
** Author:
**     Warits   <warits.wang@infotm.com>
**      
** Revision History: 
** ----------------- 
** 1.1  XXX 03/18/2010 XXX	Warits
*****************************************************************************/

#include <common.h>
#include <oem_func.h>
#include <oem_font.h>
#include <oem_graphic.h>
#include <oem_pic.h>

#include <asm/io.h>

int oem_update_prog_base, oem_update_prog_top,
	oem_update_prog_count, oem_update_prog_len;

struct oem_password {
	uint32_t magic1;
	uint32_t magic2;
	uint32_t magic3;
	uint32_t magic4;
	char	password[64];
};

const struct oem_password oem_update_password = {
	0x49445923,
	0x07816406,
	0x28620899,
	0x86280348,
	CONFIG_OEM_UPDATE_PASSWORD
};

#define str_off 250
void oem_update_get_password(void)
{
	int wordloc = oem_font_len("Please input OEM password: ") + str_off;
	int starw = oem_font_len("*");
	char c, password[32];
	int cursor = wordloc;
	int count = 0;
	int index = 0;

try_again:
	cursor = wordloc;
	count = 0;
	index = 0;
	oem_clear_screen(0);
	oem_font_puts(str_off, 270, "Please input OEM password: ");
	while(1)
	{
		/* delay 10ms */
		udelay(10000);

		/* Show Cursor */
		if((count + 50) % 100 == 0)
		  oem_set_color(cursor, 270, 30, 30, 0);
		else if(count % 100 == 0)
		  oem_font_puts(cursor, 270, "_");
		count++;

		if(tstc())
		{
			c = getc();
			printf("got key %d\n", c);
			switch(c)
			{
				case 8:
					{
						if(0 == index)
						  break;
						password[index] = '\0';
						index--;
						cursor -= starw;
						oem_set_color(cursor, 270, 30, 30, 0);
					}
					break;
				case 10:
				case 13:
					{
						printf("I got password %s\n", password);
						if(!strcmp(password, oem_update_password.password))
						  return ;
						else
						{
							oem_clear_screen(0);
							oem_p_ef("Wrong password!");
							oem_getc_exec(27);
							goto try_again;
						}
					}
					break;
				case 27:
					{
						reset_cpu(0);
					}
					break;
				default:
					{
						if(index > 7)
						  break;

						if(c < 0x20 || c > 0x7c)
						  break;

						password[index++] = c;
						password[index] = '\0';
						oem_set_color(cursor, 270, 30, 30, 0);
						oem_font_puts(cursor, 270, "*");
						cursor += starw;
					}
			}
		}
	}
}

int oem_update_check_dev(void)
{
	return oem_mmc_init();
}

void oem_update_prog_refresh(u_short n)
{
	int _spark; 

	uint32_t div;
	if((oem_update_prog_len >> 10))
	  div = (oem_update_prog_len >> 10);
	else
	  div = 1;

	_spark = oem_update_prog_count * 2000 / div;
//	printf("p %d\n", _spark);
	oem_p_spark(_spark);
	oem_update_prog_count++;
}

int oem_update_prog_init(int base, int top, int l)
{
	oem_update_prog_base = base;
	oem_update_prog_top =  top;
	oem_update_prog_len = l;
	oem_update_prog_count = 0;
	oem_progress_update = oem_update_prog_refresh;
	if(!top)
	  oem_progress_update = NULL;
	return 0;
}

void oem_update_wince(void)
{
	char cmd[256];
	/* Password banner */
	oem_update_get_password();

try_again:
	oem_clear_screen(0);
	oem_p_bg();
	oem_mid("Updating OS image is sometimes dangerous,");
	oem_mid2("do you want to continue?");

	oem_below("Press ENTER to continue, ESC exit.");

	uint8_t opt;
	while(1)
	{
		opt = oem_getc();
		printf("opt %d\n", opt);
		if(opt == 10 || opt == 13)
		{
			if(0 == oem_update_check_dev())
			  break;
			else
			{
				oem_below("No SD card is inserted!");
				udelay(1000000);
				oem_below("Press ENTER to continue, ESC exit.");
			}
		}
		else if(opt == 27)
		{
			reset_cpu(0);
		}
	}

	oem_mid("Loading image file ...");
	oem_mid2("");
	sprintf(cmd, "%x", CONFIG_SYS_PHY_NK_BASE);
	char *arg_load[] = {"fatload", "mmc", "0:1", cmd, CONFIG_PRODUCT_WINCE_IMAGE_NAME};
//	run_command("fatls mmc 0:1 /", 0);
	if(do_fat_fsload(NULL, 0, 5, arg_load))
	{
		oem_p_ef("No availiable image is found!");
		oem_getc_exec(27);
		goto try_again;
	}

	oem_mid("Checking image validity ...");
	if(!oem_check_img(CONFIG_SYS_PHY_NK_BASE))
	{
		oem_p_ef("The image file is not valid!");
		oem_getc_exec(27);
		goto try_again;
	}

	oem_p_pro();
	oem_mid("Writing image file to NAND ...");
	oem_update_prog_init(0, 1000,
	   image_get_size((image_header_t *)CONFIG_SYS_PHY_NK_BASE) - 0x100000);
	oem_burn_NK((uint8_t *)CONFIG_SYS_PHY_NK_BASE);

	oem_p_done();
	oem_getc_exec(13);
	reset_cpu(0);

	return ;
}

#if 0
void oem_update_clean_hive(void)
{
	int key_stat = 0, i, j;
	int key_pressed = 0;
	char msg[64];

	oem_clear_screen(0);
	oem_mid("RESTORE FACTORY SETTINGS");
	for(i = 0; i < 3; i++)
	{
		sprintf(msg, "Press MENUKEY twice in %ds ...", 3 - i);
		oem_below2(msg);

		for(j = 0; j < 1000; j++)
		{
			if(key_stat != CONFIG_WZF_MENUKEY_STATE)
			  if(!(key_stat = !key_stat))
			  {
				  if(++key_pressed > 2)
					goto __clean_hive__;
			  }

			udelay(1000);
		}
	}

	/* No key press, normal boot */
	oem_below(" ");
	*(uint32_t *)CONFIG_PRODUCT_WINCE_CLEANHV = 0;
	return ;

__clean_hive__:
	oem_below(" ");
	oem_mid("Factory settings will be restored.");
	udelay(1200000);
	oem_mid(" ");
	/* Give OS clean hive mark */
	*(uint32_t *)CONFIG_PRODUCT_WINCE_CLEANHV = CONFIG_CLEANHV_MAGIC;
	return ;
}
#endif
