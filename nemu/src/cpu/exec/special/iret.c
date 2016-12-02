#include "cpu/exec/helper.h"
#include <setjmp.h>
extern jmp_buf jbuf;

make_helper(iret)
{
	cpu.eip = swaddr_read(reg_l(R_ESP), 4, SS);
	reg_l(R_ESP) += 4;
	seg(CS).selector = (uint16_t)swaddr_read(reg_l(R_ESP), 4, SS);
	seg(CS).cache = false;
	reg_l(R_ESP) += 4;
	cpu.eflags = swaddr_read(reg_l(R_ESP), 4, SS);
	reg_l(R_ESP) += 4;
	print_asm("iret");
	longjmp(jbuf, 1);
	return 0;
}