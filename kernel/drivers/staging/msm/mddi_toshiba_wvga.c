

#include "msm_fb.h"
#include "mddihost.h"
#include "mddi_toshiba.h"

static int __init mddi_toshiba_wvga_init(void)
{
	int ret;
	struct msm_panel_info pinfo;

#ifdef CONFIG_FB_MSM_MDDI_AUTO_DETECT
	if (msm_fb_detect_client("mddi_toshiba_wvga"))
		return 0;
#endif

	pinfo.xres = 800;
	pinfo.yres = 480;
	pinfo.pdest = DISPLAY_2;
	pinfo.type = MDDI_PANEL;
	pinfo.mddi.vdopkt = MDDI_DEFAULT_PRIM_PIX_ATTR;
	pinfo.wait_cycle = 0;
	pinfo.bpp = 18;
	pinfo.lcd.vsync_enable = TRUE;
	pinfo.lcd.refx100 = 6118;
	pinfo.lcd.v_back_porch = 6;
	pinfo.lcd.v_front_porch = 0;
	pinfo.lcd.v_pulse_width = 0;
	pinfo.lcd.hw_vsync_mode = FALSE;
	pinfo.lcd.vsync_notifier_period = (1 * HZ);
	pinfo.bl_max = 4;
	pinfo.bl_min = 1;
	pinfo.clk_rate = 192000000;
	pinfo.clk_min =  190000000;
	pinfo.clk_max =  200000000;
	pinfo.fb_num = 2;

	ret = mddi_toshiba_device_register(&pinfo, TOSHIBA_VGA_PRIM,
					   LCD_TOSHIBA_2P4_WVGA);
	if (ret) {
		printk(KERN_ERR "%s: failed to register device!\n", __func__);
		return ret;
	}

	return ret;
}

module_init(mddi_toshiba_wvga_init);