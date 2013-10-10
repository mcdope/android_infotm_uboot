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
#include <preloader.h>

extern int dramc_init(struct irom_export *irf, int);
extern int imap_set_retry_param(int retrylevel, unsigned char * buf);
extern int imap_get_retry_param(unsigned char *buf);
extern void param_transmit_to_uboot1(void); 
extern void set_up_watchdog(int ms);

struct irom_export *irf = NULL;
uint8_t nand_param[1026];
uint8_t nand_param_copy[128];
uint32_t g_retrylevel;
uint32_t g_retrylevel_max;
uint32_t g_param;
uint32_t g_retry_param_magic;
uint32_t g_skip_magic = 0;
int bdevice_id = 0;

int nread_skip_bad(uint8_t *buf, loff_t start, int length)
{
	struct nand_config *nc = (struct nand_config *)irf->nand_get_config(NAND_CFG_NORMAL);
	int w, block = nc->blocksize, skip, remain, remain_page;
	int retrylevel = 0;
	int ret = 0;

	for(remain = length; remain; ) {
#if 0
		for(skip = 0; irf->vs_isbad(start & ~(block - 1));
					skip++, start += block) {
			irf->printf("bad block skipped @ 0x%llx\n", start & ~(block - 1));
			if(skip > 30) {
				irf->printf("two many bad blocks skipped"
						"before getting the corrent data.\n");
				return -1;
			}
		}
#endif

		skip = 0;
		retrylevel = g_retrylevel;
		while(1){
bad_retry_again:			
			ret = irf->vs_isbad(start & ~(block - 1));
			if((g_param & 0x3) == 0x3){
				if(ret != 0){
					retrylevel++;
					if(retrylevel > g_retrylevel_max)
						retrylevel = 0;
					if(retrylevel == g_retrylevel){
						imap_set_retry_param(retrylevel, nand_param);
						goto bad_retry_end;
					}
					imap_set_retry_param(retrylevel, nand_param);
					goto bad_retry_again;
				}
			}
bad_retry_end:
			if(ret != 0){
				skip++;
			       	start += block;
			}else{
				break;
			}
			if(skip > 30){
				irf->printf("two many bad blocks skipped"
					"before getting the corrent data.\n");
				return -1;
			}
		}


		w = min(block, // block
			min(block - (start & (block - 1)), // block remain
				remain)); // remain

		//spl_printf("pagesize = %d, w = %d\n", nc->pagesize, w);
		retrylevel = g_retrylevel;
		for(remain_page = w; remain_page; ){

read_retry_again:			
			ret = irf->vs_read(buf, start, nc->pagesize, 0);
			if((g_param & 0x3) == 0x3){ //H27UBG8T2BTR-BC, H27UCG8T2ATR-BC	
				//spl_printf("uboot0: read 0x%llx, ret = 0x%x\n", start, ret);

				if(ret != (nc->pagesize)){
					retrylevel++;
					if(retrylevel > g_retrylevel_max)
						retrylevel = 0;
					if(retrylevel == g_retrylevel){
						spl_printf("uboot0: read retry failed\n");
						//spl_printf("uboot0: read retry level = %d\n", retrylevel);
						imap_set_retry_param(retrylevel, nand_param);
						
						goto read_retry_end;
					}
					//spl_printf("uboot0: read retry level = %d\n", retrylevel);
					imap_set_retry_param(retrylevel, nand_param);
					goto read_retry_again;
				}else{
				
				}
			}
read_retry_end:
//			if(g_param == 0xb || (g_param == 0x13) ){ //any retry flash need set zero for next read
			if(retrylevel != 0)
			{
				g_retrylevel = 0;
				retrylevel = 0;
				imap_set_retry_param(retrylevel, nand_param);
			}
			buf		+= (nc->pagesize);
			start		+= (nc->pagesize);
			//spl_printf("buf = 0x%x, start = 0x%llx\n", buf, start);
			if(remain_page < nc->pagesize)
				remain_page = 0;
			else
				remain_page -= nc->pagesize;
		}
		g_retrylevel = retrylevel;

	//	buf    += w;
	//	start  += w;
		remain -= w;
	}

	return length;
}

int init_config(void)
{
	int id, ret, len = 0;
	void * cfg = (void *)irf->malloc(ITEM_SIZE_NORMAL);
	struct nand_config *nc;
	uint32_t tmp2 = 0, tmp3 = 0;

	id = irf->boot_device();
	bdevice_id = id;
	switch(id) {
		case DEV_EEPROM:
		case DEV_FLASH:
			len = ITEM_SIZE_EMBEDDED;
			irf->memcpy(cfg, (void *)(IRAM_BASE_PA + BL_SIZE_FIXED
					- ITEM_SIZE_EMBEDDED), len);
			break ;
		case DEV_BND:
//			irf->printf("scan nnd.\n");
			ret = irf->nand_rescan_config(infotm_nand_idt,
						infotm_nand_count());
			if (ret)
			  return ret;
			id = DEV_NAND;
//TODO
			nc = (struct nand_config *)irf->nand_get_config(NAND_CFG_NORMAL);
			g_retrylevel = 0;
			//spl_printf("seed[0] = 0x%x\n", nc->seed[0]);
			//spl_printf("seed[1] = 0x%x\n", nc->seed[1]);
			//spl_printf("seed[2] = 0x%x\n", nc->seed[2]);
			//spl_printf("seed[3] = 0x%x\n", nc->seed[3]);
			tmp2 = nc->seed[2] & 0xc000c000;
			tmp3 = nc->seed[3] & 0xc000c000;
			g_param = ((tmp2>>30 & 0x3)<<6 | (tmp2>>14 & 0x3)<<4 | (tmp3>>30 & 0x3)<<2 | (tmp3>>14 & 0x3)<<0);
			//spl_printf("g_param = 0x%x\n", g_param);

			/* H27UBG8T2BTR-BC, H27UCG8T2ATR-BC, M29F64G08CBABA-B K9GB(LC)G08U0B, SDTNQFAMA-004(8)G*/
			if((g_param & 0x3)== 0x3){
				g_retry_param_magic = 0x8a7a6a5a;
				//spl_printf("read retry find, g_param = 0x%x\n", g_param);
				imap_get_retry_param(nand_param);	
#if 0
				g_retrylevel = 6;	
				tmp = readl(RTC_INFO0) & 0xff;
				spl_printf("RTC_INFO0 = 0x%x\n", tmp);
				if(tmp == 0x71){
					nand_param[0] = readl(RTC_INFO1);
					nand_param[1] = readl(RTC_INFO2);
					nand_param[2] = readl(RTC_INFO3);
					nand_param[3] = readl(RTC_INFO5);
					spl_printf("nand param 0x%x, 0x%x, 0x%x, 0x%x\n", nand_param[0], nand_param[1], nand_param[2], nand_param[3]);
				}
				else{
					imap_get_retry_param(nand_param);	
					spl_printf("nand get param 0x%x, 0x%x, 0x%x, 0x%x\n", nand_param[0], nand_param[1], nand_param[2], nand_param[3]);
				}
#endif
			}	
		default:
			ret = irf->vs_assign_by_id(id, 1);
			if (ret)
			  return ret;

			if(id == DEV_NAND)
			  ret = nread_skip_bad(cfg, BL_LOC_CONFIG, ITEM_SIZE_NORMAL);
			else
			  ret = irf->vs_read(cfg, BL_LOC_CONFIG, ITEM_SIZE_NORMAL, 0);

			if (ret < 0)
			  return ret;
			len = ITEM_SIZE_NORMAL;
	}

	item_init(cfg, len);
	irf->free(cfg);
	return 0;
}

uint32_t try_wake(void)
{
	uint32_t tmp = 0;

	if(readl(SYS_RST_ST) & 0x2)
	{
		/* clear state off-hand */
		writel(3, SYS_RST_ST + 4);

		tmp   = readl(RTC_INFO3);
		tmp <<= 8;
		tmp  |= readl(RTC_INFO2);
		tmp <<= 8;
		tmp  |= readl(RTC_INFO1);
		tmp <<= 8;
		tmp  |= readl(RTC_INFO0);

		spl_printf("reset(0x%x)\n", tmp);

                if(tmp & 3) {
                    writel(0x0, RTC_INFO0);
                    g_skip_magic = CONFIG_SKIP_MAGIC;
                    writel(readl(RTC_INFO6) | 1, RTC_INFO6);
                    spl_printf("nPC: 0x%08x\n", tmp);
                    tmp = 0;
                }
	}

        return tmp;
}

void boot(void)
{
	__attribute__((noreturn)) void (*jump)(void);
	char buf[ITEM_MAX_LEN];
	const char *dev = buf;
	int ret, id;
	void *isi = (void *)(DRAM_BASE_PA + 0x8000);

	if(item_exist("board.disk"))
	  item_string(buf, "board.disk", 0);
	else {
		id = irf->boot_device();
		id = (id == DEV_BND)? DEV_NAND: id;
		dev = irf->vs_device_name(id);
	}

	ret = irf->vs_assign_by_name(dev, 1);
	if(ret) {
		spl_printf("assign device %s failed\n", dev);
		spl_printf("invoke iROM shell\n");
		goto __boot_failed__;
	}

	irf->memset(isi, 0, ISI_SIZE);

	if(irf->vs_is_device(DEV_CURRENT, "nnd"))
	  nread_skip_bad(isi, BL_LOC_UBOOT, ISI_SIZE);
	else
	  irf->vs_read(isi, BL_LOC_UBOOT, ISI_SIZE, 0);


	if(irf->isi_check_header(isi) != 0) {
		spl_printf("wrong uboot header: 0x%p\n", isi);
		goto __boot_failed__;
	}

	jump = (void *)isi_get_entry(isi);

	if(irf->vs_is_device(DEV_CURRENT, "nnd"))
	  nread_skip_bad((void *)(isi_get_load(isi) - ISI_SIZE),
				  BL_LOC_UBOOT, isi_get_size(isi) + ISI_SIZE);
	else
	  irf->vs_read((void *)(isi_get_load(isi) - ISI_SIZE),
				  BL_LOC_UBOOT, isi_get_size(isi) + ISI_SIZE, 0);

	/* FIXME: toggle PCLKM is fixed to divid-4 in iROM
	 * so the ECC clock can be configured to a very fast
	 * frequency here in SPL (SPL use iROM functions).
	 *
	 * so we update the frequency here again for u-boot
	 * usage. */
	irf->module_set_clock(NFECC_CLK_BASE, PLL_E, 1);

	/*
	 *  * Jump to U-Boot image
	 *   */
	spl_printf("jump\n");
        asm("dsb");
	(*jump)();

__boot_failed__:
	irf->cdbg_shell();
	for(;;);
}

//#define SPL_RESET_APLL

#if defined(SPL_RESET_APLL)
void set_clk_divider(int type, uint32_t val)    
{                                             
	uint32_t addr;

	if(type < 0 || type > 5)                  
	  return;                                 

	addr = CLK_SYSM_ADDR + 0x70 + 0x20 * type;

	writeb((val      ) & 0xff, addr + 0x0);         
	writeb((val >>  8) & 0xff, addr + 0x4);         
	writeb((val >> 16) & 0xff, addr + 0x8);         
	writeb((val >> 24) & 0xff, addr + 0xc);         
}                                             

static void reset_DIV(void)
{
	int cpu_pll = 1, div_len = 0xb;

	writeb(0, CLK_SYSM_ADDR + 0x64);                               

	set_clk_divider(0, 0x0000);                              
	set_clk_divider(1, 0x0555);                              
	set_clk_divider(2, 0x0387);                              
	set_clk_divider(3, 0x0555);                              
	set_clk_divider(4, 0x0410);                              
	set_clk_divider(5, 0x0e38);                              

	/* set length and enable the divider */                        
	writeb((div_len & 0x1f) | (1 << 7), CLK_SYSM_ADDR + 0x6c);

	/* set cpu to divide output */                                 
//	writeb(!cpu_pll, CLK_SYSM_ADDR + 0x68);                   

	/* enable load table */                                        
	writeb(1, CLK_SYSM_ADDR + 0x64);                               
}

struct FREQ_ST {
	uint32_t freq;
	uint32_t val;
};

static struct FREQ_ST fr_para[] = {
	{650,	0x1035}, // 648
	{700,	0x103a},
	{750,	0x103e},
	{792,	0x1041},
	{804,   0x1042}, // 804
	{850,   0x1046}, // 850
	{900,   0x104a}, // 900
	{936,   0x104d},
	{1008,  0x0029},
};

static void reset_APLL(uint32_t idx)
{
	writel(0x00, CLK_SYSM_ADDR + 0x0c);
#if 0	
	writel(0x4a, CLK_SYSM_ADDR + 0x00);
	writel(0x10, CLK_SYSM_ADDR + 0x04);
#else
	writel(fr_para[idx].val & 0xff, CLK_SYSM_ADDR + 0x00);
	writel((fr_para[idx].val>>8) & 0xff, CLK_SYSM_ADDR + 0x04);
#endif	
	writel(0x01, SYS_SYSM_ADDR + 0x18);

	while(readl(SYS_SYSM_ADDR + 0x18) & 0x1);
//	reset_DIV();
}
#endif

void clock_config(void)
{
    int i;

#if defined(SPL_RESET_APLL)
    uint32_t cpu_freq;

    if(item_exist("cpu.freq.max"))
    {
	    i = sizeof(fr_para)/sizeof(struct FREQ_ST);
	    cpu_freq = item_integer("cpu.freq.max", 0);
	    if(cpu_freq >= fr_para[0].freq && cpu_freq <= fr_para[i-1].freq) 
	    {
		    while(i--)
		    {
			    if(fr_para[i].freq <= cpu_freq)
				    break;
		    }
		    reset_APLL(i);
	    }
    }
#endif

    /* reconfig VPLL device to APLL,
     * leave VPLL alone for mid-wares
     */

    for(i = 1; i < 7; i++)
	irf->module_set_clock(BUS_CLK_BASE(i), PLL_A, 3);
    irf->module_set_clock(CRYPTO_CLK_BASE, PLL_A,  3); 
    irf->module_set_clock( SD_CLK_BASE(2), PLL_A, 19); 
   // irf->module_set_clock(ADC_CLK_BASE, 4, 0);

#if 0
#if (DRAM_FREQ == 266)
    irf->set_pll(PLL_V, 0x102b); // set VPLL to 533M
    irf->module_set_clock(DDRPHY_CLK_BASE, PLL_V, 1); 
#elif (DRAM_FREQ == 333)
    irf->set_pll(PLL_V, 0x1036); // set VPLL to 660M
    irf->module_set_clock(DDRPHY_CLK_BASE, PLL_V, 1); 
#elif (DRAM_FREQ == 400)
    irf->set_pll(PLL_V, 0x1042); // set VPLL to 804M
    irf->module_set_clock(DDRPHY_CLK_BASE, PLL_V, 1); 
#elif (DRAM_FREQ == 444)
    irf->set_pll(PLL_V, 0x1049); // set VPLL to 888M
    irf->module_set_clock(DDRPHY_CLK_BASE, PLL_V, 1); 
#elif (DRAM_FREQ == 533)
    irf->set_pll(PLL_V, 0x002b); // set VPLL to 1056M
    irf->module_set_clock(DDRPHY_CLK_BASE, PLL_V, 1); 
#elif (DRAM_FREQ == 492)
    irf->set_pll(PLL_V, 0x1051); // set VPLL to 1056M
    irf->module_set_clock(DDRPHY_CLK_BASE, PLL_V, 1); 
#elif (DRAM_FREQ == 504)
    irf->set_pll(PLL_V, 0x0029); // set VPLL to 1056M
    irf->module_set_clock(DDRPHY_CLK_BASE, PLL_V, 1); 
#elif (DRAM_FREQ == 480)
    irf->set_pll(PLL_V, 0x104f); // set VPLL to 1056M
    irf->module_set_clock(DDRPHY_CLK_BASE, PLL_V, 1); 
#elif (DRAM_FREQ == 200)
    irf->set_pll(PLL_V, 0x1031); // set VPLL to 600M
    irf->module_set_clock(DDRPHY_CLK_BASE, PLL_V, 2); 
#elif (DRAM_FREQ == 150)
    irf->set_pll(PLL_V, 0x1031); // set VPLL to 600M
    irf->module_set_clock(DDRPHY_CLK_BASE, PLL_V, 3); 
#else
#error Unsupported DDR frequency
#endif
#endif
    irf->module_set_clock(NFECC_CLK_BASE, PLL_E, 3);
}

void init_io(void)
{
	char *iob[] = {
		"initio.0", "initio.1", 
		"initio.2", "initio.3", 
		"initio.4", "initio.5", 
		"initio.6", "initio.7", 
		"initio.8", "initio.9"
	}, buf[64];
	int i, index, high, delay;

	for(i = 0; i < 10; i++) {
		if(item_exist(iob[i])) {
			item_string(buf, iob[i], 0);
			if(irf->strncmp(buf, "pads", 4) != 0)
				continue;
			index = item_integer(iob[i], 1);
			high  = item_integer(iob[i], 2);
			delay = item_integer(iob[i], 3);

			irf->pads_chmod(index, PADS_MODE_OUT, !!high);

			irf->printf("pads %d set to %d, delaying %dus\n",
					index, !!high, delay);
			irf->udelay(delay);
		} else
			break;
	}

	/* disable rtcgp3: pow_rnd pull down */
	writel(readl(RTC_GPPULL) | (1 << 3), RTC_GPPULL);
}

void boot_main(void)
{
	int ret;
        uint32_t tmp;

	// assign irom functions
	if(IROM_IDENTITY == IX_CPUID_820_0)
	  irf = &irf_2885;
	else if(IROM_IDENTITY == IX_CPUID_820_1)
	  irf = &irf_2939;
	else if(IROM_IDENTITY == IX_CPUID_X15)
	  irf = &irf_2974;
	else
	  // irom version can not be verified
	  while(1);
    /*
     * Add by peter Init ETMEN 
     */
    writel(0x7f,0x21E09C18);
    writel(0x0, 0x21E093C0);
    writel(0x0, 0x21E093C4);
    writel(0x0, 0x21E093C8);

	irf->cdbg_log_toggle(1);
	clock_config();

	irf->ss_jtag_en(1);
	irf->init_timer();
	irf->init_serial();

	irf->printf("\n");
	spl_printf("start\n");

	/* load and init configurations */
	init_config();

	init_io();
//	irf->printf("item: TEST.\n");
//	item_list(NULL);
//	init_mux(); // this should be in uboot1
        asm("dsb");

	tmp = try_wake();
	ret = dramc_init(irf, !!tmp);
        asm("dsb");
	if(ret) {
		spl_printf("dram error\n");
		irf->cdbg_shell();
		for(;;);
	}

        if(tmp) {
            /* 
             * set watchdog up, if 16s later, and
             * system is not resume, system will reboot 
             * time can be set in 1s, 2s, 4s, 8s, 16s 
             */
            if (item_integer("board.wdt", 0) != 0) {
                irf->module_enable(WDT_SYSM_ADDR);
                set_up_watchdog(16);
            }

            __attribute__((noreturn)) void (*wake)(void) = (void *)tmp;
            spl_printf("wake1\n");
            wake();
        }

                writel(g_skip_magic, CONFIG_RESERVED_SKIP_BUFFER);
        writel(dram_size_check(), CONFIG_RESERVED_SKIP_BUFFER + 8);


	param_transmit_to_uboot1();

	writel(irf->get_xom(1), CONFIG_RESERVED_SKIP_BUFFER + 4);
	boot();
}

void hang(void) {
h:
	goto h;
}

