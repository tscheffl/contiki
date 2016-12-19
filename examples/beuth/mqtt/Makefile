CONTIKI_PROJECT = test-mqtt
all: $(CONTIKI_PROJECT)

ifdef TARGET
 ifeq ($(TARGET),minimal-net)
#  CFLAGS += -DPROJECT_TARGET="minimal-net"
  CFLAGS += -DPROJECT_TARGET_MINIMAL_NET
 else ifeq ($(TARGET),avr-zigbit)
#  CFLAGS += -DPROJECT_TARGET="avr-zigbit"
  CFLAGS += -DPROJECT_TARGET_AVR_ZIGBIT
 else 
 endif 
else
 TARGET=minimal-net
# CFLAGS += -DPROJECT_TARGET="minimal-net"
endif

CONTIKI_WITH_IPV6 = 1

APPS += mqtt
CFLAGS += -DMY_NODE_ID=11
CONTIKI = ../..
include $(CONTIKI)/Makefile.include
