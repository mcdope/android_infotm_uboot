/***************************************************************************** 
** common/wrap_up.c
** 
** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
** 
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** Description: Update system through a wrap form.
**
** Author:
**     Warits   <warits.wang@infotm.com>
**      
** Revision History: 
** ----------------- 
** 1.1  XXX 09/16/2010 XXX	Warits
*****************************************************************************/

#include <common.h>
#include <oem_func.h>
#include <oem_font.h>
#include <oem_graphic.h>
#include <oem_pic.h>
#include <asm/io.h>
#include <mmc.h>


void oem_hibernate_resume(void)
{
	__attribute__((noreturn)) void (*p)(void);

	/* goto origianl PC */
	p = readl(INFO0);
	 printf("Resum set pc:0x%x\n",p);
	(*p)();
}
int oem_hibermark(int mark)
{
	uint8_t * tmp_buf = (uint8_t *)(CONFIG_RESV_PTBUFFER);
	int ret = 0;
	if(mark)
	{
		if(oem_read_inand(tmp_buf,0,1)) return 1;
		
		*(uint32_t *)(tmp_buf+ CONFIG_HIBERMARK_OFFSET)= 0x9527;
		*(uint32_t *)(tmp_buf + CONFIG_HIBERMARK_OFFSET + 4) = readl(INFO0);
		
		if(oem_write_inand(tmp_buf,0,1)) return 1;
	}

	return ret;
}

int oem_get_hibermark(void)
{
	uint8_t * tmp_buf = (uint8_t *)(CONFIG_RESV_PTBUFFER);
	int ret = 0;
	char cmd[256];
	
	ret=oem_read_inand(tmp_buf,0,1);

	if(ret)
	  return -1;

	if(*(uint32_t *)(tmp_buf+ CONFIG_HIBERMARK_OFFSET)== 0x9527)
	{
		printf("Hibernate mark detected!\n");
		writel(*(uint32_t *)(tmp_buf + CONFIG_HIBERMARK_OFFSET + 4), INFO0);
		*(uint32_t *)(tmp_buf + CONFIG_HIBERMARK_OFFSET)=0;
		*(uint32_t *)(tmp_buf + CONFIG_HIBERMARK_OFFSET + 4)=0;
		ret=oem_write_inand(tmp_buf,0,1);
		if(ret) {
			printf("Failed to clear hibernate flag\n");
			return 0;
		}
		return 1;
	}

	return 0;
}

void oem_hiber(void)
{
	/* hiber, power off */
	writel(0x4 , SYSMGR_BASE_REG_PA + 0x208);
	writel(0xff, SYSMGR_BASE_REG_PA + 0x204);
	writel(0x3f, SYSMGR_BASE_REG_PA + 0x218);
	writel(0x0 , SYSMGR_BASE_REG_PA + 0x210);
	writel(0x4 , SYSMGR_BASE_REG_PA + 0x200);
}
int do_hibernate(void)
{
        struct mmc *mmc = find_mmc_device(iNAND_CHANNEL);
        if (!mmc)
             return 1;


	int length=(CONFIG_HIBERNATE_LOCATION<<11);
        int steplength=0x4000;
        int startblk=CONFIG_HIBERNATE_START;
        uint8_t *base=(uint8_t *)CONFIG_SYS_SDRAM_BASE;

        if(((length+startblk )> mmc->sector_count)&&mmc->sector_count){
        	printf("Error:write out of range:start block # %x, count %x  device capacity is: %x",
                	startblk, length,mmc->sector_count);
               	return 1;
        }
       	if(startblk==0){
		printf("there is no hibernate location defined.\n");
		return 1;
	}
	
	while(length>=steplength){
		if(readl(POW_STB) & 0x1)
	   		return -1;
		mmc->Wflag=1;
	        if( mmc_init(mmc)) return 1;
		if(steplength!=mmc->block_dev.block_write(iNAND_CHANNEL, startblk, steplength, base)){
		       	printf("Error: Failed to backup menmory data for hibernate.\n");
			return 1;
		}
		startblk+=steplength;
	   	base+=steplength<<9;
	        length-=steplength;
	}

	if(length>=steplength){
		 printf("Detected power key while doing memory copy!\n");
		 printf("Hibernate aborted. Using normal resume.\n");
		 return 1;
	}else{
		if(length){
			mmc->Wflag=1;
			if( mmc_init(mmc)) return 1;
		       	if(length!=mmc->block_dev.block_write(iNAND_CHANNEL, startblk, length, base)){
				printf("Error: Failed to backup menmory data for hibernate.\n");
			       	return 1;
			}
		}
	}
	return 0;
}
int hibernate_resume(void)
{
	int length=(CONFIG_HIBERNATE_LOCATION<<11);
	int steplength=0x4000;
	int startblk=CONFIG_HIBERNATE_START;
	uint8_t *base=(uint8_t *)CONFIG_SYS_SDRAM_BASE;
	char cmd[32];
	sprintf(cmd,"mmc rescan %d",iNAND_CHANNEL);
	run_command(cmd,0);
	printf("Hibernating mark detected, try to wake up.\n");
	//oem_hibermark(0);
	oem_simpro_init("Wake up from deep sleep ...",length);	
	while(length>=steplength){
		if(oem_read_inand(base, startblk , steplength)){
			printf("Failed to resume from last sleep.\n");
			oem_simpro_finish("Failed to resume from last sleep.");
			return 1;
		}
		oem_simpro_update(steplength);
		length-=steplength;
		base+=steplength<<9;
		startblk+=steplength;
	}
	if(length) oem_read_inand(base, startblk , length);
//	oem_simpro_finish("Memory copied, try to resume");
	printf("Memory copied, try to resume.\n");
	oem_hibernate_resume();
	return 0;
}
int hibernate(void)
{
	printf("Detected wake up by RTC, try to hibernate.\n");
	char cmd[32];
	sprintf(cmd,"mmc rescan %d",iNAND_CHANNEL);
	run_command(cmd,0);
	/* Clear RTC status */
	writel(readl(WP_ST) | 0x2, WP_ST);

//		oem_mem_analysis();
	/* wake up by RTC, hibernate */
	printf("Program memory data to iNAND\n");
	if(do_hibernate()){
		oem_hibernate_resume();
	}

	oem_hibermark(1);
	printf("Dump OK, power off.\n");
	oem_hiber(); /* power off */
	return 0;
}
void oem_check_reset(void)
{
	uint32_t rstst;

	rstst = readl(RST_ST);

	if(rstst & 0x8)
	  return;

	if(rstst & 0x10)
	{
		/* This is a wakeup */
		if(readl(WP_ST) & 0x2)
		  return;

		printf("Default wakeup ...\n");
		writel(0xff, RST_ST);
		oem_hibernate_resume();
	}
}

int oem_hibernate_manage(void)
{
	
	printf("Checking hibernate ...\n");
	oem_check_reset();
	if(readl(WP_ST) & 0x2){
		return hibernate();
	}else if(oem_get_hibermark()>0){
		return hibernate_resume();
	}else return 0;
}

