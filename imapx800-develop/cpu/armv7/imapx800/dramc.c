#include <dramc_init.h>
#include <asm/io.h>
#include <preloader.h>
#include <items.h>
#include <training.h>

//#define DRAM_DBG 

#ifdef DRAM_DBG
#define dram_log(x...) spl_printf(x)
#else
#define dram_log(x...)
#endif

static volatile int wake_up_flag = 0;
//extern uint32_t dramc_training(struct DRAM_INFO_STRUCT *dr);
/*
 * check reg bit
 */
inline uint32_t check_reg_bit(uint32_t reg_value, uint32_t bit){
	if(reg_value & (1 << bit))
		return 1;
	return 0;
}

int mini_div(int v1, int v2)
{
    int v = 0;
    int v100 = v2 * 100;

    while(v1 >= v100){
        v1 -= v100;
        v += 100;
    }

    while(v1 >= v2){
        v1 -= v2;
	v += 1;
    }
    return v;
}

/*
 * move to config state
 */
void Move_to_Config(void)
{
	uint32_t rddata = 0;
	
	rddata = rUPCTL_STAT & 0x7;
	
	switch(rddata)
	{
	case STAT_CONFIG:  /* is Config already */
		break;
		
	case STAT_LOW_POWER:   
		
		rUPCTL_SCTL = WAKEUP;
		Move_to_Config(); /* moves into Access state */
		break;
	
	case STAT_INIT_MEM:   /* the state is init_mem or access */
	case STAT_ACCESS:    
		
		rUPCTL_SCTL = CFG;  /* moves into Config state */
		
		Move_to_Config();
		break;
		
	default:
		/* FIXME TODO
		 * uPCTL is in a Transitional state and not in any of the previous operational states
		 * whether if set timeout 
		 * maybe the soft would staring here */
		
		Move_to_Config();  /* go to first step , say read uPCTL_stat */
		break;
		
	}
}

/*
 * move to access state
 */
void Move_to_Access(void)
{
	uint32_t rddata = 0;
	
	rddata = rUPCTL_STAT & 0x7;
	
	switch(rddata)
	{
	case STAT_ACCESS:  /* the state is already access */
		break;
		
	case STAT_CONFIG:  /* write go to sctl.state_cmd and poll stat.ctl_stat = config */
		rUPCTL_SCTL = GO; 
		Move_to_Access();
		break;
		
	case STAT_INIT_MEM:  /* write CFG to sctl.state_cmd and poll stat.ctl_stat = config */
		rUPCTL_SCTL = CFG;
		Move_to_Access();
		break;
		
	case STAT_LOW_POWER:
		
		rUPCTL_SCTL = WAKEUP;
		Move_to_Access(); /* moves into Access state */
		break;
		
	default:
		/* FIXME TODO
		 * uPCTL is in a Transitional state and not in any of the previous operational states
		 * whether if set timeout 
		 * maybe the soft would staring here */
		
		Move_to_Access();  /* go to first step , say read uPCTL_stat */
		break;
	}
}

/*
 * MCMD send command (it`s fot DDR2/DDR3)
 * 
 * 
 *******************************************************
 *memory initialization procedure  DDR2/DDR3/mDDR/LPDDR2
 *MCMD (here it`s for DDR2/DDR3)
 *   [31]        [27:24]     [23:20]    [19:17]   [16:4]     [3:0]
 *-----------------------------------------------------------------------
 * start_cmd   cmd_add_del   rank_sel     mr     data(mr)    cmd_opcode
 * 
 * rank_sel: rank0 [20] rank1 [21] rank2 [22] rank3[23]
 * mr: mr0 0 ; mr1 1; mr2 2; mr3 3
 * cmd_opcode: deselect 0         this is only for timing purpose no actual direct
 *                prea  1         precharge all
 *                 ref  2         refresh
 *                 mrs  3         mode register set
 *                zqcs  4         zq calibration shot      only applies to LPDDR2/DDR3
 *                zqcl  5         zq calibration long      only applies to LPDDR2/DDR3
 *                rstl  6         software driven reset    only applies to DDR3
 *                      7         reserved
 *                 mrr  8         mode register read
 *                dpde  9         deep power down entry    only applies to mDDR/LPDDR2
 * 
 * cmd_add_del   internal timers clock cycles 2^n  0~10 
 */
static uint32_t MCMD_Send_Command(uint32_t cmd_add_del, uint32_t rank_sel, uint32_t MR, uint32_t data, uint32_t cmd_opcode)
{
    uint32_t cmd = 0;
    
    cmd = ((1 << MCMD_START) | (cmd_add_del << MCMD_CMD_ADD_DEL) | (rank_sel << MCMD_RANK_SEL) | MR | (data << 4) | cmd_opcode);
    rUPCTL_MCMD = cmd;
    
    while(check_reg_bit(rUPCTL_MCMD, MCMD_START)); /* poll start_cmd = 0 */
    
    return 1;
}

/*
 * DDR2 init
 */
static void DDR2_init(struct DRAM_INFO_STRUCT *dram)
{	
	uint32_t i = 0;
	uint32_t tmp = 0;
	uint32_t rank_sel = dram->rank_sel;
	uint32_t cl = dram->cl;

	while(check_reg_bit(rUPCTL_MCMD, MCMD_START));
	/* write Deselect command to MCMD, specifying an additional delay in terms of internal times clock cycles of at least 400ns */
	MCMD_Send_Command(0, rank_sel, 0, 0, MCMD_DESELECT);          /* 400ns   2^n > 4*T_100N    128>108         0x80_00000 */
	MCMD_Send_Command(0, rank_sel, 0, 0, MCMD_PREA);              /* PREA                                      0x80f00001 */
	MCMD_Send_Command(0, rank_sel, MR2, 0, MCMD_MRS);             /* MR2                                       0x80f40003 */
	MCMD_Send_Command(0, rank_sel, MR3, 0, MCMD_MRS);             /* MR3                                       0x80f60003 */
	MCMD_Send_Command(0, rank_sel, MR1, 0x0004, MCMD_MRS);        /* MR1                                       0x80f20043  DLL enable */
    tmp = 0x0702 | (cl << 4);
	MCMD_Send_Command(0, rank_sel, MR0, tmp, MCMD_MRS);        /* MR for "DLL reset"   DLL enable           0x80f07523 */
	MCMD_Send_Command(0, rank_sel, 0, 0, MCMD_PREA);              /* PREA                                      0x80f00001 */
	MCMD_Send_Command(0, rank_sel, 0, 0, MCMD_REF);               /* REF ; repeat at least onece               0x80F00002 */
	MCMD_Send_Command(0, rank_sel, 0, 0, MCMD_REF);               /*                                           0x80f00002 */
	/* MR to initialize devece operation */
	tmp = 0x0602 | (cl << 4);
	MCMD_Send_Command(0, rank_sel, MR0, tmp, MCMD_MRS);        /* MR for "DLL reset"  bl 4, normal mode, cl=5 0x80f06523 */
	
	for(i = 0; i < 100; i++);
	MCMD_Send_Command(0, rank_sel, MR1, 0x384, MCMD_MRS);         /* at least 200n_clk after "DLL Reset" .write MR1 command for "OCD Default Command" */
	MCMD_Send_Command(0, rank_sel, MR1, 0x004, MCMD_MRS);         /* write MR1 command to MCMD . for "OCD Calibration Mode Exit" */
}

static inline void check_dram_cmd(void)
{

	while(check_reg_bit(rUPCTL_MCMD, MCMD_START));
}
/*
 * DDR3 init
 */
static void DDR3_init(struct DRAM_INFO_STRUCT *dram)
{
    uint32_t tmp;
    uint32_t cl = dram->cl;
//	irf.printf("dram ddr3....   \n");

    /* TODO */
    while(check_reg_bit(rUPCTL_MCMD, MCMD_START));
    rUPCTL_MCMD = 0x86f00000;
    check_dram_cmd();
    if(!wake_up_flag){
    	tmp = 0x80f40003 | ((dram->timing.tcwl - 5) << 7);
        rUPCTL_MCMD = tmp;
        check_dram_cmd();
        rUPCTL_MCMD = 0x80f60003;
        check_dram_cmd();
        // rtt 
        rUPCTL_MCMD = 0x80f20003 | (dram->rtt << 4) | (dram->driver << 4);
        check_dram_cmd();
        /* cl, wr */
        tmp = 0x80f01003 | ((cl - 4) << 8) | ((dram->timing.twr - 4) << 13);
        rUPCTL_MCMD = tmp;	
        check_dram_cmd();
        rUPCTL_MCMD = 0x80f00005;
        check_dram_cmd();
    }
}
/*
 * LPDDR2 init
 */
void LPDDR2_init(struct DRAM_INFO_STRUCT *dram)
{
    while(check_reg_bit(rUPCTL_MCMD, MCMD_START));

    rUPCTL_MCMD = 0x86f00000;
    check_dram_cmd();
    if(!wake_up_flag){
        rUPCTL_MCMD = 0x81fc30a3;
        check_dram_cmd();
        if(dram->lpddr_type_en){
            rUPCTL_MCMD = 0x89fff0a3;
            check_dram_cmd();
        }
        rUPCTL_MCMD = 0x80f00013 | (((dram->burst_len + 2) | ((dram->timing.twr - 2) << 5)) << 12);
        check_dram_cmd();
        rUPCTL_MCMD = 0x80f00023 | ((dram->cl - 2) << 12);
        check_dram_cmd();
        rUPCTL_MCMD = 0x80f00033 | (dram->driver << 12);
        check_dram_cmd();
    }
}

/*
 * mDDR init
 */
static void mDDR_init(struct DRAM_INFO_STRUCT *dram)
{
    uint32_t cl = dram->cl;
    /* TODO */
    while(check_reg_bit(rUPCTL_MCMD, MCMD_START));

    rUPCTL_MCMD = 0x86f00000;
    check_dram_cmd();
    if(!wake_up_flag){
        rUPCTL_MCMD = 0x80f00001;
        check_dram_cmd();
        rUPCTL_MCMD = 0x80F00002;
        check_dram_cmd();
        rUPCTL_MCMD = 0x80f00002;
        check_dram_cmd();
	if(cl == CL2){
            rUPCTL_MCMD = 0x80f00223;
	}else if(cl == CL3){
            rUPCTL_MCMD = 0x80f00323;
	}
        check_dram_cmd();
        rUPCTL_MCMD = 0x80f40003;
        check_dram_cmd();
    }
}


/*
 * memory init     mDDR/LPDDR2/DDR2/DDR3
 */
static inline void DWC_Memory_init(struct DRAM_INFO_STRUCT *dram)
{
    uint32_t type = dram->dram_type;

	if(type == mDDR){
    	mDDR_init(dram);
	}else if(type == LPDDR2){
	    LPDDR2_init(dram);
	}else if(type == DDR2){
	    DDR2_init(dram);
	}else if(type == DDR3){
	    DDR3_init(dram);
	}
}

/*
 * DWC_DDR_PHY init
 * 
 * to an uninitialized PHY          
 * 
 *                          DLL soft reset (optional)
 * Impedance Calibration        DLL  Lock
 *                          ITM Soft Reset (optional)
 */ 
static inline void PHY_init(uint32_t dram_type)
{
	uint32_t rddata = 0;

    /************************
    ;Start PHY initialization
    ;FIXME TODO
    ;there`re still some registers that like DXCCR(datx8 common configuration register) dqsres/dqsnres
    ;in fact,the dqs is more important the odt and dram`s half/full
    ;and DXnGCR DXnGSR0-1 DXnDLLCR DXnDQTR DXnDQSTR ??
    ;there are the registers config to the lane ??
    ;impedance Calibration bypass or how to init impedance Calibtation
    ************************/
	/* FIXME  impedance Calibration bypass or  how to init impedance Calibtation */

	if(wake_up_flag){
	    rPHY_PIR = 0x40000001;  /* zq by pass */
	    while(1){
	        rddata = rPHY_PGSR & 0x1;
		if(rddata == 0x1)
		    break; 
	    }
            rPHY_ZQ0CR0 = 0x1000014a;
            *(volatile int *)(0x21e09c4c) = 0xe;
            *(volatile int *)(0x21e09c7c) = 0x1; 
            rPHY_ZQ0CR0 = 0x0000014a;
	    rPHY_PIR = 0x9;
	    while(1){
	        rddata = rPHY_PGSR & 0x5;
		if(rddata == 0x5)
		    break;
	    }

	}else{
	    if(dram_type == mDDR){
   	        rPHY_PIR = 0x40000007;
		while(1){
		    rddata = rPHY_PGSR & 0x3;
		    if(rddata == 0x3)
	                break;
		}
	    }else{
       	        rPHY_PIR = 0x0000000f;
	        while(1){
		    /* FIXME   whether if check impedance calibration */
		    /* rddata = rPHY_PGSR & 0x7 */
          	    rddata = rPHY_PGSR & 0x7;
		    if(rddata == 0x7)
			break;
	        }  
	    } 
        }
	rPHY_PIR = 0x0000011;
	while(1){
	        rddata = rPHY_PGSR & 0x3;
		if(rddata == 0x3)
		    break;
	}
}

const uint32_t wl[6] = {1, 2, 2, 3, 4, 4};  /* for lpddr2 */

/*
 * Program all timing T* registers
 * 
 * #define in the file dramc.h    DRAM_TYPE mDDR/LPDDR2/DDR2/DDR3
 */
static void Timing_init(struct DRAM_INFO_STRUCT *dram)
{
	int tmp = mini_div(1000000, (int)dram->fr);
	uint32_t type = dram->dram_type;
	uint32_t cl = dram->cl;
	struct dram_timing *tim = &dram->timing; 

	if(type == DDR2){
    	rUPCTL_TMRD   =  0x00000002;                      /* tmrd */
    	rUPCTL_TRFC   =  0x00000022;                      /* trfc  270M 1G 0x22  2G 0x34 */
    	rUPCTL_TRP    =  (0x00010000 | cl);                      /* trp */
    	rUPCTL_TRTW   =  0x00000004;                      /* twtr */
    	rUPCTL_TCWL   =  cl - 1;                          /* tcwl cl - 1 */
    	rUPCTL_TRCD   =  cl;     
    	rUPCTL_TRTP   =  0x00000003;    
    	rUPCTL_TWTR   =  0x00000003;  
    	rUPCTL_TEXSR  =  0x000000c8;                      /* texsr  200n_clk */
    	rUPCTL_TXPDLL =  0x00000003;                      /* tdll  legal value 3...63 ??????? */
    	rUPCTL_TZQCS  =  0x00000000;                      /* tzqcs ddr2 0 */
    	rUPCTL_TZQCSI =  0x00000002;                      /* tzqcsi ddr2 0 */
    	rUPCTL_TCKE   =  0x00000003;                      /* tcke */ 
    	rUPCTL_TZQCL  =  0x00000000;  
    	rUPCTL_TCKESR =  0x00000000;
	}else if(type == DDR3){
    	rUPCTL_TMRD   =  0x00000004;                      /* tmrd */
    	// TODO
    	rUPCTL_TRFC   =  tim->trfc - tim->trfc_d;         /* trfc fre and density */
    	rUPCTL_TRP    =  cl;                              /* trp */
    	// TODO
    	/* twtr bl8   RL + tccd(4) + 2tck - wl   rl(al + cl)  wl(al + cwl) */
    	rUPCTL_TRTW   =  cl + 6 - tim->tcwl;                      
        rUPCTL_TCWL   =  tim->tcwl;
    	rUPCTL_TRCD   =  cl;       
    	rUPCTL_TRTP   =  0x00000004;   
    	rUPCTL_TWTR   =  0x00000004;  
    	rUPCTL_TEXSR  =  0x00000200;                      /* texsr  200n_clk */
    	rUPCTL_TXPDLL =  0x0000003f;                      /* tdll  legal value 3...63 ??????? */
    	rUPCTL_TZQCS  =  0x00000040;                      /* tzqcs ddr2 0 */
    	rUPCTL_TZQCSI =  DEFAULT_ZQCSI;                   /* tzqcsi ddr2 0 */
    	rUPCTL_TCKSRE =  0x00000005;
    	rUPCTL_TCKSRX =  0x00000005;
    	rUPCTL_TCKE   =  0x00000003;                      /* tcke */
    	rUPCTL_TMOD   =  0x0000000c;                      /* 12    */  
    	rUPCTL_TRSTL  =  0x00000040; 
    	rUPCTL_TZQCL  =  0x00000210;  
    	rUPCTL_TCKESR =  0x00000004;  
	}else if(type == mDDR){
	    // TODO
        rPHY_ZQ0CR0   =  0x100631EF;

    	rUPCTL_TMRD   =  0x00000002;                      /* tmrd */
    	rUPCTL_TRFC   =  0x00000012;                      /* trfc  270M 1G 0x22  2G 0x34 */
    	rUPCTL_TRP    =  cl;                      /* trp */
    	rUPCTL_TRTW   =  0x00000003;                      /* twtr */
	    rUPCTL_TCWL   =  1;
    	rUPCTL_TRCD   =  cl;    
    	rUPCTL_TRTP   =  0x00000000;     
    	rUPCTL_TWTR   =  0x00000002;  
    	rUPCTL_TEXSR  =  0x000000d0;                      /* texsr  200n_clk */
    	rUPCTL_TXPDLL =  0x00000000;                      /* tdll  legal value 3...63 ??????? */
    	rUPCTL_TZQCS  =  0x00000000;                      /* tzqcs ddr2 0 */
    	rUPCTL_TZQCSI =  0x00000000;                      /* tzqcsi ddr2 0 */
    	rUPCTL_TCKE   =  0x00000002;                      /* tcke */ 
    	rUPCTL_TZQCL  =  0x00000000;  
    	rUPCTL_TCKESR =  0x00000000;  
	}else if(type == LPDDR2){
    	rUPCTL_TMRD   =  0x00000005;                      /* tmrd */
    	rUPCTL_TRFC   =  tim->trfc - tim->trfc_d;         /* trfc REFab */
    	rUPCTL_TRP    =  0x00020000 | (tim->trp);                      /* trp */
    	rUPCTL_TRTW   =  0x00000004;                      /* twtr */
	    rUPCTL_TCWL   =  wl[cl - 3];                      /* same to wl */
    	rUPCTL_TRCD   =  tim->trcd;  
    	rUPCTL_TRTP   =  0x00000000;    
    	rUPCTL_TWTR   =  0x00000004;  
    	rUPCTL_TEXSR  =  110;                             /* texsr  max 117 */
    	rUPCTL_TXPDLL =  0x00000000;                      /* tdll */
    	rUPCTL_TZQCS  =  mini_div(75000, tmp);                      /* tzqcs ddr2 0 */
    	if(dram->lpddr_type_en)
    	    rUPCTL_TZQCSI =  0x0000a000;                      /* tzqcsi ddr2 0 */
    	else
    		rUPCTL_TZQCSI =  0x00000000;                      /* tzqcsi ddr2 0 */
    	rUPCTL_TZQCL  =  mini_div(320000, tmp);
    	rUPCTL_TCKESR =  0x00000005;
    	rUPCTL_TDPD   =  0x00000005;  
	}
	rUPCTL_TREFI  =  tim->trefi;                         /* 7.8us   78 100n        4e */
	rUPCTL_TAL    =  0x00000000;                         /* tal */
	rUPCTL_TCL    =  cl;                                 /* tcl */
	rUPCTL_TRAS   =  tim->tras;                          /* tras */
	rUPCTL_TRC    =  tim->trc;                           /* trc     tras + trp */
	rUPCTL_TRRD   =  tim->trrd;
	rUPCTL_TWR    =  tim->twr;
	rUPCTL_TXP    =  0x00000006;                         /* txp   exit power down  */
	rUPCTL_TDQS   =  0x00000002;                         /* tdqs */
}

/*
 * Built in DQS gate training
 */
static inline void built_in_dqs_gate_trainig(void)
{
	uint32_t ret = 0;
	
	/* [11:0], column address
	 * [27:12], row address
	 * [30:28], bank address
	 * FIXME: this should be adjust according to DDR type in items
	 */
	rPHY_DTAR = 0x7ffff000;
	rPHY_PIR = 0x81;

	ret = rPHY_PGSR;
	while((ret & 0x10) != 0x10){
		ret = rPHY_PGSR;	
	}
}

/*
 * uPCTL neccessary set
 * 
 * Program if neccessary TOGCNT1U  TOGCNT100N  TINIT  TRSTH
 */
static inline void uPCTL_neccessary_set(uint32_t fr, uint32_t dram_type, uint32_t fr_100n)
{
	// TODO
//	if(dram_type == DDR3)
//	  rPHY_PIR = 0x00020000;               /* dll bypss ???  */
	rUPCTL_TOGCNT1U = fr;                  /* 1us   number of  internal timers clock , it has the same value of frequency */
	if(dram_type == mDDR)
	    rUPCTL_TINIT = 0xd0;                    /* in us   at least 200us     in Verilog simulations may set 1 */
    else
        rUPCTL_TINIT = 0xf0;                    /* in us   at least 200us     in Verilog simulations may set 1 */
	if(dram_type == DDR3)                  /* DDR2 0       DDR3 500us */
		rUPCTL_TRSTH = 0x200;          
	else
		rUPCTL_TRSTH = 0;
    /* t_1u/10 */ 
    rUPCTL_TOGCNT100N = fr_100n;
}


static inline uint32_t PHY_others_set(struct DRAM_INFO_STRUCT *dram)
{
	uint32_t tmp;
	uint32_t cl = dram->cl & 0xf;
	struct dram_timing *tim = &dram->timing;
	
    if(dram->dram_type == DDR3){
	    rPHY_PGCR = 0x01802E06 | dram->rank_sel << (PGCR_RANKEN);
        rPHY_DCR  = 0xb;
        rPHY_MR0  = ((cl - 4) << 4) | ((tim->twr - 4) << 9);
        rPHY_MR1  = (dram->rtt) | (dram->driver);
        rPHY_MR2  = (tim->tcwl - 5) << 3;
        rPHY_MR3  = 0;
        /* 
         * tmrd   trtp    twtr    trp     trcd     tras     trrd      trc       tccd
         * -----------------------------------------------------------------------------
         *  1:0   4 : 2   7 : 5   11 : 8  15 : 12  20 : 16  24 : 21   30 : 25    31
         *  0     4       4       cl      cl       ras      rrd       ras + rp   0
         *   */
        tmp = 0x90 | (cl << 8) | (cl << 12) | (tim->tras << 16) | (tim->trrd << 21) | (tim->trc << 25);
        rPHY_DTPR0 = tmp;
        /* 
         *        trtw   tfaw    tmod    trtodt     trfc     
         * --------------------------------------------------
         *  1:0    2     8 : 3   10 : 9    11      23 : 16   
         *  0      0?      faw     0       0?       rfc      
         *   */
        tmp = (tim->trfc << 16) | (tim->tfaw << 3);
        rPHY_DTPR1 = tmp;
        /* 
         *   txs      txp        tcke     tdllk        
         * -------------------------------------
         *   9 : 0    14 : 10    18 : 15  28 : 19       
         *    512     11         4        512           
         *   */
        rPHY_DTPR2 = 0x10022c00;
        rPHY_PTR0  = 0x0020051b;
    }else if(dram->dram_type == mDDR){
	    rPHY_PGCR = 0x018c2e05;
        rPHY_DCR  = 0x0;
        if(cl == CL2){
           rPHY_MR0 = 0x22;
        }else if(cl == CL3){
           rPHY_MR0 = 0x32;
        }
        rPHY_MR1  = 0x0;
        rPHY_MR2 =  0x0;
        rPHY_MR3  = 0x0;
        rPHY_DTPR0 = 0x16483342;       
        rPHY_DTPR1 = 0x001010a0;
        rPHY_DTPR2 = 0x064198c8;
        rPHY_PTR0  = 0x0020051b;
        
        rPHY_ACIOCR = 0x30c01813;
        rPHY_DXCCR  = 0x00004802;	
    }else if(dram->dram_type == LPDDR2){
	    rPHY_PGCR = 0x01802E02 | (dram->rank_sel << PGCR_RANKEN);
	    if(dram->lpddr_type_en)
            rPHY_DCR  = 0x4 | ((dram->bank - 2) << 3) | (0 << 8);
	    else
	    	rPHY_DCR  = 0x4 | ((dram->bank - 2) << 3) | (1 << 8);
        /* no mr0 */
        rPHY_MR1  = (dram->burst_len + 2) | ((tim->twr - 2) << 5);  /* not consider burst 16 */
        rPHY_MR2 =  dram->cl - 2;                                   /* for lpddr2 cl = rl */
        rPHY_MR3  = dram->driver;
        
        // TODO
        tmp = 0x80 | (tim->trp << 8) | (tim->trcd << 12) | (tim->tras << 16) | (tim->trrd << 21) | (tim->trc << 25);
        rPHY_DTPR0 = tmp;
        //rPHY_DTPR0 = 0x2a8d8893;   
        tmp = 0x19000000 | (tim->trfc << 16) | (tim->tfaw << 3);
        rPHY_DTPR1 = tmp;
        //rPHY_DTPR1 = 0x19340088;
        rPHY_DTPR2 = 0x0647a0c8;
        rPHY_PTR0  = 0x0020051b;
        
        rPHY_DXCCR = 0x00000910;
        rPHY_DSGCR = 0xfa00013f;
    }
    if(dram->rank_sel != 0x3)
    	rPHY_ODTCR = 0;

	return 0;
}

/*
 * uPCTL MCFG config
 * 
 * 
 */
static inline uint32_t uPCTL_MCFG_config(struct DRAM_INFO_STRUCT *dram)
{
    uint32_t bl_value[4] = {1, 2, 0, 3}; /* only for lpddr2 and mddr */
    uint32_t dram_type = dram->dram_type;
    uint32_t tmp;
    uint32_t burst_len = dram->burst_len;

	if(dram_type == DDR2){
	    /* Config uPCTL`s MCFG */
	    rUPCTL_MCFG = (0x00060000 | (burst_len & 0x1));             /* DDR2  tFAW = 5*tRRD  BL = 4    fast exit */
	}else if(dram_type == DDR3){
	    rUPCTL_MCFG = (0x00020020 | (burst_len & 0x1)) | ((dram->timing.faw_eff & 0x3) << 18);             /* DDR3  tFAW BL fast exit */
	    tmp = rUPCTL_MCFG1;
	    tmp = tmp | (((dram->timing.faw_eff & 0x7) >> TFAW_CFG_OFF) << 8);
	    rUPCTL_MCFG1 = tmp;
	}else if(dram_type == mDDR){
	    rUPCTL_MCFG = (0xff860000 | ((bl_value[burst_len] & 0x3) << 20));
	}else if(dram_type == LPDDR2){
		//rUPCTL_MCFG = (0xffc60040 | ((bl_value[burst_len] & 0x3) << 20) | (dram->lpddr_type_en << 6));
		rUPCTL_MCFG = (0x00c60040 | ((bl_value[burst_len] & 0x3) << 20) | (dram->lpddr_type_en << 6));
	    tmp = rUPCTL_MCFG1;
	    tmp = tmp | (((dram->timing.faw_eff & 0x7) >> TFAW_CFG_OFF) << 8);
	    rUPCTL_MCFG1 = tmp;
	}
	
    return 0;	
}

/*
 * DFI timing config
 * 
 * ensure compatibility on the read and write paths between uPCTL and PHY
 */
static inline uint32_t DFI_timing_config(struct DRAM_INFO_STRUCT *dram)
{
	uint32_t cl = dram->cl;
	uint32_t type = dram->dram_type;
	//rUPCTL_DFITCTRLDELAY = 0x00000002;    //[3:0] tctrl_delay  specifies DFI n_clk between reflection to the assertion or de-assertion

	/*********************************** 	
	; DFI ODT configuration register
	; 8n+[7:5]    8n+[4]   8n+[3]      8n+[2]    8n+[1]     8n+[0]
    ;----------------------------------------------------------------------
    ; reserved   default  write_sel  write_nsel read_sel   read_nsel
    ; default: default odt value of rank_n when there is no r/w activity                             0
    ; write_sel: en/dis odt fot rank_n when a write access is occurring on this rank                 1
    ; write_nsel: en/dis odt for rank_n when a write access is occurring on a different rank         1
    ; read_sel: en/dis odt fot rank_n when a read access is occurring on this rank                   0
	; read_nsel: en/dis odt for rank_n when a read access is occurring on a different rank           1
	***********************************/
	rUPCTL_DFITCTRLDELAY = 0x0;    // alin ::
	rUPCTL_DFITCTRLUPDMIN = 0x2;
	rUPCTL_DFITCTRLUPDMAX = 0x4;
	rUPCTL_DFITCTRLUPDDLY = 0x0;
	if((type == LPDDR2) || (type == mDDR))
		rUPCTL_DFIODTCFG = 0;
	else{
		if(dram->rank_sel == 1)
		    rUPCTL_DFIODTCFG = 0x8;      // low rank
		else if(dram->rank_sel == 2)
		    rUPCTL_DFIODTCFG = 0x800;    // high rank
		else
	    	rUPCTL_DFIODTCFG = 0x0505;   // two rank	
	}
	/***********************************
    ; DFIODTCFG1  (for latency and length)
    ;   [26:24]       [18:16]          [12:8]       [4:0]
    ;---------------------------------------------------------------
    ; odt_len_bl8_r  odt_len_bl8_w    odt_lat_r     odt_lat_w
    ;
    ;odt_len_bl8: length of dfi_odt signal for BL9  this is in terms of SDR cycles For BL4 ,the length of dfi_odt is always 2 cycles shorter than
    ;             the value in this register field For 2T mode the maximum supported value for this field is 6; otherwise it is 7
    ***********************************/
	if(dram->dram_type == DDR3)
	    rUPCTL_DFIODTCFG1 = 0x06060000 | ((cl - dram->timing.tcwl) << 8);           
	//rUPCTL_DFIODTRANKMAP = 0x00008421;        // 0000   0 odt_rank_map3, 0 odt_rank_map2, 0 odt_rank_map1, 0 odt_rank_map0
	                             
	//rUPCTL_DFITPHYWRDATA = 0x00000001;         //      
	if(type == DDR2){
    	    rUPCTL_DFITPHYWRLAT = cl - 2;            // ??   cl = 5 ??                                                                
    	    rUPCTL_DFITRDDATAEN = cl - 2;        
	}else if(type == DDR3){
	    if(dram->timing.tcwl == 5)
        	rUPCTL_DFITPHYWRLAT = 4;            // ??   cl = 5 ??                                                                
	    else
        	rUPCTL_DFITPHYWRLAT = 5;            // ??   cl = 5 ??                                                            
            rUPCTL_DFITRDDATAEN = cl - 2;        
	}else if(type == mDDR){
            rUPCTL_DFITRDDATAEN = cl - 2;         
	        rUPCTL_DFITPHYWRLAT = 0;            // ??   cl = 5 ??                                                            
	}else if(type == LPDDR2){
		    rUPCTL_DFITPHYWRLAT = wl[cl - 3];
		    rUPCTL_DFITRDDATAEN = cl - 1;     
	}
	//rUPCTL_DFITPHYRDLAT = 0x00000001;         
	                             
	//rUPCTL_DFITPHYUPDTYPE0      
	//rUPCTL_DFITPHYUPDTYPE1      
	//rUPCTL_DFITPHYUPDTYPE2      
	//rUPCTL_DFITPHYUPDTYPE3      
	//rUPCTL_DFITCTRLUPDMIN       
	//rUPCTL_DFITCTRLUPDMAX       
	//rUPCTL_DFITCTRLUPDDLY       
	rUPCTL_DFIUPDCFG = 0;
	//rUPCTL_DFITREFMSKI          
	//rUPCTL_DFITCTRLUPDI         
    
	/* TODO  
     * these regs are init after ppcfg */
#if 0    
 	rUPCTL_DFISTCFG0            
	rUPCTL_DFISTCFG1            
	//rUPCTL_DFITDRAMCLKEN        
	//rUPCTL_DFITDRAMCLKDIS       
	rUPCTL_DFISTCFG2            
	//rUPCTL_DFISTPARCLR          
	//rUPCTL_DFISTPARLOG       
	/***********************************
    ;DFI low power registers
    ;    [31:28]           [24]          [19:16]        [15:12]
    ;------------------------------------------------------------------
    ;dfi_lp_wakeup_dpd  dfi_lp_en_dpd  dfi_tlp_resp  dfi_lp_wakeup_sr
    ;      [8]             [7:4]             [0]
    ;------------------------------------------------------------------
    ; dfi_lp_en_sr     dfi_lp_wakeup_pd  dfi_lp_en_pd        
    ***********************************/          
	rUPCTL_DFILPCFG0            
#endif                                                    
	//rUPCTL_DFITRWRLVLRESP0      
	//rUPCTL_DFITRWRLVLRESP1      
	//rUPCTL_DFITRWRLVLRESP2      
	//rUPCTL_DFITRRDLVLRESP0      
	//rUPCTL_DFITRRDLVLRESP1      
	//rUPCTL_DFITRRDLVLRESP2      
	//rUPCTL_DFITRWRLVLDELAY0     
	//rUPCTL_DFITRWRLVLDELAY1     
	//rUPCTL_DFITRWRLVLDELAY2     
	//rUPCTL_DFITRRDLVLDELAY0     
	//rUPCTL_DFITRRDLVLDELAY1     
	//rUPCTL_DFITRRDLVLDELAY2     
	//rUPCTL_DFITRRDLVLGATEDELAY0 
	//rUPCTL_DFITRRDLVLGATEDELAY1 
	//rUPCTL_DFITRRDLVLGATEDELAY2 
	//rUPCTL_DFITRCMD             
	
    return 0;	
}

int intScreen = 0;
void high_res_screen(void)
{

   if( item_exist("memory.highres") && (item_integer("memory.highres", 0) == 1))
   {
       intScreen = 1;
   }else
   {
       intScreen = 0;
   }
}

/*
 * uPCTL others config
 * 
 * Configure uPCTL to refine configuration
 */
uint32_t uPCTL_others_config(struct DRAM_INFO_STRUCT *dr)
{
	uint32_t i = 0;
	uint32_t type = dr->dram_type;
	uint32_t io_w = dr->io_width;
	uint32_t density = dr->density;
	uint32_t reduce = dr->reduce_flag;
	struct dram_umctl_r *umctl = &dr->umctl;
	
	/* partially populated memories configuration
	 * [8:1] rpmem_dis   0 lane exists   1 lane is disabled,  [0]  ppmem_en (reduced size) */
	if(reduce){
		rUPCTL_PPCFG = 0x1d;
		i = rPHY_DX2GCR;
		i &= ~1;
		rPHY_DX2GCR = i;
		i = rPHY_DX3GCR;
		i &= ~1;
		rPHY_DX3GCR = i;
	}else
		rUPCTL_PPCFG = 0;
	
	/***********************************
        ;DFI low power registers
        ;    [31:28]           [24]          [19:16]        [15:12]
        ;------------------------------------------------------------------
        ;dfi_lp_wakeup_dpd  dfi_lp_en_dpd  dfi_tlp_resp  dfi_lp_wakeup_sr
        ;      [8]             [7:4]             [0]
        ;------------------------------------------------------------------
        ; dfi_lp_en_sr     dfi_lp_wakeup_pd  dfi_lp_en_pd        
        ***********************************/          
//	rUPCTL_DFILPCFG0 = 0x00070101; 
	
	/********************************
	 * UMCTL registers
	 *
	 * PCFG_n      port n configuration register    n=0:31
	 *   [26:24]   [23:16]   [13:8]         [5]        [4]         [1:0]
	 * ----------------------------------------------------------------------------
	 *   pkt_len   quantum  dir_grp_cnt   bp_rd_en   bp_wr_en    qos_class
	 *
	 * pkt_len: 1, 2, 4, 8, 16, 32, 64 (0~6)    packet length = (bl/2) * 2^pkt_len
	 * quantum: number of data beats(read and write) to send in a round form a port      must be equal or greater than max allow burst length
	 * dir_grp_cnt: direction group count, number of requests of the same derection to group together in a port ???
	 * qos_class: best effort 00,  low latency 01
         *******************************/ 
	for(i = 0; i < 8; i++)
		rUMCTL_PCFG(i) = umctl->port[i];
	/******************************************
        ;CCFG   controller configuration register
        ; [27:26]        [25:24]          [23:16]     [15:14]           [13:6]
        ;------------------------------------------------------------------------------------------
        ;timeout_en  timeout_bank_mult timeout_bank timeout_qos_mult  timeout_qos
        ;   [5]	          [4]              [3]           [1]
        ;------------------------------------------------------------------------------------------
        ;bnkwait_en    akeep_def         akeep_en     bbflags_en
        ;
        ;timeout_en: timeout disabled 00, bank timeout enabled 01, qos timeout enabled 10, both timeouts enabled 11
        ;timeout_bank_mult: CCFG.timeout_bank *1  00, *16  01, *64  10
        ;timeout_bank:  32~255
        ;timeout_qos_mult:  CCFG.timeout_qos  *1, 00, *16  01, *64  10
        ;timeout_qos:   32~255
        *****************************************/
	rUMCTL_CCFG = umctl->ccfg;
	rUMCTL_CCFG1 = 0x044320c8;
	/******************************************                
        ;DCFG  DRAM configuration register
        ; [10:8]      [6]       [5:2]         [1:0]
        ;------------------------------------------------------------------
        ;addr_map  dram_type  dram_density  dram_io_width
        ;
        ;addr_map: {rank,bank,row,col} 000, {rank,row,bank,col} 001, {bank,row,rank,col} 010, fixed(external)address mapping 011
        ;dram_density: 64M 0000,128M 0001,256M 0010,512M 0011,1G 0100,2G 0101,4G 0110,8G 0111
        ;dram_io_width: *8   01, *16   10, *32   11
    
        0x00000111                        ; {rank,bank,row,column} ,1G, *8
        0x00000112                        ; {rank,bank,row,column} ,2*1Gbit (64M*16), chip refer to DRAM deive
        *****************************************/
#if defined(FPGA_S2C_TEST)
	rUMCTL_DCFG = 0x00000111;
#elif defined(FPGA_HAPS_TEST)
	rUMCTL_DCFG = 0x00000112;
#else
	if(type == mDDR){
	    if(density == DRAM_1GB)
         	    rUMCTL_DCFG = 0x00000100 | (io_w << DCFG_IO_W) | (density << DCFG_DENSITY) | (dr->lpddr_type_en << 6);
	    else
    	        rUMCTL_DCFG = 0x00000140 | (io_w << DCFG_IO_W) | (density << DCFG_DENSITY);
	}else
        {
                if (intScreen)
                {  
		  rUMCTL_DCFG = 0x00000000 | (io_w << DCFG_IO_W) | (density << DCFG_DENSITY) | (dr->lpddr_type_en << 6);
                }else{    
                  rUMCTL_DCFG = 0x00000100 | (io_w << DCFG_IO_W) | (density << DCFG_DENSITY) | (dr->lpddr_type_en << 6);
                }
        }
#endif
	
    return 0;	
}

/*
 * uPCTL and Memory Initialization with PUBL
 */
static uint32_t uPCTL_init(struct DRAM_INFO_STRUCT *dram)
{
	uint32_t rddata = 0;
	
	/* Program if neccessary TOGCNT1U  TOGCNT100N  TINIT  TRSTH */
	uPCTL_neccessary_set(dram->fr, dram->dram_type, dram->fr_100n);
		   
	/* Config uPCTL`s MCFG */
	uPCTL_MCFG_config(dram);

#if defined(FPGA_S2C_TEST)
    /* TODO 
     * passive for fpga */
    rPHY_PGCR = 0x018c2e02;    
#elif defined(FPGA_HAPS_TEST)
	rPHY_PGCR = 0x018c2e02;
#endif
	/* configure DFI timing */
	DFI_timing_config(dram);

	PHY_others_set(dram);
	/* Start PHY initialization
	   Monitor PHY initialization status polling TGSR.IDONE */
	PHY_init(dram->dram_type);
	
	/*Monitor DFI initialization status   dfi_init_complete */
	/***********************************
    ;Monitor DFI initialization status   dfi_init_complete
    ;dfi_init_complete [0] dfi_init_start [1] dfi_freq_ratio [5:4] dif_data_byte_disable [24:16]
    ;config DFI`s register
    ***********************************/ 
	rddata = rUPCTL_DFISTSTAT0;
	while(!(rddata & 0x1)){
		rddata = rUPCTL_DFISTSTAT0;	
	};	
	rPHY_PIR = 0x00040001;

	if(!wake_up_flag){
            /* start power up by setting power_up_start , monitor power up status   power_up_done */
    	    rUPCTL_POWCTL = 0x1;                   /* power up start */
    	    rddata = rUPCTL_POWSTAT;
     	    while(!(rddata & 0x1)){
		rddata = rUPCTL_POWSTAT;
	    };
	} 
	
	/* Configure rest of uPCTL Program all timing T* registers */
	Timing_init(dram);
 
    rUPCTL_SCFG = 0x401;

	/* memory initialization procedure  DDR2/DDR3/mDDR/LPDDR2 */
	DWC_Memory_init(dram);
	
	/* uPCTL moves to Config state and sets STAT.ctl_stat = Config when completed */
    Move_to_Config();
	
	/* TODO Configure uPCTL to refine configuration */
	uPCTL_others_config(dram);
	
	/* Enable CMDTSTAT register by writing cmd_tstat_en = 1 Monitor command timers expiration by polling CMDTSTAT cmd_tstat = 1 */
	rUPCTL_CMDTSTATEN = 0x1;                            /* cmd_tstat_en */
	while(!(check_reg_bit(rUPCTL_CMDTSTAT, 0)));        /* All command timers have expired */
	
	/*TODO SDRAM init complete . Perform PUBL training as required */
#if defined(FPGA_S2C_TEST)
    /* for fpga test, default set dq */
	rPHY_DX0DQTR = 0x33333333;
	rPHY_DX1DQTR = 0x33333333;
	rPHY_DX2DQTR = 0x33333333;
	rPHY_DX3DQTR = 0x33333333;
#elif defined(FPGA_HPAS_TEST)
	rPHY_DX0DQTR = 0x33333333;
	rPHY_DX1DQTR = 0x33333333;
	rPHY_DX2DQTR = 0x33333333;
	rPHY_DX3DQTR = 0x33333333;
#else
    //setting_up_pdm();
        built_in_dqs_gate_trainig();
#endif

	/* write GO into SCTL state_cmd register and poll STAT ctl_stat = Access */
        Move_to_Access();
	return 1;
}

struct nif_info{
    uint32_t r;      /* 0 ~ 5 : 11 ~ 16 */
    uint32_t b;      /* 0: 4 bank   1 : 8 bank */
    uint32_t c;      /* 0 ~ 5 : 7 ~ 12 */
    uint32_t io_t;   /* total io bit  0 : 8, 1: 16, 2 : 32 */
    uint32_t burst;  /* 2'd0: BL2; 2'd1: BL4; 2'd2: BL8; 2'd3: BL16 */
    uint32_t map;    /* b000: {rank,bank,row,column} b001: {rank,row,bank,column} b010: {bank,row,rank,column} */
};

int nif_init_sub(struct nif_info *nif)
{
	// nif reset for development board
	writel(0xff, NIF_SYSM_ADDR + 0x0);
	writel(0xff, NIF_SYSM_ADDR + 0x4);
	writel(0x01, NIF_SYSM_ADDR + 0x8);
	writel(0xff, NIF_SYSM_ADDR + 0xc);
	writel(0xff, NIF_SYSM_ADDR + 0x18);
	writel(0xfe, NIF_SYSM_ADDR + 0x0);

	// [5:3]: arbitor mode. 0 - uMCTL first; 1 - G2D first; 2 - Auto
	// [2]: akeep default value
	// [1]: 1 - akeep enable
	// [0]: 1 - arbitor enable
	(*(volatile int *)(0x29000000)) = (0x2<<3) | (0x1<<2) | (0x1<<1) | 0x1;
	(*(volatile int *)(0x29000004)) = (nif->map << 10) | (nif->b << 8) | (nif->r << 5) | 
		                          (nif->c << 2) | nif->io_t;	// bank8, row14, col10, io3
	if(IROM_IDENTITY == IX_CPUID_X15)
		(*(volatile int *)(0x29000008)) =(0x10 <<4) | (nif->burst <<2) | (0x0<<1) | 0x0; // BL8
	else
		(*(volatile int *)(0x29000008)) = (nif->burst <<2) | (0x0<<1) | 0x0;	// BL8	

	//////////////////////////////////////
	// [12:10]
	// 3'b000: {rank,bank,row,column}
	// 3'b001: {rank,row,bank,column}
	// 3'b010: {bank,row,rank,column}
	// [9:8] - bank address width
	// 2'b00: 2
	// 2'b01: 3
	// [7:5] - row address width
	// 3'd0: 11; ... 3'd5: 16
	// [4:2] - column address width
	// 3'd0: 7; ... 3'd5: 12
	// [1:0] - io width
	// 2'd0: x8; 2'd1: x16; 2'd2: x32
	////////////////////////////////////////
	// [2:3]: dram burst length
	// 2'd0: BL2; 2'd1: BL4; 2'd2: BL8; 2'd3: BL16
	// [1]: 1 - G2D read using VPA
	// [0]: 1 - G2D write using VPA
	writel(0, NIF_SYSM_ADDR  + 0x0);
	writel(0, EMIF_SYSM_ADDR + 0x20);
	return 0;
}

uint32_t nif_init(struct DRAM_INFO_STRUCT *dram)
{
	uint32_t bl[4] = {1, 2, 0, 3};
	struct nif_info nif;


        nif.b = dram->bank - 2;
        nif.r = dram->row - 11;	
	nif.c = dram->col - 7;
	if(dram->reduce_flag)
		nif.io_t = 1;
	else
        	nif.io_t = 2;
        nif.burst = bl[dram->burst_len];
        if (intScreen)
        {
	   nif.map = 0;
        }else{
           nif.map = 1;
        }

        if(nif_init_sub(&nif))
		return 1;

        return 0;  	
}

#define TYPE_N      4
#define IO_N        3
#define DENSITY_N   8
#define CL_N        7
#define RTT_N       3
#define DRIVER_N    2
#define DRIVER_N_LP 7

static const char *dram_typeS[TYPE_N] = {"mDDR", "DDR2", "DDR3", "LPDDR2"};
static const char *dram_ioS[IO_N] = {"8", "16", "32"};
static const char *dram_densityS[DENSITY_N] = {"64MB", "128MB", "256MB", "512MB", 
                                      "1GB", "2GB", "4GB", "8GB"};
static const char *dram_clS[CL_N] = {"2", "3", "4", "5", "6", "7", "8"};
static const char *dram_rttS[RTT_N] = {"DIV_4", "DIV_2", "DIV_6"};
static const char *dram_driverS[DRIVER_N] = {"DIV_6", "DIV_7"};
static const char *dram_driverS_lp[DRIVER_N_LP] = {"34.3", "40", "48", "60", "68.6", "80", "120"};

static const uint32_t dram_typeH[TYPE_N] = {mDDR, DDR2, DDR3, LPDDR2};
static const uint32_t dram_ioH[IO_N] = {IO_WIDTH8, IO_WIDTH16, IO_WIDTH32};
static const uint32_t dram_densityH[DENSITY_N] = {DRAM_64MB, DRAM_128MB, DRAM_256MB, DRAM_512MB,
                                          DRAM_1GB, DRAM_2GB, DRAM_4GB, DRAM_8GB};
static const uint32_t dram_clH[CL_N] = {CL2, CL3, CL4, CL5, CL6, CL7, CL8};
static const uint32_t dram_rttH[RTT_N] = {RTT_DIV_4, RTT_DIV_2, RTT_DIV_6};
static const uint32_t dram_driverH[DRIVER_N] = {DRIVER_DIV_6, DRIVER_DIV_7};
static const uint32_t dram_driverH_lp[DRIVER_N_LP] = {DS_34P3_OHM, DS_40_OHM, DS_48_OHM, DS_60_OHM,
                                                      DS_68P6_OHM, DS_80_OHM, DS_120_OHM};

/*
 * flag : 0   get int
 *        1   use match 
 */
static inline uint32_t get_item_value(uint32_t *buf, void *ist, char *item_name,const char **match, 
		const uint32_t *matchH, uint32_t match_n, uint32_t flag)
{
        uint32_t i;
	uint32_t match_flag = 0;

        dram_log("%s  \n", item_name);
	if(item_exist(item_name)){
	    if(!flag){
		dram_log("get item 1  \n");
                *buf = item_integer(item_name, 0);
	    }else{
		dram_log("get item 2  \n");
		for(i = 0; i < match_n; i++){
                    if(item_equal(item_name, match[i], 0)){
		        *buf = matchH[i];
			match_flag = 1;
			break;
		    }
		}
                if(!match_flag){
		    dram_log("get item out 1  \n");
	            return 1;
		}
	    }
	}else{
	    dram_log("get item out 2  \n");
	    return 1;
	}
        

	dram_log("get item out 3  \n");
	return 0;
}

static uint32_t io_a_den_use_item_flag = 0;  

uint32_t dramc_get_item(struct DRAM_INFO_STRUCT *dram)
{
        uint32_t tmp;
        void *isi = (void *)(DRAM_BASE_PA + 0x8000);

        if(get_item_value(&dram->dram_type, isi, "memory.type", dram_typeS, dram_typeH, TYPE_N, 1))
		return 1;
	dram_log("item   1\n");
        io_a_den_use_item_flag = get_item_value(&dram->io_width, isi, "memory.io_width", dram_ioS, dram_ioH, IO_N, 1);
        io_a_den_use_item_flag |= get_item_value(&dram->density, isi, "memory.density", dram_densityS, dram_densityH, DENSITY_N, 1);
#if 0
	if(dram->dram_type != DDR3){
        if(get_item_value(&dram->io_width, isi, "memory.io_width", dram_ioS, dram_ioH, IO_N, 1))
		return 1;
        if(get_item_value(&dram->density, isi, "memory.density", dram_densityS, dram_densityH, DENSITY_N, 1))
		return 1;
	}
#else
        if(io_a_den_use_item_flag != 0){  /* no item io_width or density  */
	    if(dram->dram_type != DDR3)   /* not ddr3 */
		    return 1;
	} 
#endif
	dram_log("item   2\n");
        if(get_item_value(&dram->cl, isi, "memory.cl", dram_clS, dram_clH, CL_N, 1))
		return 1;
	if(dram->dram_type == DDR3){
            if(get_item_value(&dram->rtt, isi, "memory.rtt", dram_rttS, dram_rttH, RTT_N, 1))
    	        dram->rtt = RTT_DIV_4;
            if(get_item_value(&dram->driver, isi, "memory.driver", dram_driverS, dram_driverH, DRIVER_N, 1))
		dram->driver = DRIVER_DIV_6;
	}else if(dram->dram_type == LPDDR2){
            if(get_item_value(&dram->driver, isi, "memory.driver", dram_driverS_lp, dram_driverH_lp, DRIVER_N_LP, 1))
		dram->driver = DS_40_OHM;	
	}
        if(get_item_value(&dram->fr, isi, "memory.freq", NULL, NULL, 0, 0))
		return 1;
	dram_log("fr %d  \n", dram->fr);
	dram_log("item   3\n");
        if(get_item_value(&dram->reduce_flag, isi, "memory.reduce_en", NULL, NULL, 0, 0))
		return 1;
        if(get_item_value(&dram->training_flag, isi, "memory.train_en", NULL, NULL, 0, 0))
		return 1;
	if(dram->dram_type == DDR3)
	        dram->burst_len = BURST_8;
	else
		dram->burst_len = BURST_4;
        if(get_item_value(&tmp, isi, "memory.cscount", NULL, NULL, 0, 0))
		return 1;
	if(tmp >= 1 && tmp <= 3)
		dram->rank_sel = tmp;
	else
		return 1;
        if(get_item_value(&tmp, isi, "memory.train_en", NULL, NULL, 0, 0))
		return 1;
	dram->training_flag = tmp;
	if(tmp){
	    dram->timing_flag = 0;
            if(get_item_value(&dram->tr_fr_max, isi, "memory.tr_fr_max", NULL, NULL, 0, 0))
		    dram->tr_fr_max = DRMA_TR_FR_MAX;
	    if((dram->tr_fr_max > 600) || (dram->tr_fr_max < 264)){
		    dram->tr_fr_max = DRMA_TR_FR_MAX;
                    spl_printf("dram training max frequency error  \n");        
	    }
            if(get_item_value(&dram->tr_pre_lv, isi, "memory.tr_pre_lv", NULL, NULL, 0, 0)) 
		    dram->tr_pre_lv = DRAM_TR_PRE_LV;
	    if((dram->tr_pre_lv > FR_NUM) || (dram->tr_pre_lv == 0)){
	            dram->tr_pre_lv = DRAM_TR_PRE_LV;
                    spl_printf("dram training pre level error  \n");         
	    }

	//    spl_printf("fr_max %3.3d, lv %d   \n", dram->tr_fr_max, dram->tr_pre_lv);
	}else{
            if(get_item_value(&tmp, isi, "memory.timing_en", NULL, NULL, 0, 0))
		    tmp = 0;
	    dram->timing_flag = tmp;
	    if(tmp){
                if(get_item_value(&dram->timing.tras, isi, "memory.tras", NULL, NULL, 0, 0))
		        return 1;
                if(get_item_value(&dram->timing.trfc_d, isi, "memory.trfc", NULL, NULL, 0, 0))
	        	return 1;
	    }
	}
        if((dram->dram_type == LPDDR2) || (dram->dram_type == mDDR)){
	    if(get_item_value(&dram->lpddr_type_en, isi, "memory.lpddr_type_en", NULL, NULL, 0, 0))
		    dram->dram_type = 0;
	}else
		dram->lpddr_type_en = 0;
        
	return 0;
}

const uint32_t lpddr2_timing[6] = {3, 5, 6, 10, 12, 13}; /* cl 3 ~ 8 */

static void dram_timing_count(struct DRAM_INFO_STRUCT *dram)
{
	uint32_t fr = dram->fr;
	uint32_t io = dram->io_width;
	uint32_t type = dram->dram_type;
	uint32_t density = dram->density;
	int tmp = mini_div(1000000, (int)fr);
	struct dram_timing *tim = &dram->timing;
	
	tim->trefi = 78;   /* 7.8 us */
	if(type == LPDDR2){
		if(density < DRAM_256MB)
			tim->trefi = 156;
		else if(density < DRAM_2GB)
			tim->trefi = 78;
		else
			tim->trefi = 39;
	}
	/* trrd  tfaw */
	if((io == IO_WIDTH16) && (fr > 410)){
		tim->trrd = 6;
		tim->tfaw = 27;
		tim->faw_eff = (TRRD_X_5 << TFAW_CFG) | (3 << TFAW_CFG_OFF);
	}else{
		tim->trrd = 4;
		if((io == IO_WIDTH16) || fr > 410){
			tim->tfaw = 20;
			tim->faw_eff = (TRRD_X_5 << TFAW_CFG) | (0 << TFAW_CFG_OFF);
		}else{
			tim->tfaw = 16;
			tim->faw_eff = (TRRD_X_4 << TFAW_CFG) | (0 << TFAW_CFG_OFF);
		}
	}
	/* tras */  
	/* min is cl + bl / 2 */
	if(!dram->timing_flag){
	if(type != LPDDR2){
	    if(fr < 410)
		    tim->tras = 15 - tim->tras_d;
	    else 
	    	tim->tras = 20 - tim->tras_d;
	}else
		tim->tras = mini_div(42000, tmp);
	}
	/* trc */
	tim->trc = tim->tras + dram->cl;
	/* twr  15ns */
	if(fr <= 350)
		tim->twr = 5;
	else if(fr <= 410)
		tim->twr = 6;
	else if(fr <= 475)
		tim->twr = 7; 
	else
		tim->twr = 8;
	/* tcwl */
	if(dram->cl < 7)
		tim->tcwl = 5;
	else
		tim->tcwl = 6;
	/* trfc */
	if(type == LPDDR2){
		if(density <= DRAM_512MB)
			tim->trfc = mini_div(90000, tmp);
		if(density <= DRAM_4GB)
			tim->trfc = mini_div(130000, tmp);
		else
			tim->trfc = mini_div(210000, tmp);
	}else{
		if(density == DRAM_512MB)
			tim->trfc = mini_div(90000, tmp);
		else if(density == DRAM_1GB)
			tim->trfc = mini_div(110000, tmp);
		else if(density == DRAM_2GB)
			tim->trfc = mini_div(160000, tmp);
		else if(density == DRAM_4GB)
			tim->trfc = mini_div(300000, tmp);
		else if(density == DRAM_8GB)
			tim->trfc = mini_div(380000, tmp);
	}
	if(dram->timing_flag)
		tim->trfc_d = tim->trfc - tim->trfc_d;
	/* trcd trp */
	if(type == LPDDR2)
		tim->trcd = lpddr2_timing[dram->cl - 3];
	else
		tim->trcd = dram->cl;
	tim->trp = tim->trcd;
}

struct ddr_addr_map{
	uint32_t b;
	uint32_t r;
	uint32_t c;
};

#define SET_DDR_MAP(_b, _r, _c) \
{ \
     _b,  \
     _r,  \
     _c,  \
}

static const struct ddr_addr_map ddr3_map[10] = {
		SET_DDR_MAP(3, 13, 10),   /* 512M    8 */
		SET_DDR_MAP(3, 12, 10),   /* 512M    16 */
		SET_DDR_MAP(3, 14, 10),   /* 1G      8 */
		SET_DDR_MAP(3, 13, 10),   /* 1G      16 */
		SET_DDR_MAP(3, 15, 10),   /* 2G      8 */
		SET_DDR_MAP(3, 14, 10),   /* 2G      16 */
		SET_DDR_MAP(3, 16, 10),   /* 4G      8 */
		SET_DDR_MAP(3, 15, 10),   /* 4G      16 */
		SET_DDR_MAP(3, 15, 11),   /* 8G      8 */
		SET_DDR_MAP(3, 16, 10)    /* 8G      16 */
};

static const struct ddr_addr_map mddr_map[12] = {
		SET_DDR_MAP(2, 12,  8),   /* 64M    16 */
		SET_DDR_MAP(2, 11,  8),   /* 64M    32 */
		SET_DDR_MAP(2, 12,  9),   /* 128M   16 */
		SET_DDR_MAP(2, 12,  8),   /* 128M   32 */
		SET_DDR_MAP(2, 13,  9),   /* 256M   16 */
		SET_DDR_MAP(2, 12,  9),   /* 256M   32 */
		SET_DDR_MAP(2, 13, 10),   /* 512M   16 */
		SET_DDR_MAP(2, 13,  9),   /* 512M   32 */
		SET_DDR_MAP(2, 14, 10),   /* 1G     16 */
		SET_DDR_MAP(2, 14,  9),   /* 1G     32 */
		SET_DDR_MAP(2, 14, 11),   /* 2G     16 */
		SET_DDR_MAP(2, 14, 10)    /* 2G     32 */ 
};

static uint32_t ddr3_addr_get(struct DRAM_INFO_STRUCT *dr)
{	
	uint32_t io = dr->io_width;
	uint32_t den = dr->density;
	uint32_t n;
	const struct ddr_addr_map *map;
	
	if((io != IO_WIDTH8) && (io != IO_WIDTH16) && (den < DRAM_512MB) && (den > DRAM_8GB))
		return 1;
	n = (den - DRAM_512MB) * 2 + (io - IO_WIDTH8);
	map = &ddr3_map[n];
	dr->bank = map->b;
	dr->row = map->r;
	dr->col = map->c;
	
	return 0;
}

static uint32_t mddr_addr_get(struct DRAM_INFO_STRUCT *dr)
{	
	uint32_t io = dr->io_width;
	uint32_t den = dr->density;
	uint32_t n;
	const struct ddr_addr_map *map;
	
	if((io != IO_WIDTH16) && (io != IO_WIDTH32) && (den > DRAM_2GB))
		return 1;
	n = (den - DRAM_64MB) * 2 + (io - IO_WIDTH16);
	map = &mddr_map[n];
	dr->bank = map->b;
	dr->row = map->r;
	dr->col = map->c;
	if(dr->lpddr_type_en && (den == DRAM_1GB)){
		dr->bank = 2;
		dr->row = 13;
		dr->col = 10;
	}
	return 0;
}

static const struct ddr_addr_map lpddr2_map[30] = {
		SET_DDR_MAP(2, 12,  9),   /* 64M    08 */
		SET_DDR_MAP(2, 12,  8),   /* 64M    16 */
		SET_DDR_MAP(2, 12,  7),   /* 64M    32 */
		SET_DDR_MAP(2, 12, 10),   /* 128M   08 */
		SET_DDR_MAP(2, 12,  9),   /* 128M   16 */
		SET_DDR_MAP(2, 12,  8),   /* 128M   32 */
		SET_DDR_MAP(2, 13, 10),   /* 256M   08 */
		SET_DDR_MAP(2, 13,  9),   /* 256M   16 */
		SET_DDR_MAP(2, 13,  8),   /* 256M   32 */
		SET_DDR_MAP(2, 13, 11),   /* 512M   08 */
		SET_DDR_MAP(2, 13, 10),   /* 512M   16 */
		SET_DDR_MAP(2, 13,  9),   /* 512M   32 */
		SET_DDR_MAP(2, 14, 11),   /* 1GB S2 08 */
		SET_DDR_MAP(2, 14, 10),   /* 1GB S2 16 */
		SET_DDR_MAP(2, 14,  9),   /* 1GB S2 32 */
		SET_DDR_MAP(2, 15, 11),   /* 2GB S2 08 */
		SET_DDR_MAP(2, 15, 10),   /* 2GB S2 16 */
		SET_DDR_MAP(2, 15,  9),   /* 2GB S2 32 */
		SET_DDR_MAP(3, 14, 12),   /* 4GB    08 */
		SET_DDR_MAP(3, 14, 11),   /* 4GB    16 */
		SET_DDR_MAP(3, 14, 10),   /* 4GB    32 */
		SET_DDR_MAP(3, 15, 12),   /* 8GB    08 */
		SET_DDR_MAP(3, 15, 11),   /* 8GB    16 */
		SET_DDR_MAP(3, 15, 10),   /* 8GB    32 */
		SET_DDR_MAP(3, 13, 11),   /* 1GB S4 08 */
		SET_DDR_MAP(3, 13, 10),   /* 1GB S4 16 */
		SET_DDR_MAP(3, 13,  9),   /* 1GB S4 32 */
		SET_DDR_MAP(3, 14, 11),   /* 2GB S4 08 */
		SET_DDR_MAP(3, 14, 10),   /* 2GB S4 16 */
		SET_DDR_MAP(3, 14,  9)    /* 2GB S4 32 */
};

static uint32_t lpddr2_addr_get(struct DRAM_INFO_STRUCT *dr)
{	
	uint32_t io = dr->io_width;
	uint32_t den = dr->density;
	uint32_t n;
	const struct ddr_addr_map *map;
	
	if(dr->lpddr_type_en && (dr->density == DRAM_1GB) && (dr->density != DRAM_2GB)){
		n = (den - DRAM_1GB) * 3 + (io - IO_WIDTH8) + 24;
	}else{
		n = (den - DRAM_64MB) * 3 + (io - IO_WIDTH8);
	}
	map = &lpddr2_map[n];
	dr->bank = map->b;
	dr->row = map->r;
	dr->col = map->c;
	
	return 0;
}

uint32_t dramc_sturct_defualt_init(struct DRAM_INFO_STRUCT *dram)
{ 
    int fr = dram->fr;

    dram->fr_100n = mini_div(fr, 10);
    if(!dram->timing_flag)
        dram->timing.trfc_d = TRFC_D_DEFAULT;
    dram->timing.tras_d = TRAS_D_DEFAULT;
    dram->umctl.ccfg = CCFG_V;
    dram->umctl.port[0] = PORT0_V;
    dram->umctl.port[1] = PORT1_V;
    dram->umctl.port[2] = PORT2_V;
    dram->umctl.port[3] = PORT3_V;
    dram->umctl.port[4] = PORT4_V;
    dram->umctl.port[5] = PORT5_V;
    dram->umctl.port[6] = PORT6_V;
    dram->umctl.port[7] = PORT7_V;
    if(dram->dram_type == DDR3){
    	if(ddr3_addr_get(dram))
    		return 1;
    }else if(dram->dram_type == mDDR){
        if(mddr_addr_get(dram))
		    return 1;
    }else if(dram->dram_type == LPDDR2){
    	if(lpddr2_addr_get(dram))
    		return 1;
    }

    return 0;
}


uint32_t training(struct DRAM_INFO_STRUCT *dram)
{
#ifdef DRAM_FLAG
    dramc_training(dram);
    dram_log("training done   \n");
#endif
    return 0;
}

uint32_t dramc_init_sub(struct DRAM_INFO_STRUCT *dram)
{
    uint32_t j;
    uint32_t i;
    uint32_t train_flag = 0;
   
    dram_timing_count(dram);
    /* NIF, which will be used by G2D,
     * must be initialized before DRAM controller
     * 	 */
    nif_init(dram);

    if(!wake_up_flag){
	    *(volatile int *)(0x21e09c4c) = 0xe;
	    *(volatile int *)(0x21e09c7c) = 0x1;
    }else{
            spl_printf("dram wake up  \n");
    }

    irf->module_enable(EMIF_SYSM_ADDR);
    uPCTL_init(dram);

    if(dram->reduce_flag)
	    j = 2;
    else
	    j = 4;
    if((readl(0x21a0800c) & 0x20)){
	for(i = 0; i < j; i++){
	    if(rPHY_DXGSR0(i) & 0xf0){
                spl_printf("dram train byte%d error \n", i);
		train_flag |= 1 << i;
		dramc_bit_check(i, dram);
	    }
	}
    }
    return train_flag;
}

static void dram_fr_set_sub(uint32_t pll_p, uint32_t phy_div, struct irom_export *irf)
{ 
    irf->set_pll(PLL_V, (pll_p & 0xffff)); 
    irf->module_set_clock(DDRPHY_CLK_BASE, PLL_V, phy_div); 
}

const uint32_t fr_val[FR_NUM] = {
		600, 576, 552, 528, 504, 468, 444, 420, 
		396, 372, 348, 330, 312, 288, 264 
};

const uint32_t para_val[FR_NUM] = {
		0x0031, 0x002f, 0x002d, 0x002b, 0x0029, 0x104d, 0x1049, 0x1045,
		0x1041, 0x103d, 0x1039, 0x1036, 0x1033, 0x102f, 0x102b
};

void dram_fr_set(uint32_t fr, struct irom_export *irf)
{
    uint32_t i = 0;
    uint32_t flag = 0;

    for(i = 0; i < FR_NUM; i++){
        if(fr >= fr_val[i]){
	    dram_fr_set_sub(para_val[i], 1, irf);
	    flag = 1;
	    break;
	}
    }

    if(!flag){
        if(fr >= 200)
     	    dram_fr_set_sub(0x1041, 3, irf);
        else if(fr >= 150)
	    dram_fr_set_sub(0x1031, 3, irf);
        else
	    dram_fr_set_sub(0x1031, 3, irf);
    }
}

static void use_default_p(struct DRAM_INFO_STRUCT *dr)
{
    dr->fr = DRAM_FREQ;
    dr->burst_len = DRAM_BURST;
    dr->cl = DRAM_CL;
    dr->rank_sel = DRAM_RANK;
    dr->io_width = DRAM_IO_W;
    dr->dram_type = DRAM_TYPE;
    dr->density = DRAM_DENSITY;
    dr->rtt = DRAM_RTT; 
    dr->driver = DRAM_DRIVER;
    dr->reduce_flag = DRAM_IO_REDUCE;
    dr->training_flag = 0;
    dr->timing_flag = 0;
    dr->lpddr_type_en = DRAM_LPDDR_TYPE;
}	

#define CHECK_ADDR1 0x80000000
#define CHECK_ADDR2 0xa0000000
#define CHECK_ADDR3 0x90000000

static volatile uint32_t dram_size = 0;

uint32_t dram_size_check(void)
{   
    return dram_size;
}

/*
 * dramc_init
 */
int dramc_init(struct irom_export *irf, int wake)
{
    struct DRAM_INFO_STRUCT dram;
    volatile uint32_t tmp1, tmp2;
    volatile uint32_t addr;
    
    dram_log("dramc_init \n");
    if(dramc_get_item(&dram)){
	    spl_printf("dram item error\n");
	    use_default_p(&dram);
    }
    
    high_res_screen();

    dram_log("item done\n");

//    if(dramc_sturct_defualt_init(&dram))
//	    return 1;

    wake_up_flag = wake;

    if((dram.dram_type == DDR3) && (io_a_den_use_item_flag != 0)){
//        spl_printf("address check \n");
        dram.io_width = IO_WIDTH8;
	if(dram.reduce_flag)
		dram.density = DRAM_4GB;
        else
		dram.density = DRAM_2GB;
    }
    if(dramc_sturct_defualt_init(&dram))
	    return 1;
    if(dram.training_flag && !wake_up_flag){
            dram_log("dram train \n");
#ifdef DRAM_FLAG
            if(training(&dram)){
                spl_printf("dram training error\n");
		return 1;
	    }
#else
            dram_log("dram not train for cut down uboot0.isi\n");
#endif
    }else{
        dram_fr_set(dram.fr, irf);
	if(dramc_init_sub(&dram))
	        return 1;
    }
    if((dram.dram_type == DDR3) && (io_a_den_use_item_flag != 0)){
//	if(dram.reduce_flag){
//	    addr = CHECK_ADDR3;
//	}else
	    addr = CHECK_ADDR2;
        /* store data */
        tmp1 = *(volatile unsigned int *)(CHECK_ADDR1);
        tmp2 = *(volatile unsigned int *)(addr);
        *(volatile unsigned int *)(addr) = 0x87654321;
        *(volatile unsigned int *)(CHECK_ADDR1) = 0x12345678;
        if(*(volatile unsigned int *)(addr) == 0x87654321){
	    if(dram.reduce_flag){
	        spl_printf("reduce mode    total 1G Byte  \n");
//                dram_size = _1G_BYTE;
	    }else{
                spl_printf("1G Byte \n");
//                dram_size = _1G_BYTE;
	    }
            dram_size = _1G_BYTE;
	    *(volatile unsigned int *)(CHECK_ADDR1) = tmp1;    
	    *(volatile unsigned int *)(addr) = tmp2;    
        }else{
	    if(dram.reduce_flag){
	        dram.density = DRAM_2GB;
                spl_printf("reduce mode    total 512M Byte \n");
	    }else{
	        dram.density = DRAM_1GB;
                spl_printf("512M Byte \n");
	    }
            spl_printf("512M Byte \n");
            dram_size = _512M_BYTE;
	    *(volatile unsigned int *)(CHECK_ADDR1) = tmp1;  
	    if(dramc_sturct_defualt_init(&dram))
		    return 1;
	    if(dramc_init_sub(&dram))
		    return 1;	
        }
    }
    dram_log("dram init done\n");

    return 0;
}

uint32_t dramc_get_size(void)
{
	/* TODO: return the memory size */
	return (512 << 20);
}	

void dramc_print_list(void)
{
	/* TODO: print the supported list */
	return ;
}

