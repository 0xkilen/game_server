LIB_utils_SRC_DIR := $(SRC_DIR)

LIB_utils_INSTALL = 1

LIB_utils_SRC = $(filter-out %test.cpp,$(call find_src_recursive,$(LIB_utils_SRC_DIR),cpp))

LIB_utils_USE_PACKAGES = boost jansson google-protocol-buffers curl
