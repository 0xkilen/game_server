define PROTOBUF_CMD
	@$(call MKDIR,$(@D))
	$(GOOGLE_PROTOC_BIN) -I=$(dir $<) --cpp_out=$(BUILD_ROOT)/$(dir $<) $<
endef

%.pb.h: %.pb.cc ;

$(BUILD_ROOT)/%.pb.cc: ./%.proto
	$(PROTOBUF_CMD)

$(BUILD_ROOT)/%.pb.cc: $(BUILD_ROOT)/%.proto
	$(PROTOBUF_CMD)

