TARGET_NAME := BIN_$(BIN_NAME)
$(call ensure_defined,$(TARGET_NAME)_SRC_DIR)

#
# Add any packages that are needed in all binaries
#
$(TARGET_NAME)_USE_PACKAGES += $(GLOBAL_PKGS)

#
# Note that tcmalloc, an allocator package, must appear last on the link line.
# Let's hope this is true of all global allocators...
#
$(TARGET_NAME)_USE_PACKAGES += $(ALLOCATOR_PKG)

$(TARGET_NAME)_OUT_DIR := $(call MAKE_OUT_DIR,$($(TARGET_NAME)_SRC_DIR))
$(TARGET_NAME)_BASENAME := $(call EXECUTABLE,$(BIN_NAME))
ifneq (,$($(TARGET_NAME)_INSTALL))
 $(TARGET_NAME) := $(INSTALL_ROOT)/bin/$($(TARGET_NAME)_BASENAME)
else
 $(TARGET_NAME) := $($(TARGET_NAME)_OUT_DIR)/$($(TARGET_NAME)_BASENAME)
endif
