/*
 * Copyright (c) 2012-2016 Toradex, Inc.
 *
 * Configuration settings for the Toradex Apalis TK1 modules.
 *
 * SPDX-License-Identifier:	GPL-2.0+
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

#define CONFIG_OF_LIBFDT
#define CONFIG_OF_BOARD_SETUP
#define CONFIG_SERIAL_TAG
#define CONFIG_TEGRA_SERIAL_HIGH	0x01770000
#define CONFIG_TEGRA_SERIAL_LOW		0x034200FF

/* Use memory controller SDRAM size instead of ODMDATA */
#define CONFIG_TEGRA_USE_EMC_DRAM_SIZE

/* High-level configuration options */
#define V_PROMPT			"Apalis TK1 # "
#define CONFIG_CUSTOM_BOARDINFO		/* not from device-tree model node */
#undef CONFIG_DISPLAY_BOARDINFO
#define CONFIG_DISPLAY_BOARDINFO_LATE

/* Board-specific serial config */
#define CONFIG_SERIAL_MULTI
#define CONFIG_TEGRA_ENABLE_UARTA
#define CONFIG_SYS_NS16550_COM1		NV_PA_APB_UARTA_BASE

/* I2C */
#define CONFIG_SYS_I2C_TEGRA
#define CONFIG_CMD_I2C

/* Enable gpt partition table */
#define CONFIG_PARTITION_UUIDS
#define CONFIG_RANDOM_UUID
#define CONFIG_CMD_UUID
#define CONFIG_CMD_GPT
#define CONFIG_EFI_PARTITION
#define CONFIG_FS_EXT4
#define CONFIG_CMD_FS_GENERIC

/* SD/MMC support */
#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_TEGRA_MMC
#define CONFIG_CMD_MMC

/* Environment in eMMC, at the end of 2nd "boot sector" */
#define CONFIG_ENV_IS_IN_MMC
#define CONFIG_ENV_OFFSET		(-CONFIG_ENV_SIZE)
#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_SYS_MMC_ENV_PART		2

/* USB host support */
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_TEGRA
#define CONFIG_USB_MAX_CONTROLLER_COUNT	3
#define CONFIG_USB_STORAGE
#define CONFIG_CMD_USB

/* PCI host support */
#define CONFIG_PCI
#undef CONFIG_PCI_SCAN_SHOW
#define CONFIG_PCI_TEGRA
#define CONFIG_PCI_PNP
#define CONFIG_CMD_PCI
#define CONFIG_CMD_PCI_ENUM

/* PCI networking support */
#define CONFIG_E1000
#define CONFIG_E1000_NO_NVM

/* General networking support */
#define CONFIG_CMD_NET
#define CONFIG_CMD_DHCP

/* Android fastboot support */
#define CONFIG_CMD_FASTBOOT
#define CONFIG_FASTBOOT_FLASH
#define CONFIG_FASTBOOT_GPT_NAME	0
#define CONFIG_FASTBOOT_FLASH_MMC_DEV	0
#define CONFIG_USB_FASTBOOT_BUF_SIZE	  0x60000000
#define CONFIG_USB_FASTBOOT_BUF_ADDR	  (NV_PA_SDRAM_BASE + 0x10000000)

/* Android bootimg support */
#define CONFIG_ANDROID_BOOT_IMAGE
#define CONFIG_CMD_BOOTA
#define CONFIG_CMD_BOOTA_BOOT_PART	      "LNX"
#define CONFIG_CMD_BOOTA_RECOVERY_PART	      "SOS"
#define CONFIG_CMD_BOOTA_DT_PART	      "DTB"
#define CONFIG_ANDROID_DT_HDR_BUFF	      (NV_PA_SDRAM_BASE + 0x03000000)
#define CONFIG_ANDROID_BOOT_HDR_BUFF	  (NV_PA_SDRAM_BASE + 0x04000000)
#define BOARD_EXTRA_ENV_SETTINGS \
	"bootargs_append=" \
	"init=init console=ttyS0,115200n8 " \
	"lp0_vec=2064@0xf46ff000 mem=1862M@2048M vpr=151M@3945M tsec=32M@3913M " \
	"core_edp_mv=1150 core_edp_ma=4000 " \
	"tegraid=40.1.1.0.0 tegra_fbmem=32899072@0xad012000 fbcon=map:1 " \
	"video=tegrafb memtype=255 ddr_die=2048M@2048M section=256M " \
	"debug_uartport=lsport,0 " \
	"power_supply=Adapter audio_codec=sgtl5000 gpt " \
	"usbcore.old_scheme_first=1 usb_port_owner_info=2 " \
	"lane_owner_info=6 emc_max_dvfs=0 "

/* Increase console I/O buffer size */
#undef CONFIG_SYS_CBSIZE
#define CONFIG_SYS_CBSIZE		1024

/* Increase arguments buffer size */
#undef CONFIG_SYS_BARGSIZE
#define CONFIG_SYS_BARGSIZE CONFIG_SYS_CBSIZE

/* Increase print buffer size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)

/* Increase maximum number of arguments */
#undef CONFIG_SYS_MAXARGS
#define CONFIG_SYS_MAXARGS		32

#define CONFIG_CMD_TIME
#define CONFIG_CMD_MEMTEST
#define CONFIG_SYS_ALT_MEMTEST

#define CONFIG_SUPPORT_RAW_INITRD
#define CONFIG_SYS_BOOT_RAMDISK_HIGH

#include "tegra-common-usb-gadget.h"
#include "tegra-common-post.h"

#endif /* __CONFIG_H */
