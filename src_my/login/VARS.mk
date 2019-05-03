LIB_login_SRC_DIR := $(SRC_DIR)

LIB_login_SRC = $(filter-out %test.cpp,$(filter-out main.cpp,$(call find_src_recursive,$(LIB_login_SRC_DIR),c cc cpp,Collections))) \
	$(NULL)

LIB_login_USE_PACKAGES = utils proto net db config route

#====================================================================================

BIN_jxwy_logind_SRC_DIR := $(SRC_DIR)

BIN_jxwy_logind_SRC = main.cpp

BIN_jxwy_logind_INSTALL = 1

BIN_jxwy_logind_USE_PACKAGES = login

#====================================================================================

BIN_login_unit_test_SRC_DIR := $(SRC_DIR)

BIN_login_unit_test_SRC = $(filter %test.cpp,$(call find_src_recursive,$(BIN_login_unit_test_SRC_DIR),cpp))

BIN_login_unit_test_INSTALL = 1

BIN_login_unit_test_USE_PACKAGES = unit_runner cppunit login

UNIT_TEST_BINS += login_unit_test

