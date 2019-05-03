TARGET_NAME := SHLIB_$(SHLIB_NAME)

# all objects built for this target are destined to go in a shared library
$(TARGET_NAME)_FOR_SHLIB = 1

include makefiles/src_rules.mk

ACCUM_PKG_VARS = $(call unique_by_rev,$(call deref,$($(TARGET_NAME)_USE_PACKAGES:%=PKG_%_$(1))))

$(TARGET_NAME)_PKG_DEP_LIBS := $(call ACCUM_PKG_VARS,DEP_LIBS)
$(TARGET_NAME)_PKG_LIB_DIRS := $(call ACCUM_PKG_VARS,LIB_DIRS)
PKG_$(SHLIB_NAME)_LIB_DIRS := $($(TARGET_NAME)_OUT_DIR) $($(TARGET_NAME)_PKG_LIB_DIRS)
$(TARGET_NAME)_PKG_LIB_NAMES := $(call ACCUM_PKG_VARS,LIB_NAMES)

$(TARGET_NAME)_LDFLAGS := \
	$(PKG_$(ALLOCATOR_PKG)_LDFLAGS) \
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

# make all 'other inputs' before objects
$($(TARGET_NAME)_OBJS): | $($(TARGET_NAME)_OTHER_LINK_INPUTS)

$($(TARGET_NAME)): TARGET_NAME := $(TARGET_NAME)
$($(TARGET_NAME)): LDFLAGS := $($(TARGET_NAME)_LDFLAGS)
$($(TARGET_NAME)): LDLIBS := $($(TARGET_NAME)_LDLIBS)
$($(TARGET_NAME)): $($(TARGET_NAME)_OBJS) $($(TARGET_NAME)_OTHER_LINK_INPUTS) $($(TARGET_NAME)_PKG_DEP_LIBS)
	$(call SHARED_LINK_CMD,$+)

.PHONY: $(TARGET_NAME)
$(TARGET_NAME): $($(TARGET_NAME))

# Force evaluation of these now, so we don't repeatedly evaluate them later
PKG_$(SHLIB_NAME)_INC_DIRS := $(PKG_$(SHLIB_NAME)_INC_DIRS)
PKG_$(SHLIB_NAME)_GENERATED_HEADERS := $(PKG_$(SHLIB_NAME)_GENERATED_HEADERS)
PKG_$(SHLIB_NAME)_DEP_LIBS := $(PKG_$(SHLIB_NAME)_DEP_LIBS)
PKG_$(SHLIB_NAME)_LIB_DIRS := $(PKG_$(SHLIB_NAME)_LIB_DIRS)
PKG_$(SHLIB_NAME)_LIB_NAMES := $(PKG_$(SHLIB_NAME)_LIB_NAMES)
PKG_$(SHLIB_NAME)_USE_PACKAGES := $(PKG_$(SHLIB_NAME)_USE_PACKAGES)
PKG_$(SHLIB_NAME)_FOR_SHLIB := $(PKG_$(SHLIB_NAME)_FOR_SHLIB)
PKG_$(SHLIB_NAME)_INSTALLERS := $(PKG_$(SHLIB_NAME)_INSTALLERS)
