# bootloader/pi-u-boot: android16-riscv

Changes made for the Android 16 (AOSP, riscv64) bring-up of the BananaPi BPI-F3 (SpacemiT K1) and the BananaPi BPI-SM10 (SpacemiT K3), on branch `android16-riscv`.

U-Boot splash screen from a raw "logo" partition.

## Changes

- **splash: support raw reads from MMC**: SPLASH_STORAGE_MMC with SPLASH_STORAGE_RAW reads the BMP from a block-aligned byte offset on the boot eMMC/SD, for layouts without a filesystem partition for the splash.
- **board: spacemit: k1-x: read the splash screen from the "logo" partition**: Android layouts have no bootfs filesystem partition, so the splash BMP is flashed raw to a "logo" GPT partition (fastboot flash logo logo.bmp) and read from its offset.
- **board: spacemit: k1-x: boot OP-TEE in an OpenSBI trusted domain**: k1-x-optee.dtsi (BananaPi F3 / k1-x_deb1 and MusePi Pro): OpenSBI trusted domain at 0x36000000 (32 MiB, no-map) that every hart enters first, an untrusted domain that boots U-Boot, and per-hart MPXY OP-TEE / request-forward channels. When tee.bin is staged in the source tree, u-boot.itb also carries it as a loadable so SPL loads it before OpenSBI.
- **board: spacemit: k1-x: pass the probed DRAM banks to OpenSBI**: The dts describes 2 GB at 0 and OP-TEE takes the non-secure memory from the DT it gets through OpenSBI, so Linux shared memory in the bank above 4 GB was rejected ("Bad arg address") and no TA session could be opened. SPL fixes up the memory node with the size read from the DDR controller.
- **tee: optee: support OP-TEE in an OpenSBI domain over SBI MPXY**: On RISC-V, OP-TEE runs in an OpenSBI trusted domain and is reached through the SBI MPXY extension (RPMI OP-TEE service group), as in the RISE Linux driver: method = "mpxy" sends the eight SMC-style registers on the boot hart's rpmi_optee channel. The function IDs and messages are the ARM SMC ones, so the rest of the driver is unchanged; the shared memory is released before the OS starts.
- **riscv: dts: k1-x: add the OP-TEE firmware node**: U-Boot's OP-TEE client (method mpxy) for the AVB root of trust.
- **boot: android: hand the AVB root of trust to OP-TEE**: After AVB, send the boot state, lock state, vbmeta digest and the hash of the vbmeta signing key to OP-TEE's boot_rot PTA, which keeps them for this boot for the KeyMint TA. Boot goes on if OP-TEE is missing.

## Notes

- OP-TEE client: `CONFIG_TEE`/`CONFIG_OPTEE` come from build-bootloaders' k1_android.config, without the AVB TA (`CONFIG_OPTEE_TA_AVB`): its storage needs RPMB, so AVB keeps reporting the device unlocked (ORANGE) and the root of trust says so.
- OP-TEE: `arch/riscv/dts/k1-x-optee.dtsi` (included by k1-x_deb1 / BananaPi F3 and k1-x_MUSE-Pi-Pro) declares the OpenSBI trusted (OP-TEE, 0x36000000, 32 MiB no-map) and untrusted (U-Boot) domains and the per-hart MPXY channels. When build-bootloaders stages `tee.bin` in the source tree, `u-boot.itb` carries it as an extra loadable.
- Flash the BMP with `fastboot flash logo logo.bmp` (partition in build-bootloaders config/partition_android.json).

## Build

In the Android tree this is `bootloader/spacemit/pi-u-boot` (local manifest `spacemit-sources.xml`):

```
./build.sh k1 --bootloader-only   # -> vendor/spacemit/{k1,musepi-pro}/bootloader
```
