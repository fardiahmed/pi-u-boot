// SPDX-License-Identifier: GPL-2.0+
/* OP-TEE calls over the RISC-V SBI MPXY extension (RPMI OP-TEE service group, OpenSBI domains) */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <asm/global_data.h>
#include <asm/sbi.h>
#include <dm/ofnode.h>
#include <linux/arm-smccc.h>
#include <linux/errno.h>
#include <linux/string.h>
#include "optee_msg.h"
#include "optee_private.h"
#include "optee_smc.h"

DECLARE_GLOBAL_DATA_PTR;

#define SBI_EXT_MPXY			0x4D505859
#define SBI_EXT_MPXY_GET_SHMEM_SIZE	0
#define SBI_EXT_MPXY_SET_SHMEM		1
#define SBI_EXT_MPXY_SEND_MSG_WITH_RESP	5
#define RPMI_OPTEE_SRV_COMMUNICATE	3

/* The SMC-style registers, as the Linux driver sends them */
struct mpxy_optee_msg {
	unsigned long a[8];
};

static void *mpxy_shmem;
static u32 mpxy_channel;

/* The rpmi_optee node of the boot hart's cpu node gives its OP-TEE channel */
static int mpxy_find_channel(u32 *channel)
{
	ofnode cpus = ofnode_path("/cpus");
	ofnode cpu, node;
	u32 hart;

	if (!ofnode_valid(cpus))
		return -ENODEV;
	ofnode_for_each_subnode(cpu, cpus) {
		if (ofnode_read_u32(cpu, "reg", &hart) || hart != gd->arch.boot_hart)
			continue;
		ofnode_for_each_subnode(node, cpu) {
			if (ofnode_device_is_compatible(node, "riscv,sbi-mpxy-optee"))
				return ofnode_read_u32(node, "riscv,sbi-mpxy-channel-id",
						       channel);
		}
	}
	return -ENODEV;
}

int optee_mpxy_init(void)
{
	struct sbiret ret;
	unsigned long size;

	if (mpxy_shmem)
		return 0;
	if (mpxy_find_channel(&mpxy_channel))
		return -ENODEV;

	ret = sbi_ecall(SBI_EXT_MPXY, SBI_EXT_MPXY_GET_SHMEM_SIZE, 0, 0, 0, 0, 0, 0);
	if (ret.error)
		return -ENODEV;
	size = ret.value;
	if (size < sizeof(struct mpxy_optee_msg))
		return -EINVAL;

	mpxy_shmem = memalign(SZ_4K, ALIGN(size, SZ_4K));
	if (!mpxy_shmem)
		return -ENOMEM;
	memset(mpxy_shmem, 0, size);
	/* Overwrite mode (flags 0); Linux sets its own page the same way */
	ret = sbi_ecall(SBI_EXT_MPXY, SBI_EXT_MPXY_SET_SHMEM,
			lower_32_bits((ulong)mpxy_shmem),
			upper_32_bits((ulong)mpxy_shmem), 0, 0, 0, 0);
	if (ret.error) {
		free(mpxy_shmem);
		mpxy_shmem = NULL;
		return -EIO;
	}
	debug("optee: MPXY channel %u\n", mpxy_channel);
	return 0;
}

void optee_mpxy_exit(void)
{
	if (!mpxy_shmem)
		return;
	/* All-ones address: stop using the shared memory before the OS reuses it */
	sbi_ecall(SBI_EXT_MPXY, SBI_EXT_MPXY_SET_SHMEM, -1UL, -1UL, 0, 0, 0, 0);
	free(mpxy_shmem);
	mpxy_shmem = NULL;
}

void optee_mpxy_invoke(unsigned long a0, unsigned long a1, unsigned long a2,
		       unsigned long a3, unsigned long a4, unsigned long a5,
		       unsigned long a6, unsigned long a7,
		       struct arm_smccc_res *res)
{
	struct mpxy_optee_msg msg = { .a = { a0, a1, a2, a3, a4, a5, a6, a7 } };
	struct sbiret ret;

	memset(res, 0, sizeof(*res));
	memcpy(mpxy_shmem, &msg, sizeof(msg));
	ret = sbi_ecall(SBI_EXT_MPXY, SBI_EXT_MPXY_SEND_MSG_WITH_RESP,
			mpxy_channel, RPMI_OPTEE_SRV_COMMUNICATE, sizeof(msg), 0, 0, 0);
	if (ret.error || ret.value > sizeof(msg)) {
		log_err("optee: MPXY message failed: %ld\n", ret.error);
		res->a0 = OPTEE_SMC_RETURN_UNKNOWN_FUNCTION;
		return;
	}
	memcpy(res, mpxy_shmem, min_t(ulong, ret.value, sizeof(*res)));
}
