#include "cpu/exec/template-start.h"

#define instr cwd

make_helper(concat(cwd_, SUFFIX)) {
	DATA_TYPE temp = REG(R_EAX);
	int len = (DATA_BYTE << 3) - 1;
	REG(R_EDX) = temp >> len;
	print_asm("cwd");
	return 1;
}

#include "cpu/exec/template-end.h"
