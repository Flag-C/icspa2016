#include "FLOAT.h"

FLOAT F_mul_F(FLOAT a, FLOAT b) {

	long long ans = ((long long)a * (long long)b) >> 16;
	return (FLOAT)ans;
}

//use "shushi" to emulate the division
FLOAT F_div_F(FLOAT a, FLOAT b) {
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
	for (i = 1; i <= 16; i++)
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

FLOAT f2F(float a) {

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

FLOAT Fabs(FLOAT a) {
	return a < 0 ? -a : a;
}

/* Functions below are already implemented */

FLOAT sqrt(FLOAT x) {
	FLOAT dt, t = int2F(2);

	do {
		dt = F_div_int((F_div_F(x, t) - t), 2);
		t += dt;
	} while (Fabs(dt) > f2F(1e-4));

	return t;
}

FLOAT pow(FLOAT x, FLOAT y) {
	/* we only compute x^0.333 */
	FLOAT t2, dt, t = int2F(2);

	do {
		t2 = F_mul_F(t, t);
		dt = (F_div_F(x, t2) - t) / 3;
		t += dt;
	} while (Fabs(dt) > f2F(1e-4));

	return t;
}

