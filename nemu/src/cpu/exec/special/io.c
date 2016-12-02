#include "cpu/exec/helper.h"
#include "device/port-io.h"

make_helper(inb) {
	print_asm("in %%al,(%%dx)");
	reg_b(R_EAX) = (uint8_t) pio_read(reg_w(R_EDX), 1);
	return 1;
}

make_helper(inl) {
	print_asm("in %%eax,(%%dx)");
	reg_l(R_EAX) = pio_read(reg_w(R_EDX), 4);
	return 1;
}

make_helper(outb) {
	print_asm("out %%al,(%%dx)");
	pio_write(reg_w(R_EDX), 1, (uint32_t)reg_b(R_EAX));
	return 1;
}

make_helper(outl) {
	print_asm("out %%eax,(%%dx)");
	pio_write(reg_w(R_EDX), 4, reg_l(R_EAX));
	return 1;
}