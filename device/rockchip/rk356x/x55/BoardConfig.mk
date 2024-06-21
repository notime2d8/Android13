#
# Copyright 2014 The Android Open-Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
include device/rockchip/rk356x/BoardConfig.mk
BUILD_WITH_GO_OPT := false

# AB image definition
BOARD_USES_AB_IMAGE := false
BOARD_ROCKCHIP_VIRTUAL_AB_ENABLE := false

ifeq ($(strip $(BOARD_USES_AB_IMAGE)), true)
    include device/rockchip/common/BoardConfig_AB.mk
    TARGET_RECOVERY_FSTAB := device/rockchip/rk356x/x55/recovery.fstab_AB
endif

TARGET_RECOVERY_DEFAULT_ROTATION := ROTATION_LEFT

PRODUCT_UBOOT_CONFIG := rk3566
PRODUCT_KERNEL_DTS := rk3566-powkiddy-x55

# used for fstab_generator, sdmmc controller address
PRODUCT_BOOT_DEVICE := fe310000.sdhci,fe2b0000.dwmmc,fe000000.dwmmc
PRODUCT_SDMMC_DEVICE := fe2b0000.dwmmc

BOARD_HAVE_BLUETOOTH_RTK := true
# Properties
TARGET_VENDOR_PROP += device/rockchip/rk356x/x55/vendor.prop

