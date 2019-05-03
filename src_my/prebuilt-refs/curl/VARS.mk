_PKG_curl_DIR = $(PREBUILTS_DIR)/curl-7.54.1-endeca2
PKG_curl_INC_DIRS = $(_PKG_curl_DIR)/include
PKG_curl_LIB_DIRS = $(_PKG_curl_DIR)/$(LIBDIRNAME)
PKG_curl_LIB_NAMES = curl
PKG_curl_FOR_SHLIB = 1
PKG_curl_INSTALLERS = CP_curl
ALL_PACKAGES += curl

CP_curl_SRC_DIR = $(_PKG_curl_DIR)/$(LIBDIRNAME)
CP_curl_SRC_FILENAMES = libcurl.so.4 libcurl.so.4.4.0
CP_curl_DEST_DIR = $(INSTALL_ROOT)/$(LIBDIRNAME)
