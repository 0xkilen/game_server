ifneq (,$(ALL_$(ENTITY_TYPE)S))
$(ENTITY_TYPE)_$(INCLUDE_TYPE)_INCLUDER := $(BUILD_ROOT)/$(ENTITY_TYPE)_$(INCLUDE_TYPE).mk

$(shell (for $(ENTITY_TYPE)name in $(ALL_$(ENTITY_TYPE)S); do \
	echo $(ENTITY_TYPE)_NAME=$$$(ENTITY_TYPE)name; \
	echo '$$(call print_timestamp, "    ' $(INCLUDE_TYPE) $(ENTITY_TYPE)_$$$(ENTITY_TYPE)name '",$(ENTITY_TIMESTAMP_PRINT_FLAG))' ; \
	echo include makefiles/$(INCLUDE_TYPE)/$(ENTITY_TYPE).mk; \
	done) > $($(ENTITY_TYPE)_$(INCLUDE_TYPE)_INCLUDER))

include $($(ENTITY_TYPE)_$(INCLUDE_TYPE)_INCLUDER)
endif
