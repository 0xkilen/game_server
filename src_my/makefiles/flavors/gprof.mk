OPT = 1
CHECKED =
INSTRUMENTATION_STYLE = gprof

FLAVOR_CFLAGS = -pg
FLAVOR_LDFLAGS = -pg

ifeq ($(TARGET_OS),windows)
$(error gprof not supported on Windows)
endif
