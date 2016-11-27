#include "cpu/exec/template-start.h"

#define instr movs
make_helper(concat(movs_, SUFFIX))
{
	DATA_TYPE result = MEM_R(reg_l(R_ESI), DS);
	MEM_W(reg_l(R_EDI), result, ES);
	if (cpu.DF)
	{
		reg_l(R_ESI) -= DATA_BYTE;
		reg_l(R_EDI) -= DATA_BYTE;
	}
	else
	{
		reg_l(R_ESI) += DATA_BYTE;
		reg_l(R_EDI) += DATA_BYTE;
	}
	print_asm("movs");
	return 1;
}

#include "cpu/exec/template-end.h"