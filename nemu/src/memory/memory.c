#include "common.h"
#include "cpu/reg.h"
#include "memory/cache.h"
#include "../../lib-common/x86-inc/mmu.h"
#include <time.h>
#include <stdlib.h>

#define CACHED
//#define DEBUGGING

static Cache TLB;

extern void* l1_cache_interface;
extern uint32_t mmio_read(hwaddr_t addr, size_t len, int map_NO);
extern int is_mmio(hwaddr_t addr);
extern void mmio_write(hwaddr_t addr, size_t len, uint32_t data, int map_NO);

static inline uint32_t decompose_addr(uint32_t data, uint32_t a, uint32_t b)
{
	return (data << (31 - b)) >> (31 - b + a);
}

uint32_t L1_read(void *this, swaddr_t addr, size_t len);

void  L1_write(void *this, swaddr_t addr, size_t len, uint32_t data);

uint32_t dram_read(hwaddr_t, size_t);
void dram_write(hwaddr_t, size_t, uint32_t);

/* Memory accessing interfaces */

uint32_t hwaddr_read(hwaddr_t addr, size_t len) {
#ifdef DEBUGGING
	printf("read addr:%x\n", addr);
#endif

#ifdef HAS_DEVICE
	if (is_mmio(addr) != -1) {
		Log("hit io address, address=%x, io port=%x", addr, is_mmio(addr));
		return mmio_read(addr, len, is_mmio(addr));
	}
	else {
#endif

#ifndef CACHED
		return dram_read(addr, len) & (~0u >> ((4 - len) << 3));
#endif

#ifdef CACHED
		return L1_read(l1_cache_interface, addr, len) & (~0u >> ((4 - len) << 3));
#endif

#ifdef HAS_DEVICE
	}
#endif
}

void hwaddr_write(hwaddr_t addr, size_t len, uint32_t data) {
#ifdef DEBUGGING
	printf("write addr:%x data:%x\n", addr, data);
#endif

#ifdef HAS_DEVICE
	if (is_mmio(addr) != -1) {
		Log("hit io address, address=%x, io port=%x", addr, is_mmio(addr));
		return mmio_write(addr, len, data, is_mmio(addr));
	} else {
#endif

#ifndef CACHED
		dram_write(addr, len, data);
#endif

#ifdef CACHED
		L1_write(l1_cache_interface, addr, len, data);
#endif

#ifdef HAS_DEVICE
	}
#endif
}

void init_TLB()
{
	TLB.size = 1;
	TLB.block_num = 64;
	TLB.blocks = (Block *)malloc(sizeof(Block) * 64);
	memset(TLB.blocks, 0, sizeof(Block) * 64);
}

hwaddr_t page_translate(lnaddr_t addr)
{
	struct Cache *this = &TLB;
	uint32_t set_index = 0;
	uint32_t tag = decompose_addr(addr, 12, 31);
	uint32_t offset = decompose_addr(addr, 0, 11);

	int i = 0;
	for (i = this->block_num * set_index; i < (this->block_num * (set_index + 1)); i++)
		if (this->blocks[i].valid && this->blocks[i].tag == tag)
		{
			uint32_t *a = (uint32_t *)(this->blocks[i].data);
			//Log("tag=%x,pageframe=%x,offset=%x", tag, *a, offset);
			return (*a << 12) + offset;
		}
	//Log("TLB miss");
	srand(time(0));
	Block *victim = &(this->blocks[this->block_num * set_index + rand() % this->block_num]);

	uint32_t page = decompose_addr(addr, 12, 21);
	uint32_t dir = decompose_addr(addr, 22, 31);
	//Log("page_directory_base=%x", cpu.cr3.page_directory_base);
	hwaddr_t dir_addr = (cpu.cr3.page_directory_base << 12) + dir * 4;
	PDE page_dir;
	page_dir.val = hwaddr_read(dir_addr, 4);
	//Log("dir_addr=%x", dir_addr);
	Assert(page_dir.present == 1, "unvalid page directry, addr to be translate=%x", addr);
	hwaddr_t tab_addr = (page_dir.page_frame << 12) + page * 4;
	PTE page_tab;
	//Log("tab addr=%x", tab_addr);
	page_tab.val = hwaddr_read(tab_addr, 4);
	Assert(page_tab.present == 1, "unvalid page table");
	//Log("pageframe=%x,offset=%x", page_tab.page_frame, offset);
	uint32_t *tmp = (uint32_t *)(victim->data);
	*tmp = page_tab.page_frame;
	victim->valid = 1;
	victim->tag = tag;
	//Log("TLB save, tag=%x,data=%x,page_frame=%x", tag, *tmp, page_tab.page_frame);
	return (page_tab.page_frame << 12) + offset;
};


uint32_t lnaddr_read(lnaddr_t addr, size_t len) {
	assert(len == 1 || len == 2 || len == 4);
	if ((cpu.cr0.protect_enable == 1) && (cpu.cr0.paging == 1))
	{
		if ((addr & 0xfffff000) != ((addr + len - 1) & 0xfffff000)) {
			//read cross page
			lnaddr_t prev_page_addr = addr & 0xfffff000;
			lnaddr_t sec_page_addr = (addr + len - 1) & 0xfffff000;
			uint32_t prev_len = 4096 - (addr - prev_page_addr);
			uint32_t sec_len = len - prev_len;
			uint32_t data1 = hwaddr_read(page_translate(addr), prev_len);
			uint32_t data2 = hwaddr_read(page_translate(sec_page_addr), sec_len);
			uint32_t data = (data2 << 8 * prev_len) + data1;
			Log("read cross page,lnaddr=%x,hwaddr1=%x,hwaddr2=%x,data1=%x,data2=%x,data=%x,len1=%x,len2=%x",
			    addr, page_translate(addr), page_translate(sec_page_addr), data1, data2, data, prev_len, sec_len);
			return data;
		}
		else {
			hwaddr_t hwaddr = page_translate(addr);
			//Log("addr to be translated:%x, hwaddr=%x, data=%x", addr, hwaddr, hwaddr_read(hwaddr, len));
			return hwaddr_read(hwaddr, len);

		}
	}
	else return hwaddr_read(addr, len);
}

void lnaddr_write(lnaddr_t addr, size_t len, uint32_t data) {
	assert(len == 1 || len == 2 || len == 4);
	if (cpu.cr0.protect_enable == 1
	        && cpu.cr0.paging == 1)
	{
		if ((addr & 0xfffff000) != ((addr + len - 1) & 0xfffff000))
		{
			Log("write cross page");
			lnaddr_t prev_page_addr = addr & 0xfffff000;
			lnaddr_t sec_page_addr = (addr + len - 1) & 0xfffff000;
			uint32_t prev_len = 4096 - (addr - prev_page_addr);
			uint32_t sec_len = len - prev_len;
			uint32_t data1 = (data << sec_len * 8) >> sec_len * 8;
			uint32_t data2 = data >> prev_len * 8;
			hwaddr_write(addr, prev_len, data1);
			return hwaddr_write(sec_page_addr, sec_len, data2);
		}
		else {
			//Log("addr to be translated:%x", addr);
			hwaddr_t hwaddr = page_translate(addr);
			//Log("hwaddr=%x", hwaddr);
			return hwaddr_write(hwaddr, len, data);

		}
	}
	else return hwaddr_write(addr, len, data);
}

lnaddr_t seg_translate(swaddr_t addr, uint8_t sreg)
{
	if (cpu.cr0.protect_enable == 1) {
		if (!seg(sreg).cache) {
			lnaddr_t dis_addr = (lnaddr_t)cpu.gdtr.base_addr + (seg(sreg).index << 3);
			//translation
			SegDesc descriptor;
			uint32_t *tmp = (uint32_t *)&descriptor;
			*tmp = lnaddr_read( dis_addr, 4);
			tmp++;
			*tmp = lnaddr_read( dis_addr + 4, 4);
			seg(sreg).base = descriptor.base_15_0 + (descriptor.base_23_16 << 16) + (descriptor.base_31_24 << 24);
			seg(sreg).limit = descriptor.limit_15_0 + (descriptor.limit_19_16 << 16);
			seg(sreg).cache = true;
		}
		return addr + seg(sreg).base;
	}
	else
		return (lnaddr_t)addr;
}

uint32_t swaddr_read(swaddr_t addr, size_t len, uint8_t sreg) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	lnaddr_t lnaddr = seg_translate(addr, sreg);
	return lnaddr_read(lnaddr, len);
}

void swaddr_write(swaddr_t addr, size_t len, uint32_t data, uint8_t sreg) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	lnaddr_t lnaddr = seg_translate(addr, sreg);
	lnaddr_write(lnaddr, len, data);
}

