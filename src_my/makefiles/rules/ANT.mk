TARGET_NAME := ANT_$(ANT_NAME)

# Packages that we depend on
ACCUM_PKG_VARS = $(call unique_by_rev,$(call deref,$($(TARGET_NAME)_USE_PACKAGES:%=PKG_%_$(1))))
$(TARGET_NAME)_PKG_DEP_LIBS := $(call ACCUM_PKG_VARS,DEP_LIBS)
$(TARGET_NAME)_PKG_INSTALLERS := $(call ACCUM_PKG_VARS,INSTALLERS)

#
# We want the result files to be always out of date, so that any time someone
# wants them, we run ant.  Ant will take care of figuring out if the results
# actually need rebuilding.
#
# So make all the results depend on a phony target that actually runs ant.
#
# Note:  Ant assumes that any paths are relative to the location of build.xml, 
# not of the curdir of the ant process.  So we transform the various
# directories into absolute paths before passing them in.

$(TARGET_NAME)_ABS_CLASSPATH := $(call make_files_abs,$($(TARGET_NAME)_CLASSPATH))
$(TARGET_NAME)_ABS_OUT_DIR := $(call abs_dir,$($(TARGET_NAME)_OUT_DIR))
$(TARGET_NAME)_ABS_INSTALL_DIR := $(call abs_dir,$($(TARGET_NAME)_INSTALL_DIR))

define ANTS_GO_MARCHING
	$(ANT) $(if $(ANT_VERBOSE),-v) -noinput \
	-f $($(TARGET_NAME)_SRC_DIR)/build.xml \
	$(addprefix -lib ,$($(TARGET_NAME)_ABS_CLASSPATH)) \
	-Dapp.version=$(if $(RELEASE_VERSION),$(RELEASE_VERSION),0.0.0.0) \
	$(if $(CHECKED), -Dturn-debug-on=on) \
	-Doutput-base-dir=$($(TARGET_NAME)_ABS_OUT_DIR) \
	-Dinstall-base-dir=$(ABS_INSTALL_ROOT) \
	-Dinstall-dir=$($(TARGET_NAME)_ABS_INSTALL_DIR) \
	-Dshared-prebuilt-dir=$(ABS_PREBUILTS_DIR)/../shared \
	-Dprebuilt-dir=$(ABS_PREBUILTS_DIR) \
	-Dsrc-root=$(ABS_SRC_ROOT) \
	-Dproduct-name=$(PRODUCT_NAME) \
	-Dproduct-version=$(PRODUCT_VERSION) \
	-Dbuild-number=$(BUILD) \
	$(1)
endef

# ant build target
.PHONY: ant-$(ANT_NAME)
ant-$(ANT_NAME): TARGET_NAME := $(TARGET_NAME)
$($(TARGET_NAME)): ant-$(ANT_NAME)
ant-$(ANT_NAME): $($(TARGET_NAME)_PKG_DEP_LIBS) $($(TARGET_NAME)_PKG_INSTALLERS)
	$(call ANTS_GO_MARCHING,$($(TARGET_NAME)_BUILD_TARGET))

ifneq (,$($(TARGET_NAME)_TEST_TARGET))

# add ant test to list, and set path so we can filter on it
ALL_TEST_ANTS += $(ANT_NAME)
TEST_$(TARGET_NAME) := $($(TARGET_NAME)_OUT_DIR)/

# ant test target
.PHONY: ant-test-$(ANT_NAME)
ant-test-$(ANT_NAME): TARGET_NAME := $(TARGET_NAME)
$(TEST_$(TARGET_NAME)): ant-test-$(ANT_NAME) $($(TARGET_NAME)_PKG_INSTALLERS)
ant-test-$(ANT_NAME): $($(TARGET_NAME)_PKG_DEP_LIBS)
	$(call ANTS_GO_MARCHING,$($(TARGET_NAME)_TEST_TARGET))

endif

.PHONY: $(TARGET_NAME)
$(TARGET_NAME): $($(TARGET_NAME))

# Force evaluation of these now, so we don't repeatedly evaluate them later
PKG_$(ANT_NAME)_DEP_LIBS := $(PKG_$(ANT_NAME)_DEP_LIBS)
