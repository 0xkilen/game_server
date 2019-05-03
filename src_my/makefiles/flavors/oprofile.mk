OPT = 1
CHECKED =
INSTRUMENTATION_STYLE = oprofile
FLAVOR_CFLAGS = -fno-omit-frame-pointer -DOPROFILE
FLAVOR_LDLIBS = -lopagent -lbfd -liberty -lz

ifeq ($(TARGET_OS),solaris)
$(error oprofile not supported on Solaris)
endif

ifeq ($(TARGET_OS),windows)
$(error oprofile not supported on Windows)
endif

$(shell echo 'libopagent* must be installed for this flavor to be used.' 1>&2)
$(shell echo 'It is recommended to compile and profile on the same system in' 1>&2)
$(shell echo 'order to ensure that oprofile and libopagent match.' 1>&2)
