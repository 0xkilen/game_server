_PKG_zlib_DIR = $(PREBUILTS_DIR)/zlib-1.2.3-endeca1
PKG_zlib_INC_DIRS = $(_PKG_zlib_DIR)/include
PKG_zlib_LIB_DIRS = $(_PKG_zlib_DIR)/$(LIBDIRNAME)
PKG_zlib_LIB_NAMES = z
PKG_zlib_INSTALLERS = CP_zlib
ALL_PACKAGES += zlib
