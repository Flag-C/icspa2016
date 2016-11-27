#include "cpu/exec/helper.h"
#include "cpu/decode/modrm.h"

make_helper(leave) {
	cpu.esp = cpu.ebp;
	cpu.ebp = swaddr_read(cpu.esp, 4, SS);
	cpu.esp += 4;
	print_asm("leave");
	return 1;
}