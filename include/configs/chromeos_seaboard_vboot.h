/*
 * Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __configs_chromeos_seaboard_vboot_h__
#define __configs_chromeos_seaboard_vboot_h__

#include <configs/chromeos_seaboard_common.h>

#define CONFIG_CHROMEOS_VBEXPORT
#define CONFIG_CHROMEOS_VBOOT

/* The VBoot Global Data is stored in a region just above the stack region. */
#define CONFIG_VBGLOBAL_BASE    (CONFIG_STACKBASE + CONFIG_STACKSIZE)

#define CONFIG_CMD_VBOOT_BOOTSTUB
#define CONFIG_CMD_VBOOT_MAIN

#define CONFIG_CMD_VBEXPORT_TEST
#define CONFIG_CMD_VBOOT_TEST

#endif /* __configs_chromeos_seaboard_vboot_h__ */
