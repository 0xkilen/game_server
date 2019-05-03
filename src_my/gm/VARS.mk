LIB_gm_SRC_DIR := $(SRC_DIR)

LIB_gm_SRC = $(filter-out %test.cpp,$(filter-out gmd.cpp,$(call find_src_recursive,$(LIB_gm_SRC_DIR),c cc cpp,Collections))) \
	$(NULL)

LIB_gm_USE_PACKAGES = utils proto net config route db

#====================================================================================

BIN_jxwy_gmd_SRC_DIR := $(SRC_DIR)

BIN_jxwy_gmd_SRC = main.cpp

BIN_jxwy_gmd_INSTALL = 1

BIN_jxwy_gmd_USE_PACKAGES = gm
