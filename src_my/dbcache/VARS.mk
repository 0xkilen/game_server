LIB_dbcache_SRC_DIR := $(SRC_DIR)

LIB_dbcache_SRC = $(filter-out %test.cpp,$(call find_src_recursive,$(LIB_dbcache_SRC_DIR),c cc cpp,Collections)) \
	$(NULL)

LIB_dbcache_USE_PACKAGES = utils proto net db config

#====================================================================================

BIN_jxwy_dbcached_SRC_DIR := $(SRC_DIR)

BIN_jxwy_dbcached_SRC = main.cpp

BIN_jxwy_dbcached_INSTALL = 1

BIN_jxwy_dbcached_USE_PACKAGES = dbcache

