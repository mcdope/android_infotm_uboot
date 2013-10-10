#ifndef IMAPX800_DRAMC_INIT_H_
#define IMAPX800_DRAMC_INIT_H_

#include <common.h>

/* DRAM type */
#define mDDR            0
#define LPDDR2          1
#define DDR2            2
#define DDR3            3

/* burst len */
#define BURST_4         0
#define BURST_8         1

/* only for mDDR or LPDDR2 */
#define BURST_2         2
#define BURST_16        3

struct dram_timing{
	uint32_t tras;
	uint32_t trrd;
	uint32_t tfaw;
	uint32_t trfc;
	uint32_t trc;
	uint32_t twr;
	uint32_t tcwl;
	uint32_t trcd;
	uint32_t trp;
	uint32_t faw_eff;
	uint32_t trfc_d;
	uint32_t tras_d;
	uint32_t trefi;
};

#define TRFC_D_DEFAULT    0
#define TRAS_D_DEFAULT    0
#define TREFI_D_DEFAULT   0
#define TREFI_DEFAULT     78

#define TFAW_CFG       0
#define TFAW_CFG_OFF   16

#define TRRD_X_4       0
#define TRRD_X_5       1
#define TRRD_X_6       2

struct dram_umctl_r{
	uint32_t port[8];
	uint32_t ccfg; 
};

struct DRAM_INFO_STRUCT{
	uint32_t dram_type;            /* mDDR, LPDDR2, DDR2, DDR3 */
	uint32_t fr;                   /* frequency */
	uint32_t fr_100n;              /* fr / 10 */
	uint32_t cl;
	uint32_t burst_len;             
	uint32_t rank_sel;             /* 0000b      0: rank 0   1: rank 1    2: rank 2    3: rank 3 */
	uint32_t io_width;
	uint32_t density;
	uint32_t rtt;
	uint32_t driver;
	uint32_t bank;
	uint32_t row;
	uint32_t col;
	uint32_t reduce_flag;
	uint32_t training_flag;
	uint32_t timing_flag;
	uint32_t lpddr_type_en;        /* 0: LPDDR2 S2 or MDDR r14 c9    1: LPDDR2 S4 or MDDR r13 c10 */
	uint32_t tr_fr_max;
	uint32_t tr_pre_lv;
	struct dram_timing timing;
	struct dram_umctl_r umctl;
}; 

#define PORT0_V         0x00100130
#define PORT1_V         0x00100130
#define PORT2_V         0x00100130
#define PORT3_V         0x00100130
#define PORT4_V         0x00100130
#define PORT5_V         0x00100130
#define PORT6_V         0x00100130
#define PORT7_V         0x00100130

#define CCFG_V          0x10000018
#define DEFAULT_ZQCSI   0x00000000

/* rtt */
#define RTT_DIV_4       0x4
#define RTT_DIV_2       0x40
#define RTT_DIV_6       0x44

/* driver for ddr3 */
#define DRIVER_DIV_6    0x0
#define DRIVER_DIV_7    0x2

/* drive strength for lpddr2 */
#define DS_34P3_OHM     1    /* 34.3-ohm */
#define DS_40_OHM       2    /*   40-ohm */
#define DS_48_OHM       3    /*   48-ohm */
#define DS_60_OHM       4    /*   60-ohm */
#define DS_68P6_OHM     5    /* 68.6-ohm  reserve */
#define DS_80_OHM       6    /*   80-ohm */
#define DS_120_OHM      7    /*  120-ohm */

/* page_type */
#define P1KB_X8         1
#define P2KB_X16        0

#define CL2             2
#define CL3             3
#define CL4             4
#define CL5             5
#define CL6             6
#define CL7             7
#define CL8             8

/* SCTL   ; stat change */
#define INT             0          /* move to Init_mem from Config */
#define CFG             1          /* move to Config from Init_mem or Access */
#define GO              2          /* move to Access from Config */
#define SLEEP           3          /* move to Low_power from Access */
#define WAKEUP          4          /* move to Access from Low_power */

/* STAT */
#define STAT_INIT_MEM                 0
#define STAT_CONFIG                   1
#define STAT_CONFIG_REQ               2
#define STAT_ACCESS                   3
#define STAT_ACCESS_REQ               4
#define STAT_LOW_POWER                5
#define STAT_LOW_POWER_ENTRY_REQ      6
#define STAT_LOW_POWER_EXIT_REQ       7

/* MCMD */
#define MCMD_START                    31          /* start command    cleared when command is finished */
#define MCMD_CMD_ADD_DEL              24
#define MCMD_RANK_SEL                 20
#define MCMD_DATA                     4
 
#define MR0                           (0 << 17)     
#define MR1                           (1 << 17)
#define MR2                           (2 << 17)
#define MR3                           (3 << 17)

#define MCMD_DESELECT                 0           /* This is only for timing purpose, no actual direct */
#define MCMD_PREA                     1           /* precharge all */
#define MCMD_REF                      2           /* refresh */
#define MCMD_MRS                      3           /* mode register set */
#define MCMD_ZQCS                     4           /* ZQ calibration short       only applies to LPDDR2/DDR3 */
#define MCMD_ZQCL                     5           /* ZQ calibration long        only applies to LPDDR2/DDR3 */
#define MCMD_RSTL                     6           /* software driven reset      only applies to DDR3 */

#define MCMD_MRR                      8           /* mode register read */
#define MCMD_DPDE                     9           /* deep power down entry      only applies to mDDR/LPDDR2 */

/* PHY PGCR */
#define PGCR_RANKEN                  18

/* DCFG */
#define DCFG_IO_W                     0
#define DCFG_DENSITY                  2

#define DRAM_64MB                     0
#define DRAM_128MB                    1
#define DRAM_256MB                    2
#define DRAM_512MB                    3
#define DRAM_1GB                      4
#define DRAM_2GB                      5
#define DRAM_4GB                      6
#define DRAM_8GB                      7

#define IO_WIDTH8                     1
#define IO_WIDTH16                    2
#define IO_WIDTH32                    3 

#define DRAM_TYPE                    DDR3         /* mDDR, LPDDR2, DDR2, DDR3 */

/* for mDDR, FR should be 200 or 150 */

/* FR   0x216(533MHz) 0x1a0(400MHz) 0x150(333MHz) 0x10e(266MHz)  0xc8  0x95*/
/* 100n 0x35          0x28          0x21          0x1b           0x14  0xe*/
#define DRAM_FREQ                    444

#define DRAM_BURST                   BURST_8      /* BURST_4, BURST_8 */
#define DRAM_CL                      CL6          /* CL2 ~ 8    mDDR : CL2 or CL3 */
#define DRAM_RANK                    0x1          /* 0 : 3  rank 0~3 */
//#define DRAM_PAGE_TYPE               P1KB_X8      /* P1KB_X8, P2KB_X16 */
#define DRAM_IO_W                    IO_WIDTH8   /* IO_WIDTH8 IO_WIDTH16 IO_WIDTH32 */
#define DRAM_DENSITY                 DRAM_2GB     /* DRAM_64MB DRAM_128MB DRAM_256MB DRAM_512MB */
                                                  /* DRAM_1GB  DRAM_2GB   DRAM_4GB   DRAM_8GB   */
#define DRAM_IO_REDUCE               0            /* 1 : low 16 bits   0 : 32 bits*/
#define DRAM_RTT                     RTT_DIV_4    /* RTT_DIV_4 RTT_DIV_2 RTT_DIV_6 */
#define DRAM_DRIVER                  DRIVER_DIV_6  /* DRIVER_DIV_6 DRIVER_DIV_7 */
#define DRAM_LPDDR_TYPE              0

#define DRAM_TR_PRE_LV               5
#define DRMA_TR_FR_MAX               600

#define TEST_INC             0x8
#define TEST_WORD_NUM        0x400

/* TODO */
/*
 * H5TQ2G63DFR IO_WIDTH16 DRAM_2GB
 * H5TQ2G83CFR IO_WIDTH8  DRAM_2GB
 */

#define _512M_BYTE           0x20000000
#define _1G_BYTE             0x40000000

extern uint32_t dram_size_check(void);
extern uint32_t dramc_sturct_defualt_init(struct DRAM_INFO_STRUCT *dram);
extern uint32_t dramc_init_sub(struct DRAM_INFO_STRUCT *dram);
extern void Move_to_Config(void);
extern void Move_to_Access(void);

#endif /* IMAPX800_DRAMC_INIT_H_*/
