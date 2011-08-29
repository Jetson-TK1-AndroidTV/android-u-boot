/*
 * Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

#include <common.h>
#include <command.h>
#include <lcd.h>
#include <chromeos/common.h>
#include <chromeos/crossystem_data.h>
#include <chromeos/firmware_storage.h>
#include <chromeos/gbb_bmpblk.h>
#include <chromeos/gpio.h>
#include <chromeos/load_kernel_helper.h>
#include <chromeos/os_storage.h>
#include <chromeos/power_management.h>
#include <chromeos/vboot_nvstorage_helper.h>

#include <boot_device.h>
#include <bmpblk_header.h>
#include <load_kernel_fw.h>
#include <tlcl_stub.h>
#include <vboot_nvstorage.h>
#include <vboot_struct.h>

#define PREFIX "cros_onestop_firmware: "

#define WAIT_MS_BETWEEN_PROBING	400
#define WAIT_MS_SHOW_ERROR	2000

/* this value has to be different to all of VBNV_RECOVERY_* */
#define REBOOT_TO_CURRENT_MODE	~0U

enum {
	KEY_BOOT_INTERNAL = 0x04, /* ctrl-d */
	KEY_BOOT_EXTERNAL = 0x15, /* ctrl-u */
	KEY_GOTO_RECOVERY = 0x1b, /* escape */
	KEY_COMMAND_PROMPT = 0x03, /* ctrl-c */

	/* To do - discuss bringing this into vboot officially */
	VBNV_COMMAND_PROMPT = 0xfe,
};

static struct internal_state_t {
	firmware_storage_t file;
	VbNvContext nvcxt;
	VbSharedDataHeader *shared;
	VbKeyBlockHeader *key_block;
	uint64_t boot_flags;
	uint32_t recovery_request;
	ScreenIndex current_screen;
	crossystem_data_t cdata;
	uint8_t gbb_data[CONFIG_LENGTH_GBB];
	uint8_t key_block_buffer[CONFIG_LENGTH_VBLOCK_A];
} _state;

/**
 * This loads VbNvContext to internal state. The caller has to initialize
 * internal mmc device first.
 *
 * @return 0 if it succeeds; 1 if it fails
 */
static uint32_t init_internal_state_nvcontext(VbNvContext *nvcxt,
			uint32_t *recovery_request)
{
	if (read_nvcontext(nvcxt)) {
		VBDEBUG(PREFIX "fail to read nvcontext\n");
		return 1;
	}
	VBDEBUG(PREFIX "nvcxt: %s\n", nvcontext_to_str(nvcxt));

	if (VbNvGet(nvcxt, VBNV_RECOVERY_REQUEST,
				recovery_request)) {
		VBDEBUG(PREFIX "fail to read recovery request\n");
		return 1;
	}
	VBDEBUG(PREFIX "recovery_request = 0x%x\n", *recovery_request);

	/*
	 * we have to clean recovery request once we have it so that we will
	 * not be trapped in recovery boot
	 */
	if (VbNvSet(nvcxt, VBNV_RECOVERY_REQUEST,
				VBNV_RECOVERY_NOT_REQUESTED)) {
		VBDEBUG(PREFIX "fail to clear recovery request\n");
		return 1;
	}
	if (VbNvTeardown(nvcxt)) {
		VBDEBUG(PREFIX "fail to tear down nvcontext\n");
		return 1;
	}
	if (nvcxt->raw_changed && write_nvcontext(nvcxt)) {
		VBDEBUG(PREFIX "fail to write back nvcontext\n");
		return 1;
	}

	return 0;
}

/**
 * This initializes crossystem data blob after checking the GPIOs for
 * recovery/developer. The caller has to initialize:
 *   firmware storage device
 *   gbb
 *   nvcontext
 * before calling this function.
 *
 * @return VBNV_RECOVERY_NOT_REQUESTED if it succeeds; recovery reason if it
 *         fails
 */
static uint32_t init_internal_state_bottom_half(crossystem_data_t *cdata,
		int *dev_mode)
{
	char frid[ID_LEN];
	int write_protect_sw, recovery_sw, developer_sw;

	/* fetch gpios at once */
	write_protect_sw = is_firmware_write_protect_gpio_asserted();
	recovery_sw = is_recovery_mode_gpio_asserted();
	developer_sw = is_developer_mode_gpio_asserted();
	if (developer_sw) {
		_state.boot_flags |= BOOT_FLAG_DEVELOPER;
		_state.boot_flags |= BOOT_FLAG_DEV_FIRMWARE;
		*dev_mode = 1;
	}
	if (recovery_sw) {
		_state.boot_flags |= BOOT_FLAG_RECOVERY;
		_state.recovery_request = VBNV_RECOVERY_RO_MANUAL;
	}
	if (firmware_storage_read(&_state.file, CONFIG_OFFSET_RO_FRID,
				CONFIG_LENGTH_RO_FRID, frid)) {
		VBDEBUG(PREFIX "read frid fail\n");
		return VBNV_RECOVERY_RO_SHARED_DATA;
	}

	if (crossystem_data_init(cdata, frid, CONFIG_OFFSET_FMAP,
				_state.gbb_data, _state.nvcxt.raw,
				write_protect_sw, recovery_sw, developer_sw)) {
		VBDEBUG(PREFIX "init crossystem data fail\n");
		return VBNV_RECOVERY_RO_UNSPECIFIED;
	}

	return VBNV_RECOVERY_NOT_REQUESTED;
}

/**
 * This initializes global variable [_state] and crossystem data.
 *
 * @param cdata		pointer to kernel shared data
 * @param dev_mode	set to 1 if in developer mode, 0 if not
 * @return VBNV_RECOVERY_NOT_REQUESTED if it succeeds; recovery reason if it
 *         fails
 */
static uint32_t init_internal_state(crossystem_data_t *cdata, int *dev_mode,
		struct os_storage *oss)
{
	uint32_t reason;

	*dev_mode = 0;

	/* sad enough, SCREEN_BLANK != 0 */
	_state.current_screen = SCREEN_BLANK;

	_state.shared = (VbSharedDataHeader *)cdata->vbshared_data;
	_state.key_block = (VbKeyBlockHeader *)_state.key_block_buffer;

	/* open firmware storage device and load gbb */
	if (firmware_storage_open_spi(&_state.file)) {
		VBDEBUG(PREFIX "open_spi fail\n");
		return VBNV_RECOVERY_RO_SHARED_DATA;
	}
	if (firmware_storage_read(&_state.file,
				CONFIG_OFFSET_GBB, CONFIG_LENGTH_GBB,
				_state.gbb_data)) {
		VBDEBUG(PREFIX "fail to read gbb\n");
		return VBNV_RECOVERY_RO_SHARED_DATA;
	}

	/* initialize mmc and load nvcontext */
	if (os_storage_initialize_mmc_device(&_state.os_storage,
			MMC_INTERNAL_DEVICE)) {
		VBDEBUG(PREFIX "mmc %d init fail\n", MMC_INTERNAL_DEVICE);
		return VBNV_RECOVERY_RO_UNSPECIFIED;
	}
	if (init_internal_state_nvcontext(&_state.nvcxt,
			&_state.recovery_request)) {
		VBDEBUG(PREFIX "fail to load nvcontext\n");
		return VBNV_RECOVERY_RO_UNSPECIFIED;
	}

	reason = init_internal_state_bottom_half(cdata, dev_mode);
	if (reason != VBNV_RECOVERY_NOT_REQUESTED) {
		VBDEBUG(PREFIX "init cdata fail\n");
		return reason;
	}

	VBDEBUG(PREFIX "boot_flags = 0x%llx\n", _state.boot_flags);

	return VBNV_RECOVERY_NOT_REQUESTED;
}

/* forward declare a (private) struct of vboot_reference */
struct RSAPublicKey;

/*
 * We copy declaration of private functions of vboot_reference here so that we
 * can initialize VbSharedData without actually calling to LoadFirmware.
 */
int VbSharedDataInit(VbSharedDataHeader *header, uint64_t size);
int VbSharedDataSetKernelKey(VbSharedDataHeader *header,
		const VbPublicKey *src);
int VerifyFirmwarePreamble(const VbFirmwarePreambleHeader *preamble,
		uint64_t size, const struct RSAPublicKey *key);
struct RSAPublicKey *PublicKeyToRSA(const VbPublicKey *key);

/**
 * This loads kernel subkey from rewritable preamble A.
 *
 * @param vblock - a buffer large enough for holding verification block A
 * @return VBNV_RECOVERY_NOT_REQUESTED if it succeeds; recovery reason if it
 *         fails
 */
static uint32_t load_kernel_subkey_a(VbKeyBlockHeader *key_block)
{
	VbFirmwarePreambleHeader *preamble;
	struct RSAPublicKey *data_key;

	VBDEBUG(PREFIX "reading kernel subkey A\n");
	if (firmware_storage_read(&_state.file,
				CONFIG_OFFSET_VBLOCK_A, CONFIG_LENGTH_VBLOCK_A,
				key_block)) {
		VBDEBUG(PREFIX "read verification block fail\n");
		return VBNV_RECOVERY_RO_SHARED_DATA;
	}

	data_key = PublicKeyToRSA(&key_block->data_key);
	if (!data_key) {
		VBDEBUG(PREFIX "parse data key fail\n");
		return VBNV_RECOVERY_RO_SHARED_DATA;
	}

	preamble = (VbFirmwarePreambleHeader *)((char *)key_block +
			key_block->key_block_size);
	if (VerifyFirmwarePreamble(preamble, CONFIG_LENGTH_VBLOCK_A -
				key_block->key_block_size, data_key)) {
		VBDEBUG(PREFIX "verify preamble fail\n");
		return VBNV_RECOVERY_RO_INVALID_RW;
	}

	if (VbSharedDataSetKernelKey(_state.shared, &preamble->kernel_subkey)) {
		VBDEBUG(PREFIX "unable to save kernel subkey\n");
		return VBNV_RECOVERY_RO_INVALID_RW;
	}

	return VBNV_RECOVERY_NOT_REQUESTED;
}

/**
 * This initializes VbSharedData that is used in normal and developer boot.
 *
 * @return VBNV_RECOVERY_NOT_REQUESTED if it succeeds; recovery reason if it
 *         fails
 */
static uint32_t init_vbshared_data(int dev_mode)
{
	/*
	 * This function is adapted from LoadFirmware(). The differences
	 * between the two include:
	 *
	 *   We always pretend that we "boot" from firmware A, assuming that
	 *   you have the right kernel subkey there. We also fill in respective
	 *   fields with dummy values.
	 *
	 *   We set zero to these fields of VbSharedDataHeader
	 *     fw_version_tpm_start
	 *     fw_version_lowest
	 *     fw_version_tpm
	 *
	 *    We do not call VbGetTimer() because we have conflicting symbols
	 *    when include utility.h.
	 */
	if (VbSharedDataInit(_state.shared, VB_SHARED_DATA_REC_SIZE)) {
		VBDEBUG(PREFIX "VbSharedDataInit fail\n");
		return VBNV_RECOVERY_RO_SHARED_DATA;
	}

	if (dev_mode)
		_state.shared->flags |= VBSD_LF_DEV_SWITCH_ON;

	VbNvSet(&_state.nvcxt, VBNV_TRY_B_COUNT, 0);
	_state.shared->check_fw_a_result = VBSD_LF_CHECK_VALID;
	_state.shared->firmware_index = 0;

	return load_kernel_subkey_a(_state.key_block);
}

static void beep(void)
{
	/* TODO: implement beep */
	VBDEBUG(PREFIX "beep\n");
}

static void clear_screen(void)
{
	if (lcd_clear())
		VBDEBUG(PREFIX "fail to clear screen\n");
	else
		_state.current_screen = SCREEN_BLANK;
}

static void show_screen(ScreenIndex screen)
{
	VBDEBUG(PREFIX "show_screen: %d\n", (int) screen);

	if (_state.current_screen == screen)
		return;

	if (display_screen_in_bmpblk(_state.gbb_data, screen))
		VBDEBUG(PREFIX "fail to display screen\n");
	else
		_state.current_screen = screen;
}

/**
 * This is a boot_kernel wrapper function. It only returns to its caller when
 * it fails to boot kernel, as boot_kernel does.
 *
 * @return recvoery reason or REBOOT_TO_CURRENT_MODE
 */
static uint32_t boot_kernel_helper(struct os_storage *oss)
{
	int status;

	crossystem_data_dump(&_state.cdata);

	status = boot_kernel(oss, _state.boot_flags,
			_state.gbb_data, CONFIG_LENGTH_GBB,
			_state.shared, VB_SHARED_DATA_REC_SIZE,
			&_state.nvcxt, &_state.cdata);

	switch(status) {
	case LOAD_KERNEL_NOT_FOUND:
		/* no kernel found on device */
		return VBNV_RECOVERY_RW_NO_OS;
	case LOAD_KERNEL_INVALID:
		/* only invalid kernels found on device */
		return VBNV_RECOVERY_RW_INVALID_OS;
	case LOAD_KERNEL_RECOVERY:
		/* internal error; reboot to recovery mode */
		return VBNV_RECOVERY_RW_UNSPECIFIED;
	case LOAD_KERNEL_REBOOT:
		/* internal error; reboot to current mode */
		return REBOOT_TO_CURRENT_MODE;
	}

	return VBNV_RECOVERY_RW_UNSPECIFIED;
}

/**
 * This is the main entry point of recovery boot flow. It never returns to its
 * caller.
 *
 * @param oss		OS storage interface
 * @param cdata		kernel shared data pointer
 * @param reason	recovery reason
 * @param wait_for_unplug wait until user unplugs SD/USB before continuing
 */
static void recovery_boot(struct os_storage *oss, crossystem_data_t *cdata,
		uint32_t reason, int wait_for_unplug)
{
	VBDEBUG(PREFIX "recovery boot\n");

	/*
	 * TODO:
	 * 1. write_log()
	 * 2. test_clear_mem_regions()
	 * 3. clear_ram_not_in_use()
	 */
	crossystem_data_set_active_main_firmware(cdata, RECOVERY_FIRMWARE,
			RECOVERY_TYPE);
	crossystem_data_set_recovery_reason(cdata, reason);

	if (wait_for_unplug) {
		/* wait user unplugging external storage device */
		while (os_storage_is_any_storage_device_plugged(
				oss, NOT_BOOT_PROBED_DEVICE)) {
			show_screen(SCREEN_RECOVERY_MODE);
			udelay(WAIT_MS_BETWEEN_PROBING * 1000);
		}
	}

	for (;;) {
		/* Wait for user to plug in SD card or USB storage device */
		while (!os_storage_is_any_storage_device_plugged(
				oss, BOOT_PROBED_DEVICE)) {
			show_screen(SCREEN_RECOVERY_MISSING_OS);
			udelay(WAIT_MS_BETWEEN_PROBING * 1000);
		}

		clear_screen();

		/* even if it fails, we simply don't care */
		boot_kernel_helper(oss);

		while (os_storage_is_any_storage_device_plugged(
				oss, NOT_BOOT_PROBED_DEVICE)) {
			show_screen(SCREEN_RECOVERY_NO_OS);
			udelay(WAIT_MS_SHOW_ERROR * 1000);
		}
	}
}

/**
 * This initializes TPM and crossystem data blob (by pretending it boots
 * rewritable firmware A).
 *
 * @return VBNV_RECOVERY_NOT_REQUESTED if it succeeds; recovery reason if it
 *         fails
 */
static uint32_t rewritable_boot_init(crossystem_data_t *cdata, int boot_type)
{
	char fwid[ID_LEN];

	/* we pretend we boot from r/w firmware A */
	if (firmware_storage_read(&_state.file, CONFIG_OFFSET_RW_FWID_A,
				CONFIG_LENGTH_RW_FWID_A, fwid)) {
		VBDEBUG(PREFIX "read fwid_a fail\n");
		return VBNV_RECOVERY_RW_SHARED_DATA;
	}

	crossystem_data_set_fwid(cdata, fwid);
	crossystem_data_set_active_main_firmware(cdata, REWRITABLE_FIRMWARE_A,
			boot_type);

	if (TlclStubInit() != TPM_SUCCESS) {
		VBDEBUG(PREFIX "fail to init tpm\n");
		return VBNV_RECOVERY_RW_TPM_ERROR;
	}

	return VBNV_RECOVERY_NOT_REQUESTED;
}

/**
 * This is the main entry point of developer boot flow.
 *
 * @return VBNV_RECOVERY_NOT_REQUESTED when caller has to boot from internal
 *         storage device; recovery reason when caller has to go to recovery.
 */
static uint32_t developer_boot(struct os_storage *oss)
{
	ulong start = 0, time = 0, last_time = 0;
	int c, is_after_20_seconds = 0;

	VBDEBUG(PREFIX "developer_boot\n");
	show_screen(SCREEN_DEVELOPER_MODE);

	start = get_timer(0);
	while (1) {
		udelay(100);

		last_time = time;
		time = get_timer(start);
#ifdef DEBUG
		/* only output when time advances at least 1 second */
		if (time / CONFIG_SYS_HZ > last_time / CONFIG_SYS_HZ)
			VBDEBUG(PREFIX "time ~ %d sec\n", time / CONFIG_SYS_HZ);
#endif

		/* beep twice when time > 20 seconds */
		if (!is_after_20_seconds && time > 20 * CONFIG_SYS_HZ) {
			beep(); udelay(500); beep();
			is_after_20_seconds = 1;
			continue;
		}

		/* boot from internal storage after 30 seconds */
		if (time > 30 * CONFIG_SYS_HZ)
			break;

		if (!tstc())
			continue;
		c = getc();
		VBDEBUG(PREFIX "getc() == 0x%x\n", c);

		switch (c) {
		/* boot from internal storage */
		case KEY_BOOT_INTERNAL:
			return VBNV_RECOVERY_NOT_REQUESTED;

		/* load and boot kernel from USB or SD card */
		case KEY_BOOT_EXTERNAL:
			/* even if boot_kernel_helper fails, we don't care */
			if (os_storage_is_any_storage_device_plugged(
					oss, BOOT_PROBED_DEVICE))
				boot_kernel_helper(oss);
			beep();
			break;

		case KEY_COMMAND_PROMPT:
			/* Load developer environment variables */
			env_relocate();
			return VBNV_COMMAND_PROMPT;

		case ' ':
		case '\r':
		case '\n':
		case KEY_GOTO_RECOVERY:
			VBDEBUG(PREFIX "goto recovery\n");
			return VBNV_RECOVERY_RW_DEV_SCREEN;
		}
	}

	return VBNV_RECOVERY_NOT_REQUESTED;
}

/**
 * This is the main entry point of normal boot flow. It never returns
 * VBNV_RECOVERY_NOT_REQUESTED because if it returns, the caller always has to
 * go to recovery (or reboot).
 *
 * @return recovery reason or REBOOT_TO_CURRENT_MODE
 */
static uint32_t normal_boot(struct os_storage *oss)
{
	VBDEBUG(PREFIX "boot from internal storage\n");

	/* TODO(sjg) get from fdt */
	if (os_storage_set_bootdev(oss, "mmc", MMC_INTERNAL_DEVICE, 0)) {
		VBDEBUG(PREFIX "set_bootdev mmc_internal_device fail\n");
		return VBNV_RECOVERY_RW_NO_OS;
	}

	return boot_kernel_helper(oss);
}

/**
 * This is the main entry point of onestop firmware. It will generally do
 * a normal boot, but if it returns to its caller, the caller should enter
 * recovery or reboot.
 *
 * @param cdata	Pointer to crossystem data
 * @return required action for caller:
 *	REBOOT_TO_CURRENT_MODE	Reboot
 *	anything else		Go into recovery
 */
static unsigned onestop_boot(struct os_storage *oss, crossystem_data_t *cdata)
{
	unsigned reason = VBNV_RECOVERY_NOT_REQUESTED;
	int dev_mode;

	/* Work through our initialization one step at a time */
	reason = init_internal_state(cdata, &dev_mode, oss);
	if (reason == VBNV_RECOVERY_NOT_REQUESTED)
		reason = _state.recovery_request;

	if (reason == VBNV_RECOVERY_NOT_REQUESTED)
		reason = init_vbshared_data(dev_mode);

	if (reason == VBNV_RECOVERY_NOT_REQUESTED)
		reason = rewritable_boot_init(cdata, dev_mode ?
			DEVELOPER_TYPE : NORMAL_TYPE);

	if (reason == VBNV_RECOVERY_NOT_REQUESTED) {
		if (dev_mode)
			reason = developer_boot(oss);

		/*
		* If developer boot flow exits normally or is not requested,
		* try normal boot flow.
		*/
		if (reason == VBNV_RECOVERY_NOT_REQUESTED)
			reason = normal_boot(oss);
	}

	/* Give up and fall through to recovery */
	return reason;
}

int do_cros_onestop_firmware(cmd_tbl_t *cmdtp, int flag, int argc,
		char * const argv[])
{
	unsigned reason;
	struct os_storage os_storage;

	clear_screen();
	reason = onestop_boot(&os_storage, &_state.cdata);
	if (reason == VBNV_COMMAND_PROMPT)
		return 0;
	if (reason != REBOOT_TO_CURRENT_MODE)
		recovery_boot(&os_storage, &_state.cdata, reason,
			      _state.boot_flags & BOOT_FLAG_DEVELOPER);
	cold_reboot();
	return 0;
}

U_BOOT_CMD(cros_onestop_firmware, 1, 1, do_cros_onestop_firmware,
		"verified boot onestop firmware", NULL);
