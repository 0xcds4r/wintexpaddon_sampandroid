LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := arrtib_wintexp
LOCAL_LDLIBS := -llog

# plugin
FILE_LIST := $(wildcard $(LOCAL_PATH)/../src/*.cpp)

# vendor
FILE_LIST += $(wildcard $(LOCAL_PATH)/../vendor/*/*.cpp)

LOCAL_SRC_FILES := $(FILE_LIST:$(LOCAL_PATH)/%=%)

LOCAL_CPPFLAGS := -w -s -fvisibility=default -pthread -Wall -fpack-struct=1 -O2 -std=c++14 -fexceptions
include $(BUILD_SHARED_LIBRARY)