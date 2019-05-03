LIB_db_SRC_DIR := $(SRC_DIR)

LIB_db_INSTALL = 1

LIB_db_SRC = $(filter-out %test.cpp,$(call find_src_recursive,$(LIB_db_SRC_DIR),cpp))

LIB_db_USE_PACKAGES = mysql-connector-c++ mysql-connector-c utils boost proto

#-----------------------------------------------------------------------

BIN_db_unit_test_SRC_DIR := $(SRC_DIR)

BIN_db_unit_test_INSTALL = 1

BIN_db_unit_test_SRC = $(addprefix test/,$(notdir $(wildcard $(BIN_db_unit_test_SRC_DIR)/test/*test.cpp)))

BIN_db_unit_test_USE_PACKAGES = unit_runner cppunit db mysql-connector-c++ mysql-connector-c proto

UNIT_TEST_BINS += db_unit_test

