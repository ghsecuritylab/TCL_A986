

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/input/sh_keysc.h>
#include <linux/bitmap.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/slab.h>

static const struct {
	unsigned char kymd, keyout, keyin;
} sh_keysc_mode[] = {
	[SH_KEYSC_MODE_1] = { 0, 6, 5 },
	[SH_KEYSC_MODE_2] = { 1, 5, 6 },
	[SH_KEYSC_MODE_3] = { 2, 4, 7 },
	[SH_KEYSC_MODE_4] = { 3, 6, 6 },
	[SH_KEYSC_MODE_5] = { 4, 6, 7 },
	[SH_KEYSC_MODE_6] = { 5, 7, 7 },
};

struct sh_keysc_priv {
	void __iomem *iomem_base;
	struct clk *clk;
	DECLARE_BITMAP(last_keys, SH_KEYSC_MAXKEYS);
	struct input_dev *input;
	struct sh_keysc_info pdata;
};

#define KYCR1 0
#define KYCR2 1
#define KYINDR 2
#define KYOUTDR 3

#define KYCR2_IRQ_LEVEL    0x10
#define KYCR2_IRQ_DISABLED 0x00

static unsigned long sh_keysc_read(struct sh_keysc_priv *p, int reg_nr)
{
	return ioread16(p->iomem_base + (reg_nr << 2));
}

static void sh_keysc_write(struct sh_keysc_priv *p, int reg_nr,
			   unsigned long value)
{
	iowrite16(value, p->iomem_base + (reg_nr << 2));
}

static void sh_keysc_level_mode(struct sh_keysc_priv *p,
				unsigned long keys_set)
{
	struct sh_keysc_info *pdata = &p->pdata;

	sh_keysc_write(p, KYOUTDR, 0);
	sh_keysc_write(p, KYCR2, KYCR2_IRQ_LEVEL | (keys_set << 8));

	if (pdata->kycr2_delay)
		udelay(pdata->kycr2_delay);
}

static void sh_keysc_map_dbg(struct device *dev, unsigned long *map,
			     const char *str)
{
	int k;

	for (k = 0; k < BITS_TO_LONGS(SH_KEYSC_MAXKEYS); k++)
		dev_dbg(dev, "%s[%d] 0x%lx\n", str, k, map[k]);
}

static irqreturn_t sh_keysc_isr(int irq, void *dev_id)
{
	struct platform_device *pdev = dev_id;
	struct sh_keysc_priv *priv = platform_get_drvdata(pdev);
	struct sh_keysc_info *pdata = &priv->pdata;
	int keyout_nr = sh_keysc_mode[pdata->mode].keyout;
	int keyin_nr = sh_keysc_mode[pdata->mode].keyin;
	DECLARE_BITMAP(keys, SH_KEYSC_MAXKEYS);
	DECLARE_BITMAP(keys0, SH_KEYSC_MAXKEYS);
	DECLARE_BITMAP(keys1, SH_KEYSC_MAXKEYS);
	unsigned char keyin_set, tmp;
	int i, k, n;

	dev_dbg(&pdev->dev, "isr!\n");

	bitmap_fill(keys1, SH_KEYSC_MAXKEYS);
	bitmap_zero(keys0, SH_KEYSC_MAXKEYS);

	do {
		bitmap_zero(keys, SH_KEYSC_MAXKEYS);
		keyin_set = 0;

		sh_keysc_write(priv, KYCR2, KYCR2_IRQ_DISABLED);

		for (i = 0; i < keyout_nr; i++) {
			n = keyin_nr * i;

			/* drive one KEYOUT pin low, read KEYIN pins */
			sh_keysc_write(priv, KYOUTDR, 0xffff ^ (3 << (i * 2)));
			udelay(pdata->delay);
			tmp = sh_keysc_read(priv, KYINDR);

			/* set bit if key press has been detected */
			for (k = 0; k < keyin_nr; k++) {
				if (tmp & (1 << k))
					__set_bit(n + k, keys);
			}

			/* keep track of which KEYIN bits that have been set */
			keyin_set |= tmp ^ ((1 << keyin_nr) - 1);
		}

		sh_keysc_level_mode(priv, keyin_set);

		bitmap_complement(keys, keys, SH_KEYSC_MAXKEYS);
		bitmap_and(keys1, keys1, keys, SH_KEYSC_MAXKEYS);
		bitmap_or(keys0, keys0, keys, SH_KEYSC_MAXKEYS);

		sh_keysc_map_dbg(&pdev->dev, keys, "keys");

	} while (sh_keysc_read(priv, KYCR2) & 0x01);

	sh_keysc_map_dbg(&pdev->dev, priv->last_keys, "last_keys");
	sh_keysc_map_dbg(&pdev->dev, keys0, "keys0");
	sh_keysc_map_dbg(&pdev->dev, keys1, "keys1");

	for (i = 0; i < SH_KEYSC_MAXKEYS; i++) {
		k = pdata->keycodes[i];
		if (!k)
			continue;

		if (test_bit(i, keys0) == test_bit(i, priv->last_keys))
			continue;

		if (test_bit(i, keys1) || test_bit(i, keys0)) {
			input_event(priv->input, EV_KEY, k, 1);
			__set_bit(i, priv->last_keys);
		}

		if (!test_bit(i, keys1)) {
			input_event(priv->input, EV_KEY, k, 0);
			__clear_bit(i, priv->last_keys);
		}

	}
	input_sync(priv->input);

	return IRQ_HANDLED;
}

static int __devinit sh_keysc_probe(struct platform_device *pdev)
{
	struct sh_keysc_priv *priv;
	struct sh_keysc_info *pdata;
	struct resource *res;
	struct input_dev *input;
	char clk_name[8];
	int i;
	int irq, error;

	if (!pdev->dev.platform_data) {
		dev_err(&pdev->dev, "no platform data defined\n");
		error = -EINVAL;
		goto err0;
	}

	error = -ENXIO;
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		dev_err(&pdev->dev, "failed to get I/O memory\n");
		goto err0;
	}

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(&pdev->dev, "failed to get irq\n");
		goto err0;
	}

	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (priv == NULL) {
		dev_err(&pdev->dev, "failed to allocate driver data\n");
		error = -ENOMEM;
		goto err0;
	}

	platform_set_drvdata(pdev, priv);
	memcpy(&priv->pdata, pdev->dev.platform_data, sizeof(priv->pdata));
	pdata = &priv->pdata;

	priv->iomem_base = ioremap_nocache(res->start, resource_size(res));
	if (priv->iomem_base == NULL) {
		dev_err(&pdev->dev, "failed to remap I/O memory\n");
		error = -ENXIO;
		goto err1;
	}

	snprintf(clk_name, sizeof(clk_name), "keysc%d", pdev->id);
	priv->clk = clk_get(&pdev->dev, clk_name);
	if (IS_ERR(priv->clk)) {
		dev_err(&pdev->dev, "cannot get clock \"%s\"\n", clk_name);
		error = PTR_ERR(priv->clk);
		goto err2;
	}

	priv->input = input_allocate_device();
	if (!priv->input) {
		dev_err(&pdev->dev, "failed to allocate input device\n");
		error = -ENOMEM;
		goto err3;
	}

	input = priv->input;
	input->evbit[0] = BIT_MASK(EV_KEY);

	input->name = pdev->name;
	input->phys = "sh-keysc-keys/input0";
	input->dev.parent = &pdev->dev;

	input->id.bustype = BUS_HOST;
	input->id.vendor = 0x0001;
	input->id.product = 0x0001;
	input->id.version = 0x0100;

	input->keycode = pdata->keycodes;
	input->keycodesize = sizeof(pdata->keycodes[0]);
	input->keycodemax = ARRAY_SIZE(pdata->keycodes);

	error = request_irq(irq, sh_keysc_isr, 0, pdev->name, pdev);
	if (error) {
		dev_err(&pdev->dev, "failed to request IRQ\n");
		goto err4;
	}

	for (i = 0; i < SH_KEYSC_MAXKEYS; i++)
		__set_bit(pdata->keycodes[i], input->keybit);
	__clear_bit(KEY_RESERVED, input->keybit);

	error = input_register_device(input);
	if (error) {
		dev_err(&pdev->dev, "failed to register input device\n");
		goto err5;
	}

	clk_enable(priv->clk);

	sh_keysc_write(priv, KYCR1, (sh_keysc_mode[pdata->mode].kymd << 8) |
		       pdata->scan_timing);
	sh_keysc_level_mode(priv, 0);

	device_init_wakeup(&pdev->dev, 1);

	return 0;

 err5:
	free_irq(irq, pdev);
 err4:
	input_free_device(input);
 err3:
	clk_put(priv->clk);
 err2:
	iounmap(priv->iomem_base);
 err1:
	platform_set_drvdata(pdev, NULL);
	kfree(priv);
 err0:
	return error;
}

static int __devexit sh_keysc_remove(struct platform_device *pdev)
{
	struct sh_keysc_priv *priv = platform_get_drvdata(pdev);

	sh_keysc_write(priv, KYCR2, KYCR2_IRQ_DISABLED);

	input_unregister_device(priv->input);
	free_irq(platform_get_irq(pdev, 0), pdev);
	iounmap(priv->iomem_base);

	clk_disable(priv->clk);
	clk_put(priv->clk);

	platform_set_drvdata(pdev, NULL);
	kfree(priv);

	return 0;
}

static int sh_keysc_suspend(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct sh_keysc_priv *priv = platform_get_drvdata(pdev);
	int irq = platform_get_irq(pdev, 0);
	unsigned short value;

	value = sh_keysc_read(priv, KYCR1);

	if (device_may_wakeup(dev)) {
		value |= 0x80;
		enable_irq_wake(irq);
	} else {
		value &= ~0x80;
	}

	sh_keysc_write(priv, KYCR1, value);

	return 0;
}

static int sh_keysc_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	int irq = platform_get_irq(pdev, 0);

	if (device_may_wakeup(dev))
		disable_irq_wake(irq);

	return 0;
}

static const struct dev_pm_ops sh_keysc_dev_pm_ops = {
	.suspend = sh_keysc_suspend,
	.resume = sh_keysc_resume,
};

struct platform_driver sh_keysc_device_driver = {
	.probe		= sh_keysc_probe,
	.remove		= __devexit_p(sh_keysc_remove),
	.driver		= {
		.name	= "sh_keysc",
		.pm	= &sh_keysc_dev_pm_ops,
	}
};

static int __init sh_keysc_init(void)
{
	return platform_driver_register(&sh_keysc_device_driver);
}

static void __exit sh_keysc_exit(void)
{
	platform_driver_unregister(&sh_keysc_device_driver);
}

module_init(sh_keysc_init);
module_exit(sh_keysc_exit);

MODULE_AUTHOR("Magnus Damm");
MODULE_DESCRIPTION("SuperH KEYSC Keypad Driver");
MODULE_LICENSE("GPL");
