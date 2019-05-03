_PKG_libmemcached_DIR = $(PREBUILTS_DIR)/libmemcached-1.0.18-endeca1
PKG_libmemcached_INC_DIRS = $(_PKG_libmemcached_DIR)/include
PKG_libmemcached_LIB_DIRS = $(_PKG_libmemcached_DIR)/$(LIBDIRNAME)
PKG_libmemcached_LIB_NAMES = memcached
PKG_libmemcached_FOR_SHLIB = 1
PKG_libmemcached_INSTALLERS = CP_libmemcached
ALL_PACKAGES += libmemcached

CP_libmemcached_SRC_DIR = $(_PKG_libmemcached_DIR)/$(LIBDIRNAME)
CP_libmemcached_SRC_FILENAMES = libmemcached.so.11
CP_libmemcached_DEST_DIR = $(INSTALL_ROOT)/$(LIBDIRNAME)

