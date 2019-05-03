OPT =
CHECKED = 1

FLAVOR_CFLAGS = -fprofile-arcs -ftest-coverage
FLAVOR_LDFLAGS = -fprofile-arcs
INSTRUMENTATION_STYLE = gcov

ifeq ($(TARGET_OS),windows)
$(error gcov not supported on Windows)
endif
