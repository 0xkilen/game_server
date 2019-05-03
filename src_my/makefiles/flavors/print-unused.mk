#
# A flavor intended to be used in hunting down unused code.  The output of this
# is very heavy and noisy, and appears on the linker's stderr, where you'd
# normally look for error messages, so this flavor is not a good choice for
# normal development.
#
# This is the technique that adranse used back in 2009 (?) when he was chasing
# down dead code.
#

ifneq ($(TARGET_OS),linux)
$(error print-unused only supported on linux)
endif

# Turn off optimization
include makefiles/flavors/debug.mk

# Put each function and global variable in their own section
FLAVOR_CFLAGS += -ffunction-sections -fdata-sections
# Print to stderr a list of all unused sections
FLAVOR_LDFLAGS += -Wl,--gc-sections -Wl,--print-gc-sections

# Change the build output directory.  This produces different binaries than a
# regular debug build.
INSTRUMENTATION_STYLE += print-unused
