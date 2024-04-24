LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

SAMSUNG_OXYGEN_NETWORK=y
ifdef SAMSUNG_OXYGEN_NETWORK
LOCAL_CFLAGS += -DSAMSUNG_OXYGEN_NETWORK
endif
LOCAL_SRC_FILES:= \
        dhcpclient.c \
        dhcpmsg.c \
        dhcp_utils.c \
        ifc_utils.c \
        packet.c
#-> PPPOE SUPPORT
LOCAL_SRC_FILES += pppoe_utils.c
#<- PPPOE SUPPORT

LOCAL_SHARED_LIBRARIES := \
        libcutils \
        liblog

LOCAL_MODULE:= libnetutils

include $(BUILD_SHARED_LIBRARY)
