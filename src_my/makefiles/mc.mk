# Windows Message Compiler

MC := "$(MS_SDKS)Bin\x64\MC.exe"
MC_FLAGS = -c -b

define MC_CMD
	$(call MKDIR,$(@D))
	$(MC) $(MC_FLAGS) -h $(@D) -r $(@D) $<
endef

$(BUILD_ROOT)/%.h: $(BUILD_ROOT)/%.bin ;

$(BUILD_ROOT)/%.rc: $(BUILD_ROOT)/%.bin ;

$(BUILD_ROOT)/%.bin: ./%.mc
	$(MC_CMD)

$(BUILD_ROOT)/%.bin: $(BUILD_ROOT)/%.mc
	$(MC_CMD)


