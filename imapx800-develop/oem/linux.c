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
#include <oem_inand.h>
#ifndef CONFIG_SYS_DISK_iNAND
void oem_basic_up_uimage(void)
{
	printf("Function for updating uImage from SD is currently removed!\n");
}

int oem_load_LK(void)
{
	return oem_load_img(CONFIG_SYS_PHY_LK_BASE, CONFIG_SYS_NAND_LK1_OFFS,
	   CONFIG_SYS_NAND_LK2_OFFS, 0x600000);
}

int oem_load_RD(void)
{
	return oem_load_img(CONFIG_SYS_PHY_RD_BASE, CONFIG_SYS_NAND_RD1_OFFS,
	   CONFIG_SYS_NAND_RD2_OFFS, 0x200000);
}

int oem_load_updater(void)
{
	return oem_load_RD_();
}
int oem_load_RD_(void)
{
	return oem_load_img(CONFIG_SYS_PHY_RD_BASE, CONFIG_SYS_NAND_RD__OFFS,
	   0, 0);
}

int oem_maintain_image(void)
{
	return 0;
}
#else
void oem_basic_up_uimage(void)
{
	printf("Function for updating uImage from SD is currently removed!\n");
}

int oem_load_LK(void)
{
	return oem_load_img(CONFIG_SYS_PHY_LK_BASE,UIMAGE);
}

int oem_load_RD(void)
{
	return oem_load_img(CONFIG_SYS_PHY_RD_BASE,  RAMDISK);
}

int oem_load_updater(void)
{
	return oem_load_img(CONFIG_SYS_PHY_RD_BASE,UPDATER);		
}
int oem_load_RD_(void)
{
	return oem_load_img(CONFIG_SYS_PHY_RD_BASE, RAMDRE);
}
int do_maintain_image(char *argv)
{
       unsigned int addr=0,addrback=0, length=0; 
       unsigned int  loadaddr=0;
       char filename[32];
 

        if(!strcmp(argv,"uImage")||!strcmp(argv,"uimage")){
               loadaddr=CONFIG_SYS_PHY_LK_BASE;
               length=iNAND_LEN_UIMG;
               addr=iNAND_START_ADDR_UIMG;
               sprintf(filename,"%s","uImage");
        }else if(!strncmp(argv,"ramdisk",7)){
               loadaddr=CONFIG_SYS_PHY_RD_BASE;
               length=iNAND_LEN_RD;
               addr=iNAND_START_ADDR_RD;
               sprintf(filename,"%s","ramdisk.img");
        }else if(!strncmp(argv,"recovery",8)){
               loadaddr=CONFIG_SYS_PHY_RD_BASE;
               length=iNAND_LEN_RE;
               addr=iNAND_START_ADDR_RE;
               sprintf(filename,"%s","recovery_rd.img");
        }else if(!strcmp(argv,"updater")){
                loadaddr=CONFIG_SYS_PHY_LK_BASE;
                length=iNAND_LEN_UPDATER;
                addr=iNAND_START_ADDR_UPDATER;
                sprintf(filename,"%s","updater");
        }else{
                printf("Error: Invalid image name:%s\n",argv);
      	}

 	if( oem_read_inand((char *)loadaddr, addr,length)){
              	printf("Error: failed to read %s\n",filename);
                return 1;
        }

        if(oem_check_img(loadaddr)){
            return 0;
        }
	
        printf("Image is corrupt. Try to replace it with backup one.\n");
 	addrback=addr+iNAND_BACK_IMAGE_OFFSET;
  	if( oem_read_inand((char *)loadaddr, addrback , length)){
              printf("Error: failed to read backup %s\n",filename);
              return 1;
       	}

       	if(!oem_check_img(loadaddr)){
       		printf("backup image %s is corrupt too.\n", filename);
       		return 1;
       	}


  	if(oem_write_inand((char *) loadaddr, addr , length)){
		printf("Error: failed to replace %s\n",filename);
		return 1;
	}
 
  	 if(oem_read_inand((char *) loadaddr, addr , length)){
	        printf("Error: failed to replace %s\n",filename);
                return 1;
	 }


   	if(!oem_check_img(loadaddr)){
          	printf("Replaced image %s, but it seems no use.\n", filename);
               	return 1;
     	}

   return 0;
}
int oem_maintain_image(void)
{
	char cmd[64];

	sprintf(cmd,"mmc rescan %d",iNAND_CHANNEL);
	run_command(cmd,0);

	if(do_maintain_image("uImage"))
		return 1;		
 
      	if(do_maintain_image("ramdisk.img"))
		return 1;

       	if(do_maintain_image("updater"))
		return 1;

//	if(do_maintain_image("recovery_rd.img"))
//		 return 1;

       return 0;

}

#endif
void oem_bootl(char *args)
{
	if(args)
	  setenv("bootargs", args);

	run_command("bootm 40007fc0", 0);

	/* If bootm not successful, system hangs here */
	printf("Bootm failed, system halted...\n");
	for(;;);
}

