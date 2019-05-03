OPT = 1
CHECKED =
INSTRUMENTATION_STYLE = vtune
FLAVOR_CFLAGS = -fno-omit-frame-pointer -DVTUNE
FLAVOR_LDFLAGS = -L/opt/intel/vtune_amplifier_xe/lib64
FLAVOR_LDLIBS = -ljitprofiling

ifeq ($(TARGET_OS),solaris)
$(error vtune not supported on Solaris)
endif

ifeq ($(TARGET_OS),windows)
$(error vtune not supported on Windows)
endif


