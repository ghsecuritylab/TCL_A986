
#ifndef __ASM_ARCH_MODULE_H
#define __ASM_ARCH_MODULE_H

#include <asm/mach-types.h>

#define module_is_cc7ucamry()	(0			\
		|| machine_is_cc7ucamry()		\
		)

#define module_is_cc9c()	(0			\
		|| machine_is_cc9c()			\
		)

#define module_is_cc9p9210()	(0			\
		|| machine_is_cc9p9210()		\
		|| machine_is_cc9p9210js()		\
		)

#define module_is_cc9p9215()	(0			\
		|| machine_is_cc9p9215()		\
		|| machine_is_cc9p9215js()		\
		)

#define module_is_cc9p9360()	(0			\
		|| machine_is_a9m9360()			\
		|| machine_is_cc9p9360dev()		\
		|| machine_is_cc9p9360js()		\
		|| machine_is_cc9p9360val()		\
		)

#define module_is_cc9p9750()	(0			\
		|| machine_is_a9m9750()			\
		|| machine_is_cc9p9750dev()		\
		|| machine_is_cc9p9750js()		\
		|| machine_is_cc9p9750val()		\
		)

#define module_is_ccw9c()	(0			\
		|| machine_is_ccw9c()			\
		)

#define module_is_inc20otter()	(0			\
		|| machine_is_inc20otter()		\
		)

#define module_is_otter()	(0			\
		|| machine_is_otter()			\
		)

#endif /* ifndef __ASM_ARCH_MODULE_H */
