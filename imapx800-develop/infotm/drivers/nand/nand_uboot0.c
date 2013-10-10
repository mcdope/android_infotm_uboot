
#include <common.h>
#include <nand.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <lowlevel_api.h>
#include <preloader.h>
#include <hash.h>

extern struct irom_export *irf;
uint32_t otp_start;
uint32_t g_2atr_magic;
uint8_t retry_reg_buf[8];
uint8_t eslc_reg_buf[4];

extern uint32_t g_retrylevel_max;
extern uint32_t g_param;
extern uint8_t nand_param[1026];
extern uint8_t nand_param_copy[128];
extern uint32_t g_retrylevel; 
extern uint32_t g_retry_param_magic;


unsigned char buf_crd_param19 [63] = {
	0x00, 0x00, 0x00,
	0xf0, 0xf0, 0xf0,
	0xef, 0xe0, 0xe0,
	0xdf, 0xd0, 0xd0,
	0x1e, 0xe0, 0x10,
	0x2e, 0xd0, 0x20,
	0x3d, 0xf0, 0x30,
	0xcd, 0xe0, 0xd0,
	0x0d, 0xd0, 0x10,
	0x01, 0x10, 0x20,
	0x12, 0x20, 0x20,
	0xb2, 0x10, 0xd0,
	0xa3, 0x20, 0xd0,
	0x9f, 0x00, 0xd0,
	0xbe, 0xf0, 0xc0,
	0xad, 0xc0, 0xc0,
	0x9f, 0xf0, 0xc0,
	0x01, 0x00, 0x00,
	0x02, 0x00, 0x00,
	0x0d, 0xb0, 0x00,
	0x0c, 0xa0, 0x00,
};

unsigned char buf_crd_param21 [66] = { 
	0xa1, 0x00, 0xa7, 0xa4, 0xa5, 0xa6,
	0x00, 0x00, 0x00, 0x00,
	0x05, 0x0a, 0x00, 0x00,
	0x28, 0x00, 0xec, 0xd8,
	0xed, 0xf5, 0xed, 0xe6,
	0x0a, 0x0f, 0x05, 0x00,
	0x0f, 0x0a, 0xfb, 0xec,
	0xe8, 0xef, 0xe8, 0xdc,
	0xf1, 0xfb, 0xfe, 0xf0,
	0x0a, 0x00, 0xfb, 0xec,
	0xd0, 0xe2, 0xd0, 0xc2,
	0x14, 0x0f, 0xfb, 0xec,
	0xe8, 0xfb, 0xe8, 0xdc,
	0x1e, 0x14, 0xfb, 0xec,
	0xfb, 0xff, 0xfb, 0xf8,
	0x07, 0x0c, 0x02, 0x00,
};

void param_transmit_to_uboot1(void){
	
	uint8_t * param;
	uint32_t *retry_level;
	uint32_t *retry_param_used;
	int i = 0;

	retry_level = (uint32_t *)(CONFIG_RESERVED_RRTB_BUFFER + 0x4);
	retry_param_used = (uint32_t *)(CONFIG_RESERVED_RRTB_BUFFER + 0x8);

	if(g_param == 0x3){
		param = (uint8_t *)(CONFIG_RESERVED_RRTB_BUFFER);
		*(param + 0) = nand_param[0];
		*(param + 1) = nand_param[1];
		*(param + 2) = nand_param[2];
		*(param + 3) = nand_param[3];
	}
	if(g_param == 0x7 || g_param == 0xf){
		param = (uint8_t *)(CONFIG_RESERVED_RRTB_BUFFER + 0x100);
		for(i = 0; i<64; i++){
			*(param + i) = nand_param[otp_start + i];
		}
	}
	if(g_param == 0xb || g_param == 0x13 || g_param == 0x17){		//do nothing
	
	}
	//writel(g_retrylevel, RTC_INFO0);
	*retry_level = g_retrylevel;
	*retry_param_used = g_retry_param_magic;
	//spl_printf("uboot0: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", *(param + 0), *(param + 1), *(param + 2), *(param + 3), *retry_level);
}

int trx_afifo_read_state(int index)
{
	switch(index)
	{	
	case 0:
		return readl(NF2STSR0);
	case 1:
		return readl(NF2STSR1);
	case 2:
		return readl(NF2STSR2);
	case 3:
		return readl(NF2STSR3);
	}
	return -1;	
}

void trx_afifo_nop (unsigned int flag, unsigned int nop_cnt){
	unsigned int value = 0;

	value = ((flag & 0xf)<<10) | (nop_cnt & 0x3ff);
	writel(value, NF2TFPT);
}

void trx_afifo_ncmd (unsigned int flag, unsigned int type, unsigned int wdat)
{
	unsigned int value = 0;
	
	value = (0x1<<14) | ((flag & 0xf) <<10) | (type<<8) | wdat;
	writel(value, NF2TFPT);
}

int nf2_soft_reset(int num)
{
    volatile unsigned int tmp = 0;
    int ret = 0;

    writel(num, NF2SFTR);
    while(1)
    {
    	//TODO add time out check	    
    	tmp = readl(NF2SFTR) & 0x1f;
    	if(tmp == 0)break;
    }

    return ret;
}    

int imap_get_retry_param_21(unsigned char *buf)
{
	int i = 0;
	/*set retry param by willie*/  
	for(i=0; i<66; i++)
	{
		buf[i] = buf_crd_param21[i];
	}

	return 0;
}
int imap_get_retry_param_26(unsigned char *buf){

	int tmp = 0;
	int ret = 0;
	int retry_param = 0;
	
	nf2_soft_reset(1);
	
	writel(0x1, NF2FFCE);     // allow write trx-afifo
	
	trx_afifo_ncmd(0x0, 0x0, 0x37);    // set param CMD0
	trx_afifo_ncmd(0x0, 0x1, 0xa7);    // ADDRS
	trx_afifo_ncmd(0x0, 0x3, 0x00);    // data
	trx_afifo_ncmd(0x0, 0x1, 0xad);    // ADDRS
	trx_afifo_ncmd(0x0, 0x3, 0x00);    // data
	trx_afifo_ncmd(0x0, 0x1, 0xae);    // ADDRS
	trx_afifo_ncmd(0x0, 0x3, 0x00);    // data
	trx_afifo_ncmd(0x0, 0x1, 0xaf);    // ADDRS
	trx_afifo_ncmd(0x0, 0x3, 0x00);    // data
	
	writel(0x0, NF2FFCE);     // disable write trx-afifo
	
	writel(0xe, NF2CSR);    // cs0 valid
	writel(0x1, NF2STRR);   // start trx-afifo
	
    	while(1)
    	{
    		//TODO add time out check	    
    		tmp = readl(NF2INTR) & 0x1;
    		if(tmp == 1)break;
    	}

	writel(0xf, NF2CSR);    // cs0 invalid
    	writel(0xffffff, NF2INTR);	// clear status

	retry_param = trx_afifo_read_state(0);
	buf[0] = (retry_param >> 24) & 0xff;
	buf[1] = (retry_param >> 16) & 0xff;
	buf[2] = (retry_param >> 8) & 0xff;
	buf[3] = (retry_param >> 0) & 0xff;
	
	return ret;

}

int imap_set_retry_param_26(int retrylevel, unsigned char * buf){

	int tmp = 0;
	int ret = 0;
	unsigned char buf_crd[] = {
		0x36, 0xa7, 0xad, 0xae, 0xaf, 0x16, 
		buf[0],	buf[1], 	buf[2], 	buf[3],
	       	buf[0], buf[1] + 0x06, 	buf[2] + 0x0a, 	buf[3] + 0x06, 
		0x00, 	buf[1] - 0x3, 	buf[2] - 0x7, 	buf[3] - 0x8, 
		0x00, 	buf[1] - 0x6, 	buf[2] - 0xd, 	buf[3] - 0xf,
		0x00, 	buf[1] - 0x9, 	buf[2] - 0x14, 	buf[3] - 0x17, 
		0x00, 	0x00, 		buf[2] - 0x1a, 	buf[3] - 0x1e,
		0x00, 	0x00, 		buf[2] - 0x20, 	buf[3] - 0x25,
	};	//store cmd & register addr & data

	nf2_soft_reset(1);

    	writel(0x1, NF2FFCE);     // allow write trx-afifo

	trx_afifo_ncmd(0x0, 0x0, buf_crd[0]);    		  // set param CMD0
	trx_afifo_ncmd(0x0, 0x1, buf_crd[1]);    		  // ADDRS
	trx_afifo_ncmd(0x0, 0x2, buf_crd[retrylevel * 4 + 6]);    // data
	trx_afifo_ncmd(0x0, 0x1, buf_crd[2]);    		  // ADDRS
	trx_afifo_ncmd(0x0, 0x2, buf_crd[retrylevel * 4 + 7]);    // data
	trx_afifo_ncmd(0x0, 0x1, buf_crd[3]);   	 	  // ADDRS
	trx_afifo_ncmd(0x0, 0x2, buf_crd[retrylevel * 4 + 8]);    // data
	trx_afifo_ncmd(0x0, 0x1, buf_crd[4]);    		  // ADDRS
	trx_afifo_ncmd(0x0, 0x2, buf_crd[retrylevel * 4 + 9]);    // data
	trx_afifo_ncmd(0x0, 0x0, buf_crd[5]);    		  // set param CMD1

    	writel(0x0, NF2FFCE);     // disable write trx-afifo

    	writel(0xe, NF2CSR);	// cs0 valid
    	writel(0x1, NF2STRR);	// start trx-afifo
    	
    	while(1)
    	{
    		//TODO add time out check	    
    		tmp = readl(NF2INTR) & 0x1;
    		if(tmp == 1)break;
    	}
	
	writel(0xf, NF2CSR);    // cs0 invalid
    	writel(0xffffff, NF2INTR);	// clear status
	
	return ret;

}

int init_retry_param_sandisk19(){
	int i = 0 , tmp = 0;

	nf2_soft_reset(1);
	writel(0x1, NF2FFCE);     // allow write trx-afifo
	
	trx_afifo_ncmd(0x0, 0x0, 0xB6);
	trx_afifo_ncmd(0x0, 0x0, 0x3b);
	trx_afifo_ncmd(0x0, 0x0, 0xb9);

	for(i=4; i<13; i++){
		trx_afifo_ncmd(0x0, 0x0, 0x53);
		trx_afifo_ncmd(0x0, 0x1, i);    // set ADDR
		trx_afifo_ncmd(0x0, 0x2, 0x0);
	}
	
	writel(0x0, NF2FFCE);

	writel(0xe, NF2CSR);    // cs0 valid
	writel(0x1, NF2STRR);   // start trx-afifo

	while(1)
	{
		//TODO add time out check           
		tmp = readl(NF2INTR) & 0x1;
		if(tmp == 1)break;
	}

	writel(0xf, NF2CSR);    // cs0 invalid
	writel(0xffffff, NF2INTR);      // clear status

	return 0;
}

int imap_set_retry_param_sandisk19(int retrylevel){
	int i = 0, tmp = 0;
	int ret = 0;

	if(retrylevel == 1)                            
		init_retry_param_sandisk19();

	nf2_soft_reset(1);
	
	writel(0x1, NF2FFCE);     // allow write trx-afifo
	
	trx_afifo_ncmd(0x0, 0x0, 0x3b);
	trx_afifo_ncmd(0x0, 0x0, 0xb9);
	for(i=0; i<3; i++){
		trx_afifo_ncmd(0x0, 0x0, 0x53);
		if(i == 2)
			trx_afifo_ncmd(0x0, 0x1, 0x04 + i + 1);    // set ADDR ff
		else
			trx_afifo_ncmd(0x0, 0x1, 0x04 + i);    // set ADDR ff
		trx_afifo_ncmd(0x0, 0x2, buf_crd_param19[retrylevel * 3 + i]);    // set data
	}
	
	if(retrylevel == 0)              
		trx_afifo_ncmd(0x0, 0x0, 0xD6);    

	writel(0x0, NF2FFCE);     // disable write trx-afifo

    	writel(0xe, NF2CSR);	// cs0 valid
    	writel(0x1, NF2STRR);	// start trx-afifo
    	
    	while(1)
    	{
    		//TODO add time out check	    
    		tmp = readl(NF2INTR) & 0x1;
    		if(tmp == 1)break;
    	}
	
	writel(0xf, NF2CSR);    // cs0 invalid
    	writel(0xffffff, NF2INTR);	// clear status

	return ret;
}

int imap_set_retry_param_21(int retrylevel, unsigned char * buf){

	int tmp = 0;
	int ret = 0;
	int i = 0;
	nf2_soft_reset(1);

    	writel(0x1, NF2FFCE);     // allow write trx-afifo

//	spl_printf("retrylevel %d \n", retrylevel);

	for(i=0; i<4; i++)
	{
		trx_afifo_nop(0x0, 0x64);       //3ns * 0x64 = 300ns
		trx_afifo_ncmd(0x0, 0x0, buf[0]);
		trx_afifo_ncmd(0x0, 0x2, buf[1]);
		trx_afifo_ncmd(0x0, 0x2, buf[i + 2]);
		trx_afifo_ncmd(0x0, 0x2, buf[(retrylevel + 1)*4 + i + 2]);
	}

    	writel(0x0, NF2FFCE);     // disable write trx-afifo

    	writel(0xe, NF2CSR);	// cs0 valid
    	writel(0x1, NF2STRR);	// start trx-afifo
    	
    	while(1)
    	{
    		//TODO add time out check	    
    		tmp = readl(NF2INTR) & 0x1;
    		if(tmp == 1)break;
    	}
	
	writel(0xf, NF2CSR);    // cs0 invalid
    	writel(0xffffff, NF2INTR);	// clear status

	return ret;
}
int imap_get_retry_param_20(unsigned char *buf, int len, int param){
	
	int val = 0;
	int ret = 0;
	int tmp = 0;
	
	nf2_soft_reset(1);
	
	val  = (3<<8);
    	writel(val, NF2ECCC);
    
	val  = (0<<16) | (len);
	writel(val, NF2PGEC);
	
	writel((int)buf, NF2SADR0);
	writel(len, NF2SBLKS);
   
	val = (1024<<16) | 1;
       	writel(val, NF2SBLKN);	
    
	val = 1;
	writel(val, NF2DMAC);
	
	
	writel(0x1, NF2FFCE);     // allow write trx-afifo

	trx_afifo_ncmd(0x4, 0x0, 0xff);    // set CMD ff, check rnb
	trx_afifo_ncmd(0x0, 0x0, 0x36);    // set CMD 36
	if(param == 0x7){
		trx_afifo_ncmd(0x0, 0x1, 0xff);    // set ADDR ff
		trx_afifo_ncmd(0x0, 0x2, 0x40);    // write data 40
		trx_afifo_ncmd(0x0, 0x1, 0xcc);    // set ADDR cc
	}
	if(param == 0xf){
		trx_afifo_ncmd(0x0, 0x1, 0xae);    // set ADDR ae
		trx_afifo_ncmd(0x0, 0x2, 0x00);    // write data 00
		trx_afifo_ncmd(0x0, 0x1, 0xb0);    // set ADDR b0
	}
	trx_afifo_ncmd(0x0, 0x2, 0x4d);    // write data 4d
	trx_afifo_ncmd(0x0, 0x0, 0x16);    // set CMD 16
	trx_afifo_ncmd(0x0, 0x0, 0x17);    // set CMD 17
	trx_afifo_ncmd(0x0, 0x0, 0x04);    // set CMD 04
	trx_afifo_ncmd(0x0, 0x0, 0x19);    // set CMD 19
	trx_afifo_ncmd(0x0, 0x0, 0x00);    // set CMD 00
	trx_afifo_ncmd(0x0, 0x1, 0x00);    // set ADDR 00
	trx_afifo_ncmd(0x0, 0x1, 0x00);    // set ADDR 00
	trx_afifo_ncmd(0x0, 0x1, 0x00);    // set ADDR 00
	trx_afifo_ncmd(0x0, 0x1, 0x02);    // set ADDR 02
	trx_afifo_ncmd(0x0, 0x1, 0x00);    // set ADDR 00
	trx_afifo_ncmd(0x1, 0x0, 0x30);    // set CMD 30, check rnb and read page data
	trx_afifo_ncmd(0x4, 0x0, 0xff);    // set CMD ff, check rnb
	trx_afifo_ncmd(0x4, 0x0, 0x38);    // set CMD 38, check rnb
	
	writel(0x0, NF2FFCE); // disable write trx-afifo

    	writel(0xe, NF2CSR);	// cs0 valid
    	writel(0x5, NF2STRR);	// start trx-afifo
    	while(1)
    	{
    		//TODO add time out check	    
    		tmp = readl(NF2INTR) & (0x1<<5);
    		if(tmp == (0x1<<5))break;
    	}
	
	writel(0xf, NF2CSR);    // cs0 invalid
    	writel(0xffffff, NF2INTR);	// clear status
	
	return ret;

}

int imap_get_retry_param_20_from_page(int page, unsigned char *buf, int len){

	int val = 0;
	int ret = 0;
	int tmp = 0;
	int cycle = 5;
	int ecc_info = 0;
	int i = 0;
	
	for(i=0; i<4; i++){
		nf2_soft_reset(1);

		val  = (3<<8) | (15<<4) | 0x1;//127bit
		writel(val, NF2ECCC);

		val  = (224<<16) | (len); //127bit
		writel(val, NF2PGEC);

		writel((int)buf, NF2SADR0);
		writel(len, NF2SBLKS);

		val = (1024<<16) | 1;
		writel(val, NF2SBLKN);	

		val = 1;
		writel(val, NF2DMAC);

		writel(page, NF2RADR0);

		val = 1024<<16; //ecc offset
		writel(val, NF2CADR);


		writel(0x1, NF2FFCE);     // allow write trx-afifo

		writel(0x1, NF2FFCE); // allow write trx-afifo
		trx_afifo_ncmd(0x0, 0x0, 0x00);    // page read CMD0
		trx_afifo_ncmd((0x8 | (cycle -1)), 0x1, 0x0);    // 5 cycle addr of row_addr_0 & col_addr
		trx_afifo_ncmd(0x1, 0x0, 0x30);    // page read CMD1, check rnb and read whole page
		writel(0x0, NF2FFCE); // disable write trx-afifo
		writel(0xe, NF2CSR); // cs0 valid

		//writel(0x5, NF2STRR); // start trx-afifo,  and dma 
		writel(0x7, NF2STRR); // start trx-afifo,  and dma 

		while(1)
		{
			//TODO add time out check	    
			tmp = readl(NF2INTR) & (0x1<<5);
			if(tmp == (0x1<<5))break;
		}
		ecc_info = readl(NF2ECCINFO8);

		writel(0xf, NF2CSR);    // cs0 invalid
		writel(0xffffff, NF2INTR);	// clear status

		page += 1;
		if(ecc_info & 0x1){
			spl_printf("read otp table from page ecc failed, try next page 0x%x\n", page);
		}else{
			break;
		}
	}

	return ret;
}

int imap_program_otp(int page, uint8_t * buf, int len){

	int val = 0;
	int ret = 0;
	int tmp = 0;
	int cycle = 5;
	int i = 0;

	for(i=0; i<4; i++){	
		nf2_soft_reset(1);

		val  = (3<<8) | (15<<4) | (0x1<<1) | 0x1;//127bit
		writel(val, NF2ECCC);

		val  = (224<<16) | (len); //127bit
		writel(val, NF2PGEC);

		writel((int)buf, NF2SADR0);
		writel(len, NF2SBLKS);

		val = (1024<<16) | 1;
		writel(val, NF2SBLKN);	

		val = 1;
		writel(val, NF2DMAC);

		writel(page, NF2RADR0);

		val = 1024<<16; //ecc offset
		writel(val, NF2CADR);

		writel(0x1, NF2FFCE);     // allow write trx-afifo

		writel(0x1, NF2FFCE); // allow write trx-afifo
		trx_afifo_ncmd(0x0, 0x0, 0x80);    // page read CMD0
		trx_afifo_ncmd((0x8 | (cycle -1)), 0x1, 0x0);    // 5 cycle addr of row_addr_0 & col_addr
		trx_afifo_nop(0x3, 0x70);
		trx_afifo_ncmd(0x2, 0x0, 0x10);    // page read CMD1, check rnb and read whole page
		writel(0x0, NF2FFCE); // disable write trx-afifo
		writel(0xe, NF2CSR); // cs0 valid

		//writel(0x5, NF2STRR); // start trx-afifo,  and dma 
		writel(0x7, NF2STRR); // start trx-afifo,  and dma 

		while(1)
		{
			//TODO add time out check	    
			tmp = readl(NF2INTR) & (0x1);
			if(tmp == (0x1))break;
		}

		writel(0xf, NF2CSR);    // cs0 invalid
		writel(0xffffff, NF2INTR);	// clear status

		page += 1;
	}
	//buf[1024] = (readl(NF2STSR0) & 0xff00) >> 8;
	//buf[1025] = (readl(NF2STSR0) & 0xff);
	return ret;

}


int imap_set_retry_param_20(int retrylevel, unsigned char * buf, unsigned char *regadr){

	int tmp = 0;
	int ret = 0;

	nf2_soft_reset(1);

	writel(0x1, NF2FFCE);     // allow write trx-afifo
	
	trx_afifo_ncmd(0x0, 0x0, 0x36);    // set param CMD0
	trx_afifo_ncmd(0x0, 0x1, regadr[0]);    // ADDRS
	trx_afifo_ncmd(0x0, 0x2, buf[0 + retrylevel * 8]);    // data
	trx_afifo_ncmd(0x0, 0x1, regadr[1]);    // ADDRS
	trx_afifo_ncmd(0x0, 0x2, buf[1 + retrylevel * 8]);    // data
	trx_afifo_ncmd(0x0, 0x1, regadr[2]);    // ADDRS
	trx_afifo_ncmd(0x0, 0x2, buf[2 + retrylevel * 8]);    // data
	trx_afifo_ncmd(0x0, 0x1, regadr[3]);    // ADDRS
	trx_afifo_ncmd(0x0, 0x2, buf[3 + retrylevel * 8]);    // data
	trx_afifo_ncmd(0x0, 0x1, regadr[4]);    // ADDRS
	trx_afifo_ncmd(0x0, 0x2, buf[4 + retrylevel * 8]);    // data
	trx_afifo_ncmd(0x0, 0x1, regadr[5]);    // ADDRS
	trx_afifo_ncmd(0x0, 0x2, buf[5 + retrylevel * 8]);    // data
	trx_afifo_ncmd(0x0, 0x1, regadr[6]);    // ADDRS
	trx_afifo_ncmd(0x0, 0x2, buf[6 + retrylevel * 8]);    // data
	trx_afifo_ncmd(0x0, 0x1, regadr[7]);    // ADDRS
	trx_afifo_ncmd(0x0, 0x2, buf[7 + retrylevel * 8]);    // data
	trx_afifo_ncmd(0x0, 0x0, 0x16);    // set param CMD1	
	
	writel(0x0, NF2FFCE); // disable write trx-afifo

    	writel(0xe, NF2CSR);	// cs0 valid
    	writel(0x1, NF2STRR);	// start trx-afifo

    	while(1)
    	{
    		//TODO add time out check	    
    		tmp = readl(NF2INTR) & 0x1;
    		if(tmp == 0x1)break;
    	}
	
	writel(0xf, NF2CSR);    // cs0 invalid
    	writel(0xffffff, NF2INTR);	// clear status

	return ret;
}

int imap_set_retry_param_micro_20(int retrylevel){

	int tmp = 0;
	int ret = 0;

	nf2_soft_reset(1);

	writel(0x1, NF2FFCE);     // allow write trx-afifo
	trx_afifo_ncmd(0x0, 0x0, 0xef);    // set EFH CMD0
	trx_afifo_ncmd(0x0, 0x1, 0x89);    // ADDRS
	trx_afifo_nop(0x0, 0x100);// for tadl delay
	trx_afifo_ncmd(0x0, 0x2, retrylevel);    // P1
	trx_afifo_ncmd(0x0, 0x2, 0x0);    // P2
	trx_afifo_ncmd(0x0, 0x2, 0x0);    // P3
	trx_afifo_ncmd(0x0, 0x2, 0x0);    // P4
	trx_afifo_nop(0x7, 100);// for twb delay
	trx_afifo_nop(0x0, 0x300);// 2.4us delay
	trx_afifo_nop(0x0, 0x300);// 2.4us delay
	trx_afifo_nop(0x0, 0x300);// 2.4us delay
	
	writel(0x0, NF2FFCE); // disable write trx-afifo

    	writel(0xe, NF2CSR);	// cs0 valid
    	writel(0x1, NF2STRR);	// start trx-afifo

    	while(1)
    	{
    		//TODO add time out check	    
    		tmp = readl(NF2INTR) & 0x1;
    		if(tmp == 0x1)break;
    	}
	
	writel(0xf, NF2CSR);    // cs0 invalid
    	writel(0xffffff, NF2INTR);	// clear status
	return ret;
}

int imap_set_retry_param(int retrylevel, unsigned char * buf){

	int ret = 0;
	if(g_param == 0x3){
		ret = imap_set_retry_param_26(retrylevel, buf);
	}
	if(g_param == 0x13){
//		spl_printf("go to param_21, retrylevel  %d.\n", retrylevel);
		ret = imap_set_retry_param_21(retrylevel, buf);
	}
	if(g_param == 0x7 || g_param == 0xf){
		ret = imap_set_retry_param_20(retrylevel, &buf[otp_start], &retry_reg_buf[0]);
	}
	if(g_param == 0xb){
		ret = imap_set_retry_param_micro_20(retrylevel);
	}
	if(g_param == 0x17){
		ret = imap_set_retry_param_sandisk19(retrylevel);	
	}
	return ret;
}

int imap_otp_check(uint8_t * buf){
	
	int i = 0;
	int offset = otp_start;

check_otp:
	for(i=otp_start; i<(otp_start+64); i++){
		if(((buf[i] | buf[i+64])!= 0xff) || ((buf[i] & buf[i+64])!= 0x0)){
			spl_printf("===buf[%d] = 0x%x, buf[%d+64] = 0x%x\n", i, buf[i], i, buf[i+64]);
			break;	
		}
		//spl_printf("buf[%d] = 0x%x, buf[%d+64] = 0x%x\n", i, buf[i], i, buf[i+64]);
	}
	if(i != (otp_start + 64)){
		//TODO reboot
		otp_start += 128;
		if(otp_start >= (896 + offset)){
			//TODO:
			spl_printf("get nand param failed\n");
			return -1;
		}
		goto check_otp;
	}

	return 0;
}

int imap_otp_copy(uint8_t *buf){

	int i = 0, j = 0;

	for(i=0; i<128; i++){
		nand_param_copy[i] = buf[i];
	}

	for(i=0,j=0; i<1024; i++,j++){
		if(j>=128){
			j=0;
		}
		nand_param[i] = nand_param_copy[j];
	}

	return 0;
}	

int imapx_read_spare(int page, int pagesize){

	int val = 0;
	int ret = 0;
	int tmp = 0;
	int cycle = 5;
	uint8_t buf[20];
	
	nf2_soft_reset(1);

	val  = (1<<11) | (3<<8);
    	writel(val, NF2ECCC);
    
	val  = (20); 
	writel(val, NF2PGEC);
	
	writel((int)buf, NF2SADR0);
	writel(20, NF2SBLKS);
   
	val = (1024<<16) | 1;
       	writel(val, NF2SBLKN);	
    
	val = 1;
	writel(val, NF2DMAC);
	
	writel(page, NF2RADR0);

	val = pagesize; //ecc offset
	writel(val, NF2CADR);

	writel(0x1, NF2FFCE); // allow write trx-afifo
	trx_afifo_ncmd(0x0, 0x0, 0x00);    // page read CMD0
	trx_afifo_ncmd((0x8 | (cycle -1)), 0x1, 0x0);    // 5 cycle addr of row_addr_0 & col_addr
	trx_afifo_ncmd(0x1, 0x0, 0x30);    // page read CMD1, check rnb and read whole page
	writel(0x0, NF2FFCE); // disable write trx-afifo
	writel(0xe, NF2CSR); // cs0 valid

    	writel(0x5, NF2STRR); // start trx-afifo,  and dma 

    	while(1)
    	{
    		//TODO add time out check	    
    		tmp = readl(NF2INTR) & (0x1<<5);
    		if(tmp == (0x1<<5))break;
    	}

	writel(0xf, NF2CSR);    // cs0 invalid
    	writel(0xffffff, NF2INTR);	// clear status

	//spl_printf("0x%x, 0x%x, 0x%x, 0x%x\n", buf[0],buf[1],buf[2],buf[3]);	
	//buf[1024] = (readl(NF2STSR0) & 0xff00) >> 8;
	//buf[1025] = (readl(NF2STSR0) & 0xff);
	if(buf[0] != 0xff){
		ret = 1;
	}else{
		ret = 0;
	}
	return ret;
}


int isbad(loff_t start){

	struct nand_config *nc = (struct nand_config *)irf->nand_get_config(NAND_CFG_NORMAL);
	int ret = 0;
	int page = 0;

	page = (start >> 13) + nc->badpagemajor;
	//spl_printf("check bad 0x%x\n", page);
	ret = imapx_read_spare(page, nc->pagesize);
	if(ret != 0){
		return ret;
	}
	
	page = (start >> 13) + nc->badpageminor;
	//spl_printf("check bad 0x%x\n", page);
	ret = imapx_read_spare(page, nc->pagesize);
	
	return ret;
}

int imap_nand_erase(loff_t start){

	int ret = 0;
	int page = 0;
	int tmp = 0;
	int cycle = 5;
	int i = 0;

	page = (start >> 13);
	//spl_printf("erase block 0x%x\n", page);

	nf2_soft_reset(1);

	writel(page, NF2RADR0); // row addr 0

	writel(0x1, NF2FFCE); // allow write trx-afifo
	trx_afifo_ncmd(0x0, 0x0, 0x60);    // ERASE CMD0
	for(i=0; i<(cycle-2); i++){
		trx_afifo_ncmd(0x0, 0x1, ((page & (0xff << (i * 8))) >> i * 8));    // write addr
		
	}

	trx_afifo_ncmd(0x2, 0x0, 0xD0);    // erase CMD1,wait for rnb ready and check status
	writel(0x0, NF2FFCE); // disable write trx-afifo
	writel(0xe, NF2CSR); // cs0 valid

    	writel(0x1, NF2STRR); // start trx-afifo 
    	while(1)
    	{
    		//TODO add time out check	    
    		tmp = readl(NF2INTR) & (0x1);
    		if(tmp == (0x1))break;
    	}
	writel(0xf, NF2CSR);    // cs0 invalid
    	writel(0xffffff, NF2INTR);	// clear status

	return ret;
}

void get_eslc_param_20(uint8_t *buf, uint8_t *regbuf){

	int tmp = 0;
	int retry_param = 0;
	
	nf2_soft_reset(1);
	
	writel(0x1, NF2FFCE);     // allow write trx-afifo
	
	trx_afifo_ncmd(0x0, 0x0, 0x37);    // set param CMD0
	trx_afifo_ncmd(0x0, 0x1, regbuf[0]);    // ADDRS
	trx_afifo_ncmd(0x0, 0x3, 0x00);    // data
	trx_afifo_ncmd(0x0, 0x1, regbuf[1]);    // ADDRS
	trx_afifo_ncmd(0x0, 0x3, 0x00);    // data
	trx_afifo_ncmd(0x0, 0x1, regbuf[2]);    // ADDRS
	trx_afifo_ncmd(0x0, 0x3, 0x00);    // data
	trx_afifo_ncmd(0x0, 0x1, regbuf[3]);    // ADDRS
	trx_afifo_ncmd(0x0, 0x3, 0x00);    // data
	
	writel(0x0, NF2FFCE);     // disable write trx-afifo
	
	writel(0xe, NF2CSR);    // cs0 valid
	writel(0x1, NF2STRR);   // start trx-afifo
	
    	while(1)
    	{
    		//TODO add time out check	    
    		tmp = readl(NF2INTR) & 0x1;
    		if(tmp == 1)break;
    	}

	writel(0xf, NF2CSR);    // cs0 invalid
    	writel(0xffffff, NF2INTR);	// clear status

	retry_param = trx_afifo_read_state(0);
	buf[0] = (retry_param >> 24) & 0xff;
	buf[1] = (retry_param >> 16) & 0xff;
	buf[2] = (retry_param >> 8) & 0xff;
	buf[3] = (retry_param >> 0) & 0xff;
	spl_printf("get eslc param 0x%x, 0x%x, 0x%x, 0x%x\n", buf[0], buf[1], buf[2], buf[3]);	

}

void set_eslc_param_20(uint8_t *buf, int start, uint8_t *regbuf){

	int tmp = 0;
	int i = 0;

	nf2_soft_reset(1);
	
	writel(0x1, NF2FFCE);     // allow write trx-afifo
	
	if(start == 1)
		i = 0xa;

	trx_afifo_ncmd(0x0, 0x0, 0x36);    // set param CMD0
	trx_afifo_ncmd(0x0, 0x1, regbuf[0]);    // ADDRS
	trx_afifo_ncmd(0x0, 0x2, buf[0] + i);    // data
	trx_afifo_ncmd(0x0, 0x1, regbuf[1]);    // ADDRS
	trx_afifo_ncmd(0x0, 0x2, buf[1] + i);    // data
	trx_afifo_ncmd(0x0, 0x1, regbuf[2]);    // ADDRS
	trx_afifo_ncmd(0x0, 0x2, buf[2] + i);    // data
	trx_afifo_ncmd(0x0, 0x1, regbuf[3]);    // ADDRS
	trx_afifo_ncmd(0x0, 0x2, buf[3] + i);    // data
	trx_afifo_ncmd(0x0, 0x0, 0x16);    // set param CMD1

	if (start != 1)
	{
		trx_afifo_ncmd(0x0, 0x0, 0x00);    // set param CMD0  
		trx_afifo_ncmd(0x0, 0x1, 0x00);    // ADDRS           
		trx_afifo_ncmd(0x0, 0x0, 0x30);    // set param CMD0  
	}

	writel(0x0, NF2FFCE);     // disable write trx-afifo
	
	writel(0xe, NF2CSR);    // cs0 valid
	writel(0x1, NF2STRR);   // start trx-afifo
	
    	while(1)
    	{
    		//TODO add time out check	    
    		tmp = readl(NF2INTR) & 0x1;
    		if(tmp == 1)break;
    	}

	writel(0xf, NF2CSR);    // cs0 invalid
    	writel(0xffffff, NF2INTR);	// clear status
}


void rtc_check_time(void){

	uint8_t sec0, sec1, sec2, week0, week1;
	while(1){
		sec0 = readl(RTC_SECR0);
		sec1 = readl(RTC_SECR1);
		sec2 = readl(RTC_SECR2);
		week0 = readl(RTC_WEEK0);
		week1 = readl(RTC_WEEK1);

		if((sec0 == 1) && (sec1 == 0) && (sec2 == 0)){
			break;
		}
	}
	spl_printf("set time end\n");
}

void rtc_set_alarm(void){

	writel(0x6, RTC_ALARM_SEC0);
	writel(0x0, RTC_ALARM_SEC1);
	writel(0x0, RTC_ALARM_SEC2);
	writel(0x0, RTC_ALARM_WEEK0);
	writel(0x0, RTC_ALARM_WEEK1);
	writel(0x3, RTC_ALARM_WEN);
}

void rtc_set_time(void){

	writel(0x1, RTC_SEC0);
	writel(0x0, RTC_SEC1);
	writel(0x0, RTC_SEC2);
	writel(0x0, RTC_MSEC0);
	writel(0x0, RTC_MSEC1);
	writel(0x0, RTC_WEEK0);
	writel(0x0, RTC_WEEK1);
	writel(0x1, RTC_WEEKWEN);
	writel(0x1, RTC_SECWEN);
	writel(0x1, RTC_MSECWEN);

}

int rtc_reboot(void){

	uint8_t tmp;
#if 1
	rtc_set_alarm();
	rtc_set_time();

	writel(0xff, SYS_INT_CLR);

	tmp = readl(RTC_ALARM_WEN);
	tmp |= 0x4;
	writel(tmp, RTC_ALARM_WEN);

	rtc_check_time();

	//writel(0x7f, SYS_WAKEUP_MASK);
	writel(0x2, RTC_POWMASK); //mask charing
#endif
	writel(0x2, SYS_CFG_CMD);

	return 0;
}


int imap_get_retry_param(unsigned char *buf){

	int ret = 0;
	uint32_t tmp = 0;
	int skip = 0;
	loff_t start;
	uint32_t * pbuf;
	uint32_t crc_ori, crc_calc;
	struct nand_config *nc = (struct nand_config *)irf->nand_get_config(NAND_CFG_NORMAL);
	int block = nc->blocksize;
	int page;
	uint8_t eslc_buf[4];

	if(g_param == 0x3){
		g_retrylevel_max = 6;
		tmp = readl(RTC_INFO0) & 0xff;
		if(tmp == 0x71){
			buf[0] = readl(RTC_INFO1);
			buf[1] = readl(RTC_INFO2);
			buf[2] = readl(RTC_INFO3);
			buf[3] = readl(RTC_INFO5);
			//				spl_printf("nand param 0x%x, 0x%x, 0x%x, 0x%x\n", buf[0], buf[1], buf[2], buf[3]);
		}else{
				ret = imap_get_retry_param_26(buf);
//				spl_printf("nand get param 0x%x, 0x%x, 0x%x, 0x%x\n", buf[0], buf[1], buf[2], buf[3]);
			}
	}
	if(g_param == 0x13){			//add by willie for K9GB(LC)G08U0B
		g_retrylevel_max = 14;
		imap_get_retry_param_21(buf);

	}
	if(g_param == 0x7 || g_param == 0xf){
read_retry_again:
		if(g_param == 0x7){
			retry_reg_buf[0] = 0xcc;
			retry_reg_buf[1] = 0xbf;
			retry_reg_buf[2] = 0xaa;
			retry_reg_buf[3] = 0xab;
			retry_reg_buf[4] = 0xcd;
			retry_reg_buf[5] = 0xad;
			retry_reg_buf[6] = 0xae;
			retry_reg_buf[7] = 0xaf;
			eslc_reg_buf[0] = 0xb0;
			eslc_reg_buf[1] = 0xb1;
			eslc_reg_buf[2] = 0xa0;
			eslc_reg_buf[3] = 0xa1;
			spl_printf("H27UCG8T2ATR\n");
		}
		if(g_param == 0xf){
			retry_reg_buf[0] = 0xb0;
			retry_reg_buf[1] = 0xb1;
			retry_reg_buf[2] = 0xb2;
			retry_reg_buf[3] = 0xb3;
			retry_reg_buf[4] = 0xb4;
			retry_reg_buf[5] = 0xb5;
			retry_reg_buf[6] = 0xb6;
			retry_reg_buf[7] = 0xb7;
			eslc_reg_buf[0] = 0xa0;
			eslc_reg_buf[1] = 0xa1;
			eslc_reg_buf[2] = 0xa7;
			eslc_reg_buf[3] = 0xa8;
			spl_printf("H27UBG8T2CTR\n");
		}
		g_retrylevel_max = 7;
		//start = 4100ull*256*8192;
		start = (nc->blockcount + 4)* (unsigned long long)nc->blocksize ;
		for(skip = 0; isbad(start & ~(block - 1));
				skip++, start += block) {
			irf->printf("bad block skipped @ 0x%llx\n", start & ~(block - 1));
			if(skip > 50) {
				irf->printf("two many bad blocks skipped"
						"before getting the corrent data.\n");
				return -1;
			}
		}
		page = start >> 13;
		imap_get_retry_param_20_from_page(page, buf, 1024);
		pbuf = (uint32_t *)buf;
		g_2atr_magic = 0x379a8756;
		//spl_printf("0. nand 0x%x, page 0x%x\n", pbuf[255], page);
		if(pbuf[255] != g_2atr_magic){
			otp_start = 2;
get_retry_again:
			imap_get_retry_param_20(buf, 1024, g_param);
			//spl_printf("buf 0, 1 0x%x, 0x%x\n", buf[0], buf[1]);
		
			//check otp head	
			if(buf[0] != 0x8 || buf[1] != 0x8){
				goto get_retry_again;
			}
			ret = imap_otp_check(buf);
			if(ret != 0){
				goto get_retry_again;
			}
		
			imap_otp_copy(buf + otp_start);
			//spl_printf("1. nand 0x%x, 0x%x\n", nand_param[0], nand_param[1]);
			imap_nand_erase(start);
			
			irf->hash_init(IUW_HASH_CRC, 896);
			irf->hash_data(nand_param, 896);
			irf->hash_value(&crc_calc);
			
			pbuf[254] = crc_calc;
			spl_printf("crc_calc = 0x%x\n", crc_calc);
			pbuf[255] = g_2atr_magic;	
			get_eslc_param_20(eslc_buf, eslc_reg_buf);	
			set_eslc_param_20(eslc_buf, 1, eslc_reg_buf);	
			imap_program_otp(page, nand_param, 1024);
			set_eslc_param_20(eslc_buf, 0, eslc_reg_buf);	
			//spl_printf("otp program end\n");
			rtc_reboot();
			spl_printf("set rtc reboot ...\n");
			while(1);
		}
		//spl_printf("H27UCG8T2ATR end\n");

		
		irf->hash_init(IUW_HASH_CRC, 896);
		irf->hash_data(buf, 896);
		irf->hash_value(&crc_calc);
		crc_ori = pbuf[254];
		if(crc_ori != crc_calc){
			spl_printf("crc_ori = 0x%x not match crc_calc = 0x%x\n", crc_ori, crc_calc);
			imap_nand_erase(start);
			goto read_retry_again;
		}else{
		
			spl_printf("crc_ori = 0x%x match crc_calc = 0x%x\n", crc_ori, crc_calc);
		}

		otp_start = 0;
		ret = imap_otp_check(buf);
		if(ret != 0){
			spl_printf("otp check error\n");
			imap_nand_erase(start);
			goto read_retry_again;
		}

#if 0	
		spl_printf("default param 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", buf[0], buf[1],buf[2], buf[3],buf[4], buf[5],buf[6], buf[7]);
		spl_printf("1st     param 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", buf[8], buf[9],buf[10], buf[11],buf[12], buf[13],buf[14], buf[15]);
		spl_printf("2nd     param 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", buf[16], buf[17],buf[18], buf[19],buf[20], buf[21],buf[22], buf[23]);
		spl_printf("3th     param 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", buf[24], buf[25],buf[26], buf[27],buf[28], buf[29],buf[30], buf[31]);
		spl_printf("4th     param 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", buf[32], buf[33],buf[34], buf[35],buf[36], buf[37],buf[38], buf[39]);
		spl_printf("5th     param 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", buf[40], buf[41],buf[42], buf[43],buf[44], buf[45],buf[46], buf[47]);
		spl_printf("6th     param 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", buf[48], buf[49],buf[50], buf[51],buf[52], buf[53],buf[54], buf[55]);
		spl_printf("7th     param 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", buf[56], buf[57],buf[58], buf[59],buf[60], buf[61],buf[62], buf[63]);
#endif
	}
	if(g_param == 0xb){
		spl_printf("MT29F64G08CBABA\n");
		g_retrylevel_max = 7;
	}
	if(g_param == 0x17){			//Sandisk nand retry
		spl_printf("SDTNQFAMA-004G or SDTNQFAMA-008G\n");
//		imap_get_retry_param_sandisk19();
		g_retrylevel_max = 20;		
	}
	return ret;
}


