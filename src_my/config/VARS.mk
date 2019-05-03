LIB_config_SRC_DIR := $(SRC_DIR)

LIB_config_SRC = $(filter-out config_helper.cpp, \
	$(call find_src_recursive,$(LIB_config_SRC_DIR),c cc cpp,Collections))

LIB_config_USE_PACKAGES = utils proto

#====================================================================================

BIN_config_helper_SRC_DIR := $(SRC_DIR)

BIN_config_helper_SRC = $(call find_src_recursive,$(BIN_config_helper_SRC_DIR),c cc cpp,Collections)

BIN_config_helper_INSTALL = 1

BIN_config_helper_USE_PACKAGES = config

#====================================================================================

CP_config_SRC_DIR := $(SRC_DIR)

CP_config_SRC_FILENAMES = $(call find_src_recursive,$(CP_config_SRC_DIR),csv config template,Collections)

CP_config_DEST_DIR = $(INSTALL_ROOT)/bin/config/
