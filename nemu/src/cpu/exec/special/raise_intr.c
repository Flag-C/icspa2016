#include <setjmp.h>
#include "cpu/exec/helper.h"
#include "../../lib-common/x86-inc/mmu.h"
extern jmp_buf jbuf;

void raise_intr(uint8_t NO, uint32_t eip) {
	reg_l(R_ESP) -= 4;
	swaddr_write(reg_l(R_ESP), 4, (uint32_t)cpu.eflags, SS);
	reg_l(R_ESP) -= 4;
	swaddr_write(reg_l(R_ESP), 4, (uint32_t)seg(CS).selector, SS);
	reg_l(R_ESP) -= 4;
	swaddr_write(reg_l(R_ESP), 4, (uint32_t)(eip), SS);
	lnaddr_t dis_addr = (lnaddr_t)cpu.idtr.base_addr + (NO << 3);
	GateDesc descriptor;
	uint32_t *tmp = (uint32_t *)&descriptor;
	*tmp = lnaddr_read( dis_addr, 4);
	tmp++;
	*tmp = lnaddr_read( dis_addr + 4, 4);
	uint32_t offset = descriptor.offset_15_0 + (descriptor.offset_31_16 << 16);
	seg(CS).selector = (uint16_t)(descriptor.segment);
	seg(CS).cache = false;
	cpu.eip = offset;
	longjmp(jbuf, 1);
}