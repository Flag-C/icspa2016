#include "cpu/exec/template-start.h"

#define instr lods
make_helper(concat(lods_, SUFFIX))
{
	DATA_TYPE src = MEM_R(REG(R_ESI));
	REG(R_EAX) = src;
	if (cpu.DF)
	{
		REG(R_ESI) -= DATA_BYTE;
	}
	else
	{
		REG(R_ESI) += DATA_BYTE;
	}
	print_asm("lods");
	return 1;
}

#include "cpu/exec/template-end.h"