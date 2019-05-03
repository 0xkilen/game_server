LIB_scene_SRC_DIR := $(SRC_DIR)

LIB_scene_SRC = $(filter-out robot_main.cpp, \
	$(filter-out main.cpp, \
	$(call find_src_recursive,$(LIB_scene_SRC_DIR),c cc cpp,Collections)))

LIB_scene_USE_PACKAGES = utils net proto db config curl google-protocol-buffers route

#====================================================================================

BIN_jxwy_scened_SRC_DIR := $(SRC_DIR)

BIN_jxwy_scened_SRC = main.cpp \
	$(NULL)

BIN_jxwy_scened_INSTALL = 1

BIN_jxwy_scened_USE_PACKAGES = scene

#====================================================================================

BIN_jxwy_robot_SRC_DIR := $(SRC_DIR)

BIN_jxwy_robot_SRC = robot_main.cpp \
	$(NULL)

BIN_jxwy_robot_INSTALL = 1

BIN_jxwy_robot_USE_PACKAGES = scene


