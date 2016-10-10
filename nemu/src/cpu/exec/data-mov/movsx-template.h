#include "cpu/exec/template-start.h"

#define instr movsx

#if DATA_BYTE==2 || DATA_BYTE==4
make_helper(concat(movsxb_, SUFFIX))
{
	int len = decode_rm2r_b(eip + 1);
	OPERAND_W(op_dest, (int8_t)op_src->val);
	print_asm_template2();
	return len + 1;
}
#endif

#if DATA_BYTE==4
make_helper(movsxw_l)
{
	int len = decode_rm2r_w(eip + 1);
	OPERAND_W(op_dest, (int16_t)op_src->val);
	print_asm_template2();
	return len + 1;
}
#endif
#include "cpu/exec/template-end.h"