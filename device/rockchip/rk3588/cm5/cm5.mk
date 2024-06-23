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
include device/rockchip/rk3588/cm5/BoardConfig.mk
include device/rockchip/common/BoardConfig.mk
$(call inherit-product, device/rockchip/rk3588/device.mk)
$(call inherit-product, device/rockchip/common/device.mk)
$(call inherit-product, frameworks/native/build/tablet-10in-xhdpi-2048-dalvik-heap.mk)

DEVICE_PACKAGE_OVERLAYS += $(LOCAL_PATH)/../overlay

PRODUCT_CHARACTERISTICS := tablet

PRODUCT_NAME := cm5
PRODUCT_DEVICE := cm5
PRODUCT_BRAND := Android
PRODUCT_MODEL := Retro-Lite-CM5
PRODUCT_MANUFACTURER := StonedEdge
PRODUCT_AAPT_PREF_CONFIG := xhdpi

$(call inherit-product-if-exists, vendor/gapps/arm64/arm64-vendor.mk)

# Overlays
DEVICE_PACKAGE_OVERLAYS += \
    	device/rockchip/rk3588/cm5/overlay \

#
## add Rockchip properties
#
PRODUCT_PROPERTY_OVERRIDES += ro.sf.lcd_density=220
PRODUCT_PROPERTY_OVERRIDES += ro.wifi.sleep.power.down=true
PRODUCT_PROPERTY_OVERRIDES += persist.wifi.sleep.delay.ms=0
PRODUCT_PROPERTY_OVERRIDES += persist.bt.power.down=true
PRODUCT_PROPERTY_OVERRIDES += vendor.hwc.device.primary=DSI
PRODUCT_PROPERTY_OVERRIDES += vendor.hwc.device.extend=HDMI-A,DP
PRODUCT_PROPERTY_OVERRIDES += ro.surface_flinger.primary_display_orientation=ORIENTATION_270

PRODUCT_PROPERTY_OVERRIDES += service.adb.tcp.port=5555
BUILD_NUMBER2 := $(shell $(DATE) +%Y%m%d)
PRODUCT_PROPERTY_OVERRIDES += ro.build.display.id=CM5-android-13-v$(BUILD_NUMBER2)

# gsy remove apk
TARGET_REMOVE_PACKAGES += \
        RkExplorer \
        RKLogsTool \
        Lightning \
        Browser2 \
	QuickSearchBox \
	Camera2
