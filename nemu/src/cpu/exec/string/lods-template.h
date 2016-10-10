#include "cpu/exec/template-start.h"

#define instr lods
make_helper(concat(lods_, SUFFIX))
{
	DATA_TYPE result = REG(R_EAX);
	REG(R_EAX) = REG(R_ESI);
	if (cpu.DF)
	{
		reg_l(R_ESI) -= DATA_BYTE;
	}
	else
	{
		reg_l(R_ESI) += DATA_BYTE;
	}
	print_asm("lods");
	return 1;
}

#include "cpu/exec/template-end.h"