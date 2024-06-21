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

#overlay config
ifeq ($(TARGET_BOARD_PLATFORM_PRODUCT), box)
PRODUCT_PACKAGE_OVERLAYS += device/rockchip/rk3528/rk3528_box/overlay
else
PRODUCT_PACKAGE_OVERLAYS += device/rockchip/rk3528/overlay
endif
PRODUCT_PACKAGES += \
    libion

# Default integrate MediaCenter
PRODUCT_PACKAGES += \
    MediaCenter

#enable this for support f2fs with data partion
BOARD_USERDATAIMAGE_FILE_SYSTEM_TYPE := f2fs

# This ensures the needed build tools are available.
# TODO: make non-linux builds happy with external/f2fs-tool; system/extras/f2fs_utils
ifeq ($(HOST_OS),linux)
  TARGET_USERIMAGES_USE_F2FS := true
endif

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/init.rk3528.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/hw/init.rk3528.rc \
    $(LOCAL_PATH)/init.rk30board.usb.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/hw/init.rk30board.usb.rc \
    $(LOCAL_PATH)/wake_lock_filter.xml:system/etc/wake_lock_filter.xml \
    device/rockchip/rk3528/package_performance.xml:$(TARGET_COPY_OUT_OEM)/etc/package_performance.xml \
    device/rockchip/rk3528/external_camera_config.xml:$(TARGET_COPY_OUT_VENDOR)/etc/external_camera_config.xml \
    device/rockchip/rk3528/media_profiles_default.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_profiles_V1_0.xml \
    device/rockchip/rk3528/etc/NotoSansHans-Regular.otf:system/fonts/NotoSansHans-Regular.otf

# copy input keylayout and device config
ifeq ($(TARGET_BOARD_PLATFORM_PRODUCT), box)
PRODUCT_COPY_FILES += \
    device/rockchip/rk3528/Vendor_0416_Product_0300.kl:$(TARGET_COPY_OUT_VENDOR)/usr/keylayout/Vendor_0416_Product_0300.kl \
    device/rockchip/rk3528/Vendor_0508_Product_0110.kl:$(TARGET_COPY_OUT_VENDOR)/usr/keylayout/Vendor_0508_Product_0110.kl \
    device/rockchip/rk3528/vendor.kl:$(TARGET_COPY_OUT_VENDOR)/usr/keylayout/vendor.kl \
    device/rockchip/rk3528/vendor.kl:system/usr/keylayout/vendor.kl \
    device/rockchip/rk3528/CMIOT_REMOTE.idc:$(TARGET_COPY_OUT_VENDOR)/usr/idc/CMIOT_REMOTE.idc \
    device/rockchip/rk3528/CMIOT_REMOTE.kl:system/usr/keylayout/CMIOT_REMOTE.kl \
    device/rockchip/rk3528/CMIOT_REMOTE.kl:$(TARGET_COPY_OUT_VENDOR)/usr/keylayout/CMIOT_REMOTE.kl 

ifdef PRODUCT_PWM_KL_FILE
PRODUCT_COPY_FILES += $(PRODUCT_PWM_KL_FILE):$(TARGET_COPY_OUT_VENDOR)/usr/keylayout/ffa90030_pwm.kl
else
PRODUCT_COPY_FILES += device/rockchip/rk3528/rk3528_box/ffa90030_pwm.kl:$(TARGET_COPY_OUT_VENDOR)/usr/keylayout/ffa90030_pwm.kl
endif

else
PRODUCT_COPY_FILES += \
    device/rockchip/rk3528/110b0030_pwm.kl:system/usr/keylayout/110b0030_pwm.kl \
    device/rockchip/rk3528/ffa90030_pwm.kl:$(TARGET_COPY_OUT_VENDOR)/usr/keylayout/ffa90030_pwm.kl \
    device/rockchip/rk3528/ffa90030_pwm.idc:$(TARGET_COPY_OUT_VENDOR)/usr/idc/ffa90030_pwm.idc \
    device/rockchip/rk3528/HiRemote.kl:system/usr/keylayout/HiRemote.kl \
    device/rockchip/rk3528/HiRemote.kl:$(TARGET_COPY_OUT_VENDOR)/usr/keylayout/HiRemote.kl \
    device/rockchip/rk3528/HiRemote.idc:$(TARGET_COPY_OUT_VENDOR)/usr/idc/HiRemote.idc \
    device/rockchip/rk3528/virtual-remote.idc:$(TARGET_COPY_OUT_VENDOR)/usr/idc/virtual-remote.idc
endif

# setup dalvik vm configs.
$(call inherit-product, frameworks/native/build/tablet-10in-xhdpi-2048-dalvik-heap.mk)


$(call inherit-product-if-exists, vendor/rockchip/rk3528/device-vendor.mk)

#tv_core_hardware_3328
ifneq ($(filter rk3528 rk3528_32, $(TARGET_PRODUCT)), )
PRODUCT_COPY_FILES += \
    device/rockchip/rk3528/permissions/tv_core_hardware_3328.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/tv_core_hardware_3328.xml \
    device/rockchip/rk3528/permissions/tv_core_hardware_3328.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/tv_core_hardware_3528.xml \
    frameworks/native/data/etc/android.hardware.gamepad.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.gamepad.xml
endif

#
#add Rockchip properties here
#
PRODUCT_PROPERTY_OVERRIDES += \
    ro.prop.cmcc_split_cnt=4_6 \
    persist.sys.fuse.passthrough.enable=true \
    persist.sys.locale=zh-CN \
    persist.sys.timezone=Asia/Shanghai \
    ro.vendor.rk_sdk=1 \
    sys.video.afbc=1 \
    vendor.gralloc.disable_afbc=1 \
    vendor.gralloc.no_afbc_for_fb_target_layer=1 \
    wifi.interface=wlan0 \
    ro.audio.monitorOrientation=true \
    vendor.hwc.compose_policy=1 \
    sf.power.control=2073600 \
    ro.tether.denied=false \
    sys.resolution.changed=false \
    ro.product.usbfactory=rockchip_usb \
    wifi.supplicant_scan_interval=15 \
    ro.kernel.android.checkjni=0 \
    ro.vendor.nrdp.modelgroup=NEXUSPLAYERFUGU \
    vendor.hwc.device.primary=HDMI-A,TV\
    persist.vendor.framebuffer.main=1920x1080@60 \
    persist.vendor.framebuffer.aux=1920x1080@60 \
    ro.vendor.sdkversion=rk3528_ANDROID9.0_BOX_V1.0

ifeq ($(TARGET_BOARD_PLATFORM_PRODUCT), box)
PRODUCT_PROPERTY_OVERRIDES += \
    ro.sf.lcd_density=240 \
    vendor.hwc.video_buf_cache_max_size=29491199 \
    rt_vtunnel_enable=1 \
    rt_retriever_max_size=4096
else
PRODUCT_PROPERTY_OVERRIDES += \
    ro.sf.lcd_density=213 \
    persist.sys.usb.config=mtp
endif

PRODUCT_DEFAULT_PROPERTY_OVERRIDES += \
    ro.opengles.version=131072 \
    ro.hwui.drop_shadow_cache_size=4.0 \
    ro.hwui.gradient_cache_size=0.8 \
    ro.hwui.layer_cache_size=32.0 \
    ro.hwui.path_cache_size=24.0 \
    ro.hwui.text_large_cache_width=2048 \
    ro.hwui.text_large_cache_height=1024 \
    ro.hwui.text_small_cache_width=1024 \
    ro.hwui.text_small_cache_height=512 \
    ro.hwui.texture_cache_flushrate=0.4 \
    ro.hwui.texture_cache_size=72.0 \
    debug.hwui.use_partial_updates=false

# GTVS add the Client ID (provided by Google)
PRODUCT_PROPERTY_OVERRIDES += \
    ro.com.google.clientidbase=android-rockchip-tv

# Vendor seccomp policy files for media components:
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/seccomp_policy/mediacodec.policy:$(TARGET_COPY_OUT_VENDOR)/etc/seccomp_policy/mediacodec.policy

PRODUCT_COPY_FILES += \
    frameworks/av/media/libeffects/data/audio_effects.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_effects.xml

BOARD_VENDOR_KERNEL_MODULES += \
	device/rockchip/rk3528/rkvtunnel.ko

ifneq ($(filter rk3528_box_32 rk3528_32, $(TARGET_PRODUCT)), )
ifeq ($(strip $(BUILD_WITH_GO_OPT)),true)
	# enable swap to zRAM, kernel config
	# #Configure the zRAM size to 75% in the fstab file.
PRODUCT_COPY_FILES += \
    device/rockchip/rk3528/rk3528_box_32/fstab.enableswap:root/fstab.enableswap

#Reduces GC frequency of foreground apps by 50% 
PRODUCT_PROPERTY_OVERRIDES += dalvik.vm.foreground-heap-growth-multiplier=2.0
endif
ifeq ($(strip $(BOARD_TV_LOW_MEMOPT)),true)
PRODUCT_PROPERTY_OVERRIDES += \
    sys.video.maxMemCapacity=220 \
    sys.video.refFrameMode=1
    ro.mem_optimise.enable=true
PRODUCT_COPY_FILES += \
       device/rockchip/common/lowmem_package_filter.xml:system/etc/lowmem_package_filter.xml
endif
endif
