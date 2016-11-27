#include "cpu/exec/helper.h"
#include "../../lib-common/x86-inc/mmu.h"

#define DATA_BYTE 1
#include "jmp-template.h"
#undef DATA_BYTE

#define DATA_BYTE 2
#include "jmp-template.h"
#undef DATA_BYTE

#define DATA_BYTE 4
#include "jmp-template.h"
#undef DATA_BYTE

/* for instruction encoding overloading */

make_helper_v(jmp_i)
make_helper_v(jmp_rm)

make_helper(ljmp)
{
	// change the CS segment selector
	uint16_t a = instr_fetch(cpu.eip + 5, 2);
	// caution: you must fetch before switch to new code segment !
	swaddr_t new_eip = instr_fetch(cpu.eip + 1, 4);
	seg(CS).selector = a;

	// explicitly reload the CS segment selector cache
	lnaddr_t dis_addr = (lnaddr_t)cpu.gdtr.base_addr + 8 * seg(CS).index;
	//translation
	SegDesc descriptor;
	uint64_t *tmp = (uint64_t *)&descriptor;
	*tmp = lnaddr_read(dis_addr, 8);
	seg(CS).base = descriptor.base_15_0 + (descriptor.base_23_16 << 16) + (descriptor.base_31_24 << 24);
	seg(CS).limit = descriptor.limit_15_0 + (descriptor.limit_19_16 << 16);
	seg(CS).cache = true;
	cpu.eip = new_eip - 7;
	return 7;
}