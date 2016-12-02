#include "cpu/exec/helper.h"

void raise_intr(uint8_t NO);

make_helper(int_imm8)
{
	uint8_t NO = instr_fetch(eip + 1, 1);
	print_asm("int %x", NO);
	reg_l(R_ESP) -= 4;
	swaddr_write(reg_l(R_ESP), 4, (uint32_t)cpu.eflags, SS);
	reg_l(R_ESP) -= 4;
	swaddr_write(reg_l(R_ESP), 4, (uint32_t)seg(CS).selector, SS);
	reg_l(R_ESP) -= 4;
	swaddr_write(reg_l(R_ESP), 4, (uint32_t)(eip + 2), SS);
	raise_intr(NO);
	return 0;
}