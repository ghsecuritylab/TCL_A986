
/* @(#)w_remainder.c 1.3 95/01/18 */


#include "fdlibm.h"

#ifdef __STDC__
	double ieee_remainder(double x, double y)	/* wrapper remainder */
#else
	double ieee_remainder(x,y)			/* wrapper remainder */
	double x,y;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_remainder(x,y);
#else
	double z;
	z = __ieee754_remainder(x,y);
	if(_LIB_VERSION == _IEEE_ || ieee_isnan(y)) return z;
	if(y==0.0) 
	    return __kernel_standard(x,y,28); /* ieee_remainder(x,0) */
	else
	    return z;
#endif
}
