define SWIG_CMD
	@$(call MKDIR,$(@D))
	$(SWIG_BIN) -lua -o $(BUILD_ROOT)/$(dir $<)/$(notdir $@) $<
endef

./%.elua.hpp: ./cfun.hpp

$(BUILD_ROOT)/%.elua.cc: ./%.elua.hpp
	$(SWIG_CMD)

#$(BUILD_ROOT)/%.elua.cc: $(BUILD_ROOT)/%.elua.hpp
#	$(SWIG_CMD)
