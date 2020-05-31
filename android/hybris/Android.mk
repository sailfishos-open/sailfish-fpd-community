LOCAL_PATH := $(call my-dir)

ANDROID_VERSION_MAJOR := $(word 1, $(subst ., , $(PLATFORM_VERSION)))
ANDROID_VERSION_MINOR := $(word 2, $(subst ., , $(PLATFORM_VERSION)))
ANDROID_VERSION_PATCH := $(word 3, $(subst ., , $(PLATFORM_VERSION)))

ifeq ($(ANDROID_VERSION_MINOR),)
    ANDROID_VERSION_MINOR := 0
endif
ifeq ($(ANDROID_VERSION_PATCH),)
    ANDROID_VERSION_PATCH := 0
endif

IS_ANDROID_8 := $(shell test $(ANDROID_VERSION_MAJOR) -ge 8 && echo true)

include $(CLEAR_VARS)

LOCAL_CFLAGS += \
	-DANDROID_VERSION_MAJOR=$(ANDROID_VERSION_MAJOR) \
	-DANDROID_VERSION_MINOR=$(ANDROID_VERSION_MINOR) \
	-DANDROID_VERSION_PATCH=$(ANDROID_VERSION_PATCH)

UPAPI_PATH := $(LOCAL_PATH)/../../

ifneq ($(IS_ANDROID_8),true)
LOCAL_CFLAGS += -std=gnu++0x
endif

ifeq ($(IS_ANDROID_8),true)
LOCAL_CFLAGS += \
    -Wno-unused-parameter
endif

LOCAL_C_INCLUDES := \
	$(UPAPI_PATH)/src

ifeq ($(IS_ANDROID_8),true)
LOCAL_SRC_FILES += \
    biometry_fp_hidl_for_hybris.cpp
else
$(error "Android pre oreo not supported")
endif

LOCAL_MODULE := libbiometry_fp_api
LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES := \
	libbinder \
	libcutils \
	libinput \
	liblog \
	libutils \
	libhardware \
	libhardware_legacy \
	libdl

ifeq ($(IS_ANDROID_8),true)
LOCAL_SHARED_LIBRARIES += \
    libhidlbase \
    libhidltransport \
    libsensor \
    android.hardware.biometrics.fingerprint@2.1 \
    android.hardware.gatekeeper@1.0
endif

include $(BUILD_SHARED_LIBRARY)

