#include "cpu/exec/helper.h"

#define DATA_BYTE 1
#include "cmovp-template.h"
#undef DATA_BYTE

#define DATA_BYTE 2
#include "cmovp-template.h"
#undef DATA_BYTE

#define DATA_BYTE 4
#include "cmovp-template.h"
#undef DATA_BYTE

/* for instruction encoding overloading */

make_helper_v(cmovp_r2rm)
make_helper_v(cmovp_rm2r)