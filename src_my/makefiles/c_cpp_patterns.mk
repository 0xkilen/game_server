ifeq ($(__C_CPP_PATTERNS.MK__),)
__C_CPP_PATTERNS.MK__ := 1

#
# Because we delete on error, we don't have to worry that a missing or wrong
# depfile from a failed compile will make us forget we need to recompile an
# object.
#
.DELETE_ON_ERROR:

#
# Unfortunately, DELETE_ON_ERROR doesn't seem to always work, so we delete the
# object file explicitly.
#

#
# Helper recursive-variables for C_OR_CXX_BUILD_CMD
#
# Include the top-level directory of the source tree (.),
# and any special includes for the target.
#
# Many things also build include files, so include the top-level
# directory of the build tree - $(BUILD_ROOT).
IFLAGS = -iquote . -iquote $(BUILD_ROOT) $(foreach i,$($(TARGET_NAME)_INC_DIRS),-isystem $(i))

CFLAGS_VERSION = $(if $(RELEASE_VERSION),-DRELEASE_VERSION="$(RELEASE_VERSION)") -DBUILD_VERSION="$(BUILD_VERSION)" -DPRODUCT_NAME="$(PRODUCT_NAME)" -DPRODUCT_VERSION="$(PRODUCT_VERSION)"

#
# We enable WERROR only for 64-bit linux right now.
# Turn it on for other platforms in the future when they build without warnings.
# Note that users can turn off -Werror for a particular object
# file by using a target-specific variable override of WERROR= in a RULES.mk.
#
WERROR ?= $(WERROR_$(TARGET_OS)_$(TARGET_CPU))
WERROR_linux_x86_64 = 1

C_FLAGSETS = CPU VERSION BASE OPT CHECKED WARNLEVEL
CFLAGS = $(strip $(foreach n, $(C_FLAGSETS), $(CFLAGS_$(n))))
CXXFLAGS = $(strip $(CFLAGS) $(foreach n, $(C_FLAGSETS), $(CXXFLAGS_$(n))))


#
# Non-generated C++ source files
#
$(BUILD_ROOT)/%.$(O): %.cc
	@$(call print_if,$(PRINT_CPP),Compiling,$<,...)
	$(call C_OR_CXX_BUILD_CMD,CXX,CXX)

$(BUILD_ROOT)/%.$(O): %.cpp
	@$(call print_if,$(PRINT_CPP),Compiling,$<,...)
	$(call C_OR_CXX_BUILD_CMD,CXX,CXX)

#
# Generated C++ source files
#
$(BUILD_ROOT)/%.$(O): $(BUILD_ROOT)/%.cc
	@$(call print_if,$(PRINT_CPP),Compiling,$<,...)
	$(call C_OR_CXX_BUILD_CMD,CXX,CXX)

$(BUILD_ROOT)/%.$(O): $(BUILD_ROOT)/%.cpp
	@$(call print_if,$(PRINT_CPP),Compiling,$<,...)
	$(call C_OR_CXX_BUILD_CMD,CXX,CXX)

#
# Non-generated C source files
#
$(BUILD_ROOT)/%.$(O): %.c
	@$(call print_if,$(PRINT_CPP),Compiling,$<,...)
	$(call C_OR_CXX_BUILD_CMD,CC,C)

#
# Generated C source files
#
$(BUILD_ROOT)/%.$(O): $(BUILD_ROOT)/%.c
	@$(call print_if,$(PRINT_CPP),Compiling,$<,...)
	$(call C_OR_CXX_BUILD_CMD,CC,C)

endif # __C_CPP_PATTERNS.MK__
