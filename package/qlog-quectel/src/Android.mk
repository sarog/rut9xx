# Copyright 2006 The Android Open Source Project

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

include $(CLEAR_VARS)
LOCAL_SRC_FILES:= main.cpp serial.cpp vetify_arg.cpp
LOCAL_SRC_FILES:=main.cpp serial.cpp vetify_arg.cpp
LOCAL_CFLAGS += -pie -fPIE
LOCAL_LDFLAGS += -pie -fPIE
LOCAL_MODULE_TAGS:= optional
LOCAL_MODULE:= QAndroidLog
include $(BUILD_EXECUTABLE)
