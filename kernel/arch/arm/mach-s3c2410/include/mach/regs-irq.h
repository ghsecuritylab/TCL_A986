


#ifndef ___ASM_ARCH_REGS_IRQ_H
#define ___ASM_ARCH_REGS_IRQ_H

/* interrupt controller */

#define S3C2410_IRQREG(x)   ((x) + S3C24XX_VA_IRQ)
#define S3C2410_EINTREG(x)  ((x) + S3C24XX_VA_GPIO)
#define S3C24XX_EINTREG(x)  ((x) + S3C24XX_VA_GPIO2)

#define S3C2410_SRCPND	       S3C2410_IRQREG(0x000)
#define S3C2410_INTMOD	       S3C2410_IRQREG(0x004)
#define S3C2410_INTMSK	       S3C2410_IRQREG(0x008)
#define S3C2410_PRIORITY       S3C2410_IRQREG(0x00C)
#define S3C2410_INTPND	       S3C2410_IRQREG(0x010)
#define S3C2410_INTOFFSET      S3C2410_IRQREG(0x014)
#define S3C2410_SUBSRCPND      S3C2410_IRQREG(0x018)
#define S3C2410_INTSUBMSK      S3C2410_IRQREG(0x01C)

#define S3C2416_PRIORITY_MODE1		S3C2410_IRQREG(0x030)
#define S3C2416_PRIORITY_UPDATE1	S3C2410_IRQREG(0x034)
#define S3C2416_SRCPND2			S3C2410_IRQREG(0x040)
#define S3C2416_INTMOD2			S3C2410_IRQREG(0x044)
#define S3C2416_INTMSK2			S3C2410_IRQREG(0x048)
#define S3C2416_INTPND2			S3C2410_IRQREG(0x050)
#define S3C2416_INTOFFSET2		S3C2410_IRQREG(0x054)
#define S3C2416_PRIORITY_MODE2		S3C2410_IRQREG(0x070)
#define S3C2416_PRIORITY_UPDATE2	S3C2410_IRQREG(0x074)


#define S3C2410_EINTMASK       S3C2410_EINTREG(0x0A4)
#define S3C2410_EINTPEND       S3C2410_EINTREG(0X0A8)
#define S3C2412_EINTMASK       S3C2410_EINTREG(0x0B4)
#define S3C2412_EINTPEND       S3C2410_EINTREG(0X0B8)

#define S3C24XX_EINTMASK       S3C24XX_EINTREG(0x0A4)
#define S3C24XX_EINTPEND       S3C24XX_EINTREG(0X0A8)

#endif /* ___ASM_ARCH_REGS_IRQ_H */
