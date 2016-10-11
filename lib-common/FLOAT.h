#ifndef __FLOAT_H__
#define __FLOAT_H__

#include "trap.h"

typedef int FLOAT;

static inline int F2int(FLOAT a) {
	int s = 1;
	if (a < 0)
	{
		s = -1;
		a = -a;
	}
	a = a >> 16;
	return a * s;
}

static inline FLOAT int2F(int a) {
	int s = 1;
	if (a < 0)
	{
		s = -1;
		a = -a;
	}
	a = a << 16;
	return a * s;
}

static inline FLOAT F_mul_int(FLOAT a, int b) {
	return a * b;
}

static inline FLOAT F_div_int(FLOAT a, int b) {
	return a / b;
}

FLOAT f2F(float a)
{
	int bit = *(int *)&a;
	int sign = bit >> 31;
	int exp = (bit >> 23) & 0xff;
	FLOAT ans = bit & 0x7fffff;
	if (exp != 0) ans += 1 << 23;
	exp -= 150;
	if (exp < -16) ans >>= -16 - exp;
	if (exp > -16) ans <<= exp + 16;
	return sign == 0 ? ans : -ans;
}

FLOAT F_mul_F(FLOAT a, FLOAT b)
{
	long long ans = ((long long)a * (long long)b) >> 16;
	return (FLOAT)ans;
}

//use subtraction to emulate div
FLOAT F_div_F(FLOAT a, FLOAT b)
{
	int s = 1;
	if (a < 0)
	{
		s = -s;
		a = -a;
	}
	if (b < 0)
	{
		s = -s;
		b = -b;
	}
	int res = a / b;
	a = a % b;
	int i;
	for (i = 0; i < 16; i++)
	{
		a *= 2;
		res *= 2;
		if (a >= b)
		{
			a -= b;
			res++;
		}
	}
	return res * s;
}

FLOAT Fabs(FLOAT a)
{
	return a < 0 ? -a : a;
}

FLOAT sqrt(FLOAT n)
{
	FLOAT k = int2F(1);
	while (Fabs(F_mul_F(k, k) - n) > f2F(1e-9)) {
		k = F_div_int(k + F_div_F(n, k), 2);
	}
	return k;
}
FLOAT pow(FLOAT, FLOAT);

// used when calling printf/sprintf to format a FLOAT argument
#define FLOAT_ARG(f) (long long)f

void init_FLOAT_vfprintf(void);

#endif
