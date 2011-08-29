/*
 * Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __configs_chromeos_seaboard_onestop_h__
#define __configs_chromeos_seaboard_onestop_h__

#include <configs/chromeos_seaboard_common.h>

#define CONFIG_CHROMEOS_ONESTOP

/* TODO: Remove this flag; only need the wrapper APIs for now */
#define CONFIG_CHROMEOS_VBEXPORT

/* entry point of onestop firmware */
#define CONFIG_CMD_CROS_ONESTOP_FIRMWARE

#endif /* __configs_chromeos_seaboard_onestop_h__ */
