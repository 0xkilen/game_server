CP_sqls_SRC_DIR := $(SRC_DIR)
CP_sqls_SRC_FILENAMES = $(notdir $(wildcard $(CP_sqls_SRC_DIR)/*.sql)) $(notdir $(wildcard $(CP_sqls_SRC_DIR)/*.template))
CP_sqls_DEST_DIR = $(INSTALL_ROOT)/bin/sqls

