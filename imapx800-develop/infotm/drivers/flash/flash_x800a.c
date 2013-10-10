#include <linux/types.h>
#include <common.h>
#include <vstorage.h>
#include <asm/io.h>
#include <bootlist.h>
#include <lowlevel_api.h>
#include <linux/byteorder/swab.h>
#include <efuse.h>

#define SPI_CLK 600000

#define SPI_TFIFO_EMPTY  (1 << 0)
#define SPI_TFIFO_NFULL  (1 << 1)
#define SPI_RFIFO_NEMPTY (1 << 2)
#define SPI_BUSY         (1 << 4)

#define FLASH_CMD_WREN  0x6
#define FLASH_CMD_WRDI  0x4
#define FLASH_CMD_RDSR  0x5
#define FLASH_CMD_WRSR  0x1
#define FLASH_CMD_READ  0x3
#define FLASH_CMD_FARD  0xb
#define FLASH_CMD_WRIT  0x2
#define FLASH_CMD_WESR  0x50
#define FLASH_CMD_ERAS  0xc7

#define FLASH_READ_BLOCK 0x1000

#define FLASH_MOD_POLLREDY 0
#define FLASH_MOD_POLLWREN 1
#define FLASH_DUMMY   FLASH_CMD_RDSR

struct spi_transfer {
	uint8_t *in;
	uint8_t *out;
	int len;
};

static int spibytes = 256;

void spi_set_bytes(int bytes) {
	spibytes = bytes;
	printf("spi: bytes set to: %d\n", bytes);
}

int spi_init_clk(void)
{
	int pclk = module_get_clock(APB_CLK_BASE),
		i, j;

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
	writel((j << 8) | (3 << 6) | (0 << 4) | (7 << 0), SSP_CR0_1);
	writel((1 << 3) | (0 << 2) | (1 << 1) | (0 << 0), SSP_CR1_1);

	return 0;
}

int spi_init_pads(void) {
	int xom = get_xom(1), index;

	if(boot_device() != DEV_FLASH)
	  printf("warning: not spi boot\n");

	index = (xom == 4) ? 74: 
		((xom == 6)? 27: 121);

	pads_chmod(PADSRANGE(index, (index + 3)), PADS_MODE_CTRL, 0);
	if(ecfg_check_flag(ECFG_ENABLE_PULL_SPI)) {
		pads_pull(PADSRANGE(index, (index + 3)), 1, 1);
		pads_pull(index + 2, 1, 0);		/* clk */
	}

	return 0;
}

void spi_fifo_clear(void)
{
	while(readl(SSP_SR_1) & SPI_BUSY);
	while(readl(SSP_SR_1) & SPI_RFIFO_NEMPTY) {
		readl(SSP_DR_1);
//		printf("c:0x%02x\n", readl(SSP_DR_1));
	}
}

/* exchange support:
 *		@tx
 *		@tx rx
 *		@tx tx
 * the first tx is limited to bellow 8 bytes
 */
void spi_exchange(struct spi_transfer *td, int count)
{
	int skip = 0, len, dummy = 0, stat;
	uint8_t *p, ex[8], exlen;

#if 0
	int a;
	for(a = 0; a < count; a++)
	  printf("spi: exc(%d): in=0x%p, out=0x%p, len=0x%x\n",
				  a, td[a].in, td[a].out, td[a].len);
#endif
	len = skip = td->len;
	p     = td->out;

	if(count > 1) {
		memcpy(ex, td->out, len);
		td++;
		exlen = min(8, len + td->len);

		if(td->out) {
			memcpy(ex + len, td->out, exlen - len);
			p     = td->out + exlen - len;
			len   = td->len + len - exlen;
			skip += td->len;
		} else {
			p = td->in;
			len   = td->len;
			dummy = td->len + len - exlen;
		}

		/* fill tfifo to enable pipe line */
		for(stat = 0; stat < exlen; stat++) {
//			printf("we: %02x\n", ex[stat]);
		  writel(ex[stat], SSP_DR_1);
		}
	}

	/* ok, imediately start the loop */
__loop:
	stat = readl(SSP_RIS_1);
	if(stat & (1 << 2)) {
		if(skip > 3) { skip -= 4;
			readl(SSP_DR_1);
			readl(SSP_DR_1);
			readl(SSP_DR_1);
			readl(SSP_DR_1);
//			printf("rd4\n");
		} else if(skip) {
//			printf("rs:0x%02x\n", readl(SSP_DR_1));
			while(skip) { skip--;
//				printf("rs\n");
				readl(SSP_DR_1); }
		} else {

			if(!td->in)
			  printf("spi: unexpected dummy\n");
			else {
				if(len > 3) { len -= 4;
					*p++ = readl(SSP_DR_1);
					*p++ = readl(SSP_DR_1);
					*p++ = readl(SSP_DR_1);
					*p++ = readl(SSP_DR_1);
//					printf("r4: %02x %02x %02x %02x\n", *(p - 4), *(p - 3), *(p - 2), *(p - 1));
				} else if(len) {
					while(len) { len--;
						*p++ = readl(SSP_DR_1);
//						printf("r%02x\n", *(p - 1));
					}
//				printf("r:0x%02x\n", *(p-1));
				}
				if(!len) goto __finish;
				//					printf("r:0x%02x\n", a);
			}
		}
	}

	if(stat & (1 << 3)) {
		if(td->out) {
			if(len > 3) { len -= 4;
				writel(*p++, SSP_DR_1);
				writel(*p++, SSP_DR_1);
				writel(*p++, SSP_DR_1);
				writel(*p++, SSP_DR_1);
				//			printf("w4: %02x %02x %02x %02x\n", *(p - 4), *(p - 3), *(p - 2), *(p - 1));
			} else if(len) {
				while(len) { len--;
					//				printf("2%02x\n", *p);
					writel(*p++, SSP_DR_1); }
			}

			if(td->out && !len)
			  goto __finish;
		} else if(dummy > 3) { dummy -= 4;
//			printf("wd4\n");
			writel(FLASH_DUMMY, SSP_DR_1);
			writel(FLASH_DUMMY, SSP_DR_1);
			writel(FLASH_DUMMY, SSP_DR_1);
			writel(FLASH_DUMMY, SSP_DR_1);
		} else if(dummy) {
			while(dummy) { dummy--;
//				printf("wd\n");
				writel(FLASH_DUMMY, SSP_DR_1); }
		}
	}

	/* r time out */
	if(stat & (1 << 1)) {
		while(skip) { skip--;
			if(readl(SSP_SR_1) & SPI_RFIFO_NEMPTY) {
//				printf("rts\n");
				readl(SSP_DR_1);
			}
			else
			  printf("spi: data missed (s%d)\n", skip);
		}

		while(td->in && len) { len--;
			if(readl(SSP_SR_1) & SPI_RFIFO_NEMPTY) {
				*p++ = readl(SSP_DR_1);
//				printf("rt%02x\n", *(p-1));
			}
			else
			  printf("spi: data missed (r%d)\n", len);
		}

		/* clear SSPRTINTR */
		writel(3, SSP_ICR_1);
		goto __finish;
	}

	goto __loop;

__finish:
	spi_fifo_clear();
}

void flash_poll(int mod) {
	uint8_t buf[8] = { FLASH_CMD_RDSR, 0 };
	struct spi_transfer td[] = { {NULL, &buf[0], 1},
		{&buf[1], NULL, 1} };

	do { spi_exchange(td, ARRAY_SIZE(td));
//		printf("stat: 0x%02x, 0x%02x, 0x%02x\n", buf[0], buf[1], buf[2]);
       	} while
	  ((mod)? !(buf[1] & (1 << 1)): !!(buf[1] & 1));
}

void flash_wren(void) {
	uint8_t tx[8] = { FLASH_CMD_WREN,
		FLASH_CMD_WRDI };
	struct spi_transfer td[] = { {NULL, &tx[0], 1 },
		{NULL, &tx[1], 1} };
//	spi_exchange(&td[1], 1);
	spi_exchange(&td[0], 1);
	flash_poll(FLASH_MOD_POLLWREN);
}

int flash_vs_read(uint8_t *buf, loff_t offs, int len, int extra)
{
	uint8_t tx[8] = {0, 0, 0, FLASH_CMD_FARD };
	int remain = len, *d = (int *)&tx[4];
	struct spi_transfer td[] = {
		{NULL, &tx[3], 5 },
		{buf,  NULL, 0 }
	}, *p;

//	printf("spi: r (b=%p, o=0x%llx, l=0x%x)\n", buf, offs, len);

	for(p = &td[1]; remain;) {

		*d = offs;
		*d = __swab32(*d);
		*d >>= 8;

		p->len = (remain < FLASH_READ_BLOCK)?
			remain: FLASH_READ_BLOCK;

		spi_exchange(td, ARRAY_SIZE(td));
		remain -= p->len;
		offs   += p->len;
		p->in  += p->len;
	}

	return len;
}

int flash_vs_write(uint8_t *buf, loff_t offs, int len, int extra)
{
	uint8_t tx[8] = { FLASH_CMD_WREN, 0, 0, FLASH_CMD_WRIT };
	int remain = len, *d = (int *)&tx[4];
	struct spi_transfer td[] = {
		{NULL, &tx[4], 4 },
		{NULL, buf,    0 },
	}, *p;
	
//	printf("spi: w (b=%p, o=0x%llx, l=0x%x)\n", buf, offs, len);

	for(p = &td[1]; remain;) {
		*d = offs;
		*d = __swab32(*d);
		*d &= ~0xff;
		*d |= FLASH_CMD_WRIT;
		p->len = (remain < spibytes) ? remain: spibytes;

		flash_wren();
		spi_exchange(td, ARRAY_SIZE(td));
		flash_poll(FLASH_MOD_POLLREDY);

		remain -= p->len;
		offs   += p->len;
		p->out += p->len;
	}

	return len;
}

int flash_vs_erase(loff_t offs, uint32_t len)
{
	uint8_t tx[8] = {
		FLASH_CMD_WESR, FLASH_CMD_WRSR,
		0, FLASH_CMD_ERAS, FLASH_CMD_WREN,
		0, 0, 0};
	struct spi_transfer td[] = {
		{NULL, &tx[1], 2 },
		{NULL, &tx[3], 1 },
		{NULL, &tx[0], 1 },
	};

	/* clear sr */
//	printf("clear sr\n");
	flash_wren();
	spi_exchange(&td[0], 1);
	flash_poll(FLASH_MOD_POLLREDY);

	/* clear sr: alt */
//	printf("clear sr: alt\n");
	spi_exchange(&td[2], 1);
	spi_exchange(&td[0], 1);
	flash_poll(FLASH_MOD_POLLREDY);

	/* erase */
	flash_wren();
//	printf("erase\n");
	spi_exchange(&td[1], 1);
	flash_poll(FLASH_MOD_POLLREDY);
	return 0;
}

int flash_vs_reset(void) {
	module_enable(SSP_SYSM_ADDR);
	spi_init_clk();
	spi_init_pads();
	spi_fifo_clear();
	return 0;
}

int flash_vs_align(void) {
	return 256;
}

