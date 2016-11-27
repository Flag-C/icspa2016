#include "common.h"
#include "cpu/reg.h"
#include "../../lib-common/x86-inc/mmu.h"

#define CACHED
//#define DEBUGGING

extern void* l1_cache_interface;

uint32_t L1_read(void *this, swaddr_t addr, size_t len);

void  L1_write(void *this, swaddr_t addr, size_t len, uint32_t data);

uint32_t dram_read(hwaddr_t, size_t);
void dram_write(hwaddr_t, size_t, uint32_t);

/* Memory accessing interfaces */

uint32_t hwaddr_read(hwaddr_t addr, size_t len) {
#ifdef DEBUGGING
	printf("read addr:%x\n", addr);
#endif
#ifndef CACHED
	return dram_read(addr, len) & (~0u >> ((4 - len) << 3));
#endif

#ifdef CACHED
	return L1_read(l1_cache_interface, addr, len) & (~0u >> ((4 - len) << 3));
#endif
}

void hwaddr_write(hwaddr_t addr, size_t len, uint32_t data) {
#ifdef DEBUGGING
	printf("write addr:%x data:%x\n", addr, data);
#endif
#ifndef CACHED
	dram_write(addr, len, data);
#endif

#ifdef CACHED
	L1_write(l1_cache_interface, addr, len, data);
#endif
}

uint32_t lnaddr_read(lnaddr_t addr, size_t len) {
	return hwaddr_read(addr, len);
}

void lnaddr_write(lnaddr_t addr, size_t len, uint32_t data) {
	hwaddr_write(addr, len, data);
}

lnaddr_t seg_translate(swaddr_t addr, uint8_t sreg)
{
	if (cpu.cr0.protect_enable == 1) {
		if (!seg(sreg).cache) {
			lnaddr_t dis_addr = (lnaddr_t)cpu.gdtr.base_addr + 8 * seg(sreg).index;
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

