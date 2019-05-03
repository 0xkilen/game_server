OPT =
CHECKED =
INSTRUMENTATION_STYLE = purecov

DEFAULT_PURECOV_OPTS += -always-use-cache-dir
DEFAULT_PURECOV_OPTS += -cache-dir=$(ABS_BUILD_ROOT)/purecov-cache

$(call warn_if_from_env,PURECOVOPTIONS)
PURECOVOPTIONS ?= $(DEFAULT_PURECOV_OPTS)

# for purify, we compile and link everything with purify.
# on Windows, this is just a matter of build flags
COMPILE_PREFIX = $(PURECOV) $(PURECOVOPTIONS)

LINK_PREFIX = $(PURECOV) $(PURECOVOPTIONS)
