#include "cpu/exec/template-start.h"

#define instr cwd

make_helper(concat(cwd_, SUFFIX)) {
	if ((int)REG(R_EAX) < 0)
	{
#if (DATA_BYTE == 2)
		REG(R_EDX) = 0xffff;
#else
		REG(R_EDX) = 0xffffffff;
#endif
	}
	else REG(R_EDX) = 0;
	return 1;
}

#include "cpu/exec/template-end.h"
