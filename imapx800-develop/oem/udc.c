/*
 * (C) Copyright 2006-2008
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>
#include <oem_func.h>

#ifdef CONFIG_NAND_SPL
#include "preloader.h"
#endif

#define OTG_WAIT	100

#define PAGE_HEAD	16


static unsigned int g_wValue,g_bRequest,g_bmRequestType,g_wLength,g_wIndex;
static unsigned int device_descbuff[18];
static unsigned int config_descbuff[16];
static unsigned int string_descbuff[20];



static unsigned int usbsetconfiguration; 
static unsigned int curr_num;
// download data num
static unsigned int total_num;
// download address = run address
static unsigned int run_addr;
// image run flag
static unsigned char run_flag;
static unsigned int down_address;


/* The followint variables and externsions are not
 * for preloader.
 */
#ifndef CONFIG_NAND_SPL
// image type, 1 = nk.nb0  2 = uboot.bin
static unsigned char page_type;
// image next link, 1 = next  0 = null
static unsigned char page_link;
// sys_param size
static unsigned char sysparam_size;
//static unsigned int  reserve2;
static unsigned char burn_factor;	


/*
 * 0: usb otg ---> initialize 
 * 1: usb otg ---> attach to pc
 * 2: usb otg ---> uboot down complete
 * 3: usb otg ---> uboot burn complete
 * 4: usb otg ---> iMage down complete
 * 5: usb otg ---> iMage burn complete
 * 6: usb otg ---> download success
 * 7: usb otg ---> download error
 */
static unsigned int gOtgRetValue;
static unsigned int usb_process_state;

static unsigned char sysparam[256];

/*
 *  function declaration
 */
int usb_otg_attach(void);
void data_stage(void);
void load_usb_descriptors(void);
int build_communication(void);
unsigned int download_data(void);
void usb_otg_init(void);
unsigned int SendUsbState(void);
void CloseOTGVCC(void);
int oem_otg_check(void);
int oem_otg_server(void);
#else	/* CONFIG_NAND_SPL */
static unsigned char pcResetSign;
static void (*run)(void);
#endif


unsigned int otg_dma_op(unsigned int buff,unsigned int len,unsigned int dir,unsigned int ep)
{
	unsigned int vt;

	writel(buff,OTGS_MDAR);
	vt = len<<16;
	//1: host--->slave  0:slave--->host
	if(dir)
	{
		vt |= 0x12;
	}
	else
	{
		if(ep == 0)
			vt |= 0x22;
		else if(ep == 1)
			vt |=0x122;

		clean_cache((uint32_t)buff,(uint32_t)(buff+len));
	}
	writel(vt,OTGS_ACR);
	udelay(10);
	while(1)
	{
		vt = readl(OTGS_ACR);
		if((vt & 0x830) == 0)
			break;
		else if((vt & 0x830) == 0x800)
		{
			//translation err
			vt |= 0x800;
			writel(vt,OTGS_ACR);
			writel(0x10000,OTGS_FHHR);
			writel(0x1,OTGS_BFCR);
			return 0;
		}
	}
	if(dir)
	{
		inv_cache((uint32_t)buff,(uint32_t)(buff+len));
	}
	return 1;
}

#define CHECKOUT_CNT	(20)
int usb_otg_attach(void)
{
	uint32_t vt, i, count;

	udelay(60000);
	count = 0;
#ifdef CONFIG_OTG_GPIO	
#ifdef CONFIG_OTG_EX_GPIO
	vt = readl(CONFIG_OTG_EX_GPIO);
	if((!(vt & (0x1<< CONFIG_OTG_EX_GPIO_NUM)))
#ifndef CONFIG_NAND_SPL
		|| (*(uint16_t*)CONFIG_SYS_PHY_BOOT_STAT == CONFIG_BOOTSTAT_USB)
#endif
	  )
	{
#else	
	vt = readl(OTGL_BCSR);
	if(vt & 0x2000)
	{
#endif	
#elif CONFIG_OTG_HW_KEY
	vt = readl(CONFIG_OTG_HW_KEY + 4);
	vt &= ~(3 << (CONFIG_OTG_HW_KEY_NUM * 2));
	writel(vt, CONFIG_OTG_HW_KEY + 4);

	vt = readl(CONFIG_OTG_HW_KEY);
	if((!(vt & (1 << CONFIG_OTG_HW_KEY_NUM))))
	{
#else
	for(i=0; i<CHECKOUT_CNT; i++)
	{
		vt = readl(OTGL_BCWR);
		vt |= 0x1;
		writel(vt,OTGL_BCWR);
		udelay(100);

		vt = readl(OTGL_BCSR);
		if(!(vt & 0x80))
			count++;

		vt = readl(OTGL_BCWR);
		vt &= ~(0x1);
		writel(vt,OTGL_BCWR);
		udelay(100);
	}

	if((count > CHECKOUT_CNT - 2)
#ifndef CONFIG_NAND_SPL
	   || (*(uint16_t*)CONFIG_SYS_PHY_BOOT_STAT == CONFIG_BOOTSTAT_USB)
#endif
	  )
	{
#endif
		vt = readl(OTGL_BCWR);
		vt |= 0x800;
		writel(vt,OTGL_BCWR);
		for(i=0;i<OTG_WAIT;i++)
		{
			if(readl(OTGL_BCSR) & 0x10)
				break;
			udelay(10);
		}
		if(i < OTG_WAIT)
		{
			// otg connect
			vt = readl(OTGL_BCWR);
			vt &= ~ 0x800;
			writel(vt,OTGL_BCWR);

			vt = 0x0;
			writel(vt,OTGS_TBCR0);
			
			vt = 0x10;
			writel(vt,OTGS_TBCR1);
			
			vt = 0x110;
			writel(vt,OTGS_TBCR2);

			vt = 0x190;
			writel(vt,OTGS_TBCR3);

			vt = 0x0;
			writel(vt,OTGS_ACR);
			
			vt = 0x030f381f;
			writel(vt,OTGS_IER);

			vt = 0x0;
			writel(vt,OTGS_FNCR);

			vt = readl(OTGS_UDCR);
			vt |= 0x70001;
			writel(vt,OTGS_UDCR);
			return 1;
		}
	}
	return 0;
}

void data_stage(void)
{
	unsigned int pdescaddr;
	unsigned int des_datalen =0;


	//control endpoint0 IN transaction
	if(g_bRequest == 0x6)
	{
		//GET_DESCRIPTOR
		if(g_wValue == 0x100)
		{
			//set device descriptor
			pdescaddr = (unsigned int)device_descbuff;
			(g_wLength<18)?(des_datalen=g_wLength):(des_datalen=18);
		}
		else if(g_wValue == 0x200)
		{
			//set configuration decsriptor
			pdescaddr = (unsigned int)config_descbuff;
			(g_wLength<32)?(des_datalen=g_wLength):(des_datalen=32);
		}
		else if(g_wValue == 0x300)
		{
			pdescaddr = (unsigned int)(&string_descbuff[0]);
			(g_wLength<4)?(des_datalen=g_wLength):(des_datalen=4);
		}
		else if(g_wValue == 0x301)
		{
			pdescaddr = (unsigned int)(&string_descbuff[2]);
			(g_wLength<0x16)?(des_datalen=g_wLength):(des_datalen=0x16);
		}
		else if(g_wValue == 0x302)
		{
			pdescaddr = (unsigned int)(&string_descbuff[9]);
			(g_wLength<0x2c)?(des_datalen=g_wLength):(des_datalen=0x2c);
		}
		else{
			//unsupport
			writel(0x10000,OTGS_FHHR);
			writel(0x1,OTGS_BFCR);
			return;
		}
#ifdef CONFIG_NAND_SPL		
		otg_dma_op(pdescaddr+0x3e000000,des_datalen,0,0);
#else
		otg_dma_op(pdescaddr,des_datalen,0,0);
#endif		
	}
}

void load_usb_descriptors(void)
{

	writel(0x400000,OTGS_EDR0);
	writel(0x14000019,OTGS_EDR1);
	writel(0x4000021,OTGS_EDR2);

	device_descbuff[0]     = 0x02000112;
	device_descbuff[1]     = 0x400000FF;
	device_descbuff[2]     = 0x12345345;
	device_descbuff[4]     = 0x00000100;
	device_descbuff[5]     = 0x00000000;
	device_descbuff[6]     = 0x00000000;
	device_descbuff[7]     = 0x00000000;
	device_descbuff[8]     = 0x0110060a;
	device_descbuff[9]     = 0x08000000;
	device_descbuff[10]    = 0xbeaf0001;
	device_descbuff[11]    = 0x00010090;
	device_descbuff[12]    = 0x00000100;
	device_descbuff[13]    = 0x00000000;
	device_descbuff[14]    = 0x00000000;
	device_descbuff[15]    = 0x00000000;

	config_descbuff[0]  = 0x00200209; //0x00200209
	config_descbuff[1]  = 0xc0000101; //0xc0010101
	config_descbuff[2]  = 0x00040919; //0x00040932
	config_descbuff[3]  = 0x00ff0200; //0x06080200
	config_descbuff[4]  = 0x05070000; //0x05070050
	config_descbuff[5]  = 0x04000281; //0x00400281
	config_descbuff[6]  = 0x02050700; //0x02050700
	config_descbuff[7]  = 0x00020002; //0x00004002
	config_descbuff[8]  = 0x03830507; //0x03830507
	config_descbuff[9]  = 0x00010008; //0x00010008
	config_descbuff[10] = 0x00000000; //0x00000000
	config_descbuff[11] = 0x00000000; //0x00000000
	config_descbuff[12] = 0x00000000; //0x00000000
	config_descbuff[13] = 0x00000000; //0x00000000
	config_descbuff[14] = 0x00000000; //0x00000000
	config_descbuff[15] = 0x00000000; //0x00000000
	
	string_descbuff[0]    = 0x04090304; // String0 Descriptor
	string_descbuff[1]    = 0x00000000;
	string_descbuff[2]    = 0x00530316; // String1 Descriptor
	string_descbuff[3]    = 0x00730079; //'y' 0 's' 0
	string_descbuff[4]    = 0x00650074; //'t' 0 'e' 0
	string_descbuff[5]    = 0x0020006d; //'m' 0 ' ' 0
	string_descbuff[6]    = 0x0043004d;  //'M' 0 'C' 0
	string_descbuff[7]    = 0x00500055; // 'U' 0
	string_descbuff[8]    = 0x00000000;
	string_descbuff[9]    = 0x0053032c; // String2 Descriptor 'S' 0
	string_descbuff[10]   = 0x00430045; //'E' 0 'C' 0
	string_descbuff[11]   = 0x00530020; //' ' 0 'S' 0
	string_descbuff[12]   = 0x00430033; //'3' 0 'C' 0
	string_descbuff[13]   = 0x00340032; //'2' 0 '4' 0
	string_descbuff[14]   = 0x00300031; //'1' 0 '0' 0
	string_descbuff[15]   = 0x00200058; //'X' 0 ' ' 0 
	string_descbuff[16]   = 0x00650054; //'T' 0 'e' 0 
	string_descbuff[17]   = 0x00740073; //'s' 0 't' 0 
	string_descbuff[18]   = 0x00420020; //' ' 0 'B' 0 
	string_descbuff[19]   = 0x0044002F; //'/' 0 'D' 0 

}

int build_communication(void)
{
	//control translation
	volatile unsigned int dc_isr;

	dc_isr = readl(OTGS_ISR);
	if((dc_isr) && (dc_isr & readl(OTGS_IER)))
	{
		if(dc_isr & 0x4)
		{
			//usb reset event
			load_usb_descriptors();
			writel(0x4,OTGS_ISR);
#ifdef CONFIG_NAND_SPL
			pcResetSign = 1;
#endif
		}
		else if(dc_isr & 0x10000)
		{
			unsigned int vt;
			//control endpoint0 SETUP
			vt = readl(OTGS_STR0);
			g_wValue = vt;
			g_wValue = ((g_wValue>>16) & 0xffff);
			g_bRequest = vt;
			g_bRequest = ((g_bRequest>>8) & 0xff);
			g_bmRequestType = vt;
			g_bmRequestType &= 0xff;

			vt = readl(OTGS_STR1);
			g_wLength = vt;
			g_wLength = ((g_wLength>>16) & 0xffff);
			g_wIndex = vt;
			g_wIndex = (g_wIndex>>8)&0xff;

			writel(0x10000,OTGS_ISR);
		}
		else if(dc_isr & 0x40000)
		{
			//control endpoint0 IN transaction
			data_stage();
#ifndef CONFIG_NAND_SPL
			if(g_bRequest == 0x6 && g_wValue == 0x200 && g_wLength > 0xff)
				usbsetconfiguration = 1;
#endif
			writel(0x40000,OTGS_ISR);
		}
#ifdef CONFIG_NAND_SPL
		else if(dc_isr & 0x1000000)
		{
			//physical endpoint1 transaction
			// ACK for usb host request
			unsigned int ackrequest;
			ackrequest = 1;
			otg_dma_op((unsigned int)&ackrequest,sizeof(ackrequest),0,1);
			writel(0x1000000,OTGS_ISR);
			usbsetconfiguration = 1;
		}
#endif
		else 
		{
			writel(dc_isr,OTGS_ISR);
		}
		udelay(100);
	}
	else
	{
		udelay(10000);
	}
	return 1;
}


unsigned int download_data(void)
{
	unsigned int dc_isr = 0;
	unsigned int data_len,i;
	static unsigned int err_cn=0;
	unsigned int tmp;

#define ERR_MAX   0x20000
	do
	{	
		dc_isr = readl(OTGS_ISR);
		if(dc_isr && (dc_isr&readl(OTGS_IER)))
		{
			//printf("t:0x%x\n",dc_isr);	
			if(dc_isr & 0x2000000)
			{
				err_cn = 0;
				//physical endpoint2 transaction
				data_len = readl(OTGS_PRIR) & 0x7ff;
				if(!otg_dma_op(down_address+curr_num,data_len,1,2))
					goto err;
				if(curr_num == 0)
				{
					run_addr = *(unsigned int*)(down_address);
					total_num = *(unsigned int*)(down_address+4);
					run_flag = *(unsigned char*)(down_address+8);
#ifndef CONFIG_NAND_SPL
					page_type = *(unsigned char*)(down_address+9);
					page_link = *(unsigned char*)(down_address+10);
					sysparam_size = *(unsigned char*)(down_address+11);
					burn_factor = *(unsigned char*)(down_address+12);
					printf("run_add:0x%x, total_num:0x%x run_flag:0x%x, page_type:0x%x page_link:0x%x sysparam_size:0x%x burn_factor:0x%x\n",
							run_addr,total_num,run_flag,page_type,page_link,sysparam_size,burn_factor);
					if(run_flag == 3)
					{
						for(i=0;i<sysparam_size;i++)
							sysparam[i] = *(unsigned char*)(down_address+PAGE_HEAD+i);
						tmp = data_len-PAGE_HEAD-sysparam_size;
						for(i=0;i<tmp;i++)
						{
							*(unsigned char*)(run_addr+i) = *(unsigned char*)(down_address+PAGE_HEAD+sysparam_size+i);
						}
						down_address = run_addr-PAGE_HEAD-sysparam_size;

					}
					else
#else
					if(run_flag == 2)
					  break;

					if(run_addr == 0xffffffff)
					  run_addr = CONFIG_SYS_PHY_UBOOT_BASE;
					if(run_addr != (down_address + PAGE_HEAD))
#endif
					{
						tmp = data_len-PAGE_HEAD;
						for(i=0;i<tmp;i++)
						{
							*(unsigned char*)(run_addr+i) = *(unsigned char*)(down_address+PAGE_HEAD+i);
						}
						down_address = run_addr-PAGE_HEAD;

					}
				}
				curr_num += data_len;
			}
#ifndef CONFIG_NAND_SPL
			else if(dc_isr & 0x1000000)
			{
				//physical endpoint1 transaction
				// ACK for usb host request
				gOtgRetValue = (gOtgRetValue)|(usb_process_state<<24);
				printf("ack for usb host request! gOtgRetValue: 0x%x state: 0x%x \n",gOtgRetValue,usb_process_state);
				if(!otg_dma_op((unsigned int)&gOtgRetValue,sizeof(gOtgRetValue),0,1))
				{
					writel(dc_isr,OTGS_ISR);
					goto err;
				}
			}
#endif
			writel(dc_isr,OTGS_ISR);
			udelay(10);
		}
		else
		{
			//if(curr_num)
			err_cn++;
			udelay(1000);
		}
	}while((curr_num<total_num) && (err_cn < ERR_MAX));
#ifndef CONFIG_NAND_SPL
	printf("download_data complete \n");
#endif
	if(err_cn>=ERR_MAX)
		goto err;
	return 1;
err:
	return 0;	
}

void usb_otg_init(void)
{
	unsigned int val;

	//initail globe variant
	curr_num = 0;
	total_num = 1;	
   	usbsetconfiguration = 0;
	run_addr = 0x0;
	//down_address = CONFIG_SYS_PHY_UBOOT_BASE-PAGE_HEAD;
	down_address = CONFIG_SYS_PHY_UBOOT_SWAP-PAGE_HEAD;
	run_flag = 0;

#ifdef CONFIG_NAND_SPL
	pcResetSign = 0;
#else
	gOtgRetValue = 2;
	usb_process_state = 0;
#endif

	/* No need init GPIOs if spl already done this */
#ifdef CONFIG_NAND_SPL
#ifdef CONFIG_OTG_GPIO
#   ifdef CONFIG_OTG_EX_GPIO
	val = readl(CONFIG_OTG_EX_GPIO + 4);
	val &=~(0x3<<(CONFIG_OTG_EX_GPIO_NUM * 2));
	writel(val,CONFIG_OTG_EX_GPIO+4);

	val = readl(CONFIG_OTG_EX_GPIO + 8);
#       ifdef CONFIG_OTG_MENUKEY
	val &=~(0x1<<(CONFIG_OTG_EX_GPIO_NUM));
#       else
	val |= (0x1<<(CONFIG_OTG_EX_GPIO_NUM));
#       endif
	writel(val,CONFIG_OTG_EX_GPIO + 8);
#   endif

	val = readl(CONFIG_OTG_GPIO + 4);
	val &=~(0x3<<(CONFIG_OTG_GPIO_NUM * 2));
	val |= (0x1<<(CONFIG_OTG_GPIO_NUM * 2));
	writel(val,CONFIG_OTG_GPIO + 4);

	val = readl(CONFIG_OTG_GPIO);
#   if defined(CONFIG_OTG_EX_GPIO)
	val &= ~(0x1<<(CONFIG_OTG_GPIO_NUM));
#   else
	val |= (0x1<<(CONFIG_OTG_GPIO_NUM));
#   endif
	writel(val,CONFIG_OTG_GPIO);
#endif
#endif /* CONFIG_NAND_SPL */

	//config epll for usb
	val = readl(DIV_CFG2);
	val &=~0x7f0000;
	val |= 0x260000;
	writel(val,DIV_CFG2);

	//set usb gate
	val = readl(SCLK_MASK);
	val&=~0x2030;
	writel(val,SCLK_MASK);

	//set usb power
	val = readl(PAD_CFG);
	val &=~0xe;
	writel(val,PAD_CFG);
	writel(0x0,USB_SRST);
	//for(i=0;i<1000;i++);
	udelay(100);
	val |= 0xe;
	writel(val,PAD_CFG);
	udelay(4000);
	writel(0x5,USB_SRST);
	//for(i=0;i<1000;i++);
	udelay(200);
	writel(0xf,USB_SRST);
}

#ifndef CONFIG_NAND_SPL
int oem_otg_check(void)
#else
int usb_otg_check(void)
#endif
{
	int state = 0;
	volatile int i;

#define OTG_CLC_CN	0x3000
	usb_otg_init();
	if(usb_otg_attach())
	{
#ifndef CONFIG_NAND_SPL
		printf("build_communication init\n");
#endif
		//otg port attach pc
		for(i=0;i<OTG_CLC_CN;i++)
		{
			build_communication();
			if(usbsetconfiguration)
			{
				udelay(100);
#ifndef CONFIG_NAND_SPL
				//otg attach to pc
				gOtgRetValue = 0;
				usb_process_state = 1; 
#endif
				break;
			}
#ifdef CONFIG_NAND_SPL
			if(i>0x800 && pcResetSign==0)
				break;
#endif
		}
		if(i<OTG_CLC_CN)
			state = 1;
	}
#ifndef CONFIG_NAND_SPL
	if(state == 0)
	{
		writel(0x0,USB_SRST);
	}
#endif
//	printf("oem_otg_check end: %d\n",state);
	return state;
}

#ifdef CONFIG_NAND_SPL
int usb_otg_load(void)
{
	if(download_data())
	{
		if(run_flag == 1)
		{
			run = (void (*)(void))(run_addr);
			run();
		}
	}
	return 0;
}
#else /* NOT SPL */
unsigned int SendUsbState()
{
	unsigned int dc_isr = 0;

	do
	{	
		dc_isr = readl(OTGS_ISR);
		if(dc_isr && (dc_isr&readl(OTGS_IER)))
		{
			if(dc_isr & 0x1000000)
			{
				//physical endpoint1 transaction
				// ACK for usb host request
				gOtgRetValue = (gOtgRetValue)|(usb_process_state<<24);
				printf("ack for usb host request! gOtgRetValue: 0x%x state: 0x%x \n",gOtgRetValue,usb_process_state);
				if(!otg_dma_op((unsigned int)&gOtgRetValue,sizeof(gOtgRetValue),0,1))
				{
					writel(dc_isr,OTGS_ISR);
					break;
				}
			}
			writel(dc_isr,OTGS_ISR);
			udelay(10);
		}
	}while((1));// && (err_cn < ERR_MAX));
	return 1;
}

void CloseOTGVCC()
{
	unsigned int val;

	val = readl(GPIDAT);
	val &=~(0x1<<6);
	writel(val,GPIDAT);

}

int oem_otg_server()
{
	int ret;
	printf("oem_otg_server init\n");
	do{
		// init globle value
		curr_num = 0;
		total_num = 1;	
		run_addr = 0x0;
		//down_address = CONFIG_SYS_PHY_UBOOT_BASE-PAGE_HEAD;
		down_address = CONFIG_SYS_PHY_UBOOT_SWAP-PAGE_HEAD;
		run_flag = 0;
		
		if(download_data())
		{
			if(run_flag == 3)
			{
				oem_bootl((char *)&sysparam[0]);	
			}
			if(page_type == 1)
			{
				gOtgRetValue = 0;
				usb_process_state = 4; 
				//download nk.nb0
				printf("oem_burn_NK start\n");
				ret = oem_burn_NK((uint8_t *)run_addr) ;
				printf("oem_burn_NK end: 0x%x\n",ret);
				gOtgRetValue = 0;
				usb_process_state = 5; 
			}
			else if(page_type == 2)
			{
				gOtgRetValue = 0;
				usb_process_state = 2; 
				//download uboot.bin
				printf("oem_burn_uboot start\n");
				ret = oem_burn_uboot((uint8_t *)run_addr);
				printf("oem_burn_uboot end: 0x%x\n",ret);
				gOtgRetValue = 0;
				usb_process_state = 3; 
			}
			else if(page_type == 3)
			{
				gOtgRetValue = 0;
				usb_process_state = 20;
				//download linux-kernel
				printf("oem_burn_kernel start\n");
				ret = oem_burn_LK((uint8_t*)run_addr);
				printf("oem_burn_kernel end: 0x%x\n",ret);
			}
			else if(page_type == 4)
			{
				gOtgRetValue = 0;
				usb_process_state = 21;
				printf("oem_burn_sys start\n");
#ifndef	CONFIG_SYS_DISK_iNAND
				ret = oem_burn_SYS((uint8_t*)run_addr,(total_num-18));
#else
				ret = oem_burn_zAS((uint8_t*)run_addr);
#endif
				printf("oem_burn_sys end: 0x%x\n",ret);
			}
			else if(page_type == 5)
			{
				gOtgRetValue = 0;
				usb_process_state = 22;
				printf("oem_burn_udat start\n");
				ret = oem_burn_UDAT((uint8_t*)run_addr,(total_num-18));
				printf("oem_burn_udat end: 0x%x\n",ret);
			}	
			else if(page_type == 6)
			{
				gOtgRetValue = 0;
				usb_process_state = 24;
				printf("oem_burn_ndisk start\n");
				ret = oem_burn_NDISK((uint8_t*)run_addr,(total_num-18));
				printf("oem_burn_ndisk end: 0x%x\n",ret);
			}
			else if(page_type == 7)
			{
				gOtgRetValue = 0;
				usb_process_state = 25;
				printf("oem_burn_rdisk1 start\n");
				ret = oem_burn_RD((uint8_t*)run_addr);
				printf("oem_burn_rdisk1 end: 0x%x\n",ret);
			}
			else if(page_type == 8)
			{
				gOtgRetValue = 0;
				usb_process_state = 26;
				printf("oem_burn_rdisk2 start\n");
				ret = oem_burn_RD_((uint8_t*)run_addr);
				printf("oem_burn_rdisk2 end: 0x%x\n",ret);
			}
			else if(page_type == 9)
			{
				gOtgRetValue = 0;
				usb_process_state = 27;
				printf("oem_burn_U0 start\n");
				ret = oem_burn_U0((uint8_t*)run_addr);
				printf("oem_burn_U0_end: 0x%x\n",ret);
			}
			else if(page_type == 10)
			{
				gOtgRetValue = 0;
				usb_process_state = 28;
				printf("oem_burn_zAS start\n");
//				ret = oem_burn_zAS((uint8_t*)run_addr);

				/* iNAND do not support burn zSYS directly
				 * zSYS.img is only used in ius.
				 */
				ret = 0;
				printf("oem_burn_zAS_end: 0x%x\n",ret);
			}
			else if(page_type == 11)
			{
				gOtgRetValue = 0;
				usb_process_state = 29;
				printf("oem_burn_iuw start\n");
				ret = iuw_update((uint8_t*)run_addr);
				printf("oem_burn_iuw_end: 0x%x\n",ret);
#ifdef CONFIG_SYS_DISK_iNAND
			} else if(page_type == 12){
				gOtgRetValue = 0;
				usb_process_state = 29;
				printf("oem_burn_UPDT start\n");
				ret = oem_burn_UPDT((uint8_t*)run_addr);
				printf("oem_burn_UPDT end: 0x%x\n",ret);
#endif
			}
		}
		else
		{
			// download error
			writel(0x0,USB_SRST);
			return 0;
		}	
	}
	while(page_link);
	if(burn_factor)
		oem_disk_clear();
	printf("oem factory check start");
	ret = oem_factory_check();
	// usb download failed
	gOtgRetValue = ret;
	usb_process_state = 6;
	printf("oem factory check111 end:0x%x",ret);
	SendUsbState();
	return 1;
}
#endif
