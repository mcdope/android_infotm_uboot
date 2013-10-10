#ifndef __BOOTL_H
#define __BOOTL_H
enum boottype{
	/************************error****************************/
	BOOT_HALT_SYSTEM=0xFF,
	/*******************for recovery mode*********************/
	BOOT_NORMAL=0x0,
	BOOT_FACTORY_INIT=0x1,
	BOOT_CHARGER=0x2,
	/*************************Normal mode*********************/
};
extern int set_boot_type(int type);
extern int got_boot_type(void);
extern int bootl(void);
extern uint8_t *load_image(char *name , char* alternate, char* rambase, int src_devid);
extern int do_boot(void* , void*);
extern int boot_verify_type(void);
#endif
