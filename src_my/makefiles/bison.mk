define BISON_CMD
	@$(call MKDIR,$(@D))
	$(DEV_TOOLS)/bin/bison $< -o $@
endef

#
# bison creates the .tab.hpp as a side-effect of generating the .tab.cpp
#
%.tab.hpp: %.tab.cpp ;

$(BUILD_ROOT)/%.tab.cpp: ./%.ypp
	$(BISON_CMD)

$(BUILD_ROOT)/%.tab.cpp: $(BUILD_ROOT)/%.ypp
	$(BISON_CMD)

