ifneq ($(TARGET_OS),linux)
$(error gdb only supported on linux)
endif

include makefiles/flavors/debug.mk

CMD_PREFIX ?= $(GDB) --args
