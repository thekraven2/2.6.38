/* Copyright (c) 2011, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/dma-mapping.h>
#include <linux/msm_tsens.h>
#include <asm/hardware/gic.h>
#include <asm/mach/flash.h>
#include <mach/board.h>
#include <mach/msm_iomap.h>
#include <mach/msm_hsusb.h>
#include <mach/irqs.h>
#include <mach/socinfo.h>
#include <mach/rpm.h>
#include <asm/hardware/cache-l2x0.h>
#include <mach/msm_sps.h>
#include <mach/dma.h>
#include "devices.h"
#include "acpuclock.h"
#include "mpm.h"
#include "spm.h"
#include "pm.h"
#include "rpm_resources.h"

/* Address of GSBI blocks */
#define MSM_GSBI1_PHYS          0x16000000
#define MSM_GSBI2_PHYS          0x16100000
#define MSM_GSBI3_PHYS          0x16200000
#define MSM_GSBI4_PHYS		0x16300000
#define MSM_GSBI5_PHYS          0x16400000

#define MSM_UART4DM_PHYS	(MSM_GSBI4_PHYS + 0x40000)

/* GSBI QUP devices */
#define MSM_GSBI1_QUP_PHYS      (MSM_GSBI1_PHYS + 0x80000)
#define MSM_GSBI2_QUP_PHYS      (MSM_GSBI2_PHYS + 0x80000)
#define MSM_GSBI3_QUP_PHYS      (MSM_GSBI3_PHYS + 0x80000)
#define MSM_GSBI4_QUP_PHYS      (MSM_GSBI4_PHYS + 0x80000)
#define MSM_GSBI5_QUP_PHYS      (MSM_GSBI5_PHYS + 0x80000)
#define MSM_QUP_SIZE            SZ_4K

/* Address of SSBI CMD */
#define MSM_PMIC1_SSBI_CMD_PHYS	0x00500000
#define MSM_PMIC_SSBI_SIZE	SZ_4K

static struct resource msm_dmov_resource[] = {
	{
		.start = ADM_0_SCSS_1_IRQ,
		.end = (resource_size_t)MSM_DMOV_BASE,
		.flags = IORESOURCE_IRQ,
	},
};

struct platform_device msm9615_device_dmov = {
	.name	= "msm_dmov",
	.id	= -1,
	.resource = msm_dmov_resource,
	.num_resources = ARRAY_SIZE(msm_dmov_resource),
};

static struct resource resources_uart_gsbi4[] = {
	{
		.start	= GSBI4_UARTDM_IRQ,
		.end	= GSBI4_UARTDM_IRQ,
		.flags	= IORESOURCE_IRQ,
	},
	{
		.start	= MSM_UART4DM_PHYS,
		.end	= MSM_UART4DM_PHYS + PAGE_SIZE - 1,
		.name	= "uartdm_resource",
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= MSM_GSBI4_PHYS,
		.end	= MSM_GSBI4_PHYS + PAGE_SIZE - 1,
		.name	= "gsbi_resource",
		.flags	= IORESOURCE_MEM,
	},
};

struct platform_device msm9615_device_uart_gsbi4 = {
	.name	= "msm_serial_hsl",
	.id	= 0,
	.num_resources	= ARRAY_SIZE(resources_uart_gsbi4),
	.resource	= resources_uart_gsbi4,
};

static struct resource resources_qup_i2c_gsbi5[] = {
	{
		.name	= "gsbi_qup_i2c_addr",
		.start	= MSM_GSBI5_PHYS,
		.end	= MSM_GSBI5_PHYS + 4 - 1,
		.flags	= IORESOURCE_MEM,
	},
	{
		.name	= "qup_phys_addr",
		.start	= MSM_GSBI5_QUP_PHYS,
		.end	= MSM_GSBI5_QUP_PHYS + MSM_QUP_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	},
	{
		.name	= "qup_err_intr",
		.start	= GSBI5_QUP_IRQ,
		.end	= GSBI5_QUP_IRQ,
		.flags	= IORESOURCE_IRQ,
	},
};

struct platform_device msm9615_device_qup_i2c_gsbi5 = {
	.name		= "qup_i2c",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(resources_qup_i2c_gsbi5),
	.resource	= resources_qup_i2c_gsbi5,
};

static struct resource resources_qup_spi_gsbi3[] = {
	{
		.name   = "spi_base",
		.start  = MSM_GSBI3_QUP_PHYS,
		.end    = MSM_GSBI3_QUP_PHYS + SZ_4K - 1,
		.flags  = IORESOURCE_MEM,
	},
	{
		.name   = "gsbi_base",
		.start  = MSM_GSBI3_PHYS,
		.end    = MSM_GSBI3_PHYS + 4 - 1,
		.flags  = IORESOURCE_MEM,
	},
	{
		.name   = "spi_irq_in",
		.start  = GSBI3_QUP_IRQ,
		.end    = GSBI3_QUP_IRQ,
		.flags  = IORESOURCE_IRQ,
	},
};

struct platform_device msm9615_device_qup_spi_gsbi3 = {
	.name		= "spi_qsd",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(resources_qup_spi_gsbi3),
	.resource	= resources_qup_spi_gsbi3,
};

static struct resource resources_ssbi_pmic1[] = {
	{
		.start  = MSM_PMIC1_SSBI_CMD_PHYS,
		.end    = MSM_PMIC1_SSBI_CMD_PHYS + MSM_PMIC_SSBI_SIZE - 1,
		.flags  = IORESOURCE_MEM,
	},
};

struct platform_device msm9615_device_ssbi_pmic1 = {
	.name           = "msm_ssbi",
	.id             = 0,
	.resource       = resources_ssbi_pmic1,
	.num_resources  = ARRAY_SIZE(resources_ssbi_pmic1),
};

static struct resource resources_sps[] = {
	{
		.name	= "pipe_mem",
		.start	= 0x12800000,
		.end	= 0x12800000 + 0x4000 - 1,
		.flags	= IORESOURCE_MEM,
	},
	{
		.name	= "bamdma_dma",
		.start	= 0x12240000,
		.end	= 0x12240000 + 0x1000 - 1,
		.flags	= IORESOURCE_MEM,
	},
	{
		.name	= "bamdma_bam",
		.start	= 0x12244000,
		.end	= 0x12244000 + 0x4000 - 1,
		.flags	= IORESOURCE_MEM,
	},
	{
		.name	= "bamdma_irq",
		.start	= SPS_BAM_DMA_IRQ,
		.end	= SPS_BAM_DMA_IRQ,
		.flags	= IORESOURCE_IRQ,
	},
};

struct msm_sps_platform_data msm_sps_pdata = {
	.bamdma_restricted_pipes = 0x06,
};

struct platform_device msm_device_sps = {
	.name		= "msm_sps",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(resources_sps),
	.resource	= resources_sps,
	.dev.platform_data = &msm_sps_pdata,
};

static struct resource resources_otg[] = {
	{
		.start	= MSM9615_HSUSB_PHYS,
		.end	= MSM9615_HSUSB_PHYS + MSM9615_HSUSB_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= USB1_HS_IRQ,
		.end	= USB1_HS_IRQ,
		.flags	= IORESOURCE_IRQ,
	},
};

struct platform_device msm_device_otg = {
	.name		= "msm_otg",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(resources_otg),
	.resource	= resources_otg,
	.dev		= {
		.coherent_dma_mask	= DMA_BIT_MASK(32),
	},
};

static struct resource resources_hsusb[] = {
	{
		.start	= MSM9615_HSUSB_PHYS,
		.end	= MSM9615_HSUSB_PHYS + MSM9615_HSUSB_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= USB1_HS_IRQ,
		.end	= USB1_HS_IRQ,
		.flags	= IORESOURCE_IRQ,
	},
};

struct platform_device msm_device_gadget_peripheral = {
	.name		= "msm_hsusb",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(resources_hsusb),
	.resource	= resources_hsusb,
	.dev		= {
		.coherent_dma_mask	= DMA_BIT_MASK(32),
	},
};

static struct tsens_platform_data msm_tsens_pdata = {
	.slope			= 910,
	.tsens_factor		= 1000,
	.hw_type		= MSM_9615,
	.tsens_num_sensor	= 5,
};

struct platform_device	msm9615_device_tsens = {
	.name		= "tsens8960-tm",
	.id		= -1,
	.dev		= {
			.platform_data = &msm_tsens_pdata,
	},
};

#define MSM_NAND_PHYS		0x1B400000
static struct resource resources_nand[] = {
	[0] = {
		.name   = "msm_nand_dmac",
		.start	= DMOV_NAND_CHAN,
		.end	= DMOV_NAND_CHAN,
		.flags	= IORESOURCE_DMA,
	},
	[1] = {
		.name   = "msm_nand_phys",
		.start  = MSM_NAND_PHYS,
		.end    = MSM_NAND_PHYS + 0x7FF,
		.flags  = IORESOURCE_MEM,
	},
};

struct flash_platform_data msm_nand_data = {
	.parts		= NULL,
	.nr_parts	= 0,
};

struct platform_device msm_device_nand = {
	.name		= "msm_nand",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(resources_nand),
	.resource	= resources_nand,
	.dev		= {
		.platform_data	= &msm_nand_data,
	},
};

struct platform_device msm_device_smd = {
	.name		= "msm_smd",
	.id		= -1,
};


#define MSM_SDC1_BASE         0x12180000
#define MSM_SDC1_DML_BASE     (MSM_SDC1_BASE + 0x800)
#define MSM_SDC1_BAM_BASE     (MSM_SDC1_BASE + 0x2000)
#define MSM_SDC2_BASE         0x12140000
#define MSM_SDC2_DML_BASE     (MSM_SDC2_BASE + 0x800)
#define MSM_SDC2_BAM_BASE     (MSM_SDC2_BASE + 0x2000)

static struct resource resources_sdc1[] = {
	{
		.name   = "core_mem",
		.flags  = IORESOURCE_MEM,
		.start  = MSM_SDC1_BASE,
		.end    = MSM_SDC1_DML_BASE - 1,
	},
	{
		.name   = "core_irq",
		.flags  = IORESOURCE_IRQ,
		.start  = SDC1_IRQ_0,
		.end    = SDC1_IRQ_0
	},
#ifdef CONFIG_MMC_MSM_SPS_SUPPORT
	{
		.name   = "sdcc_dml_addr",
		.start  = MSM_SDC1_DML_BASE,
		.end    = MSM_SDC1_BAM_BASE - 1,
		.flags  = IORESOURCE_MEM,
	},
	{
		.name   = "sdcc_bam_addr",
		.start  = MSM_SDC1_BAM_BASE,
		.end    = MSM_SDC1_BAM_BASE + (2 * SZ_4K) - 1,
		.flags  = IORESOURCE_MEM,
	},
	{
		.name   = "sdcc_bam_irq",
		.start  = SDC1_BAM_IRQ,
		.end    = SDC1_BAM_IRQ,
		.flags  = IORESOURCE_IRQ,
	},
#endif
};

static struct resource resources_sdc2[] = {
	{
		.name   = "core_mem",
		.flags  = IORESOURCE_MEM,
		.start  = MSM_SDC2_BASE,
		.end    = MSM_SDC2_DML_BASE - 1,
	},
	{
		.name   = "core_irq",
		.flags  = IORESOURCE_IRQ,
		.start  = SDC2_IRQ_0,
		.end    = SDC2_IRQ_0
	},
#ifdef CONFIG_MMC_MSM_SPS_SUPPORT
	{
		.name   = "sdcc_dml_addr",
		.start  = MSM_SDC2_DML_BASE,
		.end    = MSM_SDC2_BAM_BASE - 1,
		.flags  = IORESOURCE_MEM,
	},
	{
		.name   = "sdcc_bam_addr",
		.start  = MSM_SDC2_BAM_BASE,
		.end    = MSM_SDC2_BAM_BASE + (2 * SZ_4K) - 1,
		.flags  = IORESOURCE_MEM,
	},
	{
		.name   = "sdcc_bam_irq",
		.start  = SDC2_BAM_IRQ,
		.end    = SDC2_BAM_IRQ,
		.flags  = IORESOURCE_IRQ,
	},
#endif
};

struct platform_device msm_device_sdc1 = {
	.name           = "msm_sdcc",
	.id             = 1,
	.num_resources  = ARRAY_SIZE(resources_sdc1),
	.resource       = resources_sdc1,
	.dev            = {
		.coherent_dma_mask      = 0xffffffff,
	},
};

struct platform_device msm_device_sdc2 = {
	.name           = "msm_sdcc",
	.id             = 2,
	.num_resources  = ARRAY_SIZE(resources_sdc2),
	.resource       = resources_sdc2,
	.dev            = {
		.coherent_dma_mask      = 0xffffffff,
	},
};

static struct platform_device *msm_sdcc_devices[] __initdata = {
	&msm_device_sdc1,
	&msm_device_sdc2,
};

int __init msm_add_sdcc(unsigned int controller, struct mmc_platform_data *plat)
{
	struct platform_device  *pdev;

	if (controller < 1 || controller > 2)
		return -EINVAL;

	pdev = msm_sdcc_devices[controller - 1];
	pdev->dev.platform_data = plat;
	return platform_device_register(pdev);
}

#ifdef CONFIG_CACHE_L2X0
static int __init l2x0_cache_init(void)
{
	int aux_ctrl = 0;

	/* Way Size 010(0x2) 32KB */
	aux_ctrl = (0x1 << L2X0_AUX_CTRL_SHARE_OVERRIDE_SHIFT) | \
		   (0x2 << L2X0_AUX_CTRL_WAY_SIZE_SHIFT) | \
		   (0x1 << L2X0_AUX_CTRL_EVNT_MON_BUS_EN_SHIFT);

	/* L2 Latency setting required by hardware. Default is 0x20
	   which is no good.
	 */
	writel_relaxed(0x220, MSM_L2CC_BASE + L2X0_DATA_LATENCY_CTRL);
	l2x0_init(MSM_L2CC_BASE, aux_ctrl, L2X0_AUX_CTRL_MASK);

	return 0;
}
#else
static int __init l2x0_cache_init(void){ return 0; }
#endif

struct msm_rpm_map_data rpm_map_data[] __initdata = {
	MSM_RPM_MAP(TRIGGER_TIMED_TO, TRIGGER_TIMED, 1),
	MSM_RPM_MAP(TRIGGER_TIMED_SCLK_COUNT, TRIGGER_TIMED, 1),

	MSM_RPM_MAP(RPM_CTL, RPM_CTL, 1),

	MSM_RPM_MAP(CXO_CLK, CXO_CLK, 1),
	MSM_RPM_MAP(SYSTEM_FABRIC_CLK, SYSTEM_FABRIC_CLK, 1),
	MSM_RPM_MAP(DAYTONA_FABRIC_CLK, DAYTONA_FABRIC_CLK, 1),
	MSM_RPM_MAP(SFPB_CLK, SFPB_CLK, 1),
	MSM_RPM_MAP(CFPB_CLK, CFPB_CLK, 1),
	MSM_RPM_MAP(EBI1_CLK, EBI1_CLK, 1),

	MSM_RPM_MAP(SYS_FABRIC_CFG_HALT_0, SYS_FABRIC_CFG_HALT, 2),
	MSM_RPM_MAP(SYS_FABRIC_CFG_CLKMOD_0, SYS_FABRIC_CFG_CLKMOD, 3),
	MSM_RPM_MAP(SYS_FABRIC_CFG_IOCTL, SYS_FABRIC_CFG_IOCTL, 1),
	MSM_RPM_MAP(SYSTEM_FABRIC_ARB_0, SYSTEM_FABRIC_ARB, 27),

	MSM_RPM_MAP(PM8018_S1_0, PM8018_S1, 2),
	MSM_RPM_MAP(PM8018_S2_0, PM8018_S2, 2),
	MSM_RPM_MAP(PM8018_S3_0, PM8018_S3, 2),
	MSM_RPM_MAP(PM8018_S4_0, PM8018_S4, 2),
	MSM_RPM_MAP(PM8018_S5_0, PM8018_S5, 2),
	MSM_RPM_MAP(PM8018_L1_0, PM8018_L1, 2),
	MSM_RPM_MAP(PM8018_L2_0, PM8018_L2, 2),
	MSM_RPM_MAP(PM8018_L3_0, PM8018_L3, 2),
	MSM_RPM_MAP(PM8018_L4_0, PM8018_L4, 2),
	MSM_RPM_MAP(PM8018_L5_0, PM8018_L5, 2),
	MSM_RPM_MAP(PM8018_L6_0, PM8018_L6, 2),
	MSM_RPM_MAP(PM8018_L7_0, PM8018_L7, 2),
	MSM_RPM_MAP(PM8018_L8_0, PM8018_L8, 2),
	MSM_RPM_MAP(PM8018_L9_0, PM8018_L9, 2),
	MSM_RPM_MAP(PM8018_L10_0, PM8018_L10, 2),
	MSM_RPM_MAP(PM8018_L11_0, PM8018_L11, 2),
	MSM_RPM_MAP(PM8018_L12_0, PM8018_L12, 2),
	MSM_RPM_MAP(PM8018_L13_0, PM8018_L13, 2),
	MSM_RPM_MAP(PM8018_L14_0, PM8018_L14, 2),
	MSM_RPM_MAP(PM8018_LVS1, PM8018_LVS1, 1),
	MSM_RPM_MAP(NCP_0, NCP, 2),
	MSM_RPM_MAP(CXO_BUFFERS, CXO_BUFFERS, 1),
	MSM_RPM_MAP(USB_OTG_SWITCH, USB_OTG_SWITCH, 1),
	MSM_RPM_MAP(HDMI_SWITCH, HDMI_SWITCH, 1),
};
unsigned int rpm_map_data_size = ARRAY_SIZE(rpm_map_data);

static struct msm_rpm_platform_data msm_rpm_data = {
	.reg_base_addrs = {
		[MSM_RPM_PAGE_STATUS] = MSM_RPM_BASE,
		[MSM_RPM_PAGE_CTRL] = MSM_RPM_BASE + 0x400,
		[MSM_RPM_PAGE_REQ] = MSM_RPM_BASE + 0x600,
		[MSM_RPM_PAGE_ACK] = MSM_RPM_BASE + 0xa00,
	},

	.irq_ack = RPM_APCC_CPU0_GP_HIGH_IRQ,
	.irq_err = RPM_APCC_CPU0_GP_LOW_IRQ,
	.irq_vmpm = RPM_APCC_CPU0_GP_MEDIUM_IRQ,
	.msm_apps_ipc_rpm_reg = MSM_APCS_GCC_BASE + 0x008,
	.msm_apps_ipc_rpm_val = 4,
};

struct platform_device msm_rpm_device = {
	.name   = "msm_rpm",
	.id     = -1,
};

static uint16_t msm_mpm_irqs_m2a[MSM_MPM_NR_MPM_IRQS] = {
	[1] = MSM_GPIO_TO_INT(46),
	[2] = MSM_GPIO_TO_INT(150),
	[4] = MSM_GPIO_TO_INT(103),
	[5] = MSM_GPIO_TO_INT(104),
	[6] = MSM_GPIO_TO_INT(105),
	[7] = MSM_GPIO_TO_INT(106),
	[8] = MSM_GPIO_TO_INT(107),
	[9] = MSM_GPIO_TO_INT(7),
	[10] = MSM_GPIO_TO_INT(11),
	[11] = MSM_GPIO_TO_INT(15),
	[12] = MSM_GPIO_TO_INT(19),
	[13] = MSM_GPIO_TO_INT(23),
	[14] = MSM_GPIO_TO_INT(27),
	[15] = MSM_GPIO_TO_INT(31),
	[16] = MSM_GPIO_TO_INT(35),
	[19] = MSM_GPIO_TO_INT(90),
	[20] = MSM_GPIO_TO_INT(92),
	[23] = MSM_GPIO_TO_INT(85),
	[24] = MSM_GPIO_TO_INT(83),
	[25] = USB1_HS_IRQ,
	/*[27] = HDMI_IRQ,*/
	[29] = MSM_GPIO_TO_INT(10),
	[30] = MSM_GPIO_TO_INT(102),
	[31] = MSM_GPIO_TO_INT(81),
	[32] = MSM_GPIO_TO_INT(78),
	[33] = MSM_GPIO_TO_INT(94),
	[34] = MSM_GPIO_TO_INT(72),
	[35] = MSM_GPIO_TO_INT(39),
	[36] = MSM_GPIO_TO_INT(43),
	[37] = MSM_GPIO_TO_INT(61),
	[38] = MSM_GPIO_TO_INT(50),
	[39] = MSM_GPIO_TO_INT(42),
	[41] = MSM_GPIO_TO_INT(62),
	[42] = MSM_GPIO_TO_INT(76),
	[43] = MSM_GPIO_TO_INT(75),
	[44] = MSM_GPIO_TO_INT(70),
	[45] = MSM_GPIO_TO_INT(69),
	[46] = MSM_GPIO_TO_INT(67),
	[47] = MSM_GPIO_TO_INT(65),
	[48] = MSM_GPIO_TO_INT(58),
	[49] = MSM_GPIO_TO_INT(54),
	[50] = MSM_GPIO_TO_INT(52),
	[51] = MSM_GPIO_TO_INT(49),
	[52] = MSM_GPIO_TO_INT(40),
	[53] = MSM_GPIO_TO_INT(37),
	[54] = MSM_GPIO_TO_INT(24),
	[55] = MSM_GPIO_TO_INT(14),
};

static uint16_t msm_mpm_bypassed_apps_irqs[] = {
	TLMM_MSM_SUMMARY_IRQ,
	RPM_APCC_CPU0_GP_HIGH_IRQ,
	RPM_APCC_CPU0_GP_MEDIUM_IRQ,
	RPM_APCC_CPU0_GP_LOW_IRQ,
	RPM_APCC_CPU0_WAKE_UP_IRQ,
	LPASS_SCSS_GP_LOW_IRQ,
	LPASS_SCSS_GP_MEDIUM_IRQ,
	LPASS_SCSS_GP_HIGH_IRQ,
	SPS_MTI_31,
};

struct msm_mpm_device_data msm_mpm_dev_data = {
	.irqs_m2a = msm_mpm_irqs_m2a,
	.irqs_m2a_size = ARRAY_SIZE(msm_mpm_irqs_m2a),
	.bypassed_apps_irqs = msm_mpm_bypassed_apps_irqs,
	.bypassed_apps_irqs_size = ARRAY_SIZE(msm_mpm_bypassed_apps_irqs),
	.mpm_request_reg_base = MSM_RPM_BASE + 0x9d8,
	.mpm_status_reg_base = MSM_RPM_BASE + 0xdf8,
	.mpm_apps_ipc_reg = MSM_APCS_GCC_BASE + 0x008,
	.mpm_apps_ipc_val =  BIT(1),
	.mpm_ipc_irq = RPM_APCC_CPU0_GP_MEDIUM_IRQ,
};

static uint8_t spm_wfi_cmd_sequence[] __initdata = {
	0x00, 0x03, 0x0B, 0x00,
	0x0f,
};

static uint8_t spm_power_collapse_without_rpm[] __initdata = {
	0x30, 0x20, 0x10, 0x00,
	0x50, 0x03, 0x50, 0x00,
	0x10, 0x20, 0x30, 0x0f,
};

static uint8_t spm_power_collapse_with_rpm[] __initdata = {
	0x30, 0x20, 0x10, 0x00,
	0x50, 0x07, 0x50, 0x00,
	0x10, 0x20, 0x30, 0x0f,
};

static struct msm_spm_seq_entry msm_spm_seq_list[] __initdata = {
	[0] = {
		.mode = MSM_SPM_MODE_CLOCK_GATING,
		.notify_rpm = false,
		.cmd = spm_wfi_cmd_sequence,
	},
	[1] = {
		.mode = MSM_SPM_MODE_POWER_COLLAPSE,
		.notify_rpm = false,
		.cmd = spm_power_collapse_without_rpm,
	},
	[2] = {
		.mode = MSM_SPM_MODE_POWER_COLLAPSE,
		.notify_rpm = true,
		.cmd = spm_power_collapse_with_rpm,
	},
};

static struct msm_spm_platform_data msm_spm_data[] __initdata = {
	[0] = {
		.reg_base_addr = MSM_SAW0_BASE,
		.reg_init_values[MSM_SPM_REG_SAW2_SPM_CTL] = 0x01,
		.reg_init_values[MSM_SPM_REG_SAW2_CFG] = 0x1F,
		.num_modes = ARRAY_SIZE(msm_spm_seq_list),
		.modes = msm_spm_seq_list,
	},
};

static struct msm_rpmrs_level msm_rpmrs_levels[] __initdata = {
	{
		MSM_PM_SLEEP_MODE_WAIT_FOR_INTERRUPT,
		MSM_RPMRS_LIMITS(ON, ACTIVE, MAX, ACTIVE),
		true,
		1, 8000, 100000, 1,
	},

	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE_STANDALONE,
		MSM_RPMRS_LIMITS(ON, ACTIVE, MAX, ACTIVE),
		true,
		1500, 5000, 60100000, 3000,
	},
	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE,
		MSM_RPMRS_LIMITS(ON, ACTIVE, MAX, ACTIVE),
		false,
		2800, 5000, 60350000, 3500,
	},
};

void __init msm9615_device_init(void)
{
	msm_spm_init(msm_spm_data, ARRAY_SIZE(msm_spm_data));
	msm_clock_init(&msm9615_clock_init_data);
	acpuclk_init(&acpuclk_9615_soc_data);
	BUG_ON(msm_rpm_init(&msm_rpm_data));
	BUG_ON(msm_rpmrs_levels_init(msm_rpmrs_levels,
			ARRAY_SIZE(msm_rpmrs_levels)));
}

#define MSM_SHARED_RAM_PHYS 0x40000000
void __init msm9615_map_io(void)
{
	msm_shared_ram_phys = MSM_SHARED_RAM_PHYS;
	msm_map_msm9615_io();
	l2x0_cache_init();
	if (socinfo_init() < 0)
		pr_err("socinfo_init() failed!\n");
}

void __init msm9615_init_irq(void)
{
	unsigned int i;
	gic_init(0, GIC_PPI_START, MSM_QGIC_DIST_BASE,
						(void *)MSM_QGIC_CPU_BASE);

	/* Edge trigger PPIs except AVS_SVICINT and AVS_SVICINTSWDONE */
	writel_relaxed(0xFFFFD7FF, MSM_QGIC_DIST_BASE + GIC_DIST_CONFIG + 4);

	writel_relaxed(0x0000FFFF, MSM_QGIC_DIST_BASE + GIC_DIST_ENABLE_SET);
	mb();

	/*
	 * FIXME: Not installing AVS_SVICINT and AVS_SVICINTSWDONE yet
	 * as they are configured as level, which does not play nice with
	 * handle_percpu_irq.
	 */
	for (i = GIC_PPI_START; i < GIC_SPI_START; i++) {
		if (i != AVS_SVICINT && i != AVS_SVICINTSWDONE)
			set_irq_handler(i, handle_percpu_irq);
	}
}
