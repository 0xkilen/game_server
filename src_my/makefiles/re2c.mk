RE2C_FLAGS = -w

ifneq (windows,$(TARGET_OS))

define RE2C_CMD
	$(call MKDIR,$(@D))
	$(DEV_TOOLS)/bin/re2c $(RE2C_FLAGS) $(1)
endef

else

define RE2C_CMD
	$(call MKDIR,$(@D))
	cd $(DEV_TOOLS)/bin
	re2c.exe $(RE2C_FLAGS) $(1)
endef

endif

#
# There are two conventions in use in the tree, so support them both:
#

$(BUILD_ROOT)/%.cpp: ./%.cpp.re2c
	$(call RE2C_CMD,-o $@ $<)

$(BUILD_ROOT)/%.cpp: ./%.re
	$(call RE2C_CMD,-o $@ $<)

$(BUILD_ROOT)/%.cpp: $(BUILD_ROOT)/%.cpp.re2c
	$(call RE2C_CMD,-o $@ $<)

$(BUILD_ROOT)/%.cpp: $(BUILD_ROOT)/%.re
	$(call RE2C_CMD,-o $@ $<)

