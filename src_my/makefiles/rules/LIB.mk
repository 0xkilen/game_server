TARGET_NAME := LIB_$(LIB_NAME)

# if this library is destined to go in a shared library, the variable definition
#  LIB_$(LIB_NAME)_FOR_SHLIB = 1
# should be made

include makefiles/src_rules.mk

# If this was a recursive variable, evaluate it now once and for all
$(TARGET_NAME)_OTHER_LINK_INPUTS := $($(TARGET_NAME)_OTHER_LINK_INPUTS)

# make all 'other inputs' before objects
$($(TARGET_NAME)_OBJS): | $($(TARGET_NAME)_OTHER_LINK_INPUTS)

$($(TARGET_NAME)): TARGET_NAME := $(TARGET_NAME)
$($(TARGET_NAME)): $($(TARGET_NAME)_OBJS) $($(TARGET_NAME)_OTHER_LINK_INPUTS)
	@$(call print_if,$(PRINT_LIB),Libing,$(notdir $@),...)
	$(call STATIC_LINK_CMD,$+)

.PHONY: $(TARGET_NAME)
$(TARGET_NAME): $($(TARGET_NAME))

_LIBS_OF_$(LIB_NAME) := \
  $(foreach pkg,$($(TARGET_NAME)_USE_PACKAGES),\
    $(if $(LIB_$(pkg)_SRC_DIR),$(pkg)))

.PHONY: compile-$(LIB_NAME)
compile-$(LIB_NAME): \
  $($(TARGET_NAME)_OBJS) \
  $(_LIBS_OF_$(LIB_NAME):%=compile-%)

# Force evaluation of these now, so we don't repeatedly evaluate them later
PKG_$(LIB_NAME)_INC_DIRS := $(PKG_$(LIB_NAME)_INC_DIRS)
PKG_$(LIB_NAME)_GENERATED_HEADERS := $(PKG_$(LIB_NAME)_GENERATED_HEADERS)
PKG_$(LIB_NAME)_DEP_LIBS := $(PKG_$(LIB_NAME)_DEP_LIBS)
PKG_$(LIB_NAME)_LIB_DIRS := $(PKG_$(LIB_NAME)_LIB_DIRS)
PKG_$(LIB_NAME)_LIB_NAMES := $(PKG_$(LIB_NAME)_LIB_NAMES)
PKG_$(LIB_NAME)_USE_PACKAGES := $(PKG_$(LIB_NAME)_USE_PACKAGES)
PKG_$(LIB_NAME)_FOR_SHLIB := $(PKG_$(LIB_NAME)_FOR_SHLIB)
PKG_$(LIB_NAME)_INSTALLERS := $(PKG_$(LIB_NAME)_INSTALLERS)
