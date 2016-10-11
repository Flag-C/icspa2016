#include "cpu/exec/helper.h"

#define DATA_BYTE 1
#include "cmovb-template.h"
#undef DATA_BYTE

#define DATA_BYTE 2
#include "cmovb-template.h"
#undef DATA_BYTE

#define DATA_BYTE 4
#include "cmovb-template.h"
#undef DATA_BYTE

/* for instruction encoding overloading */

make_helper_v(cmovb_r2rm)
make_helper_v(cmovb_rm2r)