#include <common.h> 
#include "imapx_iic.h"
#include <asm/io.h>
#include <lowlevel_api.h>
#include <efuse.h>

#define IIC_CLK	     320000
static uint8_t iicreadapistate;

#define IIC_IX_CPUID_820_0		0x3c00b02c
#define IIC_IX_CPUID_820_1		0x3c00b030
#define IIC_IX_CPUID_X15		0x3c00aca0

extern void gpio_mode_set(int, int);

uint8_t iic_init(uint8_t iicindex, uint8_t iicmode, uint8_t devaddr, uint8_t intren, uint8_t isrestart, uint8_t ignoreack)
{
	int ret;

	iicreadapistate = READ_DATA_FIN;	

	module_enable(I2C_SYSM_ADDR);
	
	ret = readl(0x04000000 + 0x48);
	if(ret == IIC_IX_CPUID_X15)
	{
		printf("cpu is imapx15\n");
		gpio_mode_set(0, 43);
		gpio_mode_set(0, 44);
	}
	else
	{
		printf("cpu is imapx820\n");
		pads_chmod(PADSRANGE(54, 55), PADS_MODE_CTRL, 0);
		if(ecfg_check_flag(ECFG_ENABLE_PULL_IIC))
	  		pads_pull(PADSRANGE(54, 55), 1, 1);
	}

#if 0
	temp_reg = readl(GPCCON);                   
	temp_reg &= ~(0x3<<6 | 0x3<<4 | 0x3<<2 | 0x3);
	writel(temp_reg,GPCCON);                    
		                                              
	temp_reg = readl(GPCCON);                   
	temp_reg |= (0x2<<6 | 0x2<<4 | 0x2<<2 | 0x2);
	writel(temp_reg, GPCCON);                   
#endif


	ret = iicreg_init(iicindex,iicmode,devaddr,intren,isrestart,ignoreack);

	return ret;
}
uint8_t iicreg_init(uint8_t iicindex, uint8_t iicmode, uint8_t devaddr, uint8_t intren, uint8_t isrestart, uint8_t ignoreack)
{
	uint32_t temp_reg, clk;

	temp_reg = module_get_clock(APB_CLK_BASE);
	clk = temp_reg / IIC_CLK / 2;

	if(iicindex != 0 && iicindex != 1) return FALSE;
	writel(0x0,IC_ENABLE);
	writel((STANDARD | MASTER_MODE | SLAVE_DISABLE),IC_CON);
	writel(clk, IC_SS_SCL_HCNT);
	writel(clk, IC_SS_SCL_LCNT);
	writel(clk/2,IC_IGNORE_ACK0);  
	temp_reg = readl(IC_IGNORE_ACK0);
	temp_reg &= ~(0x1<<8);
	writel(temp_reg,IC_IGNORE_ACK0);
	temp_reg = readl(IC_CON);
	temp_reg |= IC_RESTART_EN;
	writel(temp_reg,IC_CON);
	writel(0x0,IC_INTR_MASK);      
	writel(devaddr>>1,IC_TAR);
	writel(0x1,IC_ENABLE);

	return TRUE;
}

uint8_t iic_writeread(uint8_t iicindex, uint8_t subAddr, uint8_t * data)
{
	if(iicreadapistate == WRITE_ADDR_FIN || iicreadapistate == READ_DATA_FLS)
		goto READ_BEGIN;
	if(iicreg_write(iicindex, &subAddr, 1, NULL, 0, 0)) {
		iicreadapistate = WRITE_ADDR_FIN;
READ_BEGIN:
		if(iicreg_read(iicindex, data, 1))
		{
			iicreadapistate = READ_DATA_FIN;
			return TRUE;
		}
		else
		{
			iicreadapistate = READ_DATA_FLS;
			return FALSE;
		}
	}
	else
	{
		iicreadapistate = WRITE_ADDR_FLS;
		return FALSE;
	}
}

uint8_t iic_read_func(uint8_t iicindex, uint8_t *subAddr, uint32_t addr_len, uint8_t *data, uint32_t data_num)
{
	if(iicreadapistate == WRITE_ADDR_FIN || iicreadapistate == READ_DATA_FLS) 
		goto READ_BEGIN;
	if(iicreg_write(iicindex, subAddr, addr_len, NULL, 0, 1)) {
		iicreadapistate = WRITE_ADDR_FIN;
READ_BEGIN:
		if(iicreg_read(iicindex, data, data_num)) {
			iicreadapistate = READ_DATA_FIN;
			return TRUE;
		} else {
			iicreadapistate = READ_DATA_FLS;
			return FALSE;
		}
	} else {
		iicreadapistate = WRITE_ADDR_FLS;
		return FALSE;
	}
}


uint8_t iicreg_write(uint8_t iicindex, uint8_t *subAddr, uint32_t addr_len, uint8_t *data, uint32_t data_num, uint8_t isstop) {
	uint32_t temp_reg;
	int i;
	uint32_t num = addr_len + data_num;
	if(iicindex != 0 && iicindex != 1) return FALSE;
	temp_reg = readl(IC_RAW_INTR_STAT);
	if(temp_reg & TX_ABORT)
	{
		readl(IC_TX_ABRT_SOURCE);
		readl(IC_CLR_TX_ABRT);
		return FALSE;
	}
	temp_reg = readl(IC_STATUS);
	if(temp_reg & (0x1<<2)) {
		for(i=0;i<num;i++)
		{
		        while(1) {
		                temp_reg = readl(IC_STATUS);
		                if(temp_reg & (0x1<<1))
		                {
				        if (addr_len) {
//					    printf(":02%x\n", *subAddr);
	 			        	writel(*subAddr,IC_DATA_CMD);
	                                	subAddr++;
						addr_len--;
					} else if (data_num) {
					    	writel(*data, IC_DATA_CMD);
						data++;
						data_num--;
					}
	                        	break;
			         }
		       }
		}
	
		while(1) {
		        temp_reg = readl(IC_STATUS);
		        if (temp_reg & (0x1<<2))
		        {
				if (isstop) {
		                	while(1) {
		                        	temp_reg = readl(IC_RAW_INTR_STAT);
		                        	if (temp_reg & (0x1<<9)) {
		                                	readl(IC_CLR_STOP_DET);
		                                	udelay(6500);
		                                	return TRUE;
		                        	}
					}
		                } else 
					return TRUE;
			}
		}
	}
	return 0;
}

uint8_t iicreg_read(uint8_t iicindex, uint8_t *data, uint32_t num) {
	uint32_t temp_reg;
	int readnum = 0;
	int writenum = 0;

	if(iicindex != 0 && iicindex != 1) return FALSE;
	writenum = num;
	temp_reg = readl(IC_STATUS);
	while(temp_reg & (0x1<<3))
	{
		temp_reg = readl(IC_DATA_CMD);
	}
	while (1) {
	    temp_reg = readl(IC_STATUS);
	    if (temp_reg & (0x1 << 1) && writenum) {
	        writel(0x1<<8,IC_DATA_CMD);
		writenum--;
		}
	    if (temp_reg & (0x1 << 3)) {
		*data = readl(IC_DATA_CMD);
		data++;
		readnum++;
	    }
	    if (readnum == num)
		break;
#if 0
		to = get_ticks();
	        while(1) {
	        	temp_reg = readl(IC_STATUS);
		        if (temp_reg & (0x1<<3)) {
				data[i]= readl(IC_DATA_CMD);
      	                        break;
		        }

			tn = get_ticks();
			if ((tn - to) > 600000) {
				return false;
			}

		}
#endif
	}

	return TRUE;
}


