#include "cpu/exec/template-start.h"

#define instr cwd

make_helper(concat(cwd_, SUFFIX)) {
	if ((REG(R_EAX) >> 15) & 1)
	{
		REG(R_EAX) |= 0xffff0000;
	}
	else
	{
		REG(R_EAX) &= 0xffff;
	}
	print_asm("cwd");
	return 1;
}

#include "cpu/exec/template-end.h"
