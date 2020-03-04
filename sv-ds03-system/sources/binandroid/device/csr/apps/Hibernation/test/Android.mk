LOCAL_PATH := $(call my-dir)

local_target_dir := $(TARGET_OUT)/system/bin/

include $(CLEAR_VARS)
LOCAL_MODULE := hibernation_stess_test.sh
LOCAL_MODULE_TAGS := debug
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(local_target_dir)
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

