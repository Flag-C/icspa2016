#include "cpu/exec/helper.h"
make_helper(lgdt) {
	int len = decode_rm_l(eip + 1) + 1;
	lnaddr_t address = op_src->addr;

	cpu.gdtr.seg_limit = lnaddr_read(address, 2);
	cpu.gdtr.base_addr = instr_fetch(address + 2, 4);
	return len;
}