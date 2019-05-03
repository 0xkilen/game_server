ifneq ($(TARGET_OS),linux)
$(error cgdb only supported on linux)
endif

include makefiles/flavors/debug.mk

CMD_PREFIX ?= /usr/bin/cgdb -d $(GDB) --args
