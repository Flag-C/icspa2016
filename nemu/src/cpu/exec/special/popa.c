#include "cpu/exec/helper.h"
make_helper(popa)
{
	reg_l(R_EDI) = swaddr_read(reg_l(R_ESP), 4, SS);
	reg_l(R_ESP) += 4;
	reg_l(R_ESI) = swaddr_read(reg_l(R_ESP), 4, SS);
	reg_l(R_ESP) += 4;
	reg_l(R_EBP) = swaddr_read(reg_l(R_ESP), 4, SS);
	reg_l(R_ESP) += 4;
	reg_l(R_ESP) += 4;
	reg_l(R_EBX) = swaddr_read(reg_l(R_ESP), 4, SS);
	reg_l(R_ESP) += 4;
	reg_l(R_EDX) = swaddr_read(reg_l(R_ESP), 4, SS);
	reg_l(R_ESP) += 4;
	reg_l(R_ECX) = swaddr_read(reg_l(R_ESP), 4, SS);
	reg_l(R_ESP) += 4;
	reg_l(R_EAX) = swaddr_read(reg_l(R_ESP), 4, SS);
	reg_l(R_ESP) += 4;
	print_asm("popa");
	return 1;
}