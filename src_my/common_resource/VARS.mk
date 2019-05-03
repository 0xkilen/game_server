LIB_common_resource_SRC_DIR := $(SRC_DIR)

LIB_common_resource_SRC = $(filter-out %test.cpp,$(filter-out main.cpp,$(call find_src_recursive,$(LIB_common_resource_SRC_DIR),c cc cpp,Collections))) \
	$(NULL)

LIB_common_resource_USE_PACKAGES = utils proto net config route db

#====================================================================================

BIN_jxwy_common_resourced_SRC_DIR := $(SRC_DIR)

BIN_jxwy_common_resourced_SRC = main.cpp

BIN_jxwy_common_resourced_INSTALL = 1

BIN_jxwy_common_resourced_USE_PACKAGES = common_resource

