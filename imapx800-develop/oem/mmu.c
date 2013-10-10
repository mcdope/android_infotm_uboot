#include <common.h>
#include <asm/system.h>

struct oem_mmu_table {
	uint32_t va;
	uint32_t pa;
	uint32_t len; /* MB */
};

const struct oem_mmu_table oem_mmu_table_data[] = {
	{0x00000000, 0x00000000,	1},		/* Boot Area 1MB */
	{0x20c00000, 0x20c00000,	128},		/* Phrephiral Area 128MB */
	{0x3e000000, 0x3e000000,	1},		/* BRAM Area 1MB */ 
	{0x40000000, 0x40000000,	2048},	/* DRAM Area 2GB */ 
	{0x10000000, 0x10000000,	8},	/* DRAM Area 2GB */ 
		/* 0x00000000, 0x00000000,	0		 END Mark */
};

static void oem_make_page_table(void)
{
	int i, j;
	uint32_t va, pa, l, addr, val;

	for(i = 0; i < ARRAY_SIZE(oem_mmu_table_data); i++)
	{
		va = oem_mmu_table_data[i].va & 0xfff00000;
		pa = oem_mmu_table_data[i].pa & 0xfff00000;
		l = oem_mmu_table_data[i].len;
		addr = CONFIG_RESV_PG_TB + (va >> 18);
		val = pa + 0x40e;
		for(j = 0; j < l; j++)
		{
			*(uint32_t *)addr = val;
			val += 0x100000;
			addr += 4;
		}
	}

	return ;
}

static inline void oem_set_30(unsigned int val)
{
	asm volatile("mcr p15, 0, %0, c3, c0, 0	@ set CR"
	  : : "r" (val) : "cc");
	isb();
}

static inline void oem_set_20(unsigned int val)
{
	asm volatile("mcr p15, 0, %0, c2, c0, 0	@ set CR"
	  : : "r" (val) : "cc");
	isb();
}

static inline void oem_set_87(unsigned int val)
{
	asm volatile("mcr p15, 0, %0, c8, c7, 0	@ set CR"
	  : : "r" (val) : "cc");
	isb();
}

void oem_mmu_enable(void)
{
	uint32_t cr;

	oem_make_page_table();
	oem_set_30(1);	/* Setup access to domain 0 */
	oem_set_20(CONFIG_RESV_PG_TB);	/* Setup access to domain 0 */
	oem_set_87(0x40e);	/* Setup access to domain 0 */

	cr = get_cr();
	cr = cr | 0x71 | 0x4 | 0x1000;
	set_cr(cr);
	nop();

	printf("MMU :  Enabled!!\n");
	return ;
}
