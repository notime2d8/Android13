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
# This file is the build configuration for an aosp Android
# build for rockchip rk3528 hardware. This cleanly combines a set of
# device-specific aspects (drivers) with a device-agnostic
# product configuration (apps). Except for a few implementation
# details, it only fundamentally contains two inherit-product
# lines, aosp and rk3528, hence its name.

# First lunching is T, api_level is 33
PRODUCT_SHIPPING_API_LEVEL := 33
PRODUCT_DTBO_TEMPLATE := $(LOCAL_PATH)/dt-overlay.in

include device/rockchip/common/build/rockchip/DynamicPartitions.mk
include device/rockchip/rk3528/rk3528_box/BoardConfig.mk
include device/rockchip/common/BoardConfig.mk
# Inherit from those products. Most specific first.
$(call inherit-product, device/rockchip/common/device.mk)
$(call inherit-product, device/rockchip/rk3528/product.mk)

DEVICE_PACKAGE_OVERLAYS += $(LOCAL_PATH)/../overlay

#TODO TV?
PRODUCT_CHARACTERISTICS := tv

PRODUCT_NAME := rk3528_box
PRODUCT_DEVICE := rk3528_box
PRODUCT_BRAND := RockChip
PRODUCT_MODEL := rk3528
PRODUCT_MANUFACTURER := RockChip
TARGET_BOOTLOADER_BOARD_NAME := RK3528

# Get the long list of APNs
PRODUCT_COPY_FILES += vendor/rockchip/common/phone/etc/apns-full-conf.xml:system/etc/apns-conf.xml
PRODUCT_COPY_FILES += vendor/rockchip/common/phone/etc/spn-conf.xml:system/etc/spn-conf.xml

PRODUCT_AAPT_CONFIG := normal large tvdpi hdpi
PRODUCT_AAPT_PREF_CONFIG := tvdpi

# TV Input HAL
PRODUCT_PACKAGES += \
    android.hardware.tv.input@1.0-impl
