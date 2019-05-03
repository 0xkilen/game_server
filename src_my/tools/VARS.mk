BIN_merger_SRC_DIR := $(SRC_DIR)

BIN_merger_SRC = $(addprefix merger/,$(notdir $(wildcard $(BIN_merger_SRC_DIR)/merger/*.cpp)))

BIN_merger_INSTALL = 1

BIN_merger_USE_PACKAGES = boost db proto config

#-----------------------------------------------------------------------

BIN_game_data_SRC_DIR := $(SRC_DIR)

BIN_game_data_SRC = game_data.cpp \
        $(NULL)

BIN_game_data_INSTALL = 1

BIN_game_data_USE_PACKAGES = boost config db

#-----------------------------------------------------------------------

BIN_fix_role_data_SRC_DIR := $(SRC_DIR)

BIN_fix_role_data_SRC = fix_role_data.cpp \
        $(NULL)

BIN_fix_role_data_INSTALL = 1

BIN_fix_role_data_USE_PACKAGES = boost config db

#-----------------------------------------------------------------------

BIN_analyse_SRC_DIR := $(SRC_DIR)

BIN_analyse_SRC = analyse.cpp \
$(NULL)

BIN_analyse_INSTALL = 1

BIN_analyse_USE_PACKAGES = boost scene db

#-----------------------------------------------------------------------

BIN_shandang_fanli_bufa_SRC_DIR := $(SRC_DIR)

BIN_shandang_fanli_bufa_SRC = shandang_fanli_bufa.cpp \
        $(NULL)

BIN_shandang_fanli_bufa_INSTALL = 1

BIN_shandang_fanli_bufa_USE_PACKAGES = utils boost config db proto scene

#-----------------------------------------------------------------------

BIN_fix_activity_data_SRC_DIR := $(SRC_DIR)

BIN_fix_activity_data_SRC = fix_activity_data.cpp \
        $(NULL)

BIN_fix_activity_data_INSTALL = 1

BIN_fix_activity_data_USE_PACKAGES = utils boost config db proto scene

#-----------------------------------------------------------------------

BIN_check_activity_data_SRC_DIR := $(SRC_DIR)

BIN_check_activity_data_SRC = check_activity_data.cpp \
        $(NULL)

BIN_check_activity_data_INSTALL = 1

BIN_check_activity_data_USE_PACKAGES = utils boost config db proto scene
