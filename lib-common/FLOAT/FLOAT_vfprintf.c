#include <stdio.h>
#include <stdint.h>
#include "FLOAT.h"
#include <sys/mman.h>

extern int _vfprintf_internal;
extern int _fpmaxtostr;
extern char _ppfs_setargs;
extern int __stdio_fwrite(char *buf, int len, FILE *stream);

__attribute__((used)) static int format_FLOAT(FILE *stream, FLOAT f) {
	/* TODO: Format a FLOAT argument `f' and write the formating
	 * result to `stream'. Keep the precision of the formating
	 * result with 6 by truncating. For example:
	 *              f          result
	 *         0x00010000    "1.000000"
	 *         0x00013333    "1.199996"
	 */

	char buf[80];
	int sign = 1;
	uint32_t integer, frac;
	if (f < 0)
	{
		sign = -1;
		f = -f;
	}
	integer = f >> 16;
	frac = 0x0000ffff & f;
	int i, m = 0;
	for (i = 1; i <= 6; i++)
	{
		m = m * 10 + ((frac * 0xa) >> 16);
		frac = (frac * 0xa) & 0x0000ffff;
	}
	int len;
	if (sign == -1)
	{
		len = sprintf(buf, "-%d.%06d", integer, m);
	}
	else len = sprintf(buf, "%d.%06d", integer, m);
	return __stdio_fwrite(buf, len, stream);
}

static void modify_vfprintf() {
	/* TODO: Implement this function to hijack the formating of "%f"
	 * argument during the execution of `_vfprintf_internal'. Below
	 * is the code section in _vfprintf_internal() relative to the
	 * hijack.
	 */
	int* calladdr = (int*)((void*)&_vfprintf_internal + 0x08048861 - 0x0804855b + 1);
	//printf("calladdr %x:%x\n", calladdr, *calladdr);
	//mprotect((void *)((int)((int*)calladdr - 100) & 0xfffff000), 4096 * 2, PROT_READ | PROT_WRITE | PROT_EXEC);
	//int offset = (int)((int)&format_FLOAT - (int)&_fpmaxtostr);
	//printf("%d\n", offset);
	*calladdr += (int)((int)&format_FLOAT - (int)&_fpmaxtostr);
	//printf("format_FLOAT %x\n", &format_FLOAT);
	//printf("_fpmaxtostr %x\n", &_fpmaxtostr);
	//printf("calladdr %x:%x\n", calladdr, *calladdr);
	char* floataddr = (void*)calladdr - (0x8048e9b - 0x8048e8f);
	*floataddr = 0x08;
	floataddr++;
	*floataddr = 0xff;
	floataddr++;
	*floataddr = 0x32;
	floataddr++;
	*floataddr = 0x90;
	floataddr = floataddr - (0x8048e30 - 0x8048e16);
	int i;
	for (i = 1; i <= 6; i++)
	{
		*floataddr = 0x90;
		floataddr++;
	}
#if 0
	else if (ppfs->conv_num <= CONV_A) {  /* floating point */
		ssize_t nf;
		nf = _fpmaxtostr(stream,
		                 (__fpmax_t)
		                 (PRINT_INFO_FLAG_VAL(&(ppfs->info), is_long_double)
		                  ? * (long double *) *argptr
		                  : (long double) (* (double *) *argptr)),
		                 &ppfs->info, FP_OUT );
		if (nf < 0) {
			return -1;
		}
		*count += nf;

		return 0;
	} else if (ppfs->conv_num <= CONV_S) {  /* wide char or string */
#endif

		/* You should modify the run-time binary to let the code above
		 * call `format_FLOAT' defined in this source file, instead of
		 * `_fpmaxtostr'. When this function returns, the action of the
		 * code above should do the following:
		 */

#if 0
		else if (ppfs->conv_num <= CONV_A) {  /* floating point */
			ssize_t nf;
			nf = format_FLOAT(stream, *(FLOAT *) *argptr);
			if (nf < 0) {
				return -1;
			}
			*count += nf;

			return 0;
		} else if (ppfs->conv_num <= CONV_S) {  /* wide char or string */
#endif

		}

		static void modify_ppfs_setargs() {
			char* add = &_ppfs_setargs + 0x8010ef - 0x801085;
			//mprotect((void *)((int)((int*)add) & 0xfffff000), 4096 * 2, PROT_READ | PROT_WRITE | PROT_EXEC);
			*add = 0xeb;
			//*(add + 1) = (char)(0x801167 - &_ppfs_setargs);
			/* TODO: Implement this function to modify the action of preparing
			 * "%f" arguments for _vfprintf_internal() in _ppfs_setargs().
			 * Below is the code section in _vfprintf_internal() relative to
			 * the modification.
			 */

#if 0
			enum {                          /* C type: */
				PA_INT,                       /* int */
				PA_CHAR,                      /* int, cast to char */
				PA_WCHAR,                     /* wide char */
				PA_STRING,                    /* const char *, a '\0'-terminated string */
				PA_WSTRING,                   /* const wchar_t *, wide character string */
				PA_POINTER,                   /* void * */
				PA_FLOAT,                     /* float */
				PA_DOUBLE,                    /* double */
				__PA_NOARG,                   /* non-glibc -- signals non-arg width or prec */
				PA_LAST
			};

			/* Flag bits that can be set in a type returned by `parse_printf_format'.  */
			/* WARNING -- These differ in value from what glibc uses. */
#define PA_FLAG_MASK		(0xff00)
#define __PA_FLAG_CHAR		(0x0100) /* non-gnu -- to deal with hh */
#define PA_FLAG_SHORT		(0x0200)
#define PA_FLAG_LONG		(0x0400)
#define PA_FLAG_LONG_LONG	(0x0800)
#define PA_FLAG_LONG_DOUBLE	PA_FLAG_LONG_LONG
#define PA_FLAG_PTR		(0x1000) /* TODO -- make dynamic??? */

			while (i < ppfs->num_data_args) {
				switch (ppfs->argtype[i++]) {
				case (PA_INT|PA_FLAG_LONG_LONG):
					GET_VA_ARG(p, ull, unsigned long long, ppfs->arg);
					break;
				case (PA_INT|PA_FLAG_LONG):
					GET_VA_ARG(p, ul, unsigned long, ppfs->arg);
					break;
				case PA_CHAR:	/* TODO - be careful */
				/* ... users could use above and really want below!! */
				case (PA_INT|__PA_FLAG_CHAR):/* TODO -- translate this!!! */
				case (PA_INT|PA_FLAG_SHORT):
				case PA_INT:
					GET_VA_ARG(p, u, unsigned int, ppfs->arg);
					break;
				case PA_WCHAR:	/* TODO -- assume int? */
					/* we're assuming wchar_t is at least an int */
					GET_VA_ARG(p, wc, wchar_t, ppfs->arg);
					break;
				/* PA_FLOAT */
				case PA_DOUBLE:
					GET_VA_ARG(p, d, double, ppfs->arg);
					break;
				case (PA_DOUBLE|PA_FLAG_LONG_DOUBLE):
					GET_VA_ARG(p, ld, long double, ppfs->arg);
					break;
				default:
					/* TODO -- really need to ensure this can't happen */
					assert(ppfs->argtype[i - 1] & PA_FLAG_PTR);
				case PA_POINTER:
				case PA_STRING:
				case PA_WSTRING:
					GET_VA_ARG(p, p, void *, ppfs->arg);
					break;
				case __PA_NOARG:
					continue;
				}
				++p;
			}
#endif

			/* You should modify the run-time binary to let the `PA_DOUBLE'
			 * branch execute the code in the `(PA_INT|PA_FLAG_LONG_LONG)'
			 * branch. Comparing to the original `PA_DOUBLE' branch, the
			 * target branch will also prepare a 64-bit argument, without
			 * introducing floating point instructions. When this function
			 * returns, the action of the code above should do the following:
			 */

#if 0
			while (i < ppfs->num_data_args) {
				switch (ppfs->argtype[i++]) {
				case (PA_INT|PA_FLAG_LONG_LONG):
here:
					GET_VA_ARG(p, ull, unsigned long long, ppfs->arg);
					break;
				// ......
				/* PA_FLOAT */
				case PA_DOUBLE:
					goto here;
					GET_VA_ARG(p, d, double, ppfs->arg);
					break;
					// ......
				}
				++p;
			}
#endif

		}

		void init_FLOAT_vfprintf() {
			modify_vfprintf();
			modify_ppfs_setargs();
		}
