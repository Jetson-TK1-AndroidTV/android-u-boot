/*
 * (C) Copyright 2013-2014
 * NVIDIA Corporation <www.nvidia.com>
 *
 * SPDX-License-Identifier:     GPL-2.0
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/sizes.h>

/* enable PMIC */
#define CONFIG_AS3722_POWER

#include "tegra124-common.h"

#ifdef CONFIG_TEGRA_LP0
#define CONFIG_TEGRA124_LP0
#endif

#define CONFIG_MISC_INIT_R		/* call recovery during start up */

#undef CONFIG_SERIAL_TAG
#define CONFIG_SERIAL_TAG
#define CONFIG_TEGRA_SERIAL_HIGH	0x01770000
#define CONFIG_TEGRA_SERIAL_LOW		0x034200FF

/* Use memory controller SDRAM size instead of ODMDATA */
#define CONFIG_TEGRA_USE_EMC_DRAM_SIZE

/* High-level configuration options */
#define V_PROMPT			"Tegra124 (Jetson TK1) # "
#define CONFIG_TEGRA_BOARD_STRING	"NVIDIA Jetson TK1"

/* Board-specific serial config */
#define CONFIG_SERIAL_MULTI
#define CONFIG_TEGRA_ENABLE_UARTD
#define CONFIG_SYS_NS16550_COM1		NV_PA_APB_UARTD_BASE

/* I2C */
#define CONFIG_SYS_I2C_TEGRA
#define CONFIG_CMD_I2C

/* SD/MMC */
#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_TEGRA_MMC
#define CONFIG_CMD_MMC

/* Environment in eMMC, at the end of 2nd "boot sector" */
#define CONFIG_ENV_IS_IN_MMC
#define CONFIG_ENV_OFFSET		(-CONFIG_ENV_SIZE)
#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_SYS_MMC_ENV_PART		2

/* SPI */
#define CONFIG_TEGRA114_SPI		/* Compatible w/ Tegra114 SPI */
#define CONFIG_TEGRA114_SPI_CTRLS	6
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_WINBOND
#define CONFIG_SF_DEFAULT_MODE		SPI_MODE_0
#define CONFIG_SF_DEFAULT_SPEED		24000000
#define CONFIG_CMD_SPI
#define CONFIG_CMD_SF
#define CONFIG_SPI_FLASH_SIZE		(4 << 20)

/* USB Host support */
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_TEGRA
#define CONFIG_USB_MAX_CONTROLLER_COUNT	2
#define CONFIG_USB_STORAGE
#define CONFIG_CMD_USB

/* USB networking support */
#define CONFIG_USB_HOST_ETHER
#define CONFIG_USB_ETHER_ASIX

/* PCI host support */
#define CONFIG_PCI
#define CONFIG_PCI_TEGRA
#define CONFIG_PCI_PNP
#define CONFIG_CMD_PCI
#define CONFIG_CMD_PCI_ENUM

/* PCI networking support */
#define CONFIG_RTL8169

/* General networking support */
#define CONFIG_CMD_NET
#define CONFIG_CMD_DHCP

/* Android fastboot support */
#define CONFIG_CMD_FASTBOOT
#define CONFIG_FASTBOOT_FLASH
#define CONFIG_FASTBOOT_GPT_NAME	0
#define CONFIG_FASTBOOT_FLASH_MMC_DEV	0
#define CONFIG_USB_FASTBOOT_BUF_SIZE	  0x40000000
#define CONFIG_USB_FASTBOOT_BUF_ADDR	  (NV_PA_SDRAM_BASE + 0x10000000)

/* Android bootimg support */
#define CONFIG_ANDROID_BOOT_IMAGE

#define CONFIG_CMD_BOOTA
#define CONFIG_CMD_BOOTA_BOOT_PART	      "LNX"
#define CONFIG_CMD_BOOTA_RECOVERY_PART	  "SOS"
#define CONFIG_CMD_BOOTA_DT_PART	      "DTB"
#define CONFIG_ANDROID_DT_HDR_BUFF	      (NV_PA_SDRAM_BASE + 0x03000000)
#define CONFIG_ANDROID_BOOT_HDR_BUFF	  (NV_PA_SDRAM_BASE + 0x04000000)
#define BOARD_EXTRA_ENV_SETTINGS \
	"fastboot_partition_alias_boot=LNX\0" \
	"fastboot_partition_alias_boot=DTB\0" \
	"fastboot_partition_alias_recovery=SOS\0" \
	"fastboot_partition_alias_system=APP\0" \
	"fastboot_partition_alias_cache=CAC\0" \
	"fastboot_partition_alias_misc=MSC\0" \
	"fastboot_partition_alias_factory=FCT\0" \
	"fastboot_partition_alias_userdata=UDA\0" \
	"fastboot_partition_alias_vendor=VNR\0" \
	"bootargs_append=" \
	"earlyprintk init=/init console=ttyS0,115200n8 " \
	"lp0_vec=2064@0xf46ff000 mem=1862M@2048M vpr=151M@3945M tsec=32M@3913M " \
	"core_edp_mv=1150 core_edp_ma=4000 " \
	"tegraid=40.1.1.0.0 tegra_fbmem=32899072@0xad012000 fbcon=map:1 " \
	"video=tegrafb memtype=255 ddr_die=2048M@2048M section=256M " \
	"debug_uartport=lsport,3 " \
	"power_supply=Adapter audio_codec=sgtl5000 gpt " \
	"usbcore.old_scheme_first=1 usb_port_owner_info=0 " \
	"lane_owner_info=6 emc_max_dvfs=0 " \
	"pmuboard=0x0177:0x0000:0x02:0x43:0x00 " \
	"otf_key=c75e5bb91eb3bd947560357b64422f85 " \
	"board_info=0x0177:0x0000:0x02:0x43:0x00\0"

#include "tegra-common-usb-gadget.h"
#include "tegra-common-post.h"

#endif /* __CONFIG_H */
