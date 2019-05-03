# Usage:
#  make FLAVOR=json -B | awk -f makefiles/json_post.awk > compile_commands.json
# or
#  make USE_CLANG=1 FLAVOR=json -B | awk -f makefiles/json_post.awk > compile_commands.json

JSON = 1

include makefiles/flavors/debug.mk

# by default, use gcc
# this will produce a compilation database that LLVM tools don't like
# without a little hacking, but at this point the source doesn't compile
# cleanly with clang.
ifneq (,$(USE_CLANG))
TOOLCHAIN_CHOICE := clang
INSTRUMENTATION_STYLE = clang
endif

USE_DISTCC :=
USE_CCACHE :=
MAKECMDGOALS := compile
QUIET := 1
.NOTPARALLEL:

