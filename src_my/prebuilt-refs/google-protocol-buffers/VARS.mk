_PKG_google-protocol-buffers_DIR = $(PREBUILTS_DIR)/google-protocol-buffers-2.6.1-endeca1
PKG_google-protocol-buffers_INC_DIRS = $(_PKG_google-protocol-buffers_DIR)/include
PKG_google-protocol-buffers_LIB_DIRS = $(_PKG_google-protocol-buffers_DIR)/$(LIBDIRNAME)
PKG_google-protocol-buffers_INSTALLERS = CP_google-protocol-buffers-libs
PKG_google-protocol-buffers_FOR_SHLIB = 1
PKG_google-protocol-buffers_LIB_NAMES = protobuf
ALL_PACKAGES += google-protocol-buffers

CP_google-protocol-buffers-libs_SRC_DIR = $(_PKG_google-protocol-buffers_DIR)/$(LIBDIRNAME)
CP_google-protocol-buffers-libs_SRC_FILENAMES = libprotobuf.so.9.0.1 libprotobuf.so.9
CP_google-protocol-buffers-libs_DEST_DIR = $(INSTALL_ROOT)/$(LIBDIRNAME)

GOOGLE_PROTOC_BIN = $(call EXECUTABLE,$(_PKG_google-protocol-buffers_DIR)/bin/protoc)
