_PKG_mysql-connector-c_DIR = $(PREBUILTS_DIR)/mysql-connector-c-6.1.9-endeca1
PKG_mysql-connector-c_INC_DIRS = $(_PKG_mysql-connector-c_DIR)/include
PKG_mysql-connector-c_LIB_DIRS = $(_PKG_mysql-connector-c_DIR)/$(LIBDIRNAME)
PKG_mysql-connector-c_LIB_NAMES = mysqlclient
PKG_mysql-connector-c_FOR_SHLIB = 1
PKG_mysql-connector-c_INSTALLERS = CP_mysql-connector-c
ALL_PACKAGES += mysql-connector-c

CP_mysql-connector-c_SRC_DIR = $(_PKG_mysql-connector-c_DIR)/$(LIBDIRNAME)
CP_mysql-connector-c_SRC_FILENAMES = libmysqlclient.so.18
CP_mysql-connector-c_DEST_DIR = $(INSTALL_ROOT)/$(LIBDIRNAME)

