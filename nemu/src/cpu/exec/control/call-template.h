#include "cpu/exec/template-start.h"

#define instr call

static void do_execute() {
	DATA_TYPE_S displacement = op_src->val;
	print_asm("call %x", cpu.eip + 1 + len + displacement);
	cpu.eip += displacement;
}

make_instr_helper(i)

#include "cpu/exec/template-end.h"