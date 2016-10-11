#include "cpu/exec/helper.h"

#define DATA_BYTE 1
#include "cmovnp-template.h"
#undef DATA_BYTE

#define DATA_BYTE 2
#include "cmovnp-template.h"
#undef DATA_BYTE

#define DATA_BYTE 4
#include "cmovnp-template.h"
#undef DATA_BYTE

/* for instruction encoding overloading */

make_helper_v(cmovnp_r2rm)
make_helper_v(cmovnp_rm2r)