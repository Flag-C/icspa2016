#include "cpu/exec/template-start.h"

#define instr dec

static void do_execute () {
	DATA_TYPE result = op_src->val - 1;
	OPERAND_W(op_src, result);

	int len = (DATA_BYTE << 3) - 1;
	int sign;
	cpu.CF = (op_src->val < 1);
	cpu.SF = result >> len;
	sign = op_src->val >> len;
	cpu.OF = (sign != 0 && cpu.SF == 0) ;
	cpu.ZF = !result;
	OPERAND_W(op_src, result);
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

//panic("please implement me");

	print_asm_template1();
}

make_instr_helper(rm)
#if DATA_BYTE == 2 || DATA_BYTE == 4
make_instr_helper(r)
#endif

#include "cpu/exec/template-end.h"
