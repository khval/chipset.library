
#define mem_block_size 1024
#define chip_ram_size (512*mem_block_size)

extern int used_blocks[512];
extern char chip_ram[chip_ram_size];

struct allocted_mem
{
	void *start;
	int used;
};

struct mem_list
{
	struct allocted_mem  *allocted_tab;
	int used;
	int allocated;
};

char *allocChip(uint32_t size);


