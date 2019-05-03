LIB_mirror_SRC_DIR := $(SRC_DIR)

LIB_mirror_SRC = $(filter-out %test.cpp,$(filter-out main.cpp,$(call find_src_recursive,$(LIB_mirror_SRC_DIR),c cc cpp,Collections))) \
	$(NULL)

LIB_mirror_USE_PACKAGES = utils proto net config route

#====================================================================================

BIN_jxwy_mirrord_SRC_DIR := $(SRC_DIR)

BIN_jxwy_mirrord_SRC = main.cpp

BIN_jxwy_mirrord_INSTALL = 1

BIN_jxwy_mirrord_USE_PACKAGES = mirror

#====================================================================================

BIN_mirror_unit_test_SRC_DIR := $(SRC_DIR)

BIN_mirror_unit_test_SRC = $(filter %test.cpp,$(call find_src_recursive,$(BIN_mirror_unit_test_SRC_DIR),cpp))

BIN_mirror_unit_test_INSTALL = 1

BIN_mirror_unit_test_USE_PACKAGES = unit_runner cppunit mirror boost

UNIT_TEST_BINS += mirror_unit_test

