/* SPDX-License-Identifier: BSD-2-Clause */
/* OP-TEE boot root of trust PTA (CFG_BOOT_ROT_PTA), as in optee_os lib/libutee/include */

#ifndef __TEE_PTA_BOOT_ROT_H
#define __TEE_PTA_BOOT_ROT_H

#include <linux/types.h>

#define PTA_BOOT_ROT_UUID { 0x368a3590, 0x0d0c, 0x400d, \
		{ 0x8b, 0xd5, 0x35, 0xcb, 0x37, 0x85, 0xe4, 0x35 } }

/* Same values as enum avb_boot_state and KeyMint VerifiedBoot */
#define PTA_BOOT_ROT_STATE_GREEN	0
#define PTA_BOOT_ROT_STATE_YELLOW	1
#define PTA_BOOT_ROT_STATE_ORANGE	2
#define PTA_BOOT_ROT_STATE_RED		3

struct pta_boot_rot {
	u8 verified_boot_key[32];	/* SHA-256 of the vbmeta public key, zeros if none */
	u8 verified_boot_hash[32];	/* SHA-256 digest of the vbmeta images */
	u32 device_locked;
	u32 verified_boot_state;
};

/* [in] memref[0]: struct pta_boot_rot. Normal world only, once per boot */
#define PTA_BOOT_ROT_CMD_SET		0

#endif /* __TEE_PTA_BOOT_ROT_H */
