# A16_RISCV/bootloader/spacemit/k3-u-boot: android16-riscv-k3

BayLibre's K3 U-Boot (upstream U-Boot 2026.07 + SpacemiT K3, branch `v2026.07-k3`) with the
Banana Pi BPI-SM10 (SpacemiT K3-CoM260 module on the CoM260 kit V02 carrier) added, for the
Android 16 riscv64 tree. The K1 boards keep `bootloader/spacemit/pi-u-boot`.

## Changes

- **riscv: dts: spacemit: k3: add i2c0**: TWSI0 and its pads 0/1; the CoM260 carrier has its FUSB301 Type-C controller there.
- **board: spacemit: k3: make the default product name configurable**: CONFIG_SPACEMIT_K3_DEFAULT_PRODUCT_NAME (default "k3-pico-itx") names the ESOS dtb the RCPUs use when the EEPROM has no product name.
- **riscv: dts: spacemit: add the Banana Pi BPI-SM10**: k3-bananapi-sm10.dts / -u-boot.dtsi: UART0, module SPI-NOR, UFS, USB-C download port, carrier EEPROM, SPM8821 PMIC and the kit's MPQ8655 CPU rail regulator, DP1; the FIT carries the com260_kit_v02 ESOS dtbs.

## Notes

- Built by build-bootloaders `config/boards/bananapi-sm10.yaml` (SpacemiT GCC 15.2 toolchain, `k3_android.config` + `k3_bananapi_sm10.config`), staged into `vendor/spacemit/k3/bootloader`.
- Not booted yet: the board has not arrived. The boot-select resistors of the carrier decide whether the BROM reads the FSBL from the module SPI-NOR (this layout) or from UFS.

## Build

```
./build.sh k3 --bootloader-only
```
