/*
 * (C) Copyright 2005
 * 2N Telekomunikace, a.s. <www.2n.cz>
 * Ladislav Michl <michl@2n.cz>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _NAND_H_
#define _NAND_H_
#include <cdbg.h>

extern int init_nand(void);

extern int nnd_vs_erase(loff_t start, uint32_t size);
extern int bnd_vs_erase(loff_t start, uint32_t size);
extern int nnd_vs_scrub(loff_t start, uint32_t size);

extern int bnd_vs_reset(void);
extern int bnd_vs_read(uint8_t *buf, loff_t start, int length, int extra);
extern int bnd_vs_write(uint8_t *buf, loff_t start, int length, int extra);
extern int bnd_vs_align(void);

extern int nnd_vs_reset(void);
extern int nnd_vs_read(uint8_t *buf, loff_t start, int length, int extra);
extern int nnd_vs_write(uint8_t *buf, loff_t start, int length, int extra);
extern int nnd_vs_align(void);

extern int fnd_vs_reset(void);
extern int fnd_vs_read(uint8_t *buf, loff_t start, int length, int extra);
extern int fnd_vs_write(uint8_t *buf, loff_t start, int length, int extra);
extern int fnd_vs_align(void);
extern int fnd_vs_erase(loff_t start, uint32_t size);

extern int bnd_vs_isbad(loff_t offset);
extern int nnd_vs_isbad(loff_t offset);
extern int fnd_vs_isbad(loff_t offset);

extern int fnd_size_match(uint32_t size);
extern loff_t fnd_extent_offs(loff_t offs);
extern uint32_t fnd_size_shrink(uint32_t size);
extern int nand_seek_good_absolute(loff_t *offs, int leeway);
extern int nand_seek_good(loff_t *offs, int leeway);
extern uint32_t fnd_align(void);


struct nand_config {
	uint8_t		interface;		/* legacy, toggle, ONFI2 sync */
	uint8_t		randomizer;
	uint8_t		cycle;			/* address cycle */
	uint8_t		mecclvl;
	uint8_t		secclvl;
	uint8_t		eccen;			/* wheather use ECC */
	uint8_t		busw;			/* busw */
	uint8_t		resv;			/* resv */
	uint32_t	badpagemajor;	/* first page offset when checking block validity */
	uint32_t	badpageminor;	/* second page offset when checking block validity */
	uint32_t	sysinfosize;
	uint32_t	pagesize;
	uint32_t	sparesize;
	uint32_t	blocksize;
	uint32_t	pagesperblock;
	uint32_t	blockcount;
	uint32_t	timing;
	uint32_t	rnbtimeout;
	uint32_t	phyread;
	uint32_t	phydelay;
	uint32_t	seed[4];
	uint32_t	polynomial;
	uint16_t	sysinfo_seed[8];
	uint16_t	ecc_seed[8];
	uint16_t	secc_seed[8];
	uint16_t	data_last1K_seed[8];
	uint16_t	ecc_last1K_seed[8];
#if !defined(CONFIG_PRELOADER)
	uint32_t	read_retry;
	uint32_t	retry_level;
	uint32_t	nand_param0;
	uint32_t	nand_param1;
	uint32_t	nand_param2;
#endif
};


struct nand_cfg {
	int		normal_en;
	int		boot_en;
	int		bnd_size_shift;
	int		bnd_page_shift;
	int		nnd_page_shift;
	struct nand_config boot;
	struct nand_config normal;
};

extern int nand_readid(uint8_t buf[], int interface, int timing, int rnbtimeout, int phyread, int phydelay, int busw);
extern int nand_rescan_config(struct cdbg_cfg_nand t[], int count);

int imap_set_eslc_param_20(unsigned int *buf);
int imap_eslc_end(void);
int imap_set_retry_param_20(int retrylevel, unsigned char * buf, unsigned char *regadr);
int imap_get_retry_param_20(unsigned char *buf, int len, int param);
int imap_get_retry_param_26(unsigned char *buf);
int imap_set_retry_param_26(int retrylevel, unsigned char * buf);
int imap_set_retry_param_21(int retrylevel, unsigned char * buf);
int imap_get_retry_param_21(unsigned char * buf);
int isbad(loff_t start);
int imap_get_retry_param_20_from_page(int page, unsigned char *buf, int len);
int trx_afifo_read_state(int index);
void nf2_ecc_cfg(int ecc_enable, int page_mode, int ecc_type, int eram_mode,
		                 int tot_ecc_num, int tot_page_num,  int trans_dir, int half_page_en);
void nf2_secc_cfg(int secc_used, int secc_type, int rsvd_ecc_en);
void nf2_sdma_cfg(int ch0_adr, int ch0_len, int ch1_adr, int ch1_len, int sdma_2ch_mode);
void nf2_addr_cfg(int row_addr0, int col_addr, int ecc_offset, int busw);
int nf2_erase(int cs, int addr, int cycle);
int nf2_soft_reset(int num);
int nf2_page_op(int ecc_enable, int page_mode, int ecc_type, int rsvd_ecc_en,
                    int page_num, int sysinfo_num, int trans_dir, int row_addr,
		    int ch0_adr, int ch0_len, int secc_used, int secc_type,
                    int half_page_en, int main_part, int cycle, int randomizer,
		    uint32_t *data_seed, uint16_t *sysinfo_seed,  uint16_t *ecc_seed, uint16_t *secc_seed,
		    uint16_t *data_last1K_seed, uint16_t *ecc_last1K_seed, int *ecc_unfixed, int *secc_unfixed,
		    int ch1_adr, int ch1_len, int sdma_2ch_mode, int busw);
int nand_block_isbad(loff_t offset);
int nf2_timing_init(int interface, int timing, int rnbtimeout, int phyread, int phydelay, int busw);
unsigned int nf2_randc_seed (unsigned int raw_seed, unsigned int cycle, int polynomial);
unsigned int nf2_randc_seed_hw (unsigned int raw_seed, unsigned int cycle, int polynomial);
int nf2_ecc_num (int ecc_type, int page_num, int half_page_en);
int nf2_timing_async(int timing, int rnbtimeout);
int nf2_active_sync (int syn_timemode);
int nf2_asyn_reset(int cs);
int nf2_set_polynomial(int polynomial);
void trx_afifo_ncmd (unsigned int flag, unsigned int type, unsigned int wdat);
void trx_afifo_reg (unsigned int flag, unsigned int addr, unsigned int data);
void trx_afifo_nop (unsigned int flag, unsigned int nop_cnt);
uint16_t cyg_crc16(unsigned char *buf, int len);

extern int bnd_page_isbad(loff_t offset);
extern int bnd_match_bad(loff_t offset, uint32_t len);
extern struct nand_config * nand_get_config(int type);
extern void  ecc_print_stat(void);
extern int base_scan_badblocks(void);
extern int base_block_markbad(loff_t offset);
extern int nand_init_randomizer(struct nand_config *cfg);
extern int nf2_hardware_reset(struct nand_config *c, int active_sync);
extern int nand_rescan_bootcfg(void);
extern void print_nand_cfg(struct nand_config *cfg);
extern int nand_emu_init(void);
extern void nf2_set_gtiming(uint32_t a);

//#define nf_dbg(x...) printf("nf: " x)
#define nf_dbg(x...) 

#define NAND_CFG_BOOT		0
#define NAND_CFG_NORMAL		1

#endif

