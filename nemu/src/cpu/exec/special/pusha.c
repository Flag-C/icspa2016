#include "cpu/exec/helper.h"
make_helper(pusha)
{
	uint32_t temp = reg_l(R_ESP);
	reg_l(R_ESP) -= 4;
	swaddr_write(reg_l(R_ESP), 4, reg_l(R_EAX), SS);
	reg_l(R_ESP) -= 4;
	swaddr_write(reg_l(R_ESP), 4, reg_l(R_ECX), SS);
	reg_l(R_ESP) -= 4;
	swaddr_write(reg_l(R_ESP), 4, reg_l(R_EDX), SS);
	reg_l(R_ESP) -= 4;
	swaddr_write(reg_l(R_ESP), 4, reg_l(R_EBX), SS);
	reg_l(R_ESP) -= 4;
	swaddr_write(reg_l(R_ESP), 4, temp, SS);
	reg_l(R_ESP) -= 4;
	swaddr_write(reg_l(R_ESP), 4, reg_l(R_EBP), SS);
	reg_l(R_ESP) -= 4;
	swaddr_write(reg_l(R_ESP), 4, reg_l(R_ESI), SS);
	reg_l(R_ESP) -= 4;
	swaddr_write(reg_l(R_ESP), 4, reg_l(R_EDI), SS);
	print_asm("pusha");
	return 1;
}