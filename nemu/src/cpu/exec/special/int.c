#include "cpu/exec/helper.h"

extern void raise_intr(uint8_t NO, uint32_t eip);

make_helper(int_imm8)
{
	uint8_t NO = instr_fetch(eip + 1, 1);
	print_asm("int %x", NO);
	raise_intr(NO, eip + 2);
	return 0;
}