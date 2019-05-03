OPT =
CHECKED =
ALLOCATOR_PKG =
INSTRUMENTATION_STYLE = purify

# Find and use our suppressions file
# This is the un-adorned suppressions file name becaue purify will
# load this file from the same directory as the binary being executed.
# Our automated packaging/install process copies this file into the
# right place.
DEFAULT_PURIFY_OPTS += -add-suppression-files=$(ABS_SRC_ROOT)/ene/purify_suppressions -add-suppression-files=purify_suppressions

# Always use the cache-dir we specify; never use a central location.
DEFAULT_PURIFY_OPTS += -always-use-cache-dir

# If unable to instrument a library, emit a warning and keep going,
# rather than give up.
DEFAULT_PURIFY_OPTS += -best-effort

# Put instrumented code in a cache directory located within the build
# tree.
DEFAULT_PURIFY_OPTS += -cache-dir=$(ABS_BUILD_ROOT)/purify-cache

# Retain deep stack traces; the default is 6, which is much too shallow
# to identify what's actually going on.
DEFAULT_PURIFY_OPTS += -chain-length=64

# Our child processes do interesting work; follow them, and log issues.
DEFAULT_PURIFY_OPTS += -follow-child-processes=yes

# When instrumenting archives that contain shared libraries, if the
# shared library does not set DT_SONAME, use the physical name of
# the library, rather than give up.
DEFAULT_PURIFY_OPTS += -ignore-soname

# cause MIU reporting
DEFAULT_PURIFY_OPTS += -inuse-at-exit=yes

# tweak the exit status to add purify status information
# rethink this: it tanks the build: DEFAULT_PURIFY_OPTS += -exit-status=yes

# Do not limit the size of log files
DEFAULT_PURIFY_OPTS += -output-limit=0

# Write results to a compact binary file viewable with $(PURIFY) -view file.pv
DEFAULT_PURIFY_OPTS += -view-file=./%v.%p.pv

$(call warn_if_from_env,PURIFY_OPTS)
PURIFY_OPTS ?= $(DEFAULT_PURIFY_OPTS)

# for purify, we compile and link everything with purify.
# on Windows, this is just a matter of build flags
COMPILE_PREFIX = $(PURIFY) $(PURIFY_OPTS)

LINK_PREFIX = $(PURIFY) $(PURIFY_OPTS)

FLAVOR_CFLAGS=-DPURIFY -DEXIT_CLEANUP

ifeq ($(TARGET_OS),windows)
FLAVOR_LDFLAGS = /fixed:no
endif
