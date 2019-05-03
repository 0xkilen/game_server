LIB_chat_SRC_DIR := $(SRC_DIR)

LIB_chat_SRC = $(filter-out %test.cpp,$(filter-out main.cpp,$(call find_src_recursive,$(LIB_chat_SRC_DIR),c cc cpp,Collections))) \
	$(NULL)

LIB_chat_USE_PACKAGES = utils proto net config route

#====================================================================================

BIN_jxwy_chatd_SRC_DIR := $(SRC_DIR)

BIN_jxwy_chatd_SRC = main.cpp

BIN_jxwy_chatd_INSTALL = 1

BIN_jxwy_chatd_USE_PACKAGES = chat

