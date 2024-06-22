# Android13
Android 13 build files for Rockchip devices

Follow Khadas instructions to setup a build environment and download source code:

https://docs.khadas.com/products/sbc/edge2/development/android/build-android

After downloading the Khadas repo, download and replace the files found in this repo. 
Android can then be built following the Khadas instructions or with the commands below and selecting the correct device to build. 

source build/envsetup.sh && lunch

./build.sh -CUAu    (C-Kernel with Clang, U-U-boot, A-Android, u-update.img)

You can reference Firefly's docs for flashing. 

https://wiki.t-firefly.com/en/ROC-RK3588S-PC/upgrade_firmware.html
