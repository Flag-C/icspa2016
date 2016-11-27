#include "cpu/exec/template-start.h"

#define instr stos
make_helper(concat(stos_, SUFFIX))
{
	MEM_W(reg_l(R_EDI), (DATA_TYPE)REG(R_EAX), ES);
	if (cpu.DF)
	{
		reg_l(R_EDI) -= DATA_BYTE;
	}
	else
	{
		reg_l(R_EDI) += DATA_BYTE;
	}
	print_asm("stos");
	return 1;
}

#include "cpu/exec/template-end.h"