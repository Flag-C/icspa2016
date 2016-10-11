#include "cpu/exec/helper.h"

#define DATA_BYTE 1
#include "cmovno-template.h"
#undef DATA_BYTE

#define DATA_BYTE 2
#include "cmovno-template.h"
#undef DATA_BYTE

#define DATA_BYTE 4
#include "cmovno-template.h"
#undef DATA_BYTE

/* for instruction encoding overloading */

make_helper_v(cmovno_r2rm)
make_helper_v(cmovno_rm2r)