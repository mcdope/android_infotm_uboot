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
#include <command.h>
#include <oem_func.h>
#include <oem_font.h>
#include <oem_graphic.h>
#include <oem_pic.h>
#include <asm/io.h>
#include <nand.h>
#include <oem_inand.h>
#define DRIVING     0

#ifdef CONFIG_BOOT_AUTO_ADJUST
struct oem_freq_lvl {

	uint32_t apll;
	uint32_t dcfg;
	uint32_t lvl;
};
#endif

int oem_mmc_init(void)
{
	int ret;

	char *arg_rescan[] = {"mmc", "rescan", "0"};
#ifdef SOURCE_CHANNEL
	sprintf(arg_rescan[2], "%d", SOURCE_CHANNEL);
#endif
	ret = do_mmcops(NULL, 0, 3, arg_rescan);
//	printf("result is %d\n", ret);

	return ret;
}

static char * __scr_p;

static int oem_valid_bootscr(uint32_t addr)
{
	uint8_t header[] = {
		0x0a, 0x2d, 0x2d, 0x5b, 0x5b, 0x75, 0x62, 0x6f,
		0x6f, 0x74, 0x2e, 0x73, 0x63, 0x72, 0x69, 0x70,
		0x74, 0x0a,};
	uint8_t *p = (uint8_t *)addr;

	int i;
	for(i = 0; i < ARRAY_SIZE(header); i++)
	  if(*p++ ^ header[i])
		return 0;

	__scr_p = (char *)p;
	for(i = 0; i < 0x1000; i++, p++)
	  if(*p == 0xa
		 && *(p + 1) == 0x5d
		 && *(p + 2) == 0x5d)
		return 1;		// Valid Script 
	return 0;
}

static int __get_sd_script(char *str)
{
	while((*__scr_p == 0x0a)
	   || (*__scr_p == 0x2d)
	   || (*__scr_p == 0x5b)) __scr_p++;

	if(*(uint16_t *)__scr_p == 0x5d5d)
	{
		*str = 0;
		return 0;
	}

	while((*__scr_p != 0x0a)
	   && (*__scr_p != 0x5d)) *str++ = *__scr_p++;
	*str = '\0';

	return 1;
}
#ifndef CONFIG_SYS_DISK_iNAND
int oem_boot_sd(void)
{
	char cmd[64];
#ifdef SOURCE_CHANNEL
	sprintf(cmd, "mmc resacn %d",SOURCE_CHANNEL);
#else
	sprintf(cmd, "mmc resacn 0");
#endif
	run_command(cmd,0);
	printf("Try booting from SD ...\n");

	/* Load data from sd */
	memset((uint8_t *)CONFIG_RESV_SCRIPT, 0, 4096);
#ifndef SOURCE_CHANNEL
	sprintf(cmd, "mmc read 0 %x 400 8",CONFIG_RESV_SCRIPT);
#else
	sprintf(cmd, "mmc read %d %x 400 8", SOURCE_CHANNEL , CONFIG_RESV_SCRIPT);
 #endif
	run_command(cmd, 0);

	if(!oem_valid_bootscr(CONFIG_RESV_SCRIPT))
	{
		printf("Invalid boot SD!\n");
		return -1;
	}
	
	oem_simpro_finish("Booting from SD ...");

#ifdef SOURCE_CHANNEL
	while(__get_sd_script(cmd))
	{
		if(strstr(cmd, "iuw"))
		{
			printf("IUW found in command list. The commands will be executed.\n");
			sprintf(cmd, "mmc read %d 40008000 800 30000", SOURCE_CHANNEL);
			run_command(cmd, 0);
			run_command("iuw 40008000", 0);

			oem_simpro_finish("Take off SD and reboot.");
			/* This should never be reached */
			for(;;){
				if(oem_mmc_init())
				  reset_cpu(0);
				udelay(500000);
			}

		}
	}

	if(!oem_valid_bootscr(CONFIG_RESV_SCRIPT))
	{
		printf("Invalid boot SD!\n");
		return -1;
	}
#endif

	/* Run SD scripts */
	while(__get_sd_script(cmd))
	{
		printf("script: %s\n", cmd);
		run_command(cmd, 0);
	}

	oem_simpro_finish("Take off SD and reboot.");
	printf("SD boot finished, system halted.\n");
	for(;;)
	{
		if(oem_mmc_init())
		  reset_cpu(0);
		udelay(500000);
	}

	return 0;
}	
#else
int run_recommanded_commmands(void)
{
	char cmd[128];
	int preheader=0;
	int recommand=0;
	
	memset((uint8_t *)CONFIG_RESV_SCRIPT, 0, 4096);
        sprintf(cmd, "mmc read %d %x 400 8", SOURCE_CHANNEL , CONFIG_RESV_SCRIPT);
        if(run_command(cmd, 0)){
                if(!oem_valid_bootscr(CONFIG_RESV_SCRIPT)) return recommand;
       }

	while( __get_sd_script(cmd)){
		if(0==preheader){
			if(!strncmp(cmd,"recommanded:",12)){
			       	preheader=1;
				printf("%s\n",cmd);
			}
		}else{
			printf("%s\n",cmd);
			run_command(cmd,0);
			recommand=1;
		}
	}
	return recommand;
}

int load_file_sd(char *filename,unsigned int addr )
{
	char cmd[64];

	sprintf(cmd,"fatload mmc %d:1 %x %s",SOURCE_CHANNEL,addr,filename);
        if(!run_command(cmd,0)){
		return 0;
        }
	return 1;
}

int oem_boot_sd(void)
{
	char cmd[64];
	int recommand;
/* Try to load script from SD */
	sprintf(cmd, "mmc resacn %d",SOURCE_CHANNEL);
	run_command(cmd,0);
	sprintf(cmd, "mmc rescan %d",iNAND_CHANNEL);
	run_command(cmd,0);

	printf("Try booting from SD ...\n");
	/* Load data from sd */
	memset((uint8_t *)CONFIG_RESV_SCRIPT, 0, 4096);
	sprintf(cmd, "mmc read %d %x 400 8", SOURCE_CHANNEL , CONFIG_RESV_SCRIPT);
	if(run_command(cmd, 0))
	{
		if(!oem_valid_bootscr(CONFIG_RESV_SCRIPT))
		{
			printf("Invalid boot SD!\n");
			return -1;
		}
	}else return -1;
	oem_simpro_finish("Booting from SD ...");

	/* Run SD scripts */
	while(__get_sd_script(cmd))
	{
		if(strstr(cmd, "iuw"))
		{
			printf("IUW found in command list. The commands will be executed.\n");
			sprintf(cmd, "mmc read %d 40008000 800 30000", SOURCE_CHANNEL);
			run_command(cmd, 0);
			run_command("iuw 40008000", 0);

			oem_simpro_finish("IUW command list finished, you should not see this image.");
			/* This should never be reached */
			for(;;){
				if(oem_mmc_init())
				  reset_cpu(0);
				udelay(500000);
			}

		}
	}


	/* Run SD scripts */
#ifndef CONFIG_TEST_VERSION
	sprintf(cmd,"fatload mmc %d:1 47c00000 pcb_test.bin",SOURCE_CHANNEL);
	if(!run_command(cmd,0)){
		oem_simpro_finish("Starting PCB test program...");
                run_command("go 47c00000",0);
        }
#endif
	char uboot=0;
        sprintf(cmd,"fatload mmc %d:1 40008000 android/u-boot-nand.bin",SOURCE_CHANNEL);
        if(!run_command(cmd,0)){
		uboot=1;
                run_command("adr u0 40008000 ",0);
		oem_clear_screen(0);
        }

	int startblk=0;
	startblk=oem_partition();
	
//	sprintf(cmd,"fatload mmc %d:1 40008000 system.img",SOURCE_CHANNEL);	
//	if(!run_command(cmd,0)){
//		oem_burn_SYS((uint8_t *)0x40008000,1);
//	}
	
	char imagest[5]={0,0,0,0};
	char file[5][64]={"system.img","uImage","ramdisk.img","recovery_rd.img","updater"};
	unsigned int rambase[5]={CONFIG_SYS_PHY_LK_BASE,CONFIG_SYS_PHY_LK_BASE,CONFIG_SYS_PHY_RD_BASE,CONFIG_SYS_PHY_RD_BASE,CONFIG_SYS_PHY_LK_BASE};
	int (*imageburn[5])(uint8_t * data )={oem_burn_zAS,oem_burn_LK,oem_burn_RD,oem_burn_RD_, oem_burn_UPDT};
	int i=0;

	for(;i<5;i++){
		char filename[32];
		sprintf(filename,"android/%s",file[i]);
		if(!load_file_sd(filename,rambase[i] )){	
			if(imageburn[i]((uint8_t *)rambase[i])){
				imagest[i]=0x01;
			}else{
				imagest[i]=0x11;
			}
		}
	}
	
	recommand=run_recommanded_commmands();	
	printf("recommand:%d\n",recommand);
	if(0x11==imagest[2]||0x11==imagest[3]){
		oem_start_updater(1);
		for(;;);
	}

	if(0x11==imagest[1]){
		printf("Kernel image update finished.\n\r");
		oem_simpro_finish("uImage updated OK, please take off SD and reboot.");
	}else if(uboot){
		printf("U-boot update finished.\n\r");
		oem_simpro_finish("U0 updated OK, please take off SD and reboot.");
	}

#ifndef CONFIG_TEST_VERSION
	for(;;){
		if(oem_mmc_init())
			reset_cpu(0);
		udelay(500000);
	}
#endif
	return 0;//never reached here
}
#endif

void oem_consume_keypress(void)
{
	int i;
	for(i = 0; i < 100; i++)
	{
		if(tstc())
		  getc();
	}
}

uint8_t oem_getc(void)
{
	oem_consume_keypress();
	return getc();
}

void oem_getc_exec_gpio(uint32_t gpio, int num)
{
	while(!(readl(gpio) & (1 << num)));
	while((readl(gpio) & (1 << num)));
	while(!(readl(gpio) & (1 << num)));

	return ;
}

void oem_getc_exec(uint8_t a)
{
	while(1)
	  if(oem_getc() == a)
		break;
}

int oem_tstc_gpio(uint32_t gpx, uint32_t bit)
{
	return !!(readl(gpx) & (1 << bit));
}

#ifdef CONFIG_SYS_DISK_iNAND
int oem_need_maitain(void)
{
	int reg=0;
	if(readl(RST_ST) & 0x8){
		reg=readl(INFO1);
		reg&=IMAGE_ERROR;
		if(reg==IMAGE_ERROR) return 1;
	}
	return 0;
}
int oem_maintain_counter(void)
{
	int stat=0;
	stat=readl(INFO1);
        char cnt=(stat&(0x3<<27))>>27;
        if(cnt<3) {
               cnt+=1;
	        
	       stat&=~(0x3<<27);
	       stat|=cnt<<27;
	       writel(stat,INFO1);
	       return 1;
        }
	return 0;
}
int oem_clear_maintain(void)
{
	int stat=0;
	stat=readl(INFO1);
	stat&=0x0000FFFF;   
	writel(0,INFO1);
	return 0;
}
int oem_set_maintain(void)
{
	int stat=0;
	
	stat=readl(INFO1);
        stat&=0x0000FFFF;
        stat|=IMAGE_ERROR;
        writel(stat,INFO1);
	printf("set INFO1:%x",stat);
	return 0;
}
#endif

#ifdef CONFIG_SYS_DISK_iNAND
int oem_maitain_image(void)
{
	if(oem_need_maitain()){
		printf("Something error occured at last boot. now check image sotred in inand validate.\n");
        	if(oem_maintain_image()){
                	if(oem_maintain_counter())
                            	reset_cpu(0);

                      printf("Error:Try to maintain system image but failed, Plz reburn it.\n");
                      oem_simpro_finish("System is demaged, please try recover from external storage.");
                      for(;;);

             }
              oem_clear_maintain();
	      return 0;
     }
	return 0;
}
#endif


int oem_is_recover(void)
{
#ifdef CONFIG_NO_ADR_SYSRECOVER
	return 0;
#endif
#ifdef CONFIG_RECOVERY_KEY1_GPIO
	if(!(readl(CONFIG_RECOVERY_KEY1_GPIO) & (1 << CONFIG_RECOVERY_KEY1_GPIO_NUM)))
	  return 1;
#endif

	if(readl(RST_ST) & 0x8)
	  if(readl(INFO3) == CONFIG_PRODUCT_LINUX_RVFLAG)
	  {
		  /* Clear RVFLAG */
		  writel(0, INFO3);
		  return 1;
	  }

	return 0;
}

#ifdef CONFIG_BOOT_AUTO_ADJUST
uint32_t oem_get_boot_para(int para_no)
{
	uint32_t *boot_para = (uint32_t *)(CONFIG_AU_CLK_MAGIC_OFFS | (1 << 31));

	printf("get boot para %d: 0x%08x\n", para_no, boot_para[para_no]);
	if(para_no > 7)
	{
		printf("Boot para No. exceed limit.\n");
		return 0;
	}

	return boot_para[para_no];
}

int oem_save_boot_para(struct oem_freq_lvl *freq)
{
	uint32_t *boot_para = (uint32_t *)CONFIG_SYS_PHY_UBOOT_SWAP;
	uint32_t len = 0;

#ifdef CONFIG_SYS_BOOT_NOR
	nor_hw_init();
	nor_erase_block(0x70000);
#else
	oem_read_markbad((uint8_t *)CONFIG_SYS_PHY_UBOOT_SWAP, 0, 512*1024);
	boot_para = boot_spl + 8192 - 64;

#endif

	if(freq)
	{
		boot_para[0] = CONFIG_AU_CLK_MAGIC;
		boot_para[1] = freq->apll;
		boot_para[2] = freq->dcfg;
		boot_para[3] = freq->lvl;
		len = 16;
	}
	else
	{
		if(oem_get_boot_para(0) != CONFIG_AU_CLK_MAGIC)
		{
			/* save clk settings */
			boot_para[0] = CONFIG_AU_CLK_MAGIC;
			boot_para[1] = readl(APLL_CFG);
			boot_para[2] = readl(DIV_CFG0);
			boot_para[3] = 5;
			len = 16;
		}

		if(oem_get_boot_para(7) != CONFIG_AU_MEM_MAGIC)
		{
#ifdef CONFIG_MEM_POOL 	
			boot_para[4] = CONFIG_AU_MEMPOOL_MAGIC;
#endif
			/* save memory settings */

			boot_para[7] = CONFIG_AU_MEM_MAGIC;
			boot_para[0x5] = DENALI_CTL_R46;          /* r46*/
			boot_para[0x6] = DENALI_CTL_R74;          /* r74*/
			boot_para[0x8] = readl(DENALI_CTL_PA_62);
			boot_para[0x9] = readl(DENALI_CTL_PA_63);
			boot_para[0xa] = readl(DENALI_CTL_PA_64);
			boot_para[0xb] = readl(DENALI_CTL_PA_65);
			boot_para[0xc] = readl(DENALI_CTL_PA_66);
			boot_para[0xd] = readl(DENALI_CTL_PA_67);
			boot_para[0xe] = readl(DENALI_CTL_PA_68);
			boot_para[0xf] = readl(DENALI_CTL_PA_69);
			boot_para[0x10] = DENALI_CTL_R47;         /* r47*/
			boot_para[0x11] = (readl(DENALI_CTL_PA_18) & ~( 3 << 8));/* r18*/
			boot_para[0x11] |= (DENALI_CTL_R18_ODT << 8);  
			len = 72;
		}
	}

	if(len)
	{
#ifdef CONFIG_SYS_BOOT_NOR
		nor_program(CONFIG_AU_CLK_MAGIC_OFFS, (uint8_t *)boot_para, len);
#else
		/* TODO: NAND boot */
		oem_erase_markbad(0, 512*1024);		
	
		oem_write_markbad((uint8_t *)CONFIG_SYS_PHY_UBOOT_SWAP, 0, 512*1024, 512*1024);

#endif
	}
	return 0;
}

int oem_get_bootcount(int step)
{
	uint16_t *offset = (uint16_t *)(0x10070000 + step * 0x7800);
	int i;

	if(step != !!step)
	{
		printf("Invalid bootcount step number.\n");
		return -1;
	}

	for(i = 0; i < 0x3c00; i++)
	  if(*(offset + i) == 0xffff)
		break;

	return i;
}

int oem_record_boot(void)
{
#ifdef CONFIG_SYS_BOOT_NOR
	uint16_t *offset = (uint16_t *)(0x10070000);
	uint16_t i, r = 0;

	for(i = 0; i < 0x3c00; i++)
	  if(*(offset + i) == 0xffff)
		break;

	nor_hw_init();
	nor_program((uint32_t)(offset + i), (uint8_t *)&r, 2);
#else
	/* TODO: NAND method to record booting times */
#endif

	return 0;
}

int oem_boot_check(void)
{
	int c0, c1, lvl;
	struct oem_freq_lvl freq[] = {
		{0x8000002b, 0x00014481, 4},
		{0x80000041, 0x000166c2, 3},
		{0x80000029, 0x00014481, 2},
		{0x80000027, 0x00014481, 1},
		{0x80001041, 0x00014481, 0},
	};
	/* record boot parameters */
	oem_save_boot_para(0);

	/* check boot success count */
	c0 = oem_get_bootcount(0);
	c1 = oem_get_bootcount(1);
	printf("Boot count step0: %d, step1: %d\n", c0, c1);
	
	if((c1 + (c1 >> 5)) < c0)
	{
		lvl = oem_get_boot_para(3);
		/* decrease freq. */
		if(lvl)
		{
			lvl--;
			oem_save_boot_para(freq + lvl);
			printf("Set CPU to 0x%08x, 0x%08x\n", freq[lvl].apll,
			   freq[lvl].dcfg);

			/* reset */
			reset_cpu(0);
		} else {
			printf("Already the lowest frequence.\n");
		}
	}

	/* set boot step 0 */
	oem_record_boot();

	return 0;
}
#endif	/* CONFIG_BOOT_AUTO_ADJUST */

void oem_print_eye(char *s)
{
	char str[32];
	sprintf(str, "E%02X%02X%02X%02X%02X%02X%02X%02X\n",
	   (readl(DENALI_CTL_PA_62) >> 8) & 0x7f,
	   (readl(DENALI_CTL_PA_63) >> 8) & 0x7f,
	   (readl(DENALI_CTL_PA_64) >> 8) & 0x7f,
	   (readl(DENALI_CTL_PA_65) >> 8) & 0x7f,
	   (readl(DENALI_CTL_PA_66) >> 8) & 0x7f,
	   (readl(DENALI_CTL_PA_67) >> 8) & 0x7f,
	   (readl(DENALI_CTL_PA_68) >> 8) & 0x7f,
	   (readl(DENALI_CTL_PA_69) >> 8) & 0x7f);

	printf("%s", str);
	if(s)
	{
		memset(s, 0, 32);
		memcpy(s, str, 17);
	}

	return ;
}


#ifdef CONFIG_BOOT_AUTO_ADJUST

#if DRIVING

#define __wave_shift	(_sc_w/20)

void oem_init_waveform(void)
{
	int i;
        int j;
	oem_clear_screen(0);
	for(j = 0; j < 4; j++)
	{
	    for(i = 1; i < 64; i++)
	         oem_draw_box_color(__wave_shift + ((j&0x1)?(_sc_w/2):0x0)  + i * _sc_w / 160,  40 - (!(i&0x7))*10 +((j/2)?0x0:((_sc_h- 200) / 2)),
				 __wave_shift + ((j&0x1)?(_sc_w/2):0x0)  + i * _sc_w / 160, ((j/2)?((_sc_h - 200) / 2):(_sc_h - 200)) + (!(i&0x7))*10,
		     ((i&0x7)?0x4a49:0xad55));
	}
#if 0
	oem_draw_box_color(0, 100 + 4 * _sc_w/80 *3, _sc_w, 100 + 4 * _sc_w/80 *3, 0xff49);
	oem_draw_box_color(_sc_w/2 - 1, 0, _sc_w/2 - 1, _sc_h, 0xff49);
#endif

	return ;
}
#else
#define __wave_shift	(_sc_w/10)
void oem_init_waveform(void)
{
	int i;

	oem_clear_screen(0);
	for(i = 1; i < 64; i++)
	  oem_draw_box_color(__wave_shift + i * _sc_w / 80,
		 60 - (!(i&0x7))*10, __wave_shift + i * _sc_w / 80, _sc_h - 200 + (!(i&0x7))*10,
		 ((i&0x7)?0x4a49:0xad55));

#if 0
	oem_draw_box_color(0, 100 + 4 * _sc_w/80 *3, _sc_w, 100 + 4 * _sc_w/80 *3, 0xff49);
	oem_draw_box_color(_sc_w/2 - 1, 0, _sc_w/2 - 1, _sc_h, 0xff49);
#endif

	return ;
}
#endif

#if DRIVING
void oem_show_wave(int l, uint32_t d1, uint32_t d2, uint32_t driving_switch)
{
	uint32_t i;
	uint32_t x_add, y_add;
	unsigned long long data;
	u_short x, y, y_, c;

	x_add = (driving_switch&0x1)?(_sc_w / 2) : 0x0;
	y_add = (driving_switch/2)?((_sc_h - 200) / 2):0x0;
	data = d2;
	data <<= 32;
	data |= d1;
	c = (l > 3)?0x4b55:0xe945;
	for(i = 0; i < 64; i++)
	{
		int hl, hl_;

		hl = !!(data&(1LL<<i));
		hl_ = !!(data&(1LL<<(i+1)));

		x = __wave_shift + i * _sc_w / 160 + x_add;
		y = 60 - _sc_w/160*hl + l * _sc_w/160 * 3 + y_add;
		y_ = 60 + l * _sc_w/160 * 3 + y_add;
		oem_draw_box_color(x, y, x + _sc_w/160, y+1, c);
		if((hl ^ hl_) && i != 63)
		  oem_draw_box_color(x + _sc_w/160, y_ - _sc_w/160,
			 x + _sc_w/160 + 1, y_, c);
	}

	return ;
}
#else
void oem_show_wave(int l, uint32_t d1, uint32_t d2)
{
	uint32_t i;
	unsigned long long data;
	u_short x, y, y_, c;

	data = d2;
	data <<= 32;
	data |= d1;
	c = (l > 3)?0x4b55:0xe945;
	for(i = 0; i < 64; i++)
	{
		int hl, hl_;

		hl = !!(data&(1LL<<i));
		hl_ = !!(data&(1LL<<(i+1)));

		x = __wave_shift + i*_sc_w/80;
		y = 100 - _sc_w/80*hl + l * _sc_w/80 * 3;
		y_ = 100 + l * _sc_w/80 * 3;
		oem_draw_box_color(x, y, x + _sc_w/80, y+1, c);
		if((hl ^ hl_) && i != 63)
		  oem_draw_box_color(x + _sc_w/80, y_ - _sc_w/80,
			 x + _sc_w/80 + 1, y_, c);
	}

	return ;
}

#endif
void oem_test_denali(void)
{
	int i;
	char s[32];

	oem_clear_screen(0);
	oem_mid("System is about to search DDRII parameters");
	oem_mid2("The process will last about 2 minutes, LCD may flick during this action");
	oem_print_eye(s);
	oem_below2(s);
	for(i = 0; i < 5; i++)
	{
		sprintf(s, "Test will start in %ds ...", 5 - i);
		oem_below2(s);
		udelay(1000000);
	}
	backlight_power_off();
	writel(CONFIG_AU_MEM_MAGIC, INFO2);
	reset_cpu(0);
	/* never reach here */
	return ;
}

#if DRIVING 
void oem_show_denali(void)
{
        uint32_t i;
	uint32_t j;
	uint32_t bit;
	char s[128];

	if(*(uint32_t *)(0x40008000) != CONFIG_AU_MEM_MAGIC)
	  return ;

	oem_init_waveform();
        for(j = 0; j < 4; j++)
	{
	    for(i = 0; i < 8; i++)
	    {
	        	oem_show_wave(i, *(uint32_t *)(0x40008000 + j * 0x80 + (i + 1) * 8),
		        *(uint32_t *)(0x40008000 + (i + 1) * 8 + 4 + j * 0x80), j);
	    }
	}

	for(j = 0 ; j < 4; j++)
	{
	    for(i = 0; i < 8; i++)
	    {
	      	bit = *(uint32_t *)(0x40008208 + i * 4 + j * 0x20);
                bit = (bit >> 8) & 0x3f;

		oem_draw_box_color(__wave_shift + bit*_sc_w/160 + 2 + ((j&0x1)?(_sc_w / 2):0x0),
		   60 + i * _sc_w/160 * 3 - _sc_w/160 + 2 + ((j/2)?((_sc_h - 200) / 2):0x0),
		   __wave_shift + bit*_sc_w/160 + _sc_w/160 - 2 + ((j&0x1)?(_sc_w / 2):0x0),
		   60 + i * _sc_w/160 * 3 - 2 + ((j/2)?((_sc_h - 200) / 2):0x0), 0xff49);
#if 0
		if(i < 4)
		  sprintf(s, "r_%d", i%4);
		else
		  sprintf(s, "w_%d", i%4);

		oem_font_puts(18, 84 + i * _sc_w/80 * 3, s);
		sprintf(s, "(%02x)", bit);
		oem_font_puts(__wave_shift + bit*_sc_w/80 + _sc_w/80 - 2, 100 + i * _sc_w/80 * 3 - 2, s);
#endif
	    }
	}    
	i = *(uint32_t *)(0x40008208 + 0x90);
	j = *(uint32_t *)(0x40008208 + 0x94);
        sprintf(s, "eye_choose %d  drive_choose %d" ,i ,j);
	oem_font_puts(_sc_w / 2 - 200, 470 , s);
	oem_below("DDRII study OK, press BACK to continue ...");
	oem_getc_exec_gpio(CONFIG_RECOVERY_KEY2_GPIO, CONFIG_RECOVERY_KEY2_GPIO_NUM);
	oem_show_logo();
	oem_save_boot_para(NULL);
	reset_cpu(0);
}
#else
void oem_show_denali(void)
{
	int i;
	char s[128];

	if(*(uint32_t *)(0x40008000) != CONFIG_AU_MEM_MAGIC)
	  return ;

	oem_init_waveform();
	for(i = 0; i < 8; i++)
	{
		oem_show_wave(i, *(uint32_t *)(0x40008000 + (i + 1) * 8),
		   *(uint32_t *)(0x40008000 + (i + 1) * 8 + 4));
	}

	for(i = 0; i < 8; i++)
	{
		int bit = (readl(DENALI_CTL_PA_62 + i * 4) >> 8) & 0x3f;

		oem_draw_box_color(__wave_shift + bit*_sc_w/80 + 2,
		   100 + i * _sc_w/80 * 3 - _sc_w/80 + 2,
		   __wave_shift + bit*_sc_w/80 + _sc_w/80 - 2,
		   100 + i * _sc_w/80 * 3 - 2, 0xff49);

		if(i < 4)
		  sprintf(s, "r_%d", i%4);
		else
		  sprintf(s, "w_%d", i%4);

		oem_font_puts(18, 84 + i * _sc_w/80 * 3, s);
		sprintf(s, "(%02x)", bit);
		oem_font_puts(__wave_shift + bit*_sc_w/80 + _sc_w/80 - 2, 100 + i * _sc_w/80 * 3 - 2, s);
	}

	oem_below("DDRII study OK, press BACK to continue ...");
	oem_getc_exec_gpio(CONFIG_RECOVERY_KEY2_GPIO, CONFIG_RECOVERY_KEY2_GPIO_NUM);
	oem_show_logo();
	oem_save_boot_para(NULL);
	reset_cpu(0);
}
#endif

void oem_check_denali(void)
{
	int i, stat = 0;
	unsigned long t;
	char s[128], tmp[128];

	/* show denali test result if any */
	oem_show_denali();

#if defined(CONFIG_MINUS_KEY_GPIO) && defined(CONFIG_PLUS_KEY_GPIO)
	/* check to see if denali test is required */
	if(!(readl(CONFIG_RECOVERY_KEY2_GPIO) & (1 << 5)))
	{
		oem_clear_screen(0);
		oem_print_eye(tmp);
		sprintf(s, "%s %08X %08X %08X %08X %08X", tmp,
		   readl(APLL_CFG), readl(DPLL_CFG),
		   readl(EPLL_CFG), readl(DIV_CFG0),
		   readl(DIV_CFG1));
		oem_font_puts(10, 10, s);
		
		for(i = 0; i < 5; i++)
		{
			int p0 = 1, p1 = 1, m0 = 1, m1 = 1;
			sprintf(s, "System will boot in %ds ...", 5 - i);
			oem_font_puts(10, 35, s);
			t = get_ticks();
			while(get_ticks() < t + 99999)
			{
				p1 = oem_tstc_gpio(CONFIG_PLUS_KEY_GPIO, CONFIG_PLUS_KEY_GPIO_NUM);
				m1 = oem_tstc_gpio(CONFIG_MINUS_KEY_GPIO, CONFIG_MINUS_KEY_GPIO_NUM);

				if(p1 != p0)
				{
					p0 = p1;
					if(p0)
					{
						if((1 == stat) || (4 == stat))
						{
							stat++;
							printf("stat up to %d\n", stat);
						}
						else if(6 == stat)
						{
							oem_test_denali();
							/* never reach here */
							return ;
						}
						else
						{
							stat = 0;
							printf("stat reset plus\n");
						}
					}
				}

				if(m1 != m0)
				{
					m0 = m1;
					if(m0)
					{
						if((0 == stat) || (2 == stat) ||
						   (3 == stat) || (5 == stat))
						{
							stat++;
							printf("stat up to %d\n", stat);
						}
						else
						{
							if(6 == stat)
							  stat = 4;
							else
							  stat = 1;
							printf("stat reset minus\n");
						}
					}
				}
			} // while
		} // for
	}
	else if(oem_get_boot_para(7) != CONFIG_AU_MEM_MAGIC)
#endif
#if 1
	  /* or the first time system boot */
	  if(oem_get_boot_para(7) != CONFIG_AU_MEM_MAGIC)
		oem_test_denali();
#endif

	return ;
}
#endif /* CONFIG_BOOT_AUTO_ADJUST */

#ifdef CONFIG_RESET_CHECK
#define CONFIG_IMAPX200_FRIST_RESET     (0x66626f74)
#define CONFIG_IMAPX200_SECOND_RESET    (0x73626f74)
#define CONFIG_IMAPX200_REQ_REASE       (0x72656173)


void oem_check_reset(void)
{

	uint32_t resetState = 0;
	uint8_t menukey, homekey;

	resetState = readl(INFO1);


	if(resetState == CONFIG_IMAPX200_REQ_REASE)
	{
		oem_clear_screen(0);
	        oem_mid("X ERROR OCCURRED: BOOTING SYSTEM FAILURE X");
		oem_mid2("You can select YES to try recover, but your personal data will be lost.");
		oem_below("HOME=YES  MENU=NO");        	


		do
		{
			menukey = oem_tstc_gpio(CONFIG_MENU_KEY_GPIO, CONFIG_MENU_KEY_GPIO_NUM);
			homekey = oem_tstc_gpio(CONFIG_RECOVERY_KEY1_GPIO, CONFIG_RECOVERY_KEY1_GPIO_NUM);
		}
		while(menukey && homekey);

		printf("homekey = %x, menukey = %x\n", homekey, menukey);
		if(homekey == 0)
		{
			oem_erase_markbad(CONFIG_SYS_NAND_UDATA_OFFS, CONFIG_SYS_NAND_UDATA_LEN);
		}
	

		writel(0x0, INFO1);
	}

	

}
#endif

