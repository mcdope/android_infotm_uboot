/***************************************************************************** 
** infotm/drivers/pmu/pmu_axp202.c 
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
** 1.0  11/27/2012  Jcak Zhang   
*****************************************************************************/ 

#include <common.h>
#include <asm/io.h>
#include <lowlevel_api.h>
#include <efuse.h>
#include <preloader.h>
#include <items.h>
#include "pmu_axp202.h"

#define AXP202_BAT_DBG
#ifdef AXP202_BAT_DBG
#define AXP202_LOG   printf
#else
#define AXP202_LOG
#endif

#define AXP202_MODE0 0
#define AXP202_MODE1 1
#define AXP202_MODE2 2
#define AXP202_MODE3 3
#define AXP202_MODE4 4
#define AXP202_MODE5 5
#define AXP202_MODE_NULL 6

#define AXP_IX_CPUID_820_0		0x3c00b02c
#define AXP_IX_CPUID_820_1		0x3c00b030
#define AXP_IX_CPUID_X15		0x3c00aca0

#define AXP202_X15_DRVBUS 54

extern void gpio_reset(void);
extern void gpio_mode_set(int, int);
extern void gpio_dir_set(int, int);
extern void gpio_output_set(int, int);
extern void gpio_pull_en(int, int);

extern uint8_t iicreg_write(uint8_t iicindex, uint8_t *subAddr, uint32_t addr_len, uint8_t *data, uint32_t data_num, uint8_t isstop);
extern uint8_t iicreg_read(uint8_t iicindex, uint8_t *data, uint32_t num);

extern int axp202_get_voltage(void);

/* 0 - error */
int axp202_init_ready(int board)
{
	/* imapx15 */
	uint8_t buf[2], addr;
	__u32 tn, to, tst;
	__u32 val;

	/* output 1 in pin COND18*/
	gpio_reset();
	if(board == AXP202_MODE2)
	{
		gpio_pull_en(0, AXP202_X15_DRVBUS);
	}
	
	/* init iic */
	if((board == AXP202_MODE3)||(board == AXP202_MODE4)||(board == AXP202_MODE5))
	{
		if(iic_init(0, 1, 0x68, 0, 1, 0) == 0)
		{
			AXP202_LOG("[AXP202]: iic_init() fail!!!\n");
			goto err;
		}
		AXP202_LOG("[AXP202]: iic_init() success!!!\n");
	}
	
	#if 0
	/* 
		check battery state:
			bit 5: 0-not exist, 1-exist
	*/
	
	// close charger fisrt
	addr = 0x33;
	buf[0] = 0x42;
	iicreg_write(0, &addr, 0x01, buf, 0x01, 0x01);

	//delay 1ms
	udelay(10000);

	
	// check battery state
	addr = 0x01;
	buf[0] = 0x00;
	iicreg_write(0, &addr, 0x01, buf, 0x00, 0x01);
	buf[0] = 0;
	iicreg_read(0, buf, 0x01);
	AXP202_LOG("check battery state: iicreg_read() buf[0] is 0x%x\n", buf[0]);

	axp202_get_voltage();
	#if 0
	if(buf[0] & 0x20)
	{
		/* battery exist, limit input current 500mA */
		BATT_AXP202_LOG("[AXP202]: battery is exist!\n");
		
		gpio_mode_set(1, 61);
		gpio_dir_set(1, 61);
		gpio_output_set(1, 61);

	}
	else
	{
		/* no battery, input current is no limit */
		BATT_AXP202_LOG("[AXP202]: battery is none!\n");
		
		addr = 0x30;
		buf[0] = 0x83;
		iicreg_write(0, &addr, 0x01, buf, 0x01, 0x01);
		
	}
	
	
	BATT_AXP202_LOG("[AXP202]: iic_read_func() success!!!, buf = 0x%d\n", buf[0]);

	return buf[0];
	#endif
	#endif
	return 1;
err:
	return 0;
}

void axp202_supply_input_mode(int mode)
{
	uint8_t addr, buf[2], pin;
	int cpu_id;

	cpu_id = readl(0x04000000 + 0x48);
	if(cpu_id == AXP_IX_CPUID_X15)
	{
		pin = 61;
	}
	else
	{
		pin = 78;
		printf("cpu is not x15, set pin num 78\n");
	}
	
	if(mode == 0)// 100mA
	{
		addr = 0x30;
		buf[0] = 0x82;
		//iicreg_write(0, &addr, 0x01, buf, 0x01, 0x01);
		AXP202_LOG("set vbus_in max current 100mA\n");
	}
	else if(mode == 1)// 500mA
	{
		addr = 0x30;
		buf[0] = 0x81;
		//iicreg_write(0, &addr, 0x01, buf, 0x01, 0x01);
		AXP202_LOG("set vbus_in max current 500mA\n");
	}
	else if(mode == 2)// 900mA
	{
		addr = 0x30;
		buf[0] = 0x80;
		//iicreg_write(0, &addr, 0x01, buf, 0x01, 0x01);
		AXP202_LOG("set vbus_in max current 900mA\n");
	}
	else if(mode == 3)// no limit
	{
		addr = 0x30;
		buf[0] = 0x83;
		//iicreg_write(0, &addr, 0x01, buf, 0x01, 0x01);
		AXP202_LOG("set vbus_in no limit\n");
	}
	else// close
	{
		gpio_mode_set(1, pin);
		gpio_dir_set(1, pin);
		gpio_output_set(1, pin);
		AXP202_LOG("set vbus_in close\n");
	}
}

int axp202_get_voltage(void)
{
	/* 1.1mV/step */
	uint8_t addr, high, low;
	int adc;
	
	addr = 0x78;
	iicreg_write(0, &addr, 0x01, &high, 0x00, 0x01);
	iicreg_read(0, &high, 0x1);

	addr = 0x79;
	iicreg_write(0, &addr, 0x01, &low, 0x00, 0x01);
	iicreg_read(0, &low, 0x1);
	low &= 0x0f;

	adc = ((high << 4)|low);
	adc = adc * 11 / 10;
	AXP202_LOG("axp202 get voltage is %d\n", adc);

	return adc;
}
