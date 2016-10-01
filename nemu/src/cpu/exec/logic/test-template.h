#include "cpu/exec/template-start.h"

#define instr test

static void do_execute() {
	DATA_TYPE result = op_dest->val & op_src->val;
	int len = (DATA_BYTE << 3) - 1;
	cpu.CF = 0;
	cpu.OF = 0;
	cpu.SF = result >> len;
	cpu.ZF = !result;
	//Parity Flag ── Set if low-order eight bits of result contain an even number of 1 bits; cleared otherwise
	int count = 0;
	int i = 1;
	for (i = 1; i <= 8; i++)
	{
		count += result % 2;
		result /= 2;
	}
	if (count % 2 == 0)
		cpu.PF = 1;
	else cpu.PF = 0;
	print_asm_template2();
}

make_instr_helper(i2a)
make_instr_helper(i2rm)
make_instr_helper(r2rm)

#include "cpu/exec/template-end.h"
