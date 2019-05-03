LIB_proto_SRC_DIR := $(SRC_DIR)

LIB_proto_SRC = $(call find_proto_recursive,$(LIB_proto_SRC_DIR))

LIB_proto_USE_PACKAGES = google-protocol-buffers
