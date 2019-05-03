_PKG_openssl_DIR = $(PREBUILTS_DIR)/openssl-1.0.2g-endeca1
PKG_openssl_INC_DIRS = $(_PKG_openssl_DIR)/include
PKG_openssl_LIB_DIRS = $(_PKG_openssl_DIR)/$(LIBDIRNAME)
PKG_openssl_LIB_NAMES = ssl crypto
PKG_openssl_INSTALLERS = CP_openssl_libs CP_openssl_bin CP_openssl_cnf
PKG_openssl_FOR_SHLIB = 1
ALL_PACKAGES += openssl

CP_openssl_libs_SRC_DIR = $(_PKG_openssl_DIR)/$(LIBDIRNAME)
CP_openssl_libs_SRC_FILENAMES = libcrypto.so.1.0.0 libssl.so.1.0.0
CP_openssl_libs_DEST_DIR = $(INSTALL_ROOT)/$(LIBDIRNAME)

CP_openssl_bin_SRC_DIR = $(_PKG_openssl_DIR)/bin
CP_openssl_bin_SRC_FILENAMES = openssl
CP_openssl_bin_DEST_DIR = $(INSTALL_ROOT)/bin

CP_openssl_ssl_SRC_DIR = $(_PKG_openssl_DIR)/ssl
CP_openssl_ssl_SRC_FILENAMES = openssl.cnf
CP_openssl_ssl_DEST_DIR = $(INSTALL_ROOT)/ssl