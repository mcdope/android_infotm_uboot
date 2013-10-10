/***************************************************************************** 
** infotm/drivers/adc/batt_imapx800.c 
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
** 1.0  11/15/2012  Jcak Zhang   
*****************************************************************************/ 
#include <common.h>
#include <asm/io.h>
#include <lowlevel_api.h>
#include <efuse.h>
#include <preloader.h>
#include <items.h>

extern uint8_t iic_init(uint8_t iicindex, uint8_t iicmode, uint8_t devaddr, uint8_t intren, uint8_t isrestart, uint8_t ignoreack);
extern uint8_t iic_read_func(uint8_t iicindex, uint8_t *subAddr, uint32_t addr_len, uint8_t *data, uint32_t data_num);

#define BATT_DBG_OUT
#ifdef BATT_DBG_OUT
#define BATT_LOG   printf
#else
#define BATT_LOG
#endif

#define PWRON 0
#define PWROFF 1

#define PMU_NULL 0
#define PMU_UNKNOWN 1
#define PMU_TPS65910 2
#define PMU_AXP152 3
#define PMU_AXP202 4

#define AXP202_MODE0 0 //i7av2
#define AXP202_MODE1 1 //i7av2 modify
#define AXP202_MODE2 2 //i7av3
#define AXP202_MODE3 3 //ghy81_2
#define AXP202_MODE4 4 // jbsm75d_1
#define AXP202_MODE5 5 // jbsm75d_2
#define AXP202_MODE_NULL 6

#define AP_IX_CPUID_820_0		0x3c00b02c
#define AP_IX_CPUID_820_1		0x3c00b030
#define AP_IX_CPUID_X15		0x3c00aca0

struct batt_init{
	int batt_v_start;
	int batt_pwr_st;

	int pmu_model;
	int board_name;

	int charger_enable;
	int charger_pwron;

	int ap_version;
};

struct batt_init batt_item;

extern void adc_init(void);
extern int battery_get_value(void);
extern int axp202_init_ready(int);
extern void axp202_supply_input_mode(int);
extern int axp202_get_voltage(void);
extern int set_boot_type(int type);

void batt_item_init(void)
{
	BATT_LOG("batt_item_init() run \n");
	/* get item: pmu model */
	if(item_exist("pmu.model"))
	{
		if(item_equal("pmu.model", "axp202", 0))
		{
			batt_item.pmu_model = PMU_AXP202;
			BATT_LOG("PMU_AXP202 \n");
		}
		else if(item_equal("pmu.model", "tps65910", 0))
		{
			batt_item.pmu_model = PMU_TPS65910;
			BATT_LOG("PMU_TPS65910 \n");
		}
		else if(item_equal("pmu.model", "axp152", 0))
		{
			batt_item.pmu_model = PMU_AXP152;
			BATT_LOG("PMU_AXP152 \n");
		}
		else
		{
			batt_item.pmu_model = PMU_UNKNOWN;
			BATT_LOG("PMU_UNKNOWN \n");
		}
	}
	else
	{
		batt_item.pmu_model = PMU_NULL;
		BATT_LOG("PMU_NULL \n");
	}

	/* get item: start voltage */
	if(item_exist("batt.start"))
	{
		batt_item.batt_v_start = item_integer("batt.start", 0);
	}
	else
	{
		batt_item.batt_v_start = 3400;
	}
	BATT_LOG("batt_item.batt_v_start = %d \n", batt_item.batt_v_start);

	/* get item: charger mode */
	if(item_exist("charger.enable"))
	{
		if(item_equal("charger.enable", "1", 0))
		{
			batt_item.charger_enable = 1;
		}
		else
		{
			batt_item.charger_enable = 0;
		}
	}
	else
	{
		batt_item.charger_enable = 0;
	}
	
	if(item_exist("charger.pwron"))
	{
		if(item_equal("charger.pwron", "1", 0))
		{
			batt_item.charger_pwron = 1;
		}
		else
		{
			batt_item.charger_pwron = 0;
		}
	}
	else
	{
		batt_item.charger_pwron = 0;
	}
	BATT_LOG("charger_pwron = %d, charger_enable = %d \n", batt_item.charger_pwron, batt_item.charger_enable);

	/* get item: charger pattern */
	if(item_exist("charger.pattern"))
	{
		if(item_equal("charger.pattern", "mode0", 0))
		{
			batt_item.board_name = AXP202_MODE0;
			BATT_LOG("AXP202_MODE0 \n");
		}
		else if(item_equal("charger.pattern", "mode1", 0))
		{
			batt_item.board_name = AXP202_MODE1;
			BATT_LOG("AXP202_MODE1 \n");
		}
		else if(item_equal("charger.pattern", "mode2", 0))
		{
			batt_item.board_name = AXP202_MODE2;
			BATT_LOG("AXP202_MODE2 \n");
		}
		else if(item_equal("charger.pattern", "mode3", 0))
		{
			batt_item.board_name = AXP202_MODE3;
			BATT_LOG("AXP202_MODE3 \n");
		}
		else if(item_equal("charger.pattern", "mode4", 0))
		{
			batt_item.board_name = AXP202_MODE4;
			BATT_LOG("AXP202_MODE4 \n");
		}
		else if(item_equal("charger.pattern", "mode5", 0))
		{
			batt_item.board_name = AXP202_MODE5;
			BATT_LOG("AXP202_MODE5 \n");
		}
		else
		{
			batt_item.board_name = AXP202_MODE_NULL;
			BATT_LOG("AXP202_MODE_NULL \n");
		}
	}
	else
	{
		batt_item.board_name = AXP202_MODE_NULL;
		BATT_LOG("AXP202_MODE_NULL \n");
	}

	/* get cpu id */
	batt_item.ap_version= readl(0x04000000 + 0x48);
	if(batt_item.ap_version == AP_IX_CPUID_X15)
	{
		BATT_LOG("CPU IS IMAPX15 \n");
	}
	else if(batt_item.ap_version == AP_IX_CPUID_820_1)
	{
		BATT_LOG("CPU IS IMAPX820_1 \n");
	}
	else if(batt_item.ap_version == AP_IX_CPUID_820_0)
	{
		BATT_LOG("CPU IS IMAPX820_0 \n");
	}
	else
	{
		BATT_LOG("CPU IS UNKNOWN \n");
	}
		
	BATT_LOG("batt_item_init() end \n");
}

int batt_get_pwrstate(void)
{
	int temp;
	temp = readl(SYS_POWUP_ST);
	BATT_LOG("SYS_POWUP_ST = 0x%x", temp);
	temp &= 0x02;
	BATT_LOG("SYS_POWUP_ST = 0x%x, after & 0x02\n");
	return temp;
}

int battery_get_voltage(int adc)
{
	int voltage;
	if((batt_item.board_name == AXP202_MODE3)||\
		(batt_item.board_name == AXP202_MODE4)||\
		(batt_item.board_name == AXP202_MODE5))
	{
		BATT_LOG("board.name is 7053/GHY81/JBSM75D, Using axp202's adc for getting battery voltage\n");
		voltage = axp202_get_voltage();
		BATT_LOG("battery_voltage = %d \n");
		return voltage;
	}

	//other is tps65910 or axp202
	BATT_LOG("Using ap's adc for getting battery voltage!\n");
	voltage = adc * 5000/1024;
	BATT_LOG("battery_voltage = %d \n", voltage);
	return voltage;
}

#if 0
int battery_get_voltage(int adc)
{
	int voltage;
	voltage = adc * 5000/1024;
	BATT_LOG("battery_voltage= %d \n", voltage);
	return voltage;
}
#endif

int battery_rtc_gpio_cfg(void)
{
	int reg_dat;
		
	/* cfg GP2 input, dis pull-down, mask int */
	reg_dat = readl(RTC_GPDIR);
	reg_dat |= 0x04;
	writel(reg_dat, RTC_GPDIR);
}

int battery_get_pwrkey(void)
{
	/*1- push, 0-release */
	int tmp;

	tmp = readl(RTC_GPINPUT_ST);
	BATT_LOG("battery_get_pwrkey() RTC_GPINPUT_ST = 0x%x \n", tmp);
	if(batt_item.ap_version == AP_IX_CPUID_X15)
	{
		tmp &= 0x40;
		BATT_LOG("battery_get_pwrkey() RTC_GPINPUT_ST = 0x%x after &0x40\n", tmp);
	}
	else/* imapx8x0 */
	{
		tmp &= 0x10;
		BATT_LOG("battery_get_pwrkey() RTC_GPINPUT_ST = 0x%x after &0x10\n", tmp);
	}
	
	return tmp;
}

int battery_get_charger(void)
{
	/* use GP2 for charger detect */
	/* 1- charger in, 0- charger out */
	int tmp;

	tmp = readl(RTC_GPINPUT_ST);
	BATT_LOG("get battery_get_charger() RTC_GPINPUT_ST = 0x%x \n", tmp);	
	tmp &= 0x04;
	BATT_LOG("battery_get_charger() RTC_GPINPUT_ST = 0x%x after &0x04\n", tmp);

	return tmp;
}

int battery_charger_mask(void)
{
	writel(0x2, RTC_POWMASK); //unmask charing
}

int battery_charger_unmask(void)
{
	writel(0x0, RTC_POWMASK); //unmask charing
}

void battery_pmu_model_err(void)
{
	if(batt_get_pwrstate())
	{
		//charger on start
		while(battery_get_charger() == 1);
		battery_charger_unmask();
	}
	else
	{
		//pwrkey start
		while(battery_get_pwrkey());

		if((batt_item.charger_enable == 1)||(batt_item.charger_pwron == 1))
		{
			battery_charger_unmask();
		}		
	}
}

int batt_main(int pwron_st)
{
	int battery_voltage, pwron;
	int key_st = 0;
	int chg_st = 0;

	BATT_LOG("batt_main() \n");

	batt_item_init();
	batt_item.batt_pwr_st = pwron_st;

	if(*(uint32_t *)CONFIG_RESERVED_SKIP_BUFFER == CONFIG_SKIP_MAGIC)
	{
		// it's now run autoreboot or other
		BATT_LOG("start from autoreboot!\n");
		return PWRON;
	}

	/* ADC INIT */
	adc_init();
	
	/* PMU INIT */
	if(batt_item.pmu_model == PMU_AXP202)
	{
		if(axp202_init_ready(batt_item.board_name))
		{
			battery_voltage = battery_get_voltage(battery_get_value());
			BATT_LOG("battery_voltage is %d\n", battery_voltage);
			if(battery_voltage < 1000)
			{
				BATT_LOG("battery_voltage < 1000, set mode 3\n");
				udelay(1000);
				axp202_supply_input_mode(3);
				BATT_LOG("using pmu axp202, we will into factory mode\n");
				return PWRON;
			}
			else
			{
				if(batt_item.board_name == AXP202_MODE0)
				{
					BATT_LOG("set mode 4, please check start voltage, board.name is I7AV2\n");
					axp202_supply_input_mode(4);//close
				}
			}
		}
		else
		{
			BATT_LOG("axp202 can't be initialized by ap or communicated with ap by i2c");
			return PWROFF;
		}
	}
	else if(batt_item.pmu_model == PMU_AXP152)
	{
		;
	}
	else if(batt_item.pmu_model == PMU_TPS65910)
	{
		;
	}
	else
	{
		BATT_LOG("pmu.model is exist, but not set the pmu supported\n");
		BATT_LOG("only for debug, or system error\n");
		return PWRON;
	}

	/* RTC GP2 INPUT INIT */
	battery_rtc_gpio_cfg();

	/* CHECK SYSTEM POWON ST */
	//pwron = batt_get_pwrstate();
	batt_item.batt_pwr_st &= 0x02;
	BATT_LOG("power on state is 0x%x\n", batt_item.batt_pwr_st);
	if(batt_item.batt_pwr_st)/*charger in power on */
	{
		BATT_LOG("START: Charger input \n");
		
		battery_voltage = battery_get_voltage(battery_get_value());
		if(battery_voltage > batt_item.batt_v_start)
		{
			BATT_LOG("battery_voltage > batt_item.batt_v_start\n");
			if((batt_item.charger_enable == 1)||(batt_item.charger_pwron == 1))
			{
				BATT_LOG("charger.enable value is 1\n");
				return PWRON;
			}

			//A. check pwrkey
			//B. if pwrkey push(return 0), return PWRON
			//C. mask charger pwron, and pwroff
			if(battery_get_pwrkey())
			{
				return PWRON;
			}

			battery_charger_mask();
			return PWROFF;
		}	
		else
		{
			/* battery voltage is too lower, can't power on*/
			BATT_LOG("battery_voltage <= batt_item.batt_v_start\n");
			while(1)
			{
				if(battery_get_charger() == 0)
				{
					if((batt_item.charger_enable == 1)||(batt_item.charger_pwron == 1))
					{
						BATT_LOG("charger.pwron value is 1\n");
						battery_charger_unmask();
					}
					else
					{
						battery_charger_mask();
					}
					return PWROFF;
				}
				else
				{
					if(battery_get_pwrkey())
					{
						return PWRON;
					}
					else
					{
						battery_voltage = battery_get_voltage(battery_get_value());
						if(battery_voltage >= batt_item.batt_v_start)
						{
							return PWRON;
						}
					}
				}
			}
		}
	}
	else/* power key power on */
	{
		BATT_LOG("START: Push PwrKey \n");
		
		battery_voltage = battery_get_voltage(battery_get_value());
		if(battery_voltage >= batt_item.batt_v_start)
		{
			BATT_LOG("battery_voltage >= batt_item.batt_v_start\n");
			return PWRON;
		}
		else
		{
			BATT_LOG("battery_voltage < batt_item.batt_v_start\n");
			while(1)
			{
				BATT_LOG("The lowest voltage for pwring system is %d, the battery voltage is %d\n", batt_item.batt_v_start, battery_voltage);
				//A. check powkey state
				//B. if powkey release, goto poweroff
				//C. else goto A
				chg_st = battery_get_charger();
				key_st = battery_get_pwrkey();
				if(chg_st)
				{
					BATT_LOG("get charger input state, goto pwron\n");
					return PWRON;
				}
				else
				{
					if(key_st == 0)
					{
						if((batt_item.charger_enable == 1)||(batt_item.charger_pwron == 1))
						{
							battery_charger_unmask();
						}
						else
						{
							battery_charger_mask();
						}
						return PWROFF;
					}
				}
			}
		}
	}
}

void batt_shutdown(void)
{
		/*shut down */
		writel(0xff, RTC_SYSM_ADDR + 0x28);
    		writel(0, RTC_SYSM_ADDR + 0x7c);
    		writel(0x2, RTC_SYSM_ADDR);

    		asm("wfi");
}
