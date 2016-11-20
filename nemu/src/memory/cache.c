#include "common.h"
#include "memory/cache.h"
#include <time.h>
#include <stdlib.h>

#define DEBUG_CACHE

static Cache L1_cache;
const void* l1_cache_interface = &L1_cache;

inline uint32_t get_set_index(swaddr_t addr, int offsets, int blocknum)
{
	return (addr >> offsets) & ((1 << blocknum) - 1);
}

inline uint32_t get_tag(swaddr_t addr, int offsets, int blocknum)
{
	return addr >> offsets >> blocknum;
}

inline uint32_t get_offsets(swaddr_t addr, int offsets)
{
	return addr & ((1 << offsets) - 1);
}

static Block* find(struct Cache *this, swaddr_t addr, bool allocate)
{
	uint32_t set_index = get_set_index(addr, this->offsets, this->block_num);
	uint32_t tag = get_tag(addr, this->offsets, this->block_num);
#ifdef DEBUG_CACHE
	printf("set_index:%x\n", set_index);
	printf("find addr:%x\n", addr);
	printf("tag:%x\n", tag);
#endif

	//searchinging
	int i = 0;
	for (i = this->block_num * set_index; i < (this->block_num * (set_index + 1)); i++)
		if (this->blocks[i].valid && this->blocks[i].tag == tag)
			return &(this->blocks[i]);
#ifdef DEBUG_CACHE
	printf("cachemiss\n");
#endif

	if (allocate)
	{
		srand(time(0));
		Block *victim = &(this->blocks[this->block_num * set_index + rand() % this->block_num]);
		swaddr_t victim_addr = (victim->tag << (this->bits_size + this->offsets))
		                       + (set_index << (this->offsets));
#ifdef DEBUG_CACHE
		printf("victim addr:%x\n", victim_addr);
#endif

		//read from dram
		uint32_t mask = (0xffffffff << this->offsets);
		addr &= mask;
		int i = 0;
		for (i = 0; i < this->block_size; i++)
			victim->data[i] = dram_read(addr++, 1);
		victim->valid = 1;
		victim->tag = tag;
		return victim;
	}
	else
		return NULL;

}

static uint32_t read(struct Cache *this, swaddr_t addr, unsigned int len)
{
	Block *block = find(this, addr, true);
	int offset = get_offsets(addr, this->offsets);
	uint8_t temp[2 * this->block_size];
	memcpy(temp, block->data, this->block_size);
	if (offset + len > this->block_size)
	{
		block = find(this, addr + this->block_size, true);
		memcpy(temp + (this->block_size), block->data, this->block_size);
	}
	return unalign_rw(temp + offset, 4);
}

static void write(struct Cache *this, swaddr_t addr, unsigned int len, uint32_t data, bool allocate)
{
	Block* block = find(this, addr, allocate);
	if (block == NULL && !allocate)
		return;
	int offset = get_offsets(addr, this->offsets);
	uint8_t *target = (uint8_t *)&data;
	int i;
	for (i = 0; i < len; i++)
	{
		if (offset + i >= this->block_size)
		{
			block = find(this, addr + i, allocate);
			if (block == NULL && !allocate)
				return;
			offset -= i;
			i--;
		}
		else
			block->data[offset + i] = *(target++);
	}
	return;
}

uint32_t L1_read(void *this, swaddr_t addr, size_t len)
{
	return read((struct Cache *)this, addr, len);
}

void L1_write (void *this, swaddr_t addr, size_t len, uint32_t data)
{
	write((struct Cache *)this, addr, len, data, false);
	return;
}

void init_cache()
{
	L1_cache.size = 128;
	L1_cache.bits_size = 7;
	L1_cache.block_num = 8;
	L1_cache.block_size = 64;
	L1_cache.offsets = 6;
	L1_cache.find = find;
	L1_cache.read = L1_read;
	L1_cache.write = L1_write;
	L1_cache.blocks = (Block *)malloc(sizeof(Block) * 1024);
	memset(L1_cache.blocks, 0, sizeof(Block) * 1024);
}
