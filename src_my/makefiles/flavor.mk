$(call warn_if_from_env,FLAVOR)
FLAVOR ?= debug

ifneq ($(FLAVA),)
FLAVOR = $(FLAVA)
endif

FLAVORS = $(basename $(notdir $(wildcard makefiles/flavors/*.mk)))
ifeq (,$(filter $(FLAVOR),$(FLAVORS)))
$(error $(FLAVOR) is not a valid build flavor. Try one of these: $(FLAVORS))
endif
include makefiles/flavors/$(FLAVOR).mk

