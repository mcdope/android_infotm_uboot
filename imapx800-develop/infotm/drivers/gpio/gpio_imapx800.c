/***************************************************************************** 
** infotm/drivers/gpio/gpio_imapx800.c 
** 
** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description: File used for the standard function interface of output.
**
** Author:
**     Lei Zhang   <jack.zhang@infotmic.com.cn>
**      
** Revision History: 
** ----------------- 
** 1.0  11/26/2012  Jcak Zhang   only can be use in CPU x15
*****************************************************************************/ 

#include <common.h>
#include <asm/io.h>
#include <lowlevel_api.h>
#include <efuse.h>
#include <preloader.h>
#include <items.h>
#include "imapx_gpio.h"

void gpio_reset(void)
{
	unsigned int val;

	writel(0xff, GPIO_SYSM_ADDR);
	writel(0xff, GPIO_SYSM_ADDR+0x4);
	writel(0x11, GPIO_SYSM_ADDR+0x8);

	while(1)
	{
		val = readl(GPIO_SYSM_ADDR + 0x08);
		if(val & 0x2)
			break;
	}

	val = readl(GPIO_SYSM_ADDR + 0x08);
	val &= ~0x10;
	writel(val, GPIO_SYSM_ADDR + 0x08);
	writel(0xff, GPIO_SYSM_ADDR + 0x18);
	writel(0, GPIO_SYSM_ADDR);
}

/* 0-intput,  1-output*/
void gpio_dir_set(int dir, int pin)
{
	if(dir)
	{
		writel(1, GPIO_BASE_ADDR+0x40*pin+8);/*dir output*/
	}
	else
	{
		writel(0, GPIO_BASE_ADDR+0x40*pin+8);/*dir input*/
	}
}

void gpio_mode_set(int mode, int pin)
{
	int index, num, val;
	index = pin / 8;
	num = pin % 8;
	
	if(mode == 0)
	{
		val = readl(PAD_SYSM_ADDR+0x64+4*index);
		val &= ~(1<<num);
		writel(val, PAD_SYSM_ADDR+0x64+4*index);/*func mode*/

	}
	else
	{
		val = readl(PAD_SYSM_ADDR+0x64+4*index);
		val |= 1<<num;
		writel(val, PAD_SYSM_ADDR+0x64+4*index);/*gpio mode*/
	}
}

void gpio_output_set(int val, int pin)
{
	if(val)
	{
		writel(1, GPIO_BASE_ADDR+0x40*pin+4);/*output 1*/
	}
	else
		writel(0, GPIO_BASE_ADDR+0x40*pin+4);/*output 0*/
}

void gpio_pull_en(int en, int pin)
{
	int index, num, val;

	index = pin/8;
	num = pin%8;

	if(en == 0)
	{
		val = readl(PAD_SYSM_ADDR + 0x14 + 4*index);
		val &= ~(1 << num);
		writel(val, PAD_SYSM_ADDR+0x14+4*index);
	}
	else
	{
		val = readl(PAD_SYSM_ADDR + 0x14 + 4*index);
		val |= 1 << num;
		writel(val, PAD_SYSM_ADDR+0x14+4*index);
	}
}
