#include "common.h"

//#define CACHED
#define DEBUGGING

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

uint32_t swaddr_read(swaddr_t addr, size_t len) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	return lnaddr_read(addr, len);
}

void swaddr_write(swaddr_t addr, size_t len, uint32_t data) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	lnaddr_write(addr, len, data);
}

