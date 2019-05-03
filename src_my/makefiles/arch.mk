ifneq ($(filter Windows%,$(OS)),)
  TARGET_OS := windows

  # n.b. since make is a 32-bit program, it is always run
  # within a SysWow64 command environment.

  # also select Win64 if Win64 OS and WIN32 not defined
  ifeq (AMD64,$(PROCESSOR_ARCHITECTURE))
      TARGET_CPU := x86_64
  else ifeq (AMD64,$(PROCESSOR_ARCHITEW6432))
      TARGET_CPU := x86_64
  else
    TARGET_CPU := x86
    ifneq (x86,$(PROCESSOR_ARCHITECTURE))
      $(error "Unable to determine Windows TARGET_CPU from $(PROCESSOR_ARCHITECTURE)")
    endif
  endif

else
UNAME_OS=$(shell uname)

ifeq ($(UNAME_OS),Linux)
  TARGET_OS := linux

  UNAME_ARCH=$(shell uname -p)
  ifeq ($(UNAME_ARCH),unknown)
    UNAME_ARCH=$(shell uname -m)
  endif

  ifneq ($(filter i%86,$(UNAME_ARCH)),)
    TARGET_CPU := x86
  endif
  ifeq ($(UNAME_ARCH),athlon)
    TARGET_CPU := x86
  endif
  ifeq ($(UNAME_ARCH),x86_64)
    TARGET_CPU := x86_64
  endif
  ifeq ($(TARGET_CPU),)
    $(error "Unable to determine TARGET_CPU")
  endif

else
ifeq ($(UNAME_OS),SunOS)
  TARGET_OS := solaris
  UNAME_ARCH=$(shell uname -p)
  TARGET_ARCH := $(UNAME_ARCH)
  ifeq ($(UNAME_ARCH),sparc)
    TARGET_CPU := sparcv9
  endif
  ifeq ($(TARGET_CPU),)
    $(error "Unable to determine TARGET_CPU")
  endif

else
  $(error "Unable to determine TARGET_OS")
endif
endif
endif

linux_x86_64_LIBDIRNAME = lib64

# Force 32 bit mode. 
ifneq ($(BITS),)
ifeq ($(BITS),32) 
ifeq ($(TARGET_CPU), x86_64)
TARGET_CPU=x86
endif
else
  $(error "BITS is set to $(BITS). BITS=32 is the only valid setting")
endif
endif

$(TARGET_OS)_$(TARGET_CPU)_LIBDIRNAME ?= lib
LIBDIRNAME = $($(TARGET_OS)_$(TARGET_CPU)_LIBDIRNAME)
DEV_TOOLSLIBDIRNAME ?= $(LIBDIRNAME)

TARGET_PLATFORM := $(TARGET_OS)-$(TARGET_CPU)
export TARGET_OS
export TARGET_CPU
export TARGET_PLATFORM
$(shell echo 'Make target platform: $(TARGET_PLATFORM)' 1>&2)

