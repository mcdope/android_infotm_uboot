#ifndef __IMAPX_DRAMC__
#define __IMAPX_DRAMC__

/*========================================================================
 * DWC_DDR_UMCTL_MAP Registers 
 *======================================================================*/

/* uMCTL Registers */
#define rUMCTL_PCFG(x)               *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR + 0x400 + 4 * x)     /* Port x */    
#define rUMCTL_PCFG_0                *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x400)     /* Port 0   Configuration Register */ 
#define rUMCTL_PCFG_1                *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x404)     /* Port 1   Configuration Register */ 
#define rUMCTL_PCFG_2                *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x408)     /* Port 2   Configuration Register */ 
#define rUMCTL_PCFG_3                *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x40c)     /* Port 3   Configuration Register */ 
#define rUMCTL_PCFG_4                *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x410)     /* Port 4   Configuration Register */ 
#define rUMCTL_PCFG_5                *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x414)     /* Port 5   Configuration Register */ 
#define rUMCTL_PCFG_6                *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x418)     /* Port 6   Configuration Register */ 
#define rUMCTL_PCFG_7                *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x41c)     /* Port 7   Configuration Register */ 
#define rUMCTL_PCFG_8                *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x420)     /* Port 8   Configuration Register */ 
#define rUMCTL_PCFG_9                *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x424)     /* Port 9   Configuration Register */ 
#define rUMCTL_PCFG_10               *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x428)     /* Port 10  Configuration Register */ 
#define rUMCTL_PCFG_11               *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x42c)     /* Port 11  Configuration Register */ 
#define rUMCTL_PCFG_12               *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x430)     /* Port 12  Configuration Register */ 
#define rUMCTL_PCFG_13               *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x434)     /* Port 13  Configuration Register */ 
#define rUMCTL_PCFG_14               *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x438)     /* Port 14  Configuration Register */ 
#define rUMCTL_PCFG_15               *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x43c)     /* Port 15  Configuration Register */ 
#define rUMCTL_PCFG_16               *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x440)     /* Port 16  Configuration Register */ 
#define rUMCTL_PCFG_17               *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x444)     /* Port 17  Configuration Register */ 
#define rUMCTL_PCFG_18               *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x448)     /* Port 18  Configuration Register */ 
#define rUMCTL_PCFG_19               *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x44c)     /* Port 19  Configuration Register */ 
#define rUMCTL_PCFG_20               *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x450)     /* Port 20  Configuration Register */ 
#define rUMCTL_PCFG_21               *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x454)     /* Port 21  Configuration Register */ 
#define rUMCTL_PCFG_22               *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x458)     /* Port 22  Configuration Register */ 
#define rUMCTL_PCFG_23               *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x45c)     /* Port 23  Configuration Register */ 
#define rUMCTL_PCFG_24               *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x460)     /* Port 24  Configuration Register */ 
#define rUMCTL_PCFG_25               *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x464)     /* Port 25  Configuration Register */ 
#define rUMCTL_PCFG_26               *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x468)     /* Port 26  Configuration Register */ 
#define rUMCTL_PCFG_27               *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x46c)     /* Port 27  Configuration Register */ 
#define rUMCTL_PCFG_28               *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x470)     /* Port 28  Configuration Register */ 
#define rUMCTL_PCFG_29               *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x474)     /* Port 29  Configuration Register */ 
#define rUMCTL_PCFG_30               *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x478)     /* Port 30  Configuration Register */ 
#define rUMCTL_PCFG_31               *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x47c)     /* Port 31  Configuration Register */ 
                                                                                               
#define rUMCTL_CCFG                  *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x480)     /* Controller Configuration Register */
#define rUMCTL_DCFG                  *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x484)     /* DRAM Configuration Register */       
#define rUMCTL_CSTAT                 *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x488)     /* Controller Status Register */        
#define rUMCTL_CCFG1                 *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x48c)     /* Controller Status Register 1     add */
                                                                                                 
/* uPCTL Registers                                                                               
   Operational State,Control,Status Registers */                                                
#define rUPCTL_SCFG                  *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x000)     /* State Configuration Register */                                                                                                          
#define rUPCTL_SCTL                  *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x004)     /* State Control Register */                                                                                                                
#define rUPCTL_STAT                  *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x008)     /* State Status Register */                                                                                                               
#define rUPCTL_INTRSTAT              *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x00c)     /* Interrupt Status Register */                                                                                                                  
                                                                                                 
/* Initialization Control and Status Registers */                                                
#define rUPCTL_MCMD                  *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x040)     /* Memory Command Register */                                                                                                         
#define rUPCTL_POWCTL                *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x044)     /* Power Up Control Register */                                                                                                                
#define rUPCTL_POWSTAT               *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x048)     /* Power Up Status Register */                                                                                                                  
#define rUPCTL_CMDTSTAT              *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x04c)     /* Command Timing Status Register */                                                                                                                  
#define rUPCTL_CMDTSTATEN            *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x050)     /* Command Timing Status Enable Register */                                                                                                          
#define rUPCTL_MRRCFG0               *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x060)     /* MRR Configuration 0 Register */                                                                                                                 
#define rUPCTL_MRRSTAT0              *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x064)     /* MRR Status 0 Register */                                                                                                             
#define rUPCTL_MRRSTAT1              *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x068)     /* MRR Status 1 Register */              
                                                                                                
/* Memory Control and Status Registers */                                                        
#define rUPCTL_MCFG1                 *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x07c)     /* Memory Configuration Register 1  add */
#define rUPCTL_MCFG                  *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x080)     /* Memory Configuration Register */                                                                                                              
#define rUPCTL_PPCFG                 *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x084)     /* Partially Populated Memories Configuration Register */
#define rUPCTL_MSTAT                 *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x088)     /* Memory Status Register */                                                                                                    
#define rLPDDR2ZQCFG                 *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x08c)     /* LPDDR2 ZQ configuration register   add */ 
                                                                                                 
/* DTU Control and Status Registers */                                                           
#define rUPCTL_DTUPDES               *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x094)     /* DTU Status */
#define rUPCTL_DTUNA                 *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x098)     /* DTU Number of Random Addresses Created */
#define rUPCTL_DTUNE                 *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x09c)     /* DTU Number of Errors */	
#define rUPCTL_DTUPRD0               *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x0a0)     /* DTU Parallel Read 0 */
#define rUPCTL_DTUPRD1               *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x0a4)     /* DTU Parallel Read 1 */
#define rUPCTL_DTUPRD2               *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x0a8)     /* DTU Parallel Read 2 */
#define rUPCTL_DTUPRD3               *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x0ac)     /* DTU Parallel Read 3 */
#define rUPCTL_DTUAWDT               *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x0b0)     /* DTU Address Width */
                                                                                                 
/* Memory Timing Registers */                                                                   
#define rUPCTL_TOGCNT1U              *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x0c0)     /* toggle counter 1u Register */
#define rUPCTL_TINIT                 *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x0c4)     /* t_init timing Register */
#define rUPCTL_TRSTH                 *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x0c8)     /* reset high time Register */
#define rUPCTL_TOGCNT100N            *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x0cc)     /* toggle counter 100n Register */
#define rUPCTL_TREFI                 *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x0d0)     /* t_refi timing Register */
#define rUPCTL_TMRD                  *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x0d4)     /* t_mrd timing Register */
#define rUPCTL_TRFC                  *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x0d8)     /* t_rfc timing Register */
#define rUPCTL_TRP                   *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x0dc)     /* t_rp timing Register */
#define rUPCTL_TRTW                  *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x0e0)     /* t_rtw Register */
#define rUPCTL_TAL                   *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x0e4)     /* al latency Register */
#define rUPCTL_TCL                   *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x0e8)     /* cl timing Register */
#define rUPCTL_TCWL                  *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x0ec)     /* cwl Register */
#define rUPCTL_TRAS                  *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x0f0)     /* t_ras timing Register */
#define rUPCTL_TRC                   *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x0f4)     /* t_rc timing Register */
#define rUPCTL_TRCD                  *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x0f8)     /* t_rcd timing Register */
#define rUPCTL_TRRD                  *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x0fc)     /* t_rrd timing Register */
#define rUPCTL_TRTP                  *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x100)     /* t_rtp timing Register */
#define rUPCTL_TWR                   *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x104)     /* t_wr timing Register */
#define rUPCTL_TWTR                  *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x108)     /* t_wtr timing Register */
#define rUPCTL_TEXSR                 *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x10c)     /* t_exsr timing Register */
#define rUPCTL_TXP                   *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x110)     /* t_xp timing Register */
#define rUPCTL_TXPDLL                *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x114)     /* t_xpdll timing Register */
#define rUPCTL_TZQCS                 *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x118)     /* t_zqcs timing Register */
#define rUPCTL_TZQCSI                *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x11c)     /* t_zqcsi timing Register */
#define rUPCTL_TDQS                  *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x120)     /* t_dqs timing Register */
#define rUPCTL_TCKSRE                *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x124)     /* t_cksre timing Register */
#define rUPCTL_TCKSRX                *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x128)     /* t_cksrx timing Register */
#define rUPCTL_TCKE                  *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x12c)     /* t_cke timing Register */
#define rUPCTL_TMOD                  *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x130)     /* t_mod timing Register */
#define rUPCTL_TRSTL                 *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x134)     /* reset low timing Register */
#define rUPCTL_TZQCL                 *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x138)     /* t_zqcl timing Register */
#define rUPCTL_TMRR                  *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x13c)     /* t_mrr timing Register */
#define rUPCTL_TCKESR                *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x140)     /* t_ckesr timing Register */
#define rUPCTL_TDPD                  *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x144)     /* t_dpd timing Register */
                                                                                                
/* ECC Configuration,Control,and Status Registers */                                             
#define rUPCTL_ECCCFG                *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x180)     /* ECC Configuration Register */
#define rUPCTL_ECCTST                *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x184)     /* ECC Test Register */
#define rUPCTL_ECCCLR                *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x188)     /* ECC Clear Register */
#define rUPCTL_ECCLOG                *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x18c)     /* ECC Log Register */
                                                                                                
/* DTU Control and Status Registers */                                                           
#define rUPCTL_DTUWACTL              *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x200)     /* DTU write address control Register */
#define rUPCTL_DTURACTL              *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x204)     /* DTU read address control Register */
#define rUPCTL_DTUCFG                *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x208)     /* DTU configuration Register */
#define rUPCTL_DTUECTL               *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x20c)     /* DTU execute control Register */
#define rUPCTL_DTUWD0                *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x210)     /* DTU write data #0 */
#define rUPCTL_DTUWD1                *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x214)     /* DTU write data #1 */
#define rUPCTL_DTUWD2                *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x218)     /* DTU write data #2 */
#define rUPCTL_DTUWD3                *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x21c)     /* DTU write data #3 */
#define rUPCTL_DTUWDM                *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x220)     /* DTU write data mask */
#define rUPCTL_DTURD0                *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x224)     /* DTU read data #0 */
#define rUPCTL_DTURD1                *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x228)     /* DTU read data #1 */
#define rUPCTL_DTURD2                *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x22c)     /* DTU read data #2 */
#define rUPCTL_DTURD3                *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x230)     /* DTU read data #3 */
#define rUPCTL_DTULFSRWD             *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x234)     /* DTU LFSR seed for write data generation */
#define rUPCTL_DTULFSRRD             *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x238)     /* DTU LFSR seed for read data generation */
#define rUPCTL_DTUEAF                *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x23c)     /* DTU error address FIFO */
                                                                                                
/* DFI Control Registers */                                                                      
#define rUPCTL_DFITCTRLDELAY         *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x240)     /* DFI tctrl_delay Register */
#define rUPCTL_DFIODTCFG             *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x244)     /* DFI ODT Configuration Register */
#define rUPCTL_DFIODTCFG1            *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x248)     /* DFI ODT Timing Configuration Register */
#define rUPCTL_DFIODTRANKMAP         *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x24c)     /* DFI ODT Rank Mapping Register */
                                                                                                
/* DFI Write Data Registers */                                                                   
#define rUPCTL_DFITPHYWRDATA         *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x250)     /* DFI tphy_wrdata Register */
#define rUPCTL_DFITPHYWRLAT          *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x254)     /* DFI tphy_wrlat Register */
                                                                                                
/* DFI Read Data Registers */                                                                    
#define rUPCTL_DFITRDDATAEN          *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x260)     /* DFI trddata_en Register */
#define rUPCTL_DFITPHYRDLAT          *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x264)     /* DFI tphy_rddata Register */
                                                                                                 
/* DFI Update Registers */                                                                       
#define rUPCTL_DFITPHYUPDTYPE0       *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x270)     /* DFI tphyupd_type0 Register */
#define rUPCTL_DFITPHYUPDTYPE1       *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x274)     /* DFI tphyupd_type1 Register */
#define rUPCTL_DFITPHYUPDTYPE2       *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x278)     /* DFI tphyupd_type2 Register */
#define rUPCTL_DFITPHYUPDTYPE3       *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x27c)     /* DFI tphyupd_type3 Register */
#define rUPCTL_DFITCTRLUPDMIN        *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x280)     /* DFI tctrlupd_min Register */
#define rUPCTL_DFITCTRLUPDMAX        *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x284)     /* DFI tctrlupd_max Register */
#define rUPCTL_DFITCTRLUPDDLY        *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x288)     /* DFI tctrlupd_dly Register */
#define rUPCTL_DFIUPDCFG             *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x290)     /* DFI Update Configuration Register */
#define rUPCTL_DFITREFMSKI           *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x294)     /* DFI Masked Refresh Interval Register */
#define rUPCTL_DFITCTRLUPDI          *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x298)     /* DFI tctrlupd_interval Register */
                                                                                                
/* DFI Training Registers */                                                                     
#define rUPCTL_DFITRCFG0             *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x2ac)     /* DFI Training Configuration 0 Register */
#define rUPCTL_DFITRSTAT0            *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x2b0)     /* DFI Training Status 0 Register */
#define rUPCTL_DFIRWRLVLEN           *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x2b4)     /* DFI Training dfi_wrlvl_en Register */
#define rUPCTL_DFIRRDLVLEN           *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x2b8)     /* DFI Training dfi_rdlvl_en Register */
#define rUPCTL_DFIRRDLVLGATEEN       *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x2bc)     /* DFI Training dfi_rdlvl_gate_en Register */
                                                                                                 
/* DFI Status Registers */                                                                       
#define rUPCTL_DFISTSTAT0            *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x2c0)     /* DFI STATUS Status 0 Register */
#define rUPCTL_DFISTCFG0             *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x2c4)     /* DFI STATUS Configuration 0 Register */
#define rUPCTL_DFISTCFG1             *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x2c8)     /* DFI STATUS Configuration 1 Register */
#define rUPCTL_DFITDRAMCLKEN         *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x2d0)     /* DFI tdram_clk_disable Register */
#define rUPCTL_DFITDRAMCLKDIS        *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x2d4)     /* DFI tdram_clk_enable Register */
#define rUPCTL_DFISTCFG2             *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x2d8)     /* DFI STATUS Configuration 2 Register */
#define rUPCTL_DFISTPARCLR           *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x2dc)     /* DFI STATUS Parity Clear Register */
#define rUPCTL_DFISTPARLOG           *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x2e0)     /* DFI STATUS Parity Log Register */
                                                                                                
/* DFI Low Power Registers */                                                                    
#define rUPCTL_DFILPCFG0             *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x2f0)     /* DFI Low Power Configuration 0 Register */
                                                                                                 
/* DFI Training 2 Registers */                                                                  
#define rUPCTL_DFITRWRLVLRESP0       *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x300)     /* DFI Training dfi_wrlvl_resp Status 0 */
#define rUPCTL_DFITRWRLVLRESP1       *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x304)     /* DFI Training dfi_wrlvl_resp Status 1 */
#define rUPCTL_DFITRWRLVLRESP2       *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x308)     /* DFI Training dfi_wrlvl_resp Status 2 */
#define rUPCTL_DFITRRDLVLRESP0       *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x30c)     /* DFI Training dfi_rdlvl_resp Status 0 */
#define rUPCTL_DFITRRDLVLRESP1       *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x310)     /* DFI Training dfi_rdlvl_resp Status 1 */
#define rUPCTL_DFITRRDLVLRESP2       *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x314)     /* DFI Training dfi_rdlvl_resp Status 2 */
#define rUPCTL_DFITRWRLVLDELAY0      *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x318)     /* DFI Training dfi_wrlvl_delay Configuration0 */ 
#define rUPCTL_DFITRWRLVLDELAY1      *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x31c)     /* DFI Training dfi_wrlvl_delay Configuration1 */ 
#define rUPCTL_DFITRWRLVLDELAY2      *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x320)     /* DFI Training dfi_wrlvl_delay Configuration2 */ 
#define rUPCTL_DFITRRDLVLDELAY0      *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x324)     /* DFI Training dfi_rdlvl_delay Configuration0 */ 
#define rUPCTL_DFITRRDLVLDELAY1      *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x328)     /* DFI Training dfi_rdlvl_delay Configuration1 */ 
#define rUPCTL_DFITRRDLVLDELAY2      *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x32c)     /* DFI Training dfi_rdlvl_delay Configuration2 */ 
#define rUPCTL_DFITRRDLVLGATEDELAY0  *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x330)     /* DFI Training dfi_rdlvl_gate_delay Configuration0 */ 
#define rUPCTL_DFITRRDLVLGATEDELAY1  *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x334)     /* DFI Training dfi_rdlvl_gate_delay Configuration1 */ 
#define rUPCTL_DFITRRDLVLGATEDELAY2  *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x338)     /* DFI Training dfi_rdlvl_gate_delay Configuration2 */ 
#define rUPCTL_DFITRCMD              *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x33c)     /* DFI Training Command Register */
                                                                                                
/* IP Status Registers */                                                                        
#define rUPCTL_IPVR                  *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x3f8)     /* IP Version Register */
#define rUPCTL_IPTR                  *(volatile unsigned *)(DWC_DDR_UMCTL_BASE_ADDR+0x3fc)     /* IP Type Register */
                                                                                                
/*=======================================================================                        
 * DWC_DDR_PUBL Registers  PHY                                                                   
 *=====================================================================*/                        
                                                                                                 
#define rPHY_RIDR                    *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X000)      /* Revision Identification Register */
#define rPHY_PIR                     *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X004)      /* PHY Initialization Register */
#define rPHY_PGCR                    *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X008)      /* PHY General Configuration Register */
#define rPHY_PGSR                    *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X00c)      /* PHY General Status Register */
#define rPHY_DLLGCR                  *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X010)      /* DLL General Control Register */
#define rPHY_ACDLLCR                 *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X014)      /* AC DLL Control Register */
#define rPHY_PTR0                    *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X018)      /* PHY Timing Register 0 */
#define rPHY_PTR1                    *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X01c)      /* PHY Timing Register 1 */
#define rPHY_PTR2                    *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X020)      /* PHY Timing Register 2 */
#define rPHY_ACIOCR                  *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X024)      /* AC I/O Configuration Register */
#define rPHY_DXCCR                   *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X028)      /* DATX8 Common Configuratio Register */
#define rPHY_DSGCR                   *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X02c)      /* DDR System General Configuration Register */
#define rPHY_DCR                     *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X030)      /* DRAM Configuration Register */
#define rPHY_DTPR0                   *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X034)      /* DRAM Timing Parameters Register 0 */
#define rPHY_DTPR1                   *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X038)      /* DRAM Timing Parameters Register 1 */
#define rPHY_DTPR2                   *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X03c)      /* DRAM Timing Parameters Register 2 */
#define rPHY_MR0                     *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X040)      /* Mode Register 0 */
#define rPHY_MR1                     *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X044)      /* Mode Register 1 */
#define rPHY_MR2                     *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X048)      /* Mode Register 2 */
#define rPHY_MR3                     *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X04c)      /* Mode Register 3 */
#define rPHY_ODTCR                   *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X050)      /* ODT Configuration Register */
#define rPHY_DTAR                    *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X054)      /* Data Training Address Register */
#define rPHY_DTDR0                   *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X058)      /* Data Training Data Register 0 */
#define rPHY_DTDR1                   *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X05c)      /* Data Training Data Register 1 */
/* 0X18~0X2F RESERVED */                                                                        
#define rPHY_DCUAR                   *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X0c0)      /* DCU Address Register */
#define rPHY_DCUDR                   *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X0c4)      /* DCU Data Register */
#define rPHY_DCURR                   *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X0c8)      /* DCU Run Register */
#define rPHY_DCULR                   *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X0cc)      /* DCU Loop Register */
#define rPHY_DCUGCR                  *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X0d0)      /* DCU General Configuration Register */
#define rPHY_DCUTPR                  *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X0d4)      /* DCU Timing Rarameters Register */
#define rPHY_DCUSR0                  *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X0d8)      /* DCU Status Register 0 */
#define rPHY_DCUSR1                  *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X0dc)      /* DCU Status Register 1 */
/* 0X38~0X5E RESERVED */                                                                        
#define rPHY_BISTRR                  *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X100)      /* BIST Run Register */
#define rPHY_BISTMSKR0               *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X104)      /* BIST Mask Register 0 */
#define rPHY_BISTMSKR1               *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X108)      /* BIST Mask Register 1 */
#define rPHY_BISTWCR                 *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X10c)      /* BIST Word Count  Register */
#define rPHY_BISTLSR                 *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X110)      /* BIST LFSR Seed Register */
#define rPHY_BISTAR0                 *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X114)      /* BIST Address Register 0 */
#define rPHY_BISTAR1                 *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X118)      /* BIST Address Register 1 */
#define rPHY_BISTAR2                 *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X11c)      /* BIST Address Register 2 */
#define rPHY_BISTUDPR                *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X120)      /* BIST User Data Pattern Register */
#define rPHY_BISTGSR                 *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X124)      /* BIST General Status Register */
#define rPHY_BISTWER                 *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X128)      /* BIST Word Error Register */
#define rPHY_BISTBER0                *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X12c)      /* BIST Bit Error Register 0 */
#define rPHY_BISTBER1                *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X130)      /* BIST Bit Error Register 1 */
#define rPHY_BISTBER2                *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X134)      /* BIST Bit Error Register 2 */
#define rPHY_BISTWCSR                *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X138)      /* BIST Wor Count Status Register */
#define rPHY_BISTFWR0                *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X13c)      /* BIST Fail Word Register 0 */
#define rPHY_BISTFWR1                *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X140)      /* BIST Fail Word Register 1 */
/* 0X51~0X5F RESERVED */                                                                        
#define rPHY_ZQ0CR0                  *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X180)      /* ZQ 0 Impedance Control Register 0 */
#define rPHY_ZQ0CR1                  *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X184)      /* ZQ 0 Impedance Control Register 1 */
#define rPHY_ZQ0SR0                  *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X188)      /* ZQ 0 Impedance Status Register 0 */
#define rPHY_ZQ0SR1                  *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X18c)      /* ZQ 0 Impedance Status Register 1 */
#define rPHY_ZQ1CR0                  *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X190)      /* ZQ 1 Impedance Control Register 0 */
#define rPHY_ZQ1CR1                  *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X194)      /* ZQ 1 Impedance Control Register 1 */
#define rPHY_ZQ1SR0                  *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X198)      /* ZQ 1 Impedance Status Register 0 */
#define rPHY_ZQ1SR1                  *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X19c)      /* ZQ 1 Impedance Status Register 1 */
#define rPHY_ZQ2CR0                  *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X1a0)      /* ZQ 2 Impedance Control Register 0 */
#define rPHY_ZQ2CR1                  *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X1a4)      /* ZQ 2 Impedance Control Register 1 */
#define rPHY_ZQ2SR0                  *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X1a8)      /* ZQ 2 Impedance Status Register 0 */
#define rPHY_ZQ2SR1                  *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X1ac)      /* ZQ 2 Impedance Status Register 1 */
#define rPHY_ZQ3CR0                  *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X1b0)      /* ZQ 3 Impedance Control Register 0 */
#define rPHY_ZQ3CR1                  *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X1b4)      /* ZQ 3 Impedance Control Register 1 */
#define rPHY_ZQ3SR0                  *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X1b8)      /* ZQ 3 Impedance Status Register 0 */
#define rPHY_ZQ3SR1                  *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X1bc)      /* ZQ 3 Impedance Status Register 1 */
#define rPHY_DX0GCR                  *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X1c0)      /* DATX8 0 General Configuration Register */
#define rPHY_DX0GSR0                 *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X1c4)      /* DATX8 0 General Status Register 0 */
#define rPHY_DX0GSR1                 *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X1c8)      /* DATX8 0 General Status Register 1 */
#define rPHY_DX0DLLCR                *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X1cc)      /* DATX8 0 DLL Control Register */
#define rPHY_DX0DQTR                 *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X1d0)      /* DATX8 0 DQ Timing Register */
#define rPHY_DX0DQSTR                *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X1d4)      /* DATX8 0 DQS Timing Register */
/* 0X76~0X7F RESERVED */                                                                        
#define rPHY_DX1GCR                  *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X200)      /* DATX8 1 General Configuration Register */
#define rPHY_DX1GSR0                 *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X204)      /* DATX8 1 General Status Register 0 */
#define rPHY_DX1GSR1                 *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X208)      /* DATX8 1 General Status Register 1 */     
#define rPHY_DX1DLLCR                *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X20c)      /* DATX8 1 DLL Control Register */          
#define rPHY_DX1DQTR                 *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X210)      /* DATX8 1 DQ Timing Register */            
#define rPHY_DX1DQSTR                *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X214)      /* DATX8 1 DQS Timing Register */           
/* 0X86~0X8F RESERVED */                                                                        
#define rPHY_DX2GCR                  *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X240)      /* DATX8 2 General Configuration Register */
#define rPHY_DX2GSR0                 *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X244)      /* DATX8 2 General Status Register 0 */     
#define rPHY_DX2GSR1                 *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X248)      /* DATX8 2 General Status Register 1 */     
#define rPHY_DX2DLLCR                *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X24c)      /* DATX8 2 DLL Control Register */          
#define rPHY_DX2DQTR                 *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X250)      /* DATX8 2 DQ Timing Register */            
#define rPHY_DX2DQSTR                *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X254)      /* DATX8 2 DQS Timing Register */           
/* 0X96~0X9F RESERVED */                                                                        
#define rPHY_DX3GCR                  *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X280)      /* DATX8 3 General Configuration Register */
#define rPHY_DX3GSR0                 *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X284)      /* DATX8 3 General Status Register 0 */     
#define rPHY_DX3GSR1                 *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X288)      /* DATX8 3 General Status Register 1 */     
#define rPHY_DX3DLLCR                *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X28c)      /* DATX8 3 DLL Control Register */          
#define rPHY_DX3DQTR                 *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X290)      /* DATX8 3 DQ Timing Register */            
#define rPHY_DX3DQSTR                *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X294)      /* DATX8 3 DQS Timing Register */           
/* 0XA6~0XAF RESERVED */                                                                         
#define rPHY_DX4GCR                  *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X2c0)      /* DATX8 4 General Configuration Register */
#define rPHY_DX4GSR0                 *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X2c4)      /* DATX8 4 General Status Register 0 */     
#define rPHY_DX4GSR1                 *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X2c8)      /* DATX8 4 General Status Register 1 */     
#define rPHY_DX4DLLCR                *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X2cc)      /* DATX8 4 DLL Control Register */          
#define rPHY_DX4DQTR                 *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X2d0)      /* DATX8 4 DQ Timing Register */            
#define rPHY_DX4DQSTR                *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X2d4)      /* DATX8 4 DQS Timing Register */           
/* 0XB6~0XBF RESERVED */                                                                         
#define rPHY_DX5GCR                  *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X300)      /* DATX8 5 General Configuration Register */
#define rPHY_DX5GSR0                 *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X304)      /* DATX8 5 General Status Register 0 */     
#define rPHY_DX5GSR1                 *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X308)      /* DATX8 5 General Status Register 1 */     
#define rPHY_DX5DLLCR                *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X30c)      /* DATX8 5 DLL Control Register */          
#define rPHY_DX5DQTR                 *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X310)      /* DATX8 5 DQ Timing Register */            
#define rPHY_DX5DQSTR                *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X314)      /* DATX8 5 DQS Timing Register */           
/* 0XC6~0XCF RESERVED */                                                                        
#define rPHY_DX6GCR                  *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X340)      /* DATX8 6 General Configuration Register */
#define rPHY_DX6GSR0                 *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X344)      /* DATX8 6 General Status Register 0 */     
#define rPHY_DX6GSR1                 *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X348)      /* DATX8 6 General Status Register 1 */     
#define rPHY_DX6DLLCR                *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X34c)      /* DATX8 6 DLL Control Register */          
#define rPHY_DX6DQTR                 *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X350)      /* DATX8 6 DQ Timing Register */            
#define rPHY_DX6DQSTR                *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X354)      /* DATX8 6 DQS Timing Register */           
/* 0XD6~0XDF RESERVED */                                                                        
#define rPHY_DX7GCR                  *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X380)      /* DATX8 7 General Configuration Register */
#define rPHY_DX7GSR0                 *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X384)      /* DATX8 7 General Status Register 0 */     
#define rPHY_DX7GSR1                 *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X388)      /* DATX8 7 General Status Register 1 */     
#define rPHY_DX7DLLCR                *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X38c)      /* DATX8 7 DLL Control Register */          
#define rPHY_DX7DQTR                 *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X390)      /* DATX8 7 DQ Timing Register */            
#define rPHY_DX7DQSTR                *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X394)      /* DATX8 7 DQS Timing Register */           
/* 0XE6~0XEF RESERVED */                                                                        
#define rPHY_DX8GCR                  *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X3c0)      /* DATX8 8 General Configuration Register */
#define rPHY_DX8GSR0                 *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X3c4)      /* DATX8 8 General Status Register 0 */     
#define rPHY_DX8GSR1                 *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X3c8)      /* DATX8 8 General Status Register 1 */     
#define rPHY_DX8DLLCR                *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X3cc)      /* DATX8 8 DLL Control Register */          
#define rPHY_DX8DQTR                 *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X3d0)      /* DATX8 8 DQ Timing Register */            
#define rPHY_DX8DQSTR                *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR+0X3d4)      /* DATX8 8 DQS Timing Register */                         
/* 0XF6~0XFF RESERVED */ 

#define rPHY_DXGCR(x)                *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR + 0X1c0 + 0x40 * x)      /* DATX8 0 General Configuration Register */
#define rPHY_DXGSR0(x)               *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR + 0X1c4 + 0x40 * x)      /* DATX8 0 General Status Register 0 */
#define rPHY_DXGSR1(x)               *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR + 0X1c8 + 0x40 * x)      /* DATX8 0 General Status Register 1 */
#define rPHY_DXDLLCR(x)              *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR + 0X1cc + 0x40 * x)      /* DATX8 0 DLL Control Register */
#define rPHY_DXDQTR(x)               *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR + 0X1d0 + 0x40 * x)      /* DATX8 0 DQ Timing Register */
#define rPHY_DXDQSTR(x)              *(volatile unsigned *)(DWC_DDR_PUBL_BASE_ADDR + 0X1d4 + 0x40 * x)      /* DATX8 0 DQS Timing Register */

#endif /* imapx_dramc.h */
