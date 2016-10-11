#include "cpu/exec/helper.h"

#define DATA_BYTE 1
#include "cmovnz-template.h"
#undef DATA_BYTE

#define DATA_BYTE 2
#include "cmovnz-template.h"
#undef DATA_BYTE

#define DATA_BYTE 4
#include "cmovnz-template.h"
#undef DATA_BYTE

/* for instruction encoding overloading */

make_helper_v(cmovnz_r2rm)
make_helper_v(cmovnz_rm2r)