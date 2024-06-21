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

CURRENT_SDK_VERSION := rk3528_ANDROID13.0_BOX_V1.0
TARGET_BOARD_PLATFORM_PRODUCT := atv

TARGET_ARCH := arm64
TARGET_ARCH_VARIANT := armv8-a
TARGET_CPU_ABI := arm64-v8a
TARGET_CPU_ABI2 :=
TARGET_CPU_VARIANT := generic
TARGET_CPU_VARIANT_RUNTIME := cortex-a53
TARGET_CPU_SMP := true

TARGET_2ND_ARCH := arm
TARGET_2ND_ARCH_VARIANT := armv8-a
TARGET_2ND_CPU_ABI := armeabi-v7a
TARGET_2ND_CPU_ABI2 := armeabi
TARGET_2ND_CPU_VARIANT := generic
TARGET_2ND_CPU_VARIANT_RUNTIME := cortex-a53

PRODUCT_KERNEL_VERSION := 5.10

PRODUCT_UBOOT_CONFIG ?= rk3528
PRODUCT_KERNEL_ARCH ?= arm64
#PRODUCT_KERNEL_DTS ?= rk3528-demo4-ddr4-v10
PRODUCT_KERNEL_DTS ?= rk3528-evb1-ddr4-v10
PRODUCT_KERNEL_CONFIG ?= rockchip_defconfig

# used for fstab_generator, sdmmc controller address
PRODUCT_BOOT_DEVICE := ffbf0000.mmc
PRODUCT_SDMMC_DEVICE := ffc30000.mmc

#BOARD_SUPER_PARTITION_SIZE := 4294967296
#BOARD_ROCKCHIP_DYNAMIC_PARTITIONS_SIZE := $(shell expr $(BOARD_SUPER_PARTITION_SIZE) - 4194304)


# Disable emulator for "make dist" until there is a 64-bit qemu kernel
BUILD_EMULATOR := false
TARGET_BOARD_PLATFORM := rk3528
TARGET_BOARD_PLATFORM_GPU := mali450
TARGET_RK_GRALLOC_VERSION := 1
CAMERA_WITHOUT_GRALLOC4 := true
BOARD_USE_DRM := true

# RenderScript
# OVERRIDE_RS_DRIVER := libnvRSDriver.so
BOARD_OVERRIDE_RS_CPU_VARIANT_32 := cortex-a53
BOARD_OVERRIDE_RS_CPU_VARIANT_64 := cortex-a53
# DISABLE_RS_64_BIT_DRIVER := true

TARGET_USES_64_BIT_BCMDHD := true
TARGET_USES_64_BIT_BINDER := true

# HACK: Build apps as 64b for volantis_64_only
ifneq (,$(filter ro.zygote=zygote64, $(PRODUCT_DEFAULT_PROPERTY_OVERRIDES)))
TARGET_PREFER_32_BIT_APPS :=
TARGET_SUPPORTS_64_BIT_APPS := true
endif

ENABLE_CPUSETS := true

# Enable Dex compile opt as default
WITH_DEXPREOPT := true

BOARD_NFC_SUPPORT := false
BOARD_HAS_GPS := false

BOARD_GRAVITY_SENSOR_SUPPORT := false
BOARD_COMPASS_SENSOR_SUPPORT := false
BOARD_GYROSCOPE_SENSOR_SUPPORT := false
BOARD_PROXIMITY_SENSOR_SUPPORT := false
BOARD_LIGHT_SENSOR_SUPPORT := false
BOARD_PRESSURE_SENSOR_SUPPORT := false
BOARD_TEMPERATURE_SENSOR_SUPPORT := false
BOARD_USB_HOST_SUPPORT := true
BOARD_USER_FAKETOUCH := false

PRODUCT_HAVE_RKAPPS := true

# for optee support
PRODUCT_HAVE_OPTEE := true

BOARD_USE_SPARSE_SYSTEM_IMAGE := true

# Google Service and frp overlay
BUILD_WITH_GOOGLE_MARKET := false
BUILD_WITH_GOOGLE_MARKET_ALL := false
BUILD_WITH_GOOGLE_FRP := false
BUILD_WITH_GOOGLE_GMS_EXPRESS := false

# for widevine drm
BOARD_WIDEVINE_OEMCRYPTO_LEVEL := 3

# camera enable
BOARD_CAMERA_SUPPORT := true
BOARD_CAMERA_SUPPORT_EXT := true
ALLOW_MISSING_DEPENDENCIES=true
# Config GO Optimization
BUILD_WITH_GO_OPT := true

# enable SVELTE malloc
MALLOC_SVELTE := true

#Config omx to support codec type.
BOARD_SUPPORT_VP9 := true
BOARD_SUPPORT_VP6 := false
BOARD_SUPPORT_HEVC_ENC := true

# rktoolbox
BOARD_WITH_RKTOOLBOX := false
BOARD_MEMTRACK_SUPPORT := false

# Allow deprecated BUILD_ module types to get DDK building
BUILD_BROKEN_USES_BUILD_COPY_HEADERS := true
BUILD_BROKEN_USES_BUILD_HOST_EXECUTABLE := true
BUILD_BROKEN_USES_BUILD_HOST_SHARED_LIBRARY := true
BUILD_BROKEN_USES_BUILD_HOST_STATIC_LIBRARY := true

BOARD_SHOW_HDMI_SETTING ?= true
BOARD_SUPPORT_HDMI_CEC := false

# for dynamaic afbc target 
BOARD_HS_DYNAMIC_AFBC_TARGET := false

#trust is merging into uboot
BOARD_ROCKCHIP_TRUST_MERGE_TO_UBOOT := true

BOARD_BASEPARAMETER_SUPPORT := true
