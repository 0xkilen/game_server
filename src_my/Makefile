# Fast usage message
ifeq (help,$(MAKECMDGOALS))
.PHONY: help
help:
	@cat makefiles/doc/help
else
# All the actual code is under the makefiles directory
include makefiles/main.mk
endif
