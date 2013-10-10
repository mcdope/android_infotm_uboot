#include <common.h>
#include <command.h>
#include <vstorage.h>

struct debug {
	int value;
	int fault;
	char s[16];
};

static char *name = NULL;
static int id = 0;

int output_debug(struct debug bug)
{
	switch (bug.value) {
		case -3:
			printf("\nOperation is unavailable on device %d\n", id);
			break;
		case -1:
			printf("\n%s %d failed\n", bug.s, id);
			break;
		case -2:
			printf("\nPlease assign a device.\n");
			break;
		default:
			if (bug.fault == 1)
					printf("\nReset OK\n");
			else if (bug.fault == 2)
					printf("\nErase done.\n");
			else
					printf("%d bytes %s device %d\n", bug.value, bug.s, id);
	}

	return 0;
}

int do_vs(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	loff_t dev_start = 0;
	struct debug vs_bug;
	uint64_t len = 0;
	uint32_t mem_addr = 0;
	uint32_t extra = 0;
	int i;
	uint8_t *buf;

	if (argc == 6) 
		extra = simple_strtoul(argv[5], NULL, 16);

	switch (argc) {
		case 1:
			printf("\n%s\n", cmdtp->help);
			return 0;
		case 2:
			if (strcmp(argv[1], "info") == 0) {
				printf("Device list:\n");
				for (i = 0;i < vs_device_count();i++)
					printf("%-4d%s\n", i, vs_list[i].name);
				printf("\nCurrent device: ");
				if (vs_is_assigned()) 
					printf("%d(0x%x)\n", id, vs_align(id));
				else
					printf("not assigned yet.\n");
				
				return 0;
			} else if (strcmp(argv[1], "reset") == 0) {
				vs_bug.value = vs_reset();
				strcpy(vs_bug.s, "reset");
				vs_bug.fault = 1;
				output_debug(vs_bug);

				return 0;
			} else {
				printf("\n%s\n", cmdtp->help);
				return -1;
			}
		case 3:
			if (strcmp(argv[1], "assign") == 0) {
				if (vs_assign_by_name(argv[2], 1) == 0) {
					name = argv[2];
					id = vs_device_id(name);
					printf("Assign OK.\n");
				} else
					printf("No such device is found in virtual storage.\n");
				
				return 0;
			} else {
				printf("\n%s\n", cmdtp->help);
				return -1;
			}
		case 4:
			dev_start = simple_strtoull(argv[2], NULL, 16);
			len = simple_strtoull(argv[3], NULL, 16);
			strcpy(vs_bug.s, "erase");
			vs_bug.fault = 2;
			if (strcmp(argv[1], "erase") == 0) {
				vs_bug.value = vs_erase(dev_start , len);	
			} else if (strcmp(argv[1], "scrub") == 0) {
				vs_bug.value = vs_scrub(dev_start , len);
			} else {
				printf("\n%s\n", cmdtp->help);
				return -1;
			}
			output_debug(vs_bug);
			return 0;
		case 5:
		case 6:
			mem_addr = simple_strtoul(argv[2], NULL, 16);
			dev_start = simple_strtoull(argv[3], NULL, 16);
			len = simple_strtoull(argv[4], NULL, 16);
			buf = (uint8_t *)mem_addr;
			vs_bug.fault = 0;	
			strcpy(vs_bug.s, "write to");

			if (strcmp(argv[1], "write") == 0) {
				vs_bug.value = vs_write(buf, dev_start, len, extra);
			} else if (strcmp(argv[1], "read") == 0) {
				vs_bug.value = vs_read(buf, dev_start, len, extra);
				strcpy(vs_bug.s, "read from");
			} else {
				printf("\n%s\n", cmdtp->help);
				return -1;
			}
			output_debug(vs_bug);
			return 0;
		default:
			printf("\n%s\n", cmdtp->help);
	}	

	return 0;					
}

U_BOOT_CMD(vs, CONFIG_SYS_MAXARGS,1, do_vs,
		"vs\n",
		"vs info\n"
		"vs assign number\n"
		"vs reset\n"
		"vs read (memory address) (device start address) len\n"
		"vs write (memory address) (device start address) len\n"
		"vs erase (device start address) len\n"
		"vs scrub (memory address) (device start address) len"
);
