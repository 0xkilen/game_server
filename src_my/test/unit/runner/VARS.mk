LIB_unit_runner_SRC_DIR := $(SRC_DIR)

LIB_unit_runner_SRC = \
	$(call find_src,$(LIB_unit_runner_SRC_DIR),cpp) \
	$(NULL)

LIB_unit_runner_USE_PACKAGES = cppunit

