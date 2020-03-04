LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_PRELINK_MODULE := false
LOCAL_SRC_FILES:= \
	com_android_calibration_NativeCalibration.cpp

LOCAL_C_INCLUDES += \
	$(JNI_H_INCLUDE) \
	nativehelper/jni.h\

LOCAL_SHARED_LIBRARIES := \
    libnativehelper \
    libcutils \
    libutils

#LOCAL_CFLAGS += -O0 -g

LOCAL_MODULE := libcalibrate_jni
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)
