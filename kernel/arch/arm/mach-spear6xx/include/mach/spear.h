

#ifndef __MACH_SPEAR6XX_H
#define __MACH_SPEAR6XX_H

#include <mach/hardware.h>
#include <mach/spear600.h>

#define SPEAR6XX_ML_SDRAM_BASE		0x00000000
#define SPEAR6XX_ML_SDRAM_SIZE		0x40000000

/* ICM1 - Low speed connection */
#define SPEAR6XX_ICM1_BASE		0xD0000000
#define SPEAR6XX_ICM1_SIZE		0x08000000

#define SPEAR6XX_ICM1_UART0_BASE	0xD0000000
#define VA_SPEAR6XX_ICM1_UART0_BASE	IO_ADDRESS(SPEAR6XX_ICM1_UART0_BASE)
#define SPEAR6XX_ICM1_UART0_SIZE	0x00080000

#define SPEAR6XX_ICM1_UART1_BASE	0xD0080000
#define SPEAR6XX_ICM1_UART1_SIZE	0x00080000

#define SPEAR6XX_ICM1_SSP0_BASE		0xD0100000
#define SPEAR6XX_ICM1_SSP0_SIZE		0x00080000

#define SPEAR6XX_ICM1_SSP1_BASE		0xD0180000
#define SPEAR6XX_ICM1_SSP1_SIZE		0x00080000

#define SPEAR6XX_ICM1_I2C_BASE		0xD0200000
#define SPEAR6XX_ICM1_I2C_SIZE		0x00080000

#define SPEAR6XX_ICM1_JPEG_BASE		0xD0800000
#define SPEAR6XX_ICM1_JPEG_SIZE		0x00800000

#define SPEAR6XX_ICM1_IRDA_BASE		0xD1000000
#define SPEAR6XX_ICM1_IRDA_SIZE		0x00800000

#define SPEAR6XX_ICM1_FSMC_BASE		0xD1800000
#define SPEAR6XX_ICM1_FSMC_SIZE		0x00800000

#define SPEAR6XX_ICM1_NAND_BASE		0xD2000000
#define SPEAR6XX_ICM1_NAND_SIZE		0x00800000

#define SPEAR6XX_ICM1_SRAM_BASE		0xD2800000
#define SPEAR6XX_ICM1_SRAM_SIZE		0x00800000

/* ICM2 - Application Subsystem */
#define SPEAR6XX_ICM2_BASE		0xD8000000
#define SPEAR6XX_ICM2_SIZE		0x08000000

#define SPEAR6XX_ICM2_TMR0_BASE		0xD8000000
#define SPEAR6XX_ICM2_TMR0_SIZE		0x00080000

#define SPEAR6XX_ICM2_TMR1_BASE		0xD8080000
#define SPEAR6XX_ICM2_TMR1_SIZE		0x00080000

#define SPEAR6XX_ICM2_GPIO_BASE		0xD8100000
#define SPEAR6XX_ICM2_GPIO_SIZE		0x00080000

#define SPEAR6XX_ICM2_SPI2_BASE		0xD8180000
#define SPEAR6XX_ICM2_SPI2_SIZE		0x00080000

#define SPEAR6XX_ICM2_ADC_BASE		0xD8200000
#define SPEAR6XX_ICM2_ADC_SIZE		0x00080000

/* ML-1, 2 - Multi Layer CPU Subsystem */
#define SPEAR6XX_ML_CPU_BASE		0xF0000000
#define SPEAR6XX_ML_CPU_SIZE		0x08000000

#define SPEAR6XX_CPU_TMR_BASE		0xF0000000
#define SPEAR6XX_CPU_TMR_SIZE		0x00100000

#define SPEAR6XX_CPU_GPIO_BASE		0xF0100000
#define SPEAR6XX_CPU_GPIO_SIZE		0x00100000

#define SPEAR6XX_CPU_VIC_SEC_BASE	0xF1000000
#define VA_SPEAR6XX_CPU_VIC_SEC_BASE	IO_ADDRESS(SPEAR6XX_CPU_VIC_SEC_BASE)
#define SPEAR6XX_CPU_VIC_SEC_SIZE	0x00100000

#define SPEAR6XX_CPU_VIC_PRI_BASE	0xF1100000
#define VA_SPEAR6XX_CPU_VIC_PRI_BASE	IO_ADDRESS(SPEAR6XX_CPU_VIC_PRI_BASE)
#define SPEAR6XX_CPU_VIC_PRI_SIZE	0x00100000

/* ICM3 - Basic Subsystem */
#define SPEAR6XX_ICM3_BASE		0xF8000000
#define SPEAR6XX_ICM3_SIZE		0x08000000

#define SPEAR6XX_ICM3_SMEM_BASE		0xF8000000
#define SPEAR6XX_ICM3_SMEM_SIZE		0x04000000

#define SPEAR6XX_ICM3_SMI_CTRL_BASE	0xFC000000
#define SPEAR6XX_ICM3_SMI_CTRL_SIZE	0x00200000

#define SPEAR6XX_ICM3_CLCD_BASE		0xFC200000
#define SPEAR6XX_ICM3_CLCD_SIZE		0x00200000

#define SPEAR6XX_ICM3_DMA_BASE		0xFC400000
#define SPEAR6XX_ICM3_DMA_SIZE		0x00200000

#define SPEAR6XX_ICM3_SDRAM_CTRL_BASE	0xFC600000
#define SPEAR6XX_ICM3_SDRAM_CTRL_SIZE	0x00200000

#define SPEAR6XX_ICM3_TMR_BASE		0xFC800000
#define SPEAR6XX_ICM3_TMR_SIZE		0x00080000

#define SPEAR6XX_ICM3_WDT_BASE		0xFC880000
#define SPEAR6XX_ICM3_WDT_SIZE		0x00080000

#define SPEAR6XX_ICM3_RTC_BASE		0xFC900000
#define SPEAR6XX_ICM3_RTC_SIZE		0x00080000

#define SPEAR6XX_ICM3_GPIO_BASE		0xFC980000
#define SPEAR6XX_ICM3_GPIO_SIZE		0x00080000

#define SPEAR6XX_ICM3_SYS_CTRL_BASE	0xFCA00000
#define VA_SPEAR6XX_ICM3_SYS_CTRL_BASE	IO_ADDRESS(SPEAR6XX_ICM3_SYS_CTRL_BASE)
#define SPEAR6XX_ICM3_SYS_CTRL_SIZE	0x00080000

#define SPEAR6XX_ICM3_MISC_REG_BASE	0xFCA80000
#define VA_SPEAR6XX_ICM3_MISC_REG_BASE	IO_ADDRESS(SPEAR6XX_ICM3_MISC_REG_BASE)
#define SPEAR6XX_ICM3_MISC_REG_SIZE	0x00080000

/* ICM4 - High Speed Connection */
#define SPEAR6XX_ICM4_BASE		0xE0000000
#define SPEAR6XX_ICM4_SIZE		0x08000000

#define SPEAR6XX_ICM4_GMAC_BASE		0xE0800000
#define SPEAR6XX_ICM4_GMAC_SIZE		0x00800000

#define SPEAR6XX_ICM4_USBD_FIFO_BASE	0xE1000000
#define SPEAR6XX_ICM4_USBD_FIFO_SIZE	0x00100000

#define SPEAR6XX_ICM4_USBD_CSR_BASE	0xE1100000
#define SPEAR6XX_ICM4_USBD_CSR_SIZE	0x00100000

#define SPEAR6XX_ICM4_USBD_PLDT_BASE	0xE1200000
#define SPEAR6XX_ICM4_USBD_PLDT_SIZE	0x00100000

#define SPEAR6XX_ICM4_USB_EHCI0_BASE	0xE1800000
#define SPEAR6XX_ICM4_USB_EHCI0_SIZE	0x00100000

#define SPEAR6XX_ICM4_USB_OHCI0_BASE	0xE1900000
#define SPEAR6XX_ICM4_USB_OHCI0_SIZE	0x00100000

#define SPEAR6XX_ICM4_USB_EHCI1_BASE	0xE2000000
#define SPEAR6XX_ICM4_USB_EHCI1_SIZE	0x00100000

#define SPEAR6XX_ICM4_USB_OHCI1_BASE	0xE2100000
#define SPEAR6XX_ICM4_USB_OHCI1_SIZE	0x00100000

#define SPEAR6XX_ICM4_USB_ARB_BASE	0xE2800000
#define SPEAR6XX_ICM4_USB_ARB_SIZE	0x00010000

/* Debug uart for linux, will be used for debug and uncompress messages */
#define SPEAR_DBG_UART_BASE		SPEAR6XX_ICM1_UART0_BASE
#define VA_SPEAR_DBG_UART_BASE		VA_SPEAR6XX_ICM1_UART0_BASE

/* Sysctl base for spear platform */
#define SPEAR_SYS_CTRL_BASE		SPEAR6XX_ICM3_SYS_CTRL_BASE
#define VA_SPEAR_SYS_CTRL_BASE		VA_SPEAR6XX_ICM3_SYS_CTRL_BASE

#endif /* __MACH_SPEAR6XX_H */
