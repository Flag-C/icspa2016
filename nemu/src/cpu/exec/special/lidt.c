#include "cpu/exec/helper.h"
make_helper(lidt)
{
	int len = decode_rm_l(eip + 1) + 1;
	lnaddr_t mem_addr = op_src->addr;

	cpu.idtr.seg_limit = lnaddr_read(mem_addr, 2);
	cpu.idtr.base_addr = instr_fetch(mem_addr + 2, 4);
	print_asm("lidt");
	return len;
}