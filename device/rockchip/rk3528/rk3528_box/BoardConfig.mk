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
include device/rockchip/rk3528/BoardConfig.mk
BUILD_WITH_GO_OPT := false

TARGET_BOARD_PLATFORM_PRODUCT := box
TARGET_BASE_PARAMETER_IMAGE := device/rockchip/rk3528/etc/baseparameter_auto.img

# AB image definition
BOARD_USES_AB_IMAGE := false
BOARD_ROCKCHIP_VIRTUAL_AB_ENABLE := false

# for widevine drm
BOARD_WIDEVINE_OEMCRYPTO_LEVEL := 3

BOARD_HS_ETHERNET := true
ifeq ($(strip $(BOARD_USES_AB_IMAGE)), true)
    include device/rockchip/common/BoardConfig_AB.mk
    TARGET_RECOVERY_FSTAB := device/rockchip/rk3588/rk3588_t/recovery.fstab_AB
endif
BOARD_HS_ETHERNET := true
