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

# First lunching is T, api_level is 33
PRODUCT_SHIPPING_API_LEVEL := 33
PRODUCT_DTBO_TEMPLATE := $(LOCAL_PATH)/dt-overlay.in

include device/rockchip/common/build/rockchip/DynamicPartitions.mk
include device/rockchip/rk3588/nova/BoardConfig.mk
include device/rockchip/common/BoardConfig.mk
$(call inherit-product, device/rockchip/rk3588/device.mk)
$(call inherit-product, device/rockchip/common/device.mk)
$(call inherit-product, frameworks/native/build/tablet-10in-xhdpi-2048-dalvik-heap.mk)

DEVICE_PACKAGE_OVERLAYS += $(LOCAL_PATH)/../overlay

PRODUCT_CHARACTERISTICS := tablet

PRODUCT_NAME := nova
PRODUCT_DEVICE := nova
PRODUCT_BRAND := rockchip
PRODUCT_MODEL := Nova
PRODUCT_MANUFACTURER := Indiedroid
PRODUCT_AAPT_PREF_CONFIG := xhdpi

$(call inherit-product-if-exists, vendor/gapps/arm64/arm64-vendor.mk)

$(shell python device/rockchip/rk3588/auto_generator.py preinstall)
-include device/rockchip/rk3588/preinstall/preinstall.mk

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/rtkbt.conf:$(TARGET_COPY_OUT_VENDOR)/etc/bluetooth/rtkbt.conf \

PRODUCT_PACKAGES += \
Browser2 \

# Overlays
DEVICE_PACKAGE_OVERLAYS += \
    	device/rockchip/rk3588/nova/overlay \

#
## add Rockchip properties
#
PRODUCT_PROPERTY_OVERRIDES += ro.sf.lcd_density=240
PRODUCT_PROPERTY_OVERRIDES += ro.wifi.sleep.power.down=true
PRODUCT_PROPERTY_OVERRIDES += persist.wifi.sleep.delay.ms=0
PRODUCT_PROPERTY_OVERRIDES += persist.bt.power.down=true
PRODUCT_PROPERTY_OVERRIDES += persist.sys.rotation.efull=true
PRODUCT_PROPERTY_OVERRIDES += ro.config.media_vol_default=12
PRODUCT_PROPERTY_OVERRIDES += vendor.hwc.device.primary=HDMI-A,DP
PRODUCT_PROPERTY_OVERRIDES += vendor.hwc.device.extend=DSI

PRODUCT_PROPERTY_OVERRIDES += service.adb.tcp.port=5555
BUILD_NUMBER2 := $(shell $(DATE) +%Y%m%d)
PRODUCT_PROPERTY_OVERRIDES += ro.build.display.id=Nova-android-13-v$(BUILD_NUMBER2)
