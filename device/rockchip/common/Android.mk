LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE                                       := remove-BlockedNumberProvider
EXECUTABLES.remove-BlockedNumberProvider.OVERRIDES := BlockedNumberProvider
include $(BUILD_PHONY_PACKAGE)

include $(CLEAR_VARS)
LOCAL_MODULE                             := remove-TeleService
EXECUTABLES.remove-TeleService.OVERRIDES := TeleService
include $(BUILD_PHONY_PACKAGE)

include $(CLEAR_VARS)
LOCAL_MODULE                            := remove-MmsService
EXECUTABLES.remove-MmsService.OVERRIDES := MmsService
include $(BUILD_PHONY_PACKAGE)

include $(CLEAR_VARS)
LOCAL_MODULE                         := remove-Telecom
EXECUTABLES.remove-Telecom.OVERRIDES := Telecom
include $(BUILD_PHONY_PACKAGE)

include $(CLEAR_VARS)
LOCAL_MODULE                         := remove-Bluetooth
EXECUTABLES.remove-Bluetooth.OVERRIDES := Bluetooth
include $(BUILD_PHONY_PACKAGE)

include $(CLEAR_VARS)
LOCAL_MODULE                         := remove-Traceur
EXECUTABLES.remove-Traceur.OVERRIDES := Traceur
include $(BUILD_PHONY_PACKAGE)

include $(CLEAR_VARS)
LOCAL_MODULE                         := remove-HTMLViewer
EXECUTABLES.remove-HTMLViewer.OVERRIDES := HTMLViewer 
include $(BUILD_PHONY_PACKAGE)

include $(CLEAR_VARS)
LOCAL_MODULE                         := remove-UserDictionaryProvider
EXECUTABLES.remove-UserDictionaryProvider.OVERRIDES := UserDictionaryProvider
include $(BUILD_PHONY_PACKAGE)

include $(CLEAR_VARS)
LOCAL_MODULE                         := remove-android.hardware.configstore@1.1-service
EXECUTABLES.remove-android.hardware.configstore@1.1-service.OVERRIDES := android.hardware.configstore@1.1-service
include $(BUILD_PHONY_PACKAGE)

include $(CLEAR_VARS)
LOCAL_MODULE                         := remove-SimAppDialog
EXECUTABLES.remove-SimAppDialog.OVERRIDES := SimAppDialog
include $(BUILD_PHONY_PACKAGE)

include $(CLEAR_VARS)
LOCAL_MODULE                         := remove-ExactCalculator
EXECUTABLES.remove-ExactCalculator.OVERRIDES := ExactCalculator
include $(BUILD_PHONY_PACKAGE)

include $(CLEAR_VARS)
LOCAL_MODULE                         := remove-SoundPicker
EXECUTABLES.remove-SoundPicker.OVERRIDES := SoundPicker
include $(BUILD_PHONY_PACKAGE)

include $(CLEAR_VARS)
LOCAL_MODULE                         := remove-SecureElement
EXECUTABLES.remove-SecureElement.OVERRIDES := SecureElement
include $(BUILD_PHONY_PACKAGE)

include $(CLEAR_VARS)
LOCAL_MODULE                         := remove-EasterEgg
EXECUTABLES.remove-EasterEgg.OVERRIDES := EasterEgg
include $(BUILD_PHONY_PACKAGE)

include $(CLEAR_VARS)
LOCAL_MODULE                         := remove-CalendarProvider
EXECUTABLES.remove-CalendarProvider.OVERRIDES := CalendarProvider
include $(BUILD_PHONY_PACKAGE)

include $(CLEAR_VARS)
LOCAL_MODULE                         := remove-BookmarkProvider
EXECUTABLES.remove-BookmarkProvider.OVERRIDES := BookmarkProvider
include $(BUILD_PHONY_PACKAGE)

include $(CLEAR_VARS)
LOCAL_MODULE                         := remove-BasicDreams
EXECUTABLES.remove-BasicDreams.OVERRIDES := BasicDreams
include $(BUILD_PHONY_PACKAGE)

include $(CLEAR_VARS)
LOCAL_MODULE                         := remove-LatinIME
EXECUTABLES.remove-LatinIME.OVERRIDES := LatinIME
include $(BUILD_PHONY_PACKAGE)

include $(CLEAR_VARS)
LOCAL_MODULE                         := remove-DeskClock
EXECUTABLES.remove-DeskClock.OVERRIDES := DeskClock
include $(BUILD_PHONY_PACKAGE)

include $(CLEAR_VARS)
LOCAL_MODULE                         := remove-Contacts
EXECUTABLES.remove-Contacts.OVERRIDES := Contacts
include $(BUILD_PHONY_PACKAGE)

include $(CLEAR_VARS)
LOCAL_MODULE                         := remove-Calendar
EXECUTABLES.remove-Calendar.OVERRIDES := Calendar
include $(BUILD_PHONY_PACKAGE)

include $(CLEAR_VARS)
LOCAL_MODULE                         := remove-Etar
EXECUTABLES.remove-Etar.OVERRIDES := Etar
include $(BUILD_PHONY_PACKAGE)

include $(CLEAR_VARS)
LOCAL_MODULE                         := remove-Email
EXECUTABLES.remove-Email.OVERRIDES := Email
include $(BUILD_PHONY_PACKAGE)

include $(CLEAR_VARS)
LOCAL_MODULE                         := remove-QuickSearchBox
EXECUTABLES.remove-QuickSearchBox.OVERRIDES := QuickSearchBox
include $(BUILD_PHONY_PACKAGE)

include $(CLEAR_VARS)
LOCAL_MODULE                         := remove-Camera2
EXECUTABLES.remove-Camera2.OVERRIDES := Camera2
include $(BUILD_PHONY_PACKAGE)

include $(CLEAR_VARS)
LOCAL_MODULE                         := remove-NfcNci
EXECUTABLES.remove-NfcNci.OVERRIDES := NfcNci
include $(BUILD_PHONY_PACKAGE)

include $(CLEAR_VARS)
LOCAL_MODULE                         := remove-vr
EXECUTABLES.remove-vr.OVERRIDES := vr
include $(BUILD_PHONY_PACKAGE)

include $(CLEAR_VARS)
LOCAL_MODULE                         := remove-PrintRecommendationService
EXECUTABLES.remove-PrintRecommendationService.OVERRIDES := PrintRecommendationService
include $(BUILD_PHONY_PACKAGE)

include $(CLEAR_VARS)
LOCAL_MODULE                         := remove-PrintSpooler
EXECUTABLES.remove-PrintSpooler.OVERRIDES := PrintSpooler
include $(BUILD_PHONY_PACKAGE)

# if some modules are built directly from this directory (not subdirectories),
# their rules should be written here.

$(shell mkdir -p $(TARGET_ROOT_OUT)/mnt/vendor/metadata)

include $(call all-makefiles-under,$(LOCAL_PATH))
