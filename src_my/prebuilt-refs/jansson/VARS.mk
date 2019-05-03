_PKG_jansson_DIR = $(PREBUILTS_DIR)/jansson-2.8-endeca1
PKG_jansson_INC_DIRS = $(_PKG_jansson_DIR)/include
PKG_jansson_LIB_DIRS = $(_PKG_jansson_DIR)/$(LIBDIRNAME)
PKG_jansson_LIB_NAMES = jansson
PKG_jansson_FOR_SHLIB = 1
PKG_jansson_INSTALLERS = CP_jansson
ALL_PACKAGES += jansson

CP_jansson_SRC_DIR = $(_PKG_jansson_DIR)/$(LIBDIRNAME)
CP_jansson_SRC_FILENAMES = libjansson.so.4 libjansson.so.4.8.0
CP_jansson_DEST_DIR = $(INSTALL_ROOT)/$(LIBDIRNAME)

