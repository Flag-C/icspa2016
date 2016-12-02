#include "cpu/exec/helper.h"

make_helper(hlt)
{
	if (cpu.IF == 1)
		return 1;
	else return 0;
}