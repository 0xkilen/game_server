_PKG_mysql-connector-c++_DIR = $(PREBUILTS_DIR)/mysql-connector-c++-1.1.8-endeca1
PKG_mysql-connector-c++_INC_DIRS = $(_PKG_mysql-connector-c++_DIR)/include
PKG_mysql-connector-c++_LIB_DIRS = $(_PKG_mysql-connector-c++_DIR)/$(LIBDIRNAME)
PKG_mysql-connector-c++_LIB_NAMES = mysqlcppconn

PKG_mysql-connector-c++_INSTALLERS = CP_mysql-connector-c++
ALL_PACKAGES += mysql-connector-c++

CP_mysql-connector-c++_SRC_DIR = $(_PKG_mysql-connector-c++_DIR)/$(LIBDIRNAME)
CP_mysql-connector-c++_SRC_FILENAMES = libmysqlcppconn.so.7
CP_mysql-connector-c++_DEST_DIR = $(INSTALL_ROOT)/$(LIBDIRNAME)
