BUILD_MODE_OPT := $(if $(OPT),-opt,-noopt)
BUILD_MODE_ALLOC := $(if $(ALLOCATOR_PKG),-$(ALLOCATOR_PKG))
BUILD_MODE_INST := $(if $(INSTRUMENTATION_STYLE),-$(INSTRUMENTATION_STYLE))
BUILD_MODE := $(BUILD_MODE_OPT)$(BUILD_MODE_ALLOC)$(BUILD_MODE_INST)

