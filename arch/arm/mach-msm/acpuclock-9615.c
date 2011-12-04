/*
 * Copyright (c) 2011, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#define pr_fmt(fmt) "%s: " fmt, __func__

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/errno.h>
#include <linux/cpufreq.h>
#include <linux/clk.h>

#include <asm/cpu.h>

#include <mach/board.h>
#include <mach/msm_iomap.h>

#include "acpuclock.h"

#define dprintk(msg...) \
	cpufreq_debug_printk(CPUFREQ_DEBUG_DRIVER, "cpufreq-msm", msg)

#define REG_CLKSEL_0	(MSM_APCS_GLB_BASE + 0x08)
#define REG_CLKDIV_0	(MSM_APCS_GLB_BASE + 0x0C)
#define REG_CLKSEL_1	(MSM_APCS_GLB_BASE + 0x10)
#define REG_CLKDIV_1	(MSM_APCS_GLB_BASE + 0x14)
#define REG_CLKOUTSEL	(MSM_APCS_GLB_BASE + 0x18)

enum clk_src {
	SRC_CXO,
	SRC_PLL0,
	SRC_PLL8,
	SRC_PLL9,
	NUM_SRC,
};

struct src_clock {
	struct clk *clk;
	const char *name;
};

static struct src_clock clocks[NUM_SRC] = {
	[SRC_CXO].name  = "cxo",
	[SRC_PLL0].name = "pll0",
	[SRC_PLL8].name = "pll8",
	[SRC_PLL9].name = "pll9",
};

struct clkctl_acpu_speed {
	bool		use_for_scaling;
	unsigned int	khz;
	int		src;
	unsigned int	src_sel;
	unsigned int	src_div;
};

struct acpuclk_state {
	struct mutex			lock;
	struct clkctl_acpu_speed	*current_speed;
};

static struct acpuclk_state drv_state = {
	.current_speed = &(struct clkctl_acpu_speed){ 0 },
};

static struct clkctl_acpu_speed acpu_freq_tbl[] = {
	{ 0,  19200, SRC_CXO,  0, 0 },
	{ 1, 138000, SRC_PLL0, 6, 1 },
	{ 1, 276000, SRC_PLL0, 6, 0 },
	{ 1, 384000, SRC_PLL8, 3, 0 },
	{ 1, 440000, SRC_PLL9, 2, 0 },
	{ 0 }
};

static void select_clk_source_div(struct clkctl_acpu_speed *s)
{
	static void * __iomem const sel_reg[] = {REG_CLKSEL_0, REG_CLKSEL_1};
	static void * __iomem const div_reg[] = {REG_CLKDIV_0, REG_CLKDIV_1};
	uint32_t next_bank;

	next_bank = !(readl_relaxed(REG_CLKOUTSEL) & 1);
	writel_relaxed(s->src_sel, sel_reg[next_bank]);
	writel_relaxed(s->src_div, div_reg[next_bank]);
	writel_relaxed(next_bank, REG_CLKOUTSEL);

	/* Wait for switch to complete. */
	mb();
	udelay(1);
}

static int acpuclk_9615_set_rate(int cpu, unsigned long rate,
				 enum setrate_reason reason)
{
	struct clkctl_acpu_speed *tgt_s, *strt_s;
	int rc = 0;

	if (reason == SETRATE_CPUFREQ)
		mutex_lock(&drv_state.lock);

	strt_s = drv_state.current_speed;

	/* Return early if rate didn't change. */
	if (rate == strt_s->khz)
		goto out;

	/* Find target frequency. */
	for (tgt_s = acpu_freq_tbl; tgt_s->khz != 0; tgt_s++)
		if (tgt_s->khz == rate)
			break;
	if (tgt_s->khz == 0) {
		rc = -EINVAL;
		goto out;
	}

	dprintk("Switching from CPU rate %u KHz -> %u KHz\n",
		strt_s->khz, tgt_s->khz);

	/* Switch CPU speed. */
	clk_enable(clocks[tgt_s->src].clk);
	select_clk_source_div(tgt_s);
	clk_disable(clocks[strt_s->src].clk);

	drv_state.current_speed = tgt_s;
	dprintk("CPU speed change complete\n");

out:
	if (reason == SETRATE_CPUFREQ)
		mutex_unlock(&drv_state.lock);
	return rc;
}

static unsigned long acpuclk_9615_get_rate(int cpu)
{
	return drv_state.current_speed->khz;
}

#ifdef CONFIG_CPU_FREQ_MSM
static struct cpufreq_frequency_table freq_table[30];

static void __init cpufreq_table_init(void)
{
	int i, freq_cnt = 0;

	/* Construct the freq_table tables from acpu_freq_tbl. */
	for (i = 0; acpu_freq_tbl[i].khz != 0
			&& freq_cnt < ARRAY_SIZE(freq_table); i++) {
		if (acpu_freq_tbl[i].use_for_scaling) {
			freq_table[freq_cnt].index = freq_cnt;
			freq_table[freq_cnt].frequency
				= acpu_freq_tbl[i].khz;
			freq_cnt++;
		}
	}
	/* freq_table not big enough to store all usable freqs. */
	BUG_ON(acpu_freq_tbl[i].khz != 0);

	freq_table[freq_cnt].index = freq_cnt;
	freq_table[freq_cnt].frequency = CPUFREQ_TABLE_END;

	pr_info("CPU: %d scaling frequencies supported.\n", freq_cnt);

	/* Register table with CPUFreq. */
	cpufreq_frequency_table_get_attr(freq_table, smp_processor_id());
}
#else
static void __init cpufreq_table_init(void) {}
#endif

static struct acpuclk_data acpuclk_9615_data = {
	.set_rate = acpuclk_9615_set_rate,
	.get_rate = acpuclk_9615_get_rate,
	.power_collapse_khz = 19200,
	.wait_for_irq_khz = 19200,
};

static int __init acpuclk_9615_init(struct acpuclk_soc_data *soc_data)
{
	unsigned long max_cpu_khz = 0;
	int i;

	mutex_init(&drv_state.lock);
	for (i = 0; i < NUM_SRC; i++) {
		if (clocks[i].name) {
			clocks[i].clk = clk_get_sys(NULL, clocks[i].name);
			BUG_ON(IS_ERR(clocks[i].clk));
		}
	}

	/* Improve boot time by ramping up CPU immediately. */
	for (i = 0; acpu_freq_tbl[i].khz != 0; i++)
		max_cpu_khz = acpu_freq_tbl[i].khz;
	acpuclk_9615_set_rate(smp_processor_id(), max_cpu_khz, SETRATE_INIT);

	acpuclk_register(&acpuclk_9615_data);
	cpufreq_table_init();

	return 0;
}

struct acpuclk_soc_data acpuclk_9615_soc_data __initdata = {
	.init = acpuclk_9615_init,
};
