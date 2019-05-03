OPT = 1
CHECKED = 1
INSTRUMENTATION_STYLE = checked

# Currently (01/09), the checked flavor does not successfully build due to
# unresolved symbols in linking. According to jkearney, "the problem is that
# you have to build some of the prebuilt libraries checked as well, because the
# checked STL mode adds members."
ifeq ($(TARGET_OS),windows)
$(error checked flavor is not supported on Windows\; see checked.mk for details)
endif
