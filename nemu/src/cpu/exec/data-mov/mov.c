#include "cpu/exec/helper.h"

#define DATA_BYTE 1
#include "mov-template.h"
#undef DATA_BYTE

#define DATA_BYTE 2
#include "mov-template.h"
#undef DATA_BYTE

#define DATA_BYTE 4
#include "mov-template.h"
#undef DATA_BYTE

/* for instruction encoding overloading */

make_helper_v(mov_i2r)
make_helper_v(mov_i2rm)
make_helper_v(mov_r2rm)
make_helper_v(mov_rm2r)
make_helper_v(mov_a2moffs)
make_helper_v(mov_moffs2a)

make_helper(mov_crx2r)
{
	uint8_t registers = instr_fetch(eip + 1, 1);
	int dest = registers & 0x7;
	int src = (registers >> 3) & 0x7;
	if (src == 0)
		reg_l(dest) = cpu.cr0.val;
	else Assert(1, "Not accomplish such cr reg");
	return 2;
}

make_helper(mov_r2crx)
{
	uint8_t registers = instr_fetch(eip + 1, 1);
	int src = registers & 0x7;
	int dest = (registers >> 3) & 0x7;
	if (dest == 0)
		cpu.cr0.val = reg_l(src);
	else Assert(1, "Not accomplish such cr reg");
	return 2;
}

make_helper(mov_rm2sreg)
{
	int len = decode_rm_w(eip + 1);
	uint8_t sreg = instr_fetch(eip + 1, 1);
	sreg = (sreg >> 3) & 0x7;
	seg(sreg).selector = op_src->val;
	return len + 1;
}
