/***************************************************************************** 
** XXX nand_spl/board/infotm/imapx/boot_main.c XXX
** 
** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
** 
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** Description: The first C function in PRELOADER.
**
** Author:
**     Warits   <warits.wang@infotm.com>
**      
** Revision History: 
** ----------------- 
** 1.1  XXX 03/17/2010 XXX	Warits
*****************************************************************************/

#include <common.h>
#include <asm/io.h>
#include <bootlist.h>
#include <dramc_init.h>
#include <items.h>
#include <nand.h>
#include <vstorage.h>
#include <preloader.h>

int init_config(void)
{
	int id, ret, len = 0;
	void * cfg = (void *)ITEMS_LOWBASE;

	printf("cfg: 0x%p\n", cfg);
	id = boot_device();
	switch(id) {
		case DEV_EEPROM:
		case DEV_FLASH:
			cfg = (void *)(IRAM_BASE_PA + BL_SIZE_FIXED
					- ITEM_SIZE_EMBEDDED);
			len = ITEM_SIZE_EMBEDDED;
			break ;
		case DEV_BND:
			id = DEV_NAND;
		default:
			printf("read items: id=%d\n", id);
			ret = vs_assign_by_id(id, 1);
			if (ret)
			  return ret;
			ret = vs_read(cfg, BL_LOC_CONFIG, ITEM_SIZE_NORMAL, 0);
			if (ret < 0)
			  return ret;
			len = ITEM_SIZE_NORMAL;
	}

	printf("begin init.\n");
	return item_init(cfg, len);
}

