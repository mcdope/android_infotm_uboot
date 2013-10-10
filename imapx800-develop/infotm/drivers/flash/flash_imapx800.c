#include <linux/types.h>
#include <common.h>
#include <vstorage.h>
#include <asm/io.h>
#include <bootlist.h>
#include <lowlevel_api.h>

#define SPI_CLK 800000

struct spi_transfer {
	uint32_t spi_addr;
	uint32_t spi_addition_addr;
	uint32_t spi_erase;
	uint32_t spi_current_len;
	uint32_t spi_addition;
	uint32_t spi_len;
	uint8_t spi_current_addr[4];
	uint8_t spi_command[8];
};

static int spibytes = 256;

#if 0
int  spi_cs(int direction)
{
	int gpedat;

	gpedat = readl(GPEDAT);
	if (direction)
		gpedat |= (0x1 << 7);
	else
		gpedat &= ~(0x1 << 7);
	writel(gpedat, GPEDAT);

	return 0;
}
#endif

void spi_set_bytes(int bytes) {
	spibytes = bytes;
	printf("spi: bytes set to: %d\n", bytes);
}

#if 0
unsigned long long fake_get_ticks(void)
{
	unsigned long long  lower, upper, tmp;

	/* read the upper 32-bit */
	tmp = readl(GTIMER_CNTR_UPPER);

	/* read the lower 32-bit */
	lower = readl(GTIMER_CNTR_LOWER);

	/* incase upper changed */
	if((upper = readl(GTIMER_CNTR_UPPER))
				!= tmp)
	  lower = readl(GTIMER_CNTR_LOWER);

	return (upper << 32 | lower);

}
#endif

int spi_tranfer(uint8_t *dout, uint8_t *din, uint32_t len, uint8_t *addr,
		uint32_t addr_len, uint8_t *command, uint32_t com_len)
{
	int status, i;
	int sum, tx = len + addr_len + com_len, rx = 0;
	uint8_t none = 0;
//    uint64_t to, tn;

	sum = tx;
//	spi_cs(0);
	
#if 0
	printf("tran: do=0x%p, di=0x%p\n", dout, din);
	printf("tran: l=0x%x, ad=0x%p\n", len, addr);
	printf("tran: adl=0x%x, cmd=0x%p, cmdl=0x%x\n", addr_len, command, com_len);
#endif
	printf("\n");
	udelay(1);
//	fake_get_ticks();
	status = readl(SSP_SR_1);
	while (status & (0x1 << 2)) {
        	readl(SSP_DR_1);
		status = readl(SSP_SR_1);
		printf("spi: clear rx\n");
	}
	status = readl(SSP_SR_1);
	while(!(status & (0x1 << 0))) {
		status = readl(SSP_SR_1);
		printf("spi: clear tx\n");
	}
    //to = get_ticks();
	while (1) {
		if (tx <= 0 && rx >= sum)	
			break;
		status = readl(SSP_RIS_1);
		if (status & (0x1 << 3)) {
      //      to = get_ticks();
			i = sum - tx;
			if (com_len > i) {
//				printf("*command is %d\n", *command);
				writel(*command, SSP_DR_1);
				command++;
			} else if (addr_len > (i - com_len)) {
				writel(*addr, SSP_DR_1);
				addr++;
			} else if (len > (i - com_len - addr_len)) {
//				printf("*din is %d\n", *din);
				writel(din? *din++: 0, SSP_DR_1);
			}
			tx--;
		}
		status = readl(SSP_SR_1);
		if (status & (0x1 << 2)) {
        //    to = get_ticks();
			if (rx >= (com_len + addr_len) && dout != NULL) {
				dout[rx - com_len - addr_len] = readl(SSP_DR_1);
// 				printf("dout is %d\n", dout[0]);
			}
			else {
				none = readl(SSP_DR_1);
				//printf("....none is %d\n", none);
			}
			rx++;
		}
        //tn = get_ticks();
        //if ((tn - to) > 10000000)
          //  break;
	}

//	spi_cs(1);
	
	return 0;
}

#if 0
int spi_tranfer(struct test_sct *sct)
{
	while(readl(SSP_SR_1) & (1 << 2));
	while(!(readl(SSP_SR_1) & (0x1 << 0)));

	while(1) {

		if(sct->tx <= 0 && rx >= sum)
			break;
    //to = get_ticks();
	while (1) {
		if (tx <= 0 && rx >= sum)	
			break;
		status = readl(SSP_RIS_1);
		if (status & (0x1 << 3)) {
      //      to = get_ticks();
			i = sum - tx;
			if (com_len > i) {
//				printf("*command is %d\n", *command);
				writel(*command, SSP_DR_1);
				command++;
			} else if (addr_len > (i - com_len)) {
				writel(*addr, SSP_DR_1);
				addr++;
			} else if (len > (i - com_len - addr_len)) {
//				printf("*din is %d\n", *din);
				writel(*din, SSP_DR_1);
				din++;
			}
			tx--;
		}
		status = readl(SSP_SR_1);
		if (status & (0x1 << 2)) {
        //    to = get_ticks();
			if (rx >= (com_len + addr_len) && dout != NULL) {
				dout[rx - com_len - addr_len] = readl(SSP_DR_1);
// 				printf("dout is %d\n", dout[0]);
			}
			else {
				none = readl(SSP_DR_1);
				//printf("....none is %d\n", none);
			}
			rx++;
		}
        //tn = get_ticks();
        //if ((tn - to) > 10000000)
          //  break;
	}

//	spi_cs(1);
	
	return 0;
}
#endif

uint32_t spi_addition_func(loff_t offs)
{
	return offs & (spibytes - 1);
}

uint32_t spi_write_get_len(struct spi_transfer *spi) 
{
	spi->spi_addition_addr += spi->spi_current_len;
	if (spi->spi_addition_addr == spibytes) {
	        spi->spi_addition_addr = 0;
	        spi->spi_addr += spi->spi_current_len;
	}

	spi->spi_addition = spibytes - spi->spi_addition_addr;
	if (spi->spi_addition < spi->spi_len) {
	        spi->spi_current_len = spi->spi_addition;
	        spi->spi_len -= spi->spi_addition;
	        return 1;
	} else if (spi->spi_len > 0){
	        spi->spi_current_len = spi->spi_len;
	        spi->spi_len -= spi->spi_current_len;
	        return 1;
	} else
	        return 0;
}
		

int flash_vs_read(uint8_t *buf, loff_t offs, int len, uint32_t extra)
{
	struct spi_transfer addr;
	int i;
	addr.spi_addr = offs;
	addr.spi_command[0] = 0x0B;
	
	printf("spi: r (b=%p, o=0x%llx, l=0x%x)\n", buf, offs, len);
	for (i =0;i<len / 4096;i++) {
		addr.spi_current_addr[0] = (addr.spi_addr >> 16) & 0xff;
		addr.spi_current_addr[1] = (addr.spi_addr >> 8) & 0xff;
		addr.spi_current_addr[2] = addr.spi_addr & 0xff;
		addr.spi_current_addr[3] = 0;

		spi_tranfer(buf, NULL, 4096, addr.spi_current_addr, 4, addr.spi_command, 1);
		addr.spi_addr += 4096;
		buf += 4096;
	}
	if (len % 4096) {
	        addr.spi_current_addr[0] = (addr.spi_addr >> 16) & 0xff;
		addr.spi_current_addr[1] = (addr.spi_addr >> 8) & 0xff;
		addr.spi_current_addr[2] = addr.spi_addr & 0xff;
		addr.spi_current_addr[3] = 0;
		spi_tranfer(buf, NULL, len % 4096, addr.spi_current_addr, 4, addr.spi_command, 1);
	}

#if 0	
	printf("what you read is \n");
	for (i = 0;i < len;i++) {
	        printf("%c",buf[i]);
	        if (i!=0 && i%60 ==0)
	                printf("\n");
	}
	printf("\n");
#endif
	return len;
}

uint32_t spi_flash_start(void)
{
	struct spi_transfer addr;
	addr.spi_command[0] = 0x06;
	spi_tranfer(NULL, NULL, 0, addr.spi_current_addr, 0, addr.spi_command , 1);

	return 0;
}

uint32_t spi_flash_finish(void)
{
	struct spi_transfer addr;
	uint8_t buf[4];
	uint8_t s[4];
	addr.spi_command[0] = 0x05;
	spi_tranfer(buf, s, 1, addr.spi_current_addr, 0, addr.spi_command, 1);
	printf("st: 0x%02x\n", buf[0]);

	return buf[0];
}

uint32_t spi_flash_enable_write(void)
{
	int tmp;
	
	spi_flash_start();
	tmp = spi_flash_finish();
	while (!(tmp & (0x1 << 1))) {
			tmp = spi_flash_finish();
	        printf("spi: wait WEN\n");
	}
	return 0;
}

uint32_t spi_flash_finish_write(void)
{
	int tmp;
	uint64_t to,tn;

	tmp = spi_flash_finish();
	to = get_ticks();
	while (tmp & (0x1)) {
			tn = get_ticks();
			if ((tn - to) > 50000000) {
				printf("spi: time out\n");
				return -1;
			}
	        tmp = spi_flash_finish();
	}
	return 0;
}

uint32_t spi_flash_enable_register(void)
{
		struct spi_transfer addr;
		addr.spi_command[0] = 0x50;
		spi_tranfer(NULL, NULL, 0, addr.spi_current_addr, 0, addr.spi_command , 1);
		
		return 0;
}

uint32_t spi_flash_zero(void)
{
		struct spi_transfer addr;
		uint8_t s[4] = {0};

		spi_flash_enable_write();	
		addr.spi_command[0] = 0x01;
		spi_tranfer(NULL, s, 1, addr.spi_current_addr, 0, addr.spi_command, 1);
		spi_flash_finish_write();

		spi_flash_enable_register();
		addr.spi_command[0] = 0x01;
		spi_tranfer(NULL, s, 1, addr.spi_current_addr, 0, addr.spi_command, 1);
		spi_flash_finish_write();

		return 0;
}

int flash_vs_write(uint8_t *buf, loff_t offs, int len, uint32_t extra)
{
	struct spi_transfer addr;
	int i;
	addr.spi_addr = offs;
	addr.spi_command[0] = 0x02;
	addr.spi_len = len;
	
	if (spibytes != 1) {
		addr.spi_addition_addr = spi_addition_func(offs);
		addr.spi_current_len = 0;
		while (spi_write_get_len(&addr)) {
			spi_flash_enable_write();
			
			addr.spi_current_addr[0] = (addr.spi_addr >> 16) & 0xff;
			addr.spi_current_addr[1] = (addr.spi_addr >> 8) & 0xff;
			addr.spi_current_addr[2] = addr.spi_addr & 0xff;
	//		printf("addr.spi_current_addr[0] is %d, addr.spi_current_addr[1] is %d, addr.spi_current_addr[2] is %d,addr.spi_current_len is %d\n",
	//				addr.spi_current_addr[0], addr.spi_current_addr[1], addr.spi_current_addr[2], addr.spi_current_len);

			spi_tranfer(NULL, buf, addr.spi_current_len, addr.spi_current_addr, 3, addr.spi_command , 1);

			spi_flash_finish_write();
			buf += addr.spi_current_len;
		}
    } else {
		for (i = 0;i < len;i++) {
			spi_flash_enable_write();

			addr.spi_current_addr[0] = (addr.spi_addr >> 16) & 0xff;
			addr.spi_current_addr[1] = (addr.spi_addr >> 8) & 0xff;
			addr.spi_current_addr[2] = addr.spi_addr & 0xff;

			spi_tranfer(NULL, buf, 1, addr.spi_current_addr, 3, addr.spi_command , 1);
			
			spi_flash_finish_write();
			buf += 1;
			addr.spi_addr += 1;
		}
	}
	
	return len;
}

int flash_vs_erase(uint64_t offs, uint32_t len)
{
	struct spi_transfer addr;

	printf("?\n");
	spi_flash_zero();
	addr.spi_command[0] = 0xC7;
	spi_flash_enable_write();
	spi_tranfer(NULL, NULL, 0, addr.spi_current_addr, 0, addr.spi_command , 1);
	spi_flash_finish_write();

	return 0;
}

int spi_init(void)
{
    int pclk = module_get_clock(APB_CLK_BASE), i, j;

	for(i = 2; i < 255; i += 2)
	  for(j = 0; j < 256; j++)
		  if(pclk / (i * (j + 1)) <= SPI_CLK)
			  goto calc_finish;

calc_finish:
	if(i >= 255 || j >= 256) {
		printf("spi: calc clock failed(%d, %d), use default.\n", i, j);
		i = 2; j = 39;
	}
    
    printf("PCLK: %d, PS: %d, SCR: %d, Fout: %d\n", 
            pclk, i, j, pclk / (i * (j + 1)));

    writel(0, SSP_CR1_1);
    writel(0, SSP_IMSC_1);
    writel(i, SSP_CPSR_1);
    writel(j << 8 | (0x3 << 6) | (0x0 << 4) | (0x7 << 0), SSP_CR0_1);
    writel((0x1 << 3) | (0x0 << 2) | (0x1 << 1) | (0 << 0), SSP_CR1_1);

    return 0;
}

int spi_pads_init(void)
{
	int xom = get_xom(1);

	if(boot_device() != DEV_FLASH)
		printf("warning: not spi boot\n");

	switch(xom) {
		case 6:
			pads_chmod(PADSRANGE(27, 30), PADS_MODE_CTRL, 0);
			pads_pull(PADSRANGE(27, 30), 1);
			break;
		case 4:
			pads_chmod(PADSRANGE(74, 77), PADS_MODE_CTRL, 0);
			pads_pull(PADSRANGE(74, 77), 1);
			break;
		default:
			pads_chmod(PADSRANGE(121, 124), PADS_MODE_CTRL, 0);
			pads_pull(PADSRANGE(121, 124), 1);
			break;
	}

	return 0;
}

int flash_vs_reset(void)
{
	spi_pads_init();
	module_enable(SSP_SYSM_ADDR);
	spi_init();
	return 0;
}

int flash_vs_align(void)
{
	return 256;
}
