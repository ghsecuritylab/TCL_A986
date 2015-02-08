
#include <linux/init.h>
#include <linux/kernel.h>
#include <asm/clock.h>
#include <asm/freq.h>
#include <asm/io.h>

static const int pll1rate[]={1,2,3,4,6,8};
static const int pfc_divisors[]={1,2,3,4,6,8,12};
#define ifc_divisors pfc_divisors

#if (CONFIG_SH_CLK_MD == 2)
#define PLL2 (4)
#elif (CONFIG_SH_CLK_MD == 6)
#define PLL2 (2)
#elif (CONFIG_SH_CLK_MD == 7)
#define PLL2 (1)
#else
#error "Illigal Clock Mode!"
#endif

static void master_clk_init(struct clk *clk)
{
	clk->rate *= PLL2 * pll1rate[(__raw_readw(FREQCR) >> 8) & 0x0007];
}

static struct clk_ops sh7206_master_clk_ops = {
	.init		= master_clk_init,
};

static unsigned long module_clk_recalc(struct clk *clk)
{
	int idx = (__raw_readw(FREQCR) & 0x0007);
	return clk->parent->rate / pfc_divisors[idx];
}

static struct clk_ops sh7206_module_clk_ops = {
	.recalc		= module_clk_recalc,
};

static unsigned long bus_clk_recalc(struct clk *clk)
{
	return clk->parent->rate / pll1rate[(__raw_readw(FREQCR) >> 8) & 0x0007];
}

static struct clk_ops sh7206_bus_clk_ops = {
	.recalc		= bus_clk_recalc,
};

static unsigned long cpu_clk_recalc(struct clk *clk)
{
	int idx = (__raw_readw(FREQCR) & 0x0007);
	return clk->parent->rate / ifc_divisors[idx];
}

static struct clk_ops sh7206_cpu_clk_ops = {
	.recalc		= cpu_clk_recalc,
};

static struct clk_ops *sh7206_clk_ops[] = {
	&sh7206_master_clk_ops,
	&sh7206_module_clk_ops,
	&sh7206_bus_clk_ops,
	&sh7206_cpu_clk_ops,
};

void __init arch_init_clk_ops(struct clk_ops **ops, int idx)
{
	if (idx < ARRAY_SIZE(sh7206_clk_ops))
		*ops = sh7206_clk_ops[idx];
}
