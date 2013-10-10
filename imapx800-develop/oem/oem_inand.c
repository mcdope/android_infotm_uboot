#include <common.h>
#include <mmc.h>
#include <oem_inand.h>
#include <oem_func.h>

int oem_write_inand(char *membase, int startblk, int cnt)
{
   	if(NULL==membase) {
		printf("Error: invalid image buffer address %x", (int)membase);
		return 1;
	}
	
        struct mmc *mmc = find_mmc_device(iNAND_CHANNEL);
        if (!mmc)
              return 1;
	mmc->Wflag=1;
        if( mmc_init(mmc)) return 1;

        if(((cnt+startblk )> mmc->sector_count)&&mmc->sector_count){
              printf("Error:write out of range:start block # %x, count %x  device capacity is: %x",
                           startblk, cnt,mmc->sector_count);
	      return 1;
	 }
	if(cnt<=10) goto finish;
	int steplength=0;
	int n=0;
	if (cnt>BURN_STEP_LENGTH*10) steplength=BURN_STEP_LENGTH;
		else steplength=BURN_STEP_LENGTH>>4;
	while(cnt>steplength){
       		n= mmc->block_dev.block_write(iNAND_CHANNEL, startblk, steplength, membase);
		if(n==steplength){
			startblk+=steplength;
			membase+=steplength<<9;
			cnt-=steplength;
			oem_simpro_update(steplength);
		}else{
			printf("writen failed.\n");
			return 1;
		}
	}
	oem_simpro_update(cnt);
finish:
	if(cnt){
		n= mmc->block_dev.block_write(iNAND_CHANNEL, startblk, cnt, membase);
		if(n==cnt) return 0;
	        printf("writen failed.\n");
		return 1;	
	}

	return 0;
}

int oem_read_inand(char *membase, int startblk, int cnt)
{
 	struct mmc *mmc = find_mmc_device(iNAND_CHANNEL);
	if (!mmc)
		return 1;

	mmc->Wflag=0;
	if(  mmc_init(mmc)) return 1;
                //printf("MMC read: dev # %d, block # %d, count %d ...\n",
                //      dev, blk, cnt);
	if(((cnt+startblk )> mmc->sector_count)&&mmc->sector_count)
		printf("Error:read out of range:start block # %x, count %x device capacity  is:%x\n ",
				startblk, cnt,mmc->sector_count);
	int n = mmc->block_dev.block_read(iNAND_CHANNEL, startblk, cnt, membase);
	
		 /* flush cache after read */
	flush_cache((ulong)membase, cnt * 512); /* FIXME */
	if(n!=cnt)  printf("%d blocks read: %s\n",n,"ERROR");
	return (n == cnt) ? 0 : 1;
}
int init_part_table(struct part_table* locatition, struct part_table* parttable, int* tableaddr, char step)
{
	static int lasttableaddr;

	memset(parttable,0,sizeof(struct part_table)<<2);
	if(!step){
			memcpy(parttable,locatition, sizeof(struct part_table)<<2) ;
			*tableaddr=0 ;
			lasttableaddr=*tableaddr;
	}else{		
		if(step>PARTITION_NUM-3){
			printf("Error: outof partition range.\n");
			return -1;
		}
		parttable[0]= locatition[step+3];
		if(step<(PARTITION_NUM-4)){

			printf("step:%d\n",step);
			int i=1;
			parttable[1]= locatition[step+4];
			if((step+1)<(PARTITION_NUM-3))parttable[1].parttype=0x05;
			parttable[1].startLBA=0;
			for(;i<=step;i++){
					parttable[1].startLBA+=0x0010;
		      		parttable[1].startLBA+=locatition[i+3].sizeinsectors;
			}
		}
		if(step==1) *tableaddr= locatition[3].startLBA;
			else *tableaddr=lasttableaddr+ 0x000010 + locatition[step+2].sizeinsectors;
		lasttableaddr=*tableaddr;
	}	
	return 0 ;
}
int init_part_location(int sectorcount,struct part_table* locationtable)
{
	unsigned int count=0;
	unsigned int i=0;

	unsigned int sectors[MAX_iNAND_PARTITION]={
		0,
		SYSTEM_PART<<11,
		DATA_PART<<11,
		0x30*(PARTITION_NUM-3)+((EXTEND_LOCATION)<<11),	
		0x30+(CACHE_PART<<11),
		0x30+(BACK_PART<<11)
	};
	unsigned int sparesectors=SPARE_LOCATION<<11;
	
	for(i=1;i<PARTITION_NUM;i++) count+=sectors[i];
	printf("count:%x\n",count);
	sectors[0]=sectorcount-count - SPARE_LOCATION-0x80;
	if((sectors[0]+0x10)%0x40) sectors[0]-=sectors[0]%0x40;
	printf("User area:0x%x\n",sectors[0]<<9);

	for(i=0;i<PARTITION_NUM;i++){
		locationtable[i].sizeinsectors=sectors[i];
		locationtable[i].parttype=0x83;
		locationtable[i].startCHS[0]=0x03;
		locationtable[i].startCHS[1]=0xd0;
		locationtable[i].startCHS[2]=0xFF;
		locationtable[i].endCHS[0]=0x03;
		locationtable[i].endCHS[1]=0xd0;
		locationtable[i].endCHS[2]=0xFF;
		locationtable[i].bootableflag=0;
	}
	locationtable[0].parttype=0x0C;
	locationtable[PARTITION_NUM-1].parttype=0x0C;
	if(PARTITION_NUM>4) locationtable[3].parttype=0x05;	 

	locationtable[0].startCHS[0]=0x01;
	locationtable[0].startCHS[1]=0x01;
	locationtable[0].startCHS[2]=0x00;
	locationtable[0].startLBA=0x000010;

	i=1;
	for(i=1;(i<PARTITION_NUM)&&(i<4);i++){
		locationtable[i].startLBA=locationtable[i-1].startLBA+locationtable[i-1].sizeinsectors;
		if(locationtable[i].startLBA%0x40)locationtable[i].startLBA+=0x40-locationtable[i].startLBA%0x40;
	}

	for(;i<PARTITION_NUM;i++)
		locationtable[i].startLBA=0x000010;
	
	locationtable[PARTITION_NUM].sizeinsectors=sparesectors;
	locationtable[PARTITION_NUM].startLBA= sectorcount-sparesectors;
	return 0;
}
int oem_partition(void)
{
	struct mmc *mmc;
	int dev_num;
	dev_num=iNAND_CHANNEL;

	mmc = find_mmc_device(dev_num);
        if (mmc) {
                mmc_init(mmc);
        }
	printf("Device Capacity:0x%x\n",mmc->sector_count);
	struct part_table partitiontable[4];
	struct part_table *locationtable=(struct part_table *)CONFIG_RESV_LOCTABLE;
	init_part_location(mmc->sector_count,locationtable);
	
	
	char partitionnum=0;
	if(PARTITION_NUM-3) partitionnum=PARTITION_NUM-3;
		else partitionnum=1;

	unsigned int i=0;
	char* buffer=(char*)CONFIG_RESV_PTBUFFER;
	memset(buffer,0,iNAND_BLOCK_SIZE);

	for(i=0;i<partitionnum;i++){
		int tableaddr=0;
		init_part_table(locationtable,partitiontable,&tableaddr,i);
		memcpy(buffer+446 , partitiontable , sizeof(struct part_table)<<2);
		
		buffer[510]=0x55;
		buffer[511]=0xaa;

		if(i==0) {
			int *p=(int*)buffer;
			printf("PARTITION_NUM :%d\n   image start:%x\n\n", PARTITION_NUM ,locationtable[PARTITION_NUM].startLBA );
			p[CONFIG_IMAGE_BASE]=locationtable[PARTITION_NUM].startLBA;
			p[CONFIG_SYSPT_BASE]=locationtable[1].startLBA;
#ifdef CONFIG_HIBERNATE
			p[CONFIG_HIBER_ADDRBUF]=locationtable[PARTITION_NUM].startLBA+(IMAGE_LOCATION<<11);
#else
			p[CONFIG_HIBER_ADDRBUF]=0;
#endif
			printf("Hibernate start:%x\n",p[CONFIG_HIBER_ADDRBUF]);
		}
	//	printf("Table address:%x\n",tableaddr);
		oem_write_inand(buffer,tableaddr,1);
	}
	int ret =locationtable[PARTITION_NUM].startLBA;
	return ret;

}
int oem_get_base(int basetype)
{
    int startblk=0;
    int *buffer=(int *)CONFIG_RESV_PTBUFFER;
    memset((char *)buffer,0,512);
    if(oem_read_inand((char *) buffer , 0 ,1)) return 0;

    startblk=buffer[basetype];
//    printf("startblk:%x\n",startblk);
    
    return startblk;
}
int oem_get_imagebase(void)
{

	return	oem_get_base(CONFIG_IMAGE_BASE);
}
int oem_get_systembase(void)
{
	return  oem_get_base(CONFIG_SYSPT_BASE);
}
#ifdef CONFIG_HIBERNATE
int oem_get_hibernatebase(void)
{
	return  oem_get_base(CONFIG_HIBER_ADDRBUF);
}
#endif
