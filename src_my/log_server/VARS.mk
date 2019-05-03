LIB_log_server_SRC_DIR := $(SRC_DIR)

LIB_log_server_SRC = $(filter-out %test.cpp,$(filter-out main.cpp,$(call find_src_recursive,$(LIB_log_server_SRC_DIR),c cc cpp,Collections))) \
	 $(NULL)

LIB_log_server_USE_PACKAGES = utils proto net config

#====================================================================================

BIN_jxwy_log_serverd_SRC_DIR := $(SRC_DIR)

BIN_jxwy_log_serverd_SRC = main.cpp

BIN_jxwy_log_serverd_INSTALL = 1

BIN_jxwy_log_serverd_USE_PACKAGES = log_server

