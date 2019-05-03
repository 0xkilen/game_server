# Avoid this problem, which seems to be specific to Ubuntu:
#
# xargs: /localdisk/dev_tools_build/dev_tools/source/findutils-4.2.31/xargs/xargs.c:445: main: Assertion `bc_ctl.arg_max <= (131072-2048)' failed.
#
XARGS ?= /usr/bin/xargs

# libdl, crt1, et al, are in a different place on Mint than on RedHat
# if multiple dirs, separate by ':'
DISTRIB_LIB_DIRS=/usr/lib/x86_64-linux-gnu

# system headers are also
DISTRIB_INC_DIRS=/usr/include/x86_64-linux-gnu

