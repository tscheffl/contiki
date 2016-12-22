CONTIKI_PROJECT = mqtt-client
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

# for Valgrind
CFLAGS += -g

CONTIKI_WITH_IPV6 = 1

#CFLAGS += -DUIP_CONF_IPV6_RPL=0
#CFLAGS += -DRPL_BORDER_ROUTER=0

# Defines in platform/x/contki-conf.h seem 
# to take silent precedence:

#CFLAGS += -DUIP_CONF_DS6_DEFAULT_PREFIX=\"aaaa::\"
CFLAGS += -DHARD_CODED_ADDRESS=\"aaaa::10\"
CFLAGS += -DUIP_CONF_BUFFER_SIZE=1300

#RPL_BORDER_ROUTER=0
#UIP_CONF_IPV6_RPL=0


APPS += mqtt
CFLAGS += -DMY_NODE_ID=11
CONTIKI = ../..
include $(CONTIKI)/Makefile.include
