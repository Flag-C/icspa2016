#include "common.h"
#include "memory/cache.h"
#include <time.h>
#include <stdlib.h>

#define DEBUG_CACHE

static Cache L1_cache;
static Cache L2_cache;
const void* l1_cache_interface = &L1_cache;

static inline uint32_t get_set_index(swaddr_t addr, int offsets, int blocknum)
{
	return (addr >> offsets) & ((1 << blocknum) - 1);
}

static inline uint32_t get_tag(swaddr_t addr, int offsets, int blocknum)
{
	return addr >> offsets >> blocknum;
}

static inline uint32_t get_offsets(swaddr_t addr, int offsets)
{
	return addr & ((1 << offsets) - 1);
}

static Block* find(struct Cache *this, swaddr_t addr, bool allocate, int cache)
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
		if (cache == 1)
		{
			Block *victim = &(this->blocks[this->block_num * set_index + rand() % this->block_num]);
#ifdef DEBUG_CACHE
			Log("l1 miss");
			swaddr_t victim_addr = (victim->tag << (this->bits_size + this->offsets))
			                       + (set_index << (this->offsets));
			printf("victim addr:%x\n", victim_addr);
#endif
			Block *constitude = L2_cache.find(&L2_cache, addr, true, 2);
			memcpy(victim->data, constitude->data, this->block_size);
			victim->valid = 1;
			victim->tag = tag;
			return victim;
		}
		else if (cache == 2)
		{
			Block *victim = &(this->blocks[this->block_num * set_index + rand() % this->block_num]);
#ifdef DEBUG_CACHE
			Log("l2 miss");
			swaddr_t victim_addr = (victim->tag << (this->bits_size + this->offsets))
			                       + (set_index << (this->offsets));
			printf("victim addr:%x, vaild:%x, dirty:%x, tag: %x\n", victim_addr, victim->valid, victim->dirty, victim->tag);
#endif
			if (victim->dirty == 1 && victim->valid == 1)
			{
				for (i = 0; i < this->block_size; i++)
					dram_write(victim_addr++, 1, victim->data[i]);
			}
			//read from dram
			uint32_t mask = (0xffffffff << this->offsets);
			addr &= mask;
			int i = 0;
			for (i = 0; i < this->block_size; i++)
				victim->data[i] = dram_read(addr++, 1);
			victim->valid = 1;
			victim->dirty = 0;
			victim->tag = tag;
			return victim;
		}
	}
	return NULL;
}

static uint32_t read(struct Cache *this, swaddr_t addr, unsigned int len, int cache)
{
	Block *block = find(this, addr, true, cache);
	int offset = get_offsets(addr, this->offsets);
	uint32_t ret;
	uint8_t *target = (uint8_t *)&ret;
	int i = 0;
	for (i = 0; i < len; i++)
	{
		if (offset + i >= this->block_size)
		{
			block = find(this, addr + i, true, cache);
			offset = -i;
			i--;
		}
		else
			*(target++) = block->data[offset + i];
	}
	return ret;

}

static void write(struct Cache *this, swaddr_t addr, unsigned int len, uint32_t data, bool allocate, int cache)
{
	Block* block = find(this, addr, allocate, cache);
	if (block == NULL && !allocate)
		return;
	if (cache == 2)
		block->dirty = 1;
	int offset = get_offsets(addr, this->offsets);
	uint8_t *target = (uint8_t *)&data;
	int i;
	for (i = 0; i < len; i++)
	{
		if (offset + i >= this->block_size)
		{
			block = find(this, addr + i, allocate, cache);
			if (block == NULL && !allocate)
				return;
			offset = -i;
			i--;
		}
		else
			block->data[offset + i] = *(target++);
	}
	return;
}

uint32_t L1_read(void *this, swaddr_t addr, size_t len)
{
	return read((struct Cache *)this, addr, len, 1);
}

void L1_write (void *this, swaddr_t addr, size_t len, uint32_t data)
{
	L2_cache.write(&L2_cache, addr, len, data);
	write((struct Cache *)this, addr, len, data, false, 1);
	return;
}

uint32_t L2_read(void *this, swaddr_t addr, size_t len)
{
	return read((struct Cache *)this, addr, len, 2);
}

void L2_write (void *this, swaddr_t addr, size_t len, uint32_t data)
{
	write((struct Cache *)this, addr, len, data, true, 2);
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
	L2_cache.size = 4096;
	L2_cache.bits_size = 12;
	L2_cache.block_num = 16;
	L2_cache.block_size = 64;
	L2_cache.offsets = 6;
	L2_cache.find = find;
	L2_cache.read = L2_read;
	L2_cache.write = L2_write;
	L2_cache.blocks = (Block *)malloc(sizeof(Block) * 65536);
	memset(L2_cache.blocks, 0, sizeof(Block) * 65536);
	return;
}
