/***************************************************************************** 
** XXX common/oem_nand.c XXX
** 
** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
** 
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** Description: More secure nand r/w/e functions.
**
** Author:
**     Warits   <warits.wang@infotm.com>
**      
** Revision History: 
** ----------------- 
** 1.1  XXX 03/24/2010 XXX	Warits
*****************************************************************************/

#include <common.h>
#include <malloc.h>
#include <nand.h>
#include <oem_func.h>

static int oem_block_bad_scrub(struct mtd_info *mtd, loff_t ofs, int getchip)
{
	return 0;
}

int oem_scrub(uint32_t start, uint32_t len)
{
	struct erase_info erase;
	ulong erase_length;
	int result;
	int (*nand_block_bad_old)(struct mtd_info *, loff_t, int) = NULL;
	struct mtd_oob_ops oob_opts;
	struct mtd_info *oem_mtd = &nand_info[nand_curr_device];
	struct nand_chip *chip = oem_mtd->priv;
	const char *mtd_device = oem_mtd->name;

	memset(&erase, 0, sizeof(erase));
	memset(&oob_opts, 0, sizeof(oob_opts));

	erase.mtd = oem_mtd;
	erase.len  = oem_mtd->erasesize;
	erase.addr = start;
	erase_length = len;

	/* Release the original bbt */
	nand_block_bad_old = chip->block_bad;
	chip->block_bad = oem_block_bad_scrub;
	/* we don't need the bad block table anymore...
	 * after scrub, there are no bad blocks left!
	 */
	if (chip->bbt) {
		kfree(chip->bbt);
	}
	chip->bbt = NULL;

	if (erase_length < oem_mtd->erasesize) {
		printf("Warning: Erase size 0x%08lx smaller than one "	\
		       "erase block 0x%08x\n",erase_length, oem_mtd->erasesize);
		printf("         Erasing 0x%08x instead\n", oem_mtd->erasesize);
		erase_length = oem_mtd->erasesize;
	}

	for (;
	     erase.addr < start + len;
	     erase.addr += oem_mtd->erasesize) {

//		WATCHDOG_RESET ();

		result = oem_mtd->erase(oem_mtd, &erase);
		if (result != 0) {
			printf("\n%s: MTD Erase failure: %d\n",
			       mtd_device, result);
		}
	}

	/* Resume bbt */
	chip->block_bad = nand_block_bad_old;
	chip->scan_bbt(oem_mtd);

	return 0;
}

/* FIXME: This function may have problem when giving
 * unaligned start addr.
 */
int oem_erase_markbad(uint32_t start, uint32_t len)
{
	struct erase_info erase;
	ulong erase_length;
	int result;
	struct mtd_oob_ops oob_opts;
	struct mtd_info *oem_mtd = &nand_info[nand_curr_device];
	struct nand_chip *chip = oem_mtd->priv;
	const char *mtd_device = oem_mtd->name;

	memset(&erase, 0, sizeof(erase));
	memset(&oob_opts, 0, sizeof(oob_opts));

	erase.mtd = oem_mtd;
	erase.len  = oem_mtd->erasesize;
	erase.addr = start;
	erase_length = len;

	if (erase_length < oem_mtd->erasesize) {
		printf("Warning: Erase size 0x%08lx smaller than one "	\
		       "erase block 0x%08x\n",erase_length, oem_mtd->erasesize);
		printf("         Erasing 0x%08x instead\n", oem_mtd->erasesize);
		erase_length = oem_mtd->erasesize;
	}

	for (;
	     erase.addr < start + len;
	     erase.addr += oem_mtd->erasesize) {

//		WATCHDOG_RESET ();

		/* Surely do bbtest */
		if (1) {
			int ret = oem_mtd->block_isbad(oem_mtd, erase.addr);
			if (ret > 0) {
				printf("\rSkipping bad block at  "
				   "0x%08x%08x                 "
				   "                         \n",
				   *(unsigned int *)(&(erase.addr)), *((unsigned int *)(&(erase.addr)) + 1));
				continue;
			} else if (ret < 0) {
				printf("\n%s: MTD get bad block failed: %d\n",
				       mtd_device,
				       ret);
				return -1;
			}
		}

		result = oem_mtd->erase(oem_mtd, &erase);
		if (result != 0) {
			printf("\n%s: MTD Erase failure: %d\n",
			       mtd_device, result);

			/* Put Bad Mark */
			if (oem_mtd->block_markbad(oem_mtd, erase.addr)) {
				printf("Block 0x%08lx NOT marked as bad!\n",
					(long unsigned int)erase.addr);
			}
			continue;
		}
	}
	/* Do not check JFFS2, no percentage information */

	/* Update BBT */
	chip->scan_bbt(oem_mtd);

	return 0;
}


/* This function write data to NAND.
 * Mark current block bad if a program error happened.
 * And copy current block to next block.
 * Automatically skip originally marked bad blocks.
 * FIXME: This function may not do very well with
 * not-block-aligned data.
 */
int oem_write_markbad(uint8_t *data, uint32_t start, uint32_t length, uint32_t max_len)
{
	uint32_t write_len, tmp_len;
	int ret;
	struct mtd_info *oem_mtd = &nand_info[nand_curr_device];
	struct nand_chip *chip = oem_mtd->priv;

	length = (length + (oem_mtd->writesize - 1)) & ~(oem_mtd->writesize - 1);
	while(length && max_len)
	{
		write_len = ((length > oem_mtd->erasesize)? oem_mtd->erasesize : length);
		tmp_len = write_len;

		/* Skip originally mark bad blocks */
		if(nand_block_isbad(oem_mtd, start))
		{
			printf("Skipping original bad block at 0x%08x\n", start);
			start += oem_mtd->erasesize;
			max_len -= oem_mtd->erasesize;
			continue;
		}

		ret = nand_write(oem_mtd, start, &tmp_len, data);
		/* If program failed, mark current bad & write data to next block*/
		if(ret)
		{
			/* Erase the fault block, to avoid mark fail */
			oem_erase_markbad(start, oem_mtd->erasesize);

			/* Put Bad Mark */
			printf("Mark bad: 0x%08x\n", start);
			if (oem_mtd->block_markbad(oem_mtd, start)) {
				/* FIXME: This case is very dangerous */
				printf("Block 0x%08x NOT marked as bad!\n",
					start);
			}

			start += oem_mtd->erasesize;
			max_len -= oem_mtd->erasesize;
			continue;
		}

		/* Program successed */
		start += oem_mtd->erasesize;
		data += oem_mtd->erasesize;
		max_len -= oem_mtd->erasesize;
		length -= write_len;
	}

	if(length)
	{
		printf("OEM NAND program failed because of too many bad blks.\n");
		return -1;
	}

	chip->scan_bbt(oem_mtd);
	printf("OEM NAND program finished.\n");
	return 0;
}

/* This function read data from NAND.
 * Mark current block bad if a uncorrectable error happened.
 * Automatically skip originally marked bad blocks.
 * FIXME: This function may not do very well with
 * not-block-aligned data.
 */
int oem_read_markbad(uint8_t *buf, uint32_t start, uint32_t length)
{
	uint32_t read_len, tmp_len;
	int ret;
	struct mtd_info *oem_mtd = &nand_info[nand_curr_device];
	struct nand_chip *chip = oem_mtd->priv;

	while(length)
	{
		read_len = ((length > oem_mtd->erasesize)? oem_mtd->erasesize : length);
		tmp_len = read_len;

		/* Check if exceed NAND size */
		if(start > chip->chipsize)
		{
			printf("Read exceed NAND size!\n");
			return -1;
		}

		/* Skip originally mark bad blocks */
		if(nand_block_isbad(oem_mtd, start))
		{
			printf("Skipping original bad block at 0x%08x\n", start);
			start += oem_mtd->erasesize;
			continue;
		}

		ret = nand_read(oem_mtd, start, &tmp_len, buf);
		/* If program failed, mark current bad & write data to next block*/
		if(ret)
		{
			printf("OEM NAND read failed because uncorrectalbe ECC error!\n");
			return -1;
#if 0
			/* Put Bad Mark */
			printf("Mark bad: 0x%08x\n", start);
			if (oem_mtd->block_markbad(oem_mtd, start)) {
				printf("Block 0x%08lx NOT marked as bad!\n",
					start);
			}

			start += oem_mtd->erasesize;
			max_len -= oem_mtd->erasesize;
			continue;
#endif
		}

		/* Program successed */
		start += oem_mtd->erasesize;
		buf += oem_mtd->erasesize;
		length -= read_len;
	}

//	chip->scan_bbt(oem_mtd);
	printf("OEM NAND read finished.\n");
	return 0;
}

/* Write data and oob informations to NAND, mark bad if program error happend */
/* This function will not erase NAND automatically, do it if needed */
/* FIXME: start must be block aligned */
int oem_write_yaffs(uint8_t *img, uint32_t size, uint32_t start, uint32_t max_len)
{
	struct mtd_oob_ops ops;
	struct mtd_info *oem_mtd = &nand_info[nand_curr_device];
	struct nand_chip *chip = oem_mtd->priv;
	int ret;
	int fpg = oem_mtd->writesize + oem_mtd->oobsize;
	int fblk =oem_mtd->oobsize*oem_mtd->erasesize/oem_mtd->writesize+oem_mtd->erasesize;
	uint32_t pg_start, consume_size, tmp_len;

	if((size % fpg) != 0)
	{
		printf("OEM_YAFFS: Data is not full page aligned, may be it is not yaffs data\n");
		return -1;
	}
	else if((size / fpg) > (max_len / oem_mtd->writesize))
	{
		printf("OEM_YAFFS: Attempt to write longer than space provided.\n");
		return -1;
	}

	// no need to pad, we do not write unaligned data
#if 0
	/* Pad length to page aligned */
	size = size + ((fpg - (size % fpg)) % fpg);
	printf("Padded size is %x\n", size);
#endif
	oem_erase_markbad(start, max_len);
	while(size && max_len)
	{
from_the_beginning:
		consume_size = ((size > fblk)? fblk : size);
		tmp_len = consume_size;

		/* Skip originally mark bad blocks */
		if(nand_block_isbad(oem_mtd, start))
		{
			printf("Skipping original bad block at 0x%08x\n", start);
			start += oem_mtd->erasesize;
			max_len -= oem_mtd->erasesize;
			continue;
		}

		/* Write data to current blk */
		ops.mode = MTD_OOB_AUTO;
		ops.ooboffs = 0;
		ops.len = oem_mtd->writesize;
		ops.ooblen = oem_mtd->oobsize;
		ops.datbuf = img;
		pg_start = start;
		while(tmp_len)
		{
			ops.oobbuf = ops.datbuf + oem_mtd->writesize;
			ret = oem_mtd->write_oob(oem_mtd, pg_start, &ops);
			if(ret)
			{
				/* Erase the fault block, to avoid mark fail */
				oem_erase_markbad(start, oem_mtd->erasesize);

				/* Put Bad Mark */
				printf("Mark bad: 0x%08x\n", start);
				if (oem_mtd->block_markbad(oem_mtd, start)) {
					/* FIXME: This case is very dangerous */
					printf("Block 0x%08x NOT marked as bad!\n",
					   start);
				}

				start += oem_mtd->erasesize;
				max_len -= oem_mtd->erasesize;

				goto from_the_beginning;

			} else {
				ops.datbuf += fpg;
				pg_start += oem_mtd->writesize;
				tmp_len -= fpg;

				if(oem_progress_update)
				{
					oem_progress_update(0);
				}
//				printf("write_oob OK: retoob: %d, retlen: %d\n",
//				   ops.oobretlen, ops.retlen);
			}
		}

		start += oem_mtd->erasesize;
		max_len -= oem_mtd->erasesize;
		size -= consume_size;
		img += fblk;
		printf("\r%d KB remains..", size / 1024);
	}
	printf("\n");

	if(size)
	{
		printf("OEM NAND program failed because of too many bad blks.\n");
		return -1;
	}

	chip->scan_bbt(oem_mtd);
	printf("OEM NAND program finished.\n");
	return 0;
}
