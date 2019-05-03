CP_managescripts_SRC_DIR := $(SRC_DIR)
CP_managescripts_SRC_FILENAMES = $(notdir $(wildcard $(CP_managescripts_SRC_DIR)/*.py))
CP_managescripts_DEST_DIR = $(INSTALL_ROOT)/bin/managescripts
