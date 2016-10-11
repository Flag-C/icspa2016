#include "cpu/exec/helper.h"

#define DATA_BYTE 1
#include "cmovo-template.h"
#undef DATA_BYTE

#define DATA_BYTE 2
#include "cmovo-template.h"
#undef DATA_BYTE

#define DATA_BYTE 4
#include "cmovo-template.h"
#undef DATA_BYTE

/* for instruction encoding overloading */

make_helper_v(cmovo_r2rm)
make_helper_v(cmovo_rm2r)