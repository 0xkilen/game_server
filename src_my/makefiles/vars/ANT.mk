TARGET_NAME := ANT_$(ANT_NAME)

$(call ensure_defined,$(TARGET_NAME)_SRC_DIR)
$(call ensure_defined,$(TARGET_NAME)_BUILD_TARGET)
$(call ensure_defined,$(TARGET_NAME)_INSTALL_DIR)

$(TARGET_NAME)_OUT_DIR := $(call MAKE_OUT_DIR,$($(TARGET_NAME)_SRC_DIR))

$(TARGET_NAME) := $($(TARGET_NAME)_INSTALL_DIR)
