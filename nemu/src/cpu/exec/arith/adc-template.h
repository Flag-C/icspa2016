#include "cpu/exec/template-start.h"

#define instr adc

static void do_execute() {
	DATA_TYPE result = op_dest->val + op_src->val + cpu.CF;
	int len = (DATA_BYTE << 3) - 1;
	int s1, s2;
	cpu.CF = (result < op_dest->val);
	cpu.SF = result >> len;
	s1 = op_dest->val >> len;
	s2 = op_src->val >> len;
	cpu.OF = (s1 == s2 && s1 != cpu.SF) ;
	cpu.ZF = !result;
	OPERAND_W(op_dest, result);
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
#if DATA_BYTE == 2 || DATA_BYTE == 4
make_instr_helper(si2rm)
#endif

make_instr_helper(i2a)
make_instr_helper(i2rm)
make_instr_helper(r2rm)
make_instr_helper(rm2r)

#include "cpu/exec/template-end.h"