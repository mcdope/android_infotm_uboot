
#include <common.h>
#include <cdbg.h>
#include <isi.h>
#include <ius.h>
#include <hash.h>
#include <crypto.h>
#include <bootlist.h>
#include <vstorage.h>
#include <malloc.h>
#include <nand.h>
#include <led.h>
#include <irerrno.h>
#include <items.h>
#include <storage.h>
#include <linux/types.h>
extern int partnum;
extern unsigned int sectors[];
extern loff_t spare_area;
extern int has_local;
struct part_sz default_cfg[] = {//config must be: part.system 256M  part.user 1G etc,. 
	{ "uboot0", 2, 6 },
	{ "uboot1", 8, 8 },
	{ "items", 16, 8 },
	{ "logo", 24, 8 },
	{ "ramdisk", 32, 8 },
	{ "flags", 40, 8 },
	{ "reserved", 48, 8 },
	{ "kernel0", 56, 16 },
	{ "kernel1", 72, 16 },
	{ "recovery-rd", 88, 24 },//spare area 2-112M
	{ "system", 0, 152 },
	{ "misc", 0, 20 },
	{ "cache", 0, 64 },
	{ "userdata", 0, 164 },
	{"local",0,0},
	{"add1",0,0},
	{"add2",0,0},
};

int storage_device(void)
{
	char buf[32], *devs[] = {
		"bnd", "nnd", "fnd", "eeprom",
		"flash", "hdisk", "mmc0",
		"mmc1", "mmc2", "udisk0",
		"udisk1", "udc", "eth", "ram"};
	int i;

	if(item_string(buf, "board.disk", 0) < 0)
		return DEV_NAND;
	else {
		for(i = 0; i < ARRAY_SIZE(devs); i++)
			if(strncmp(devs[i], buf, 20) == 0)
				return i;
	}

	/* if error hanppens, always NAND ... */
	return DEV_NAND;
}
int item_strtoul(char* info)
{
	char * p=info;
	int val=0;
	while(*p!='\0'){
		if((*p<'0')||(*p>'9')) break;
		val=val*10;
		val+=*p-'0';
		p++;
	}
	
	if(*p=='G'||*p=='g'){
		printf("unit %c\n",*p);
		val<<=10;
	}else if(0<val&&val<10){
		printf("treate as unit G\n");
		val<<=10;
	}

	return val;
}
static int storage_get_new_config(void)
{
	int i=0 , index=0;
	char info[32]={0,};
	while(i<INFO_MAX_PARTITIONS){
		char part[16];
		memset(info,'\0',32);
		index=i+INFO_SPARE_IMAGES;
		sprintf(part,"part.%s",default_cfg[index].name);// config must be like: (part.system 128M ) or (part.user 2G) 
		item_string(info, part, 0);
		int n=strlen(info);
		int size=0;
		if(info[n-1]=='M'||info[n-1]=='m'){
			info[n-1]='\0';
		}else if(i<4&&info[0]=='\0'){
			printf("Configuration for part %s is invalid, use default size:%dM\n",part, default_cfg[index].size);
		}else if(info[0]=='\0'){
			printf("Erro: invalid partition size unit for part %s\n",part);
			return 0;
		}

		size=item_strtoul(info);
		if(size!=0)
			default_cfg[index].size=size;//<<(shift*10); ////all sectors are united as MBytes
		sectors[partnum] = default_cfg[index].size;
		partnum+=1;

		if(size==0&&i>=3&&i!=4) break;
		i++;

	}
	printf("part number %d\n",partnum);
	//Now the sectors table is united by MBytes
	if(partnum==INFO_MAX_PARTITIONS){
		printf("Erro: too many partitions configured, FAILED\n");
		return -1;
	}
	return 0;

}
int storage_part(void)
{


	/* do nothing if a NAND based system */
	int ret=0;
	int dev_id=storage_device();
	if(DEV_NAND==dev_id) return 0;
	struct part_sz* partcfg=NULL;
	struct part_sz* partcfg_old=NULL;
	if(6<=dev_id&&dev_id<=10){

//		partcfg=malloc((INFO_SPARE_IMAGES+INFO_MAX_PARTITIONS)*sizeof(struct part_sz));
		//if(partcfg==NULL){
		//	printf("Erro:failed to alloc buffer for part config\n");
		//	return -1;
	//	}
//		memset(partcfg,0,(INFO_SPARE_IMAGES+INFO_MAX_PARTITIONS)*sizeof(struct part_sz));
//		memcpy(partcfg, default_cfg , sizeof(default_cfg));
		storage_get_new_config();
			
		ret=block_partition(default_cfg);
/*
		partcfg_old=malloc((INFO_SPARE_IMAGES+INFO_MAX_PARTITIONS)*sizeof(struct part_sz));
		if(partcfg_old==NULL) {
			printf("Erro:Failed to alloc buffer for part info\n");
			ret= -1;
			goto end;
		}
		memset(partcfg_old,0,INFO_MAX_PARTITIONS*sizeof(struct part_sz));
*/		//block_get_part_info(partcfg_old);

		/* check if it is a new configuration */
			/*
		if(memcmp(partcfg_old+INFO_SPARE_IMAGES , partcfg+INFO_SPARE_IMAGES , INFO_MAX_PARTITIONS*sizeof( struct part_sz)))
			ret= block_partition( partcfg);//
		goto end; //disk has been partitioned and satisfy the newest config
		*/
	}else{
		/* TODO: */
		/********For other types of disk is still to go**********/
	}
	if(NULL!=partcfg) 
		free(partcfg);
	if(NULL!=partcfg_old)		
		free(partcfg_old);
	return ret;
}

loff_t storage_offset(const char *p)
{
	loff_t parted = 0, i, ret;
	char s[32];

	if(!p) return 0;
	int dev_id=storage_device();
	/* check the reserved part */
	if(DEV_NAND==dev_id){
		if(strncmp(p, "uboot0", 10) == 0)
		  return 0;
		for(i = 0; i < ARRAY_SIZE(default_cfg); i++)
		{
			if(strncmp(default_cfg[i].name, p, 20) == 0)
			  return (parted << 20);

			if(default_cfg[i].offs)
			  parted = default_cfg[i].offs + default_cfg[i].size;
			else {
				/* not a raw partition, let's check
				 * the fs partitions.
				 */
				sprintf(s, "part.%s", default_cfg[i].name);
				ret = item_integer(s, 0);
				parted += (ret == ITEMS_EINT)?
					default_cfg[i].size: ret;
			}
		}
	}else if(6<=dev_id && dev_id<=10){ //for non-NAND devices
	    	int index=0;
		while(index<INFO_SPARE_IMAGES){
		    if(default_cfg[index].name==NULL){
			printf("Erro: can not find infomation of part:%s\n",p);
			return -1;
		    }
		    //	printf("%s\n",buffer[index].name);

		    if(!strcmp(default_cfg[index].name,p)){
//			printf("%d\n",index);
			parted = default_cfg[index].offs<<20;
			break;
		    }
		    index++;
		}
		if(parted==0)
			parted=block_get_base(p);
//		printf("got offs:0x%llx\n",parted);
	}

	return parted;  //units: Byte
}

loff_t storage_size(const char *p)
{
	loff_t i, ret;
	char s[32];

	if(!p) return 0;
	/* check the reserved part */
	for(i = 0; i < ARRAY_SIZE(default_cfg); i++)
	{
		if(strncmp(default_cfg[i].name, p, 20) == 0)
		{
			/* not a raw partition, let's check
			 * the fs partitions.
			 */
			sprintf(s, "part.%s", p);
			ret = item_integer(s, 0);
			if(ret == ITEMS_EINT)
				ret = default_cfg[i].size;

			return (ret << 20);
		}
	}

	return 0;
}

