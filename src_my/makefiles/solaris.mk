include makefiles/unix.mk

# needed by various parts of ENE
PLATFORM_CFLAGS += -DPATH_MAX=4096 -DSOLARIS -D_GLIBCXX_USE_C99_MATH -gdwarf-3
PLATFORM_LDLIBS += -lmalloc -lsocket -lnsl -lpicl /usr/lib/$(TARGET_CPU)/values-xpg6.o

# platform version of sort is different
FIND_MKS = $(patsubst ./%,%,$(shell $(FIND) . -name \*.mk | $(SORT) -f))

# valgrind is not actually available yet for Sparc
$(call warn_if_from_env,VALGRIND)
VALGRIND ?= $(DEV_TOOLS)/bin/valgrind

ALLOCATOR_PKG = mtmalloc

SOLARIS_VERSION := $(shell /usr/bin/uname -r 2>/dev/null )
IS_OS_SUPPORTED ?= $(if $(filter 5.11 ,$(SOLARIS_VERSION)),YES)
ifneq ($(IS_OS_SUPPORTED),YES)
$(error Only Solaris 11 is supported.  If you really want to continue, add a line saying IS_OS_SUPPORTED=YES to your trunk/src/Makefile.local or in your environment)
endif

TIMESTAMP = $(DATE) +%s.%N
