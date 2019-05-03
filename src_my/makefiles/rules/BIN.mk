TARGET_NAME := BIN_$(BIN_NAME)

include makefiles/src_rules.mk

ACCUM_PKG_VARS = $(call unique_by_rev,$(call deref,$($(TARGET_NAME)_USE_PACKAGES:%=PKG_%_$(1))))

$(TARGET_NAME)_PKG_DEP_LIBS := $(call ACCUM_PKG_VARS,DEP_LIBS)
$(TARGET_NAME)_PKG_LIB_DIRS := $(call ACCUM_PKG_VARS,LIB_DIRS)
$(TARGET_NAME)_PKG_LIB_NAMES := $(call ACCUM_PKG_VARS,LIB_NAMES)
$(TARGET_NAME)_PKG_INSTALLERS := $(call unique,$(TOOLCHAIN_INSTALLERS) $(call ACCUM_PKG_VARS,INSTALLERS))

$(TARGET_NAME)_LDFLAGS := \
	$(PKG_$(ALLOCATOR_PKG)_LDFLAGS) \
	$(PLATFORM_LDFLAGS) \
	$(TOOL_LDFLAGS) \
	$(LDFLAGS_CPU) \
	$(call map,LINK_PATH_ARG,$($(TARGET_NAME)_PKG_LIB_DIRS)) \
	$(FLAVOR_LDFLAGS)

$(TARGET_NAME)_LDLIBS := \
	$(TOOL_LDLIBS) \
	$(call map,LINK_LIB_ARG,$($(TARGET_NAME)_PKG_LIB_NAMES))

# get symbol file names from the toolchain
$(TARGET_NAME)_SYMS := $(call SYMBOLS,$($(TARGET_NAME)))
ifneq (,$($(TARGET_NAME)_SYMS))
# symbols are a side effect of linking
$($(TARGET_NAME)_SYMS) : $($(TARGET_NAME)) ;
endif

# If this was a recursive variable, evaluate it now once and for all
$(TARGET_NAME)_OTHER_LINK_INPUTS := $($(TARGET_NAME)_OTHER_LINK_INPUTS)

# make dependent installs and all 'other inputs' before objects
$($(TARGET_NAME)_OBJS): |  $(call deref,$($(TARGET_NAME)_PKG_INSTALLERS)) $($(TARGET_NAME)_OTHER_LINK_INPUTS)

# see valgrind.mk
$(TARGET_NAME)_TRACE_CHILDREN ?= 1

#
# The following is to allow make to regenerate the wrapper scripts
# iff BUILD_WRAPPER_SCRIPT has changed for a particular file.
#
# The general scheme is to maintain a file in BUILD_ROOT/script-hashes
# for each wrapper script that has the md5 checksum of the script
# appended to the file name (SCRIPT_HASH_FILE). The wrapper script
# depends on the SCRIPT_HASH_FILE as well as the binary that it's wrapping.
# If the script is updated (either by changing a variable on the command
# line, or changing the script definition itself), then the newly computed
# checksum will be different than that on the existing file, so the new
# SCRIPT_HASH_FILE won't exist, which will cause the wrapper file to be
# rewritten. The SCRIPT_HASH_FILE rule will delete the old SCRIPT_HASH_FILE,
# and then touch the new one, thus indicating that the wrapper script has
# been updated.
#
ifneq (,$(REAL_BINARY_MARKER))
$(TARGET_NAME)_SCRIPT := $(call BUILD_WRAPPER_SCRIPT,$(TARGET_NAME))
$($(TARGET_NAME)): $(CP_valgrind_suppressions)
$($(TARGET_NAME)): TARGET_NAME := $(TARGET_NAME)
$($(TARGET_NAME)): $($(TARGET_NAME))$(REAL_BINARY_MARKER) $(call DEPEND_ON_VAR,$(TARGET_NAME)_SCRIPT)
	$(shell $($(TARGET_NAME)_SCRIPT))
	$(call CHMOD_X, $@)
endif

$($(TARGET_NAME))$(REAL_BINARY_MARKER): TARGET_NAME := $(TARGET_NAME)
$($(TARGET_NAME))$(REAL_BINARY_MARKER): LDFLAGS := $($(TARGET_NAME)_LDFLAGS)
$($(TARGET_NAME))$(REAL_BINARY_MARKER): LDLIBS := $($(TARGET_NAME)_LDLIBS)
$($(TARGET_NAME))$(REAL_BINARY_MARKER): $($(TARGET_NAME)_OBJS) $($(TARGET_NAME)_OTHER_LINK_INPUTS) $($(TARGET_NAME)_DEP_LIBS) $($(TARGET_NAME)_PKG_DEP_LIBS)
	@$(call print_if,$(PRINT_BIN),Linking,$(notdir $@),...)
	$(call EXECUTABLE_LINK_CMD,$+)

ifneq (windows,$(TARGET_OS))
# on Unix, library and executable paths are separate
format_path_list = $(subst $(space),:,$(strip $(1)))
$(TARGET_NAME)_LD_LIBRARY_PATH := LD_LIBRARY_PATH=$(LIB_DIRS):$(LD_LIBRARY_PATH):$(call format_path_list,$($(TARGET_NAME)_PKG_LIB_DIRS))
$(TARGET_NAME)_PATH :=
else
# on Windows, both types of path are combined, so there is an added mechanism ($(TARGET_NAME)_PATH_PREFIX) to
# enable adding additional search paths in for use with CMD_PREFIX
# When you invoke a command like this, you fully enter Cygwin land, so the path also needs to be filtered 
# through cygpath else you get strange escaping
format_path_list = $(subst $(space),\;,$(subst /,\\\,$(strip $(1))))
$(TARGET_NAME)_PATH := PATH="$(shell cygpath -p '$($(TARGET_NAME)_PATH_PREFIX);$(call format_path_list,$($(TARGET_NAME)_PKG_LIB_DIRS));$(PATH)')"
endif

.PHONY: run-$(BIN_NAME)
run-$(BIN_NAME): TARGET_NAME := $(TARGET_NAME)
run-$(BIN_NAME): $($(TARGET_NAME))
	$($(TARGET_NAME)_PATH) $($(TARGET_NAME)_LD_LIBRARY_PATH) $(CMD_PREFIX) $(RUN_PREFIX) $< $(CMD_ARGS)

.PHONY: $(TARGET_NAME)
$(TARGET_NAME): $($(TARGET_NAME))

_LIBS_OF_$(BIN_NAME) := \
  $(foreach pkg,$($(TARGET_NAME)_USE_PACKAGES),\
    $(if $(LIB_$(pkg)_SRC_DIR),$(pkg)))

.PHONY: compile-$(BIN_NAME)
compile-$(BIN_NAME): \
  $($(TARGET_NAME)_OBJS) \
  $(_LIBS_OF_$(BIN_NAME):%=compile-%)
