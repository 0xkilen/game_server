# Windows resource Compiler

RC := "$(MS_SDKS)Bin\x64\rc.exe"
RC_FLAGS = -l 0x409

define RC_CMD
	$(call MKDIR,$(@D))
	$(RC) $(RC_FLAGS) -i $(@D) -fo $@ $<
endef

# user-supplied resource files
$(BUILD_ROOT)/%.res: ./%.rc
	$(RC_CMD)

# generated resource files
$(BUILD_ROOT)/%.res: $(BUILD_ROOT)/%.rc
	$(RC_CMD)

