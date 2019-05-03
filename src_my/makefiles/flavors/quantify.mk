OPT = 1
CHECKED =
INSTRUMENTATION_STYLE = quantify

# for quantify, we compile and link everything with quantify. 
# on Windows, this is just a matter of build flags
# Also on Windows, we need the binaries to contain relocation info --
# that's already taken care of by the default build flags.
COMPILE_PREFIX = $(QUANTIFY)
LINK_PREFIX = $(QUANTIFY) -cache-dir=$(ABS_BUILD_ROOT)/quantify-cache -always-use-cache-dir -linker=$(DEV_TOOLS)/bin/ld

FLAVOR_CFLAGS=-DQUANTIFY
