#include "common.h"

extern uint32_t dram_read(hwaddr_t, size_t);
extern void dram_write(hwaddr_t, size_t, uint32_t);

typedef struct Block {
	uint32_t valid;
	uint32_t dirty;
	uint32_t tag;
	char data[64];
} Block;

typedef struct Cache {
	int size;
	int set_index_bits_size;
	int block_num;
	int block_size;
	int offsets;//Cache_B
	Block* blocks;
	Block *(* find)(struct Cache *this, swaddr_t addr, bool allocate, int cache);
	uint32_t (* read) (void *this, swaddr_t addr, size_t len);
	void (* write) (void *this, swaddr_t addr, size_t len, uint32_t data);
} Cache;
