LIB_net_SRC_DIR := $(SRC_DIR)

LIB_net_INSTALL = 1

LIB_net_SRC = $(filter-out %test.cpp,$(call find_src_recursive,$(LIB_net_SRC_DIR),cpp c))

LIB_net_USE_PACKAGES = boost utils openssl config

#-----------------------------------------------------------------------

BIN_net_unit_test_SRC_DIR := $(SRC_DIR)

BIN_net_unit_test_INSTALL = 1

BIN_net_unit_test_SRC = $(addprefix test/,$(notdir $(wildcard $(BIN_net_unit_test_SRC_DIR)/test/*test.cpp)))

BIN_net_unit_test_USE_PACKAGES = unit_runner cppunit net

UNIT_TEST_BINS += net_unit_test
