#include "cpu/exec/template-start.h"

#define instr jecxz


static void do_execute() {
	DATA_TYPE_S displacement = op_src->val;
	print_asm("jecxz %x", cpu.eip + 1 + DATA_BYTE + displacement);
	if (reg_l(R_ECX) == 0)cpu.eip += displacement;
}
make_instr_helper(i)


#include "cpu/exec/template-end.h"