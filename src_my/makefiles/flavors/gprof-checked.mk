OPT = 1
CHECKED = 1
INSTRUMENTATION_STYLE = gprof-checked

FLAVOR_CFLAGS = -pg
FLAVOR_LDFLAGS = -pg

ifeq ($(TARGET_OS),windows)
$(error gprof not supported on Windows)
endif
