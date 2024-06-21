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
include device/rockchip/rk356x/x55/BoardConfig.mk
include device/rockchip/common/BoardConfig.mk
$(call inherit-product, device/rockchip/rk356x/device.mk)
$(call inherit-product, device/rockchip/common/device.mk)
$(call inherit-product, frameworks/native/build/tablet-10in-xhdpi-2048-dalvik-heap.mk)

DEVICE_PACKAGE_OVERLAYS += $(LOCAL_PATH)/../overlay

PRODUCT_CHARACTERISTICS := tablet

PRODUCT_NAME := x55
PRODUCT_DEVICE := x55
PRODUCT_BRAND := rockchip
PRODUCT_MODEL := x55
PRODUCT_MANUFACTURER := Powkiddy
PRODUCT_AAPT_PREF_CONFIG := hdpi

#Gapps
##$(call inherit-product-if-exists, vendor/gapps/arm64/arm64-vendor.mk)

#Bluetooth Conf
PRODUCT_COPY_FILES += \
    device/rockchip/rk356x/x55/rtkbt.conf:$(TARGET_COPY_OUT_VENDOR)/etc/bluetooth/rtkbt.conf \
    device/rockchip/rk356x/x55/zed_keyboard.kcm:$(TARGET_COPY_OUT_SYSTEM)/usr/keychars/odroidgo_joypad.kcm \
    device/rockchip/rk356x/x55/zed_keyboard.kl:$(TARGET_COPY_OUT_SYSTEM)/usr/keylayout/odroidgo_joypad.kl \

# Overlays
DEVICE_PACKAGE_OVERLAYS += \
    	device/rockchip/rk356x/x55/overlay \
    	
##TvSettingsTwoPanel
PRODUCT_PACKAGES += \
	ATVLauncher \
	LeanbackIME \

#
## add Rockchip properties
#
PRODUCT_PROPERTY_OVERRIDES += ro.sf.lcd_density=220
PRODUCT_PROPERTY_OVERRIDES += ro.wifi.sleep.power.down=true
PRODUCT_PROPERTY_OVERRIDES += persist.wifi.sleep.delay.ms=0
PRODUCT_PROPERTY_OVERRIDES += persist.sys.show.battery=1
PRODUCT_PROPERTY_OVERRIDES += persist.bt.power.down=true
PRODUCT_PROPERTY_OVERRIDES += ro.vendor.hdmirotationlock=false
PRODUCT_PROPERTY_OVERRIDES += vendor.hwc.device.primary=DSI
PRODUCT_PROPERTY_OVERRIDES += vendor.hwc.device.extend=HDMI-A
PRODUCT_PROPERTY_OVERRIDES += ro.surface_flinger.primary_display_orientation=ORIENTATION_270 \



PRODUCT_PROPERTY_OVERRIDES += service.adb.tcp.port=5555
BUILD_NUMBER2 := $(shell $(DATE) +%Y%m%d)
PRODUCT_PROPERTY_OVERRIDES += ro.build.display.id=X55-android-13-v$(BUILD_NUMBER2)
