LIB_route_SRC_DIR := $(SRC_DIR)

LIB_route_SRC = $(filter-out %test.cpp,$(filter-out main.cpp,$(call find_src_recursive,$(LIB_route_SRC_DIR),c cc cpp,Collections))) \
	$(NULL)

LIB_route_USE_PACKAGES = utils proto net config

#====================================================================================

BIN_jxwy_routed_SRC_DIR := $(SRC_DIR)

BIN_jxwy_routed_SRC = main.cpp

BIN_jxwy_routed_INSTALL = 1

BIN_jxwy_routed_USE_PACKAGES = route
