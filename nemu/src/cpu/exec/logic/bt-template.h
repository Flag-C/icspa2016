#include "cpu/exec/template-start.h"

#define instr bt

static inline void do_execute () {

	uint8_t val = op_src->val;
#if DATA_BYTE == 2
	val &= 15;
#elif DATA_BYTE == 4
	val &= 31;
#endif
	int result = op_dest->val & (1 << val);
	cpu.CF = (!!result);

	print_asm_template2();
}

make_instr_helper(rm_imm)
make_instr_helper(r2rm)

#include "cpu/exec/template-end.h"