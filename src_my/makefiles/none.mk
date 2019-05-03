ifeq ($(NONE_MK),)
NONE_MK := true

# If we support a memcheck or profiler on a platform but NOT the other
# Currently we support neither on windows at build time
# we'll want to break this file out into none-memcheck and none-profiler

ifneq ($(MEMCHECK),)
$(error $(MEMCHECKER) not currently supported on this platform)
endif

ifneq ($(PROFILE),)
$(error $(PROFILER) not currently supported on this platform)
endif

endif
