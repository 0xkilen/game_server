BIN_utils_unit_test_SRC_DIR := $(SRC_DIR)

BIN_utils_unit_test_INSTALL = 1

BIN_utils_unit_test_SRC = $(filter %test.cpp,$(call find_src,$(BIN_utils_unit_test_SRC_DIR),cpp))

BIN_utils_unit_test_USE_PACKAGES = unit_runner cppunit utils proto

UNIT_TEST_BINS += utils_unit_test