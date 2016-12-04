#include "cpu/exec/helper.h"

make_helper(cbw_w)
{
	reg_w(R_AX) = (int16_t)reg_b(R_AL);
	print_asm("cbw");
	return 1;
}

make_helper(cbw_l)
{
	reg_l(R_EAX) = (int32_t)reg_l(R_AX);
	print_asm("cwde");
	return 1;
}

make_helper_v(cbw)