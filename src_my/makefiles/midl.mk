# Windows Interface Description Language compiler

MIDL := "$(MS_SDKS)Bin/x64/midl.exe"
MIDL_FLAGS = -nologo -char signed -env win32 -Oicf

define MIDL_CMD
	$(call MKDIR,$(@D))
	$(MIDL) $(MIDL_FLAGS) -tlb $(BUILD_ROOT)/$*.tlb -h $(BUILD_ROOT)/$*.h -iid $(BUILD_ROOT)/$*_i.c $<
endef

$(BUILD_ROOT)/%.h: $(BUILD_ROOT)/%_i.c ;

$(BUILD_ROOT)/%.tlb: $(BUILD_ROOT)/%_i.c ;

$(BUILD_ROOT)/%_i.c: ./%.idl
	$(MIDL_CMD)

