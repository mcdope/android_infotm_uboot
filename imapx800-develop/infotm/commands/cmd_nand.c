/*
 * Driver for NAND support, Rick Bronson
 * borrowed heavily from:
 * (c) 1999 Machine Vision Holdings, Inc.
 * (c) 1999, 2000 David Woodhouse <dwmw2@infradead.org>
 *
 * Added 16-bit nand support
 * (C) 2004 Texas Instruments
 */

#include <common.h>
#include <command.h>
#include <asm/io.h>
#include <bootlist.h>
#include <malloc.h>
#include <cdbg.h>
#include <nand.h>


int do_nand(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if(argc < 2)
	  return -1;

	if(strncmp(argv[1], "eccstat", 7) == 0)
	  ecc_print_stat();
	else if(strncmp(argv[1], "bad", 3) == 0)
	  base_scan_badblocks();
	else if(strncmp(argv[1], "markbad", 7) == 0) {
		if(argc < 3) {
			printf("nand markbad offset\n");
			return -1;
		}
		base_block_markbad(simple_strtoul(argv[2], NULL, 16));
	}
	else if(strncmp(argv[1], "id", 7) == 0) {
		uint32_t timing = 0x16323;
		uint8_t id[20];
		if(argc >= 3)
		  timing = simple_strtoul(argv[2], NULL, 16);
		nand_readid(id, 0, timing, 0xfffaf0, 0x4023, 0x3818, 8);
		printf("got id: %02x %02x %02x %02x %02x %02x %02x %02x\n",
					id[0], id[1], id[2],
					id[3], id[4], id[5],
					id[6], id[7]);
	}
	else if(strncmp(argv[1], "settiming", 10) == 0) {
		nf2_set_gtiming(simple_strtoul(argv[2], NULL, 16));
	}


	return 0;
}

U_BOOT_CMD(nand, CONFIG_SYS_MAXARGS, 1, do_nand,
	"nand utilities:",
	"eccstat\n"
	"bad\n"
	"markbad\n");


