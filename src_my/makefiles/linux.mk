include makefiles/unix.mk

# needed by various parts of ENE
PLATFORM_CFLAGS += -DLINUX -D_FILE_OFFSET_BITS=64 -gdwarf-4

# enable link-section based tracing initialization
ifeq (,$(USE_GOLD))
PLATFORM_CFLAGS += -DLINKER_PROVIDES_LAYOUT_CONTROL
PLATFORM_LDFLAGS += -fuse-ld=bfd -Wl,-T,makefiles/ldscript
endif

# "export all dynamic symbols".  Not sure why this is necessary.
PLATFORM_LDFLAGS += -Wl,-E

$(call warn_if_from_env,PURIFY)
PURIFY ?= /usr/dev_tools/purify/releases/purify.i386_linux2.7.0.0.0-010/purify

$(call warn_if_from_env,PURECOV)
PURECOV ?= /usr/dev_tools/purify/releases/purecov.i386_linux2.7.0.0.0-010/purecov

$(call warn_if_from_env,QUANTIFY)
QUANTIFY ?= /usr/dev_tools/purify/releases/quantify.i386_linux2.7.0.0.0-010/quantify

$(call warn_if_from_env,VALGRIND)
VALGRIND ?= $(DEV_TOOLS)/bin/valgrind

ALLOCATOR_PKG = tcmalloc

REDHAT_VERSION := $(shell cat /etc/redhat-release 2>/dev/null \
	| $(SED) 's/.*(\([^ ]*\).*).*/\1/')
IS_OS_SUPPORTED ?= $(if $(filter Tikanga,$(REDHAT_VERSION)),YES)
ifneq ($(IS_OS_SUPPORTED),YES)
$(error Only Red Hat Enterprise Linux version 5 is supported.  If you really want to continue, add a line saying IS_OS_SUPPORTED=YES to your trunk/src/Makefile.local or in your environment)
endif

TIMESTAMP = $(DATE) +%s.%N
