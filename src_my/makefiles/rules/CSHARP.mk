TARGET_NAME := CSHARP_$(CSHARP_NAME)

$(TARGET_NAME)_CSCFLAGS := $(TOOL_CSCFLAGS) $($(TARGET_NAME)_OTHER_CSCFLAGS)

# If these were recursive variables, evaluate them now once and for all
$(TARGET_NAME)_OTHER_INPUTS := $($(TARGET_NAME)_OTHER_INPUTS)
$(TARGET_NAME)_SRC := $($(TARGET_NAME)_SRC)

$(TARGET_NAME)_GENERATED_SRC := \
	$(foreach s,$($(TARGET_NAME)_SRC), \
		$(if $(wildcard $(s:%=$($(TARGET_NAME)_SRC_DIR)/%)),,$(s)))

# prefix the source path on all names not in the source dir (i.e. generated)
$(TARGET_NAME)_PATHS := $($(TARGET_NAME)_GENERATED_SRC) \
	$(addprefix $($(TARGET_NAME)_SRC_DIR)/,$(filter-out $($(TARGET_NAME)_GENERATED_SRC),$($(TARGET_NAME)_SRC)))

# C# assemblies that we depend on
ACCUM_PKG_VARS = $(call unique_by_rev,$(call deref,$($(TARGET_NAME)_USE_PACKAGES:%=PKG_%_$(1))))
$(TARGET_NAME)_PKG_DEP_LIBS := $(call ACCUM_PKG_VARS,DEP_LIBS)
$(TARGET_NAME)_PKG_INSTALLERS := $(call unique,$(TOOLCHAIN_INSTALLERS) $(call ACCUM_PKG_VARS,INSTALLERS))

# get symbol file names from the toolchain
$(TARGET_NAME)_SYMS := $(call CS_SYMBOLS,$($(TARGET_NAME)))
ifneq (,$($(TARGET_NAME)_SYMS))
# symbols are a side effect of linking
$($(TARGET_NAME)_SYMS) : $($(TARGET_NAME)) ;
endif

# make dependent installs and all 'other inputs' before objects
$($(TARGET_NAME)_OBJS): |  $(call deref,$($(TARGET_NAME)_PKG_INSTALLERS)) $($(TARGET_NAME)_OTHER_INPUTS)

# compiling will also update the XML doc file
$($(TARGET_NAME)_OUT_DIR)/$(CSHARP_NAME).xml: $($(TARGET_NAME));

$($(TARGET_NAME)): TARGET_NAME := $(TARGET_NAME)
$($(TARGET_NAME)): $($(TARGET_NAME)_PATHS) $($(TARGET_NAME)_PKG_DEP_LIBS) $($(TARGET_NAME)_OTHER_INPUTS)
	$(call CSC_CMD,$($(TARGET_NAME)_PATHS) $($(TARGET_NAME)_OTHER_INPUTS))

ifeq (exe,$($(TARGET_NAME)_TYPE))
.PHONY: run-$(CSHARP_NAME)
run-$(CSHARP_NAME): TARGET_NAME := $(TARGET_NAME)
run-$(CSHARP_NAME): $($(TARGET_NAME))
	$(CMD_PREFIX) $< $(CMD_ARGS)
endif

.PHONY: $(TARGET_NAME)
$(TARGET_NAME): $($(TARGET_NAME))

# Force evaluation of these now, so we don't repeatedly evaluate them later
PKG_$(CSHARP_NAME)_DEP_LIBS := $(PKG_$(CSHARP_NAME)_DEP_LIBS)
PKG_$(CSHARP_NAME)_USE_PACKAGES := $(PKG_$(CSHARP_NAME)_USE_PACKAGES)
