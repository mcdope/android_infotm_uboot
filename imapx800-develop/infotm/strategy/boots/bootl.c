/***********************************************************************************
File Name	:	bootl.c
Description	:	This file will supply interfaces needed when booting a linux system.
Author		:	John Zeng
Data		:	Apr 26  2012
Version		:	init
************************************************************************************/


#include <common.h>
#include <linux/types.h>
#include <malloc.h>
#include <bootl.h>
#include <storage.h>
#include <vstorage.h>
#include <asm/io.h>
#include <bootlist.h>
#include <items.h>
int set_boot_type(int type)
{
	char env[16];
	sprintf(env,"%d",type);
	setenv("boottype",env);
	return type;
}
int got_boot_type(void)
{
	char* s=getenv("boottype");
	return s? simple_strtoul (s, NULL, 10) : 0;
}

uint8_t *load_image(char *name , char* alternate, char* rambase, int src_devid)
{
	int tryed = 0;
	char* imagename = name;
	uint8_t* dst = NULL; 
	int t =0;
	loff_t image =0;
retry:

	image = storage_offset(imagename);
	
	printf("fetch %s@0x%llx ...\n",imagename , image);
	t = get_timer(0);
	if(!strcmp("logo",imagename)){
		vs_read((uint8_t *)rambase, image, CONFIG_MAX_LOGO_SIZE , 0);
		dst = rambase;
		goto __out;
	}

	image_header_t *hdr = (image_header_t *)rambase;
	/* Load the first 4K */
	int ret = vs_read((uint8_t *)rambase, image,0x1000, 0);
	if(0x1000 > ret)
	{
		printf("read header failed 0x%x\n",ret);
		goto failed;
	}
	/* Check Magic */
	if(image_check_hcrc(hdr))
	{
		dst = (uint8_t *)rambase;
		/* Load the rest OS image */
		unsigned long int l = image_get_size(hdr);
		if(image_get_comp(hdr) == 1){
			dst += 0x6000000;
			cfg_mmu_table();
		}
		printf("%dms\n",(int)get_timer(t));
		int ret = vs_read( dst , image , l+sizeof(image_header_t), 0);
		if((l+sizeof(image_header_t)) > ret){

			printf("failed 0x%x 0x%x\n",ret , l+sizeof(image_header_t));
			goto failed;
		}
/*
		if(tryed&&strcmp(name,"ramdisk")) {
		//	char cmd[128];
		//	sprintf(cmd,"adr %s %x",name, (unsigned int)dst);
			if(!strcmp(name,"kernel0"))
			burn_single_image((uint8_t *) dst, 7);
			else burn_single_image((uint8_t *) dst, 8);
		//	run_command(cmd,0);
		}
		*/
		/*memcpy(rambase, dst ,sizeof(image_header_t));
		  int ret = gunzip((void*)(rambase + sizeof(image_header_t)), 
		  0x800000, 
		  ( unsigned char*)(0x86000000+sizeof(image_header_t)), 
		  &l);

		  mmu_disable();
		  if(ret!=0){
		  printf("Failed to decompress image");
		  return -1;
		  }*/
	}
	else{
		printf("Wrong Image type!\n");
		goto failed;
	}
__out:
	return dst;

failed:
	if(tryed) return NULL;
	tryed = 1;
	if(NULL == alternate) return NULL;
	imagename = alternate;
	goto retry;
}

int charger(void)
{
	return (readl(SYS_POWUP_ST)&0x2);
}

int calc_dram_M(void)
{
	int cs = 1, ch = 8, ca = 1, rd = 0;
	uint64_t sz;

	if((sz = readl(CONFIG_RESERVED_SKIP_BUFFER + 8)))
		goto __ok;

	if(item_exist("memory.size")) {
		sz = item_integer("memory.size", 0);
		printf("Size: %lld MB (memory.size)\n", sz);
		sz <<= 20;
		goto __ok;
	}

	if(item_exist("memory.cscount")
			&& item_exist("memory.density")
			&& item_exist("memory.io_width")
			&& item_exist("memory.reduce_en")
	  ) {
		cs = item_integer("memory.cscount", 0);
		ch = item_integer("memory.io_width", 0);
		rd = item_integer("memory.reduce_en", 0);
		ca = item_integer("memory.density", 0);

        cs = (cs == 3)?2: 1;
	}

	/* calculation */
	sz = (ca > 20)?
		(ca << 17): /* MB */
		(ca << 27); /* GB */
	sz *= (ch == 8)? 4: 2;
	sz >>= !!rd;
	sz *= cs;

	printf("Size: %lld MB (cs|%d bw|%d ca|%d rd|%d)\n", sz >> 20,
			cs, ch, ca, rd);

__ok:
	return (int)((sz >> 20) - 1);
}

int do_boot(void* kernel , void* ramdisk)
{
	
	char args[1024];
	memset(args,0,1024);
	int tmp, mm;

	mm = calc_dram_M();

	switch(got_boot_type())
	{
		case BOOT_NORMAL:
			sprintf(args,"%s mem=%dM androidboot.serialno=%s",
                                         CONFIG_LINUX_DEFAULT_BOOTARGS, mm, getenv("serialno"));
			break;
		case BOOT_FACTORY_INIT:
			sprintf(args,"%s mem=%dM androidboot.mode=recovery androidboot.serialno=%s",
                                        CONFIG_LINUX_DEFAULT_BOOTARGS, mm, getenv("serialno"));
			break;
		case BOOT_CHARGER:
			sprintf(args,"%s mem=%dM androidboot.mode=charger androidboot.serialno=%s",
                                        CONFIG_LINUX_DEFAULT_BOOTARGS, mm, getenv("serialno"));
			break;
	}

	setenv("bootargs", args);
	char cmd[128];
	memset(cmd,0,64);
	sprintf(cmd,"bootm %x %x",(unsigned int)kernel ,(unsigned int)ramdisk);
	run_command(cmd,0);

	/*
	 * hope never reach here.
	 *   
	 */
	printf("Failed to recover system since there is something wrong when bootting recovery, system halted..\n");
	for(;;);
	return 0; //Make compiler happy.
}

int boot_verify_type(void)
{
	int pwr_st;
	if (readl(RTC_INFO4) == 0x1) {
		set_boot_type(BOOT_FACTORY_INIT);
	} else if ((item_equal("charger.enable", "1", 0)
				&& charger()
				&& (got_boot_type() == BOOT_NORMAL))
			|| (readl(RTC_INFO4) == 0x2)) {
		set_boot_type(BOOT_CHARGER);
	}

	writel(0x0, RTC_INFO4);
	if (charger())
	{
		pwr_st = readl(SYS_POWUP_ST);
		writel(0x2, SYS_POWUP_CLR);
	}
	
    if(readl(RTC_SYSM_ADDR + 0x60) & 0x2) {
		if(*(uint32_t *)CONFIG_RESERVED_SKIP_BUFFER != CONFIG_SKIP_MAGIC)
		  writel(readl(RTC_INFO6) | 0x2, RTC_INFO6);
		else
		  writel(readl(RTC_INFO6) & ~0x2, RTC_INFO6);
	}
    else {
        writel(readl(RTC_INFO6) & ~0x2, RTC_INFO6);
        writel(readl(RTC_SYSM_ADDR + 0x60) | 0x2,
                (RTC_SYSM_ADDR + 0x60));
    }

	writel(0xff, RTC_SYSM_ADDR + 0x04);

	return pwr_st;
}

int bootl(void)
{
	int boottype=got_boot_type();
	if(BOOT_HALT_SYSTEM==boottype){
		printf("Erro: something erro happened, system halted\n");
		for(;;);
	}
	int bootdev=storage_device();
	if(!vs_device_burndst(bootdev)){
		printf("Erro: Invalid system disk specified, it not a burndst\n");
		return -1;
	}
	int ret = vs_assign_by_id(bootdev, 1);
	if(ret){
		printf("Erro: Can not get the system disk ready\n");
		return -1;
	}
	char *ramdisk = NULL;
	char * alternatedisk = NULL;
	char *kernel = NULL;
    char *alternatekernel = NULL;
	if(boottype==BOOT_NORMAL || boottype==BOOT_CHARGER){
		ramdisk="ramdisk";
		alternatedisk = "recovery-rd";
		kernel="kernel0";
		alternatekernel = "kernel1" ;

	}else{
		ramdisk="recovery-rd";
		kernel="kernel1";
		alternatekernel = "kernel0" ;
	}
	void* kerneladd = NULL;
	void* ramdiskadd = NULL;
	void* logoadd = NULL;
	 logoadd = load_image("logo",NULL , (char*)CONFIG_RESERVED_LOGO_BUFFER, bootdev);
	if(logoadd == NULL){
	    printf("Erro: Failed to load logo from gaven disk\n");
	}else if(logoadd != (void*)CONFIG_RESERVED_LOGO_BUFFER){
	//	gunzip();
	}
	

	ramdiskadd = load_image(ramdisk,alternatedisk,(char*) CONFIG_SYS_PHY_RD_BASE , bootdev);
	if(ramdiskadd == NULL){
		 printf("Erro: Failed to load %s from gaven disk, system halted\n",ramdisk);
		 for(;;);
	}
	kerneladd = load_image(kernel,alternatekernel ,(char*) CONFIG_SYS_PHY_LK_BASE , bootdev);
	if(kerneladd == NULL){
		printf("Erro: Failed to load %s from gaven disk, try kernel1\n",kernel);
		for(;;);
	}
   
    /*
     * We load uboot0.isi to SDRAM here for use of customer tool
     */
    ret = vs_assign_by_id(DEV_BND,1);
    if(ret)
    {
        printf("can no find bnd\n");
    } else {
        vs_read((uint8_t *)CONFIG_RESERVED_UBOOT0_BUFFER,0, 0x8000,0);
    }
	 return do_boot(kerneladd, ramdiskadd);
}

