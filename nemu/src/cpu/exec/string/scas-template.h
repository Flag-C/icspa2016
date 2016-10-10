#include "cpu/exec/template-start.h"

#define instr scas
make_helper(concat(lods_, SUFFIX))
{
	DATA_TYPE src = MEM_R(reg_l(R_EDI));
	DATA_TYPE dest = REG(R_EAX);

	DATA_TYPE result = dest - src;
	int len = (DATA_BYTE << 3) - 1;
	cpu.CF = dest < src;
	cpu.SF = result >> len;
	int s1, s2;
	s1 = dest >> len;
	s2 = src >> len;
	cpu.OF = (s1 != s2 && s2 == cpu.SF) ;
	cpu.ZF = !result;
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

	if (cpu.DF)
	{
		reg_l(R_EDI) -= DATA_BYTE;
	}
	else
	{
		reg_l(R_EDI) += DATA_BYTE;
	}
	print_asm("scas");
	return 1;
}

#include "cpu/exec/template-end.h"