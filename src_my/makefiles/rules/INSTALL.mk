TARGET_NAME := INSTALL_$(INSTALL_NAME)

$(TARGET_NAME)_SRC_DIR := $(INSTALL_ROOT)

$(TARGET_NAME)_SRC_FILES_UNIQUE_DEP_LIBS := \
	$(call unique, $(foreach bin, $($(TARGET_NAME)_BINS),\
		$(BIN_$(bin)_PKG_DEP_LIBS)))

foo = $(ANT_data_ingest_client_PKG_INSTALLERS)

$(TARGET_NAME)_SRC_FILES_UNIQUE_INSTALLERS := \
	$(call unique, $(call deref, $(call unique, \
		$(foreach bin, $($(TARGET_NAME)_BINS), \
			$(BIN_$(bin)_PKG_INSTALLERS)) \
		$(foreach ant, $($(TARGET_NAME)_ANTS), \
			$(ANT_$(ant)_PKG_INSTALLERS)) \
		$(foreach cp, $($(TARGET_NAME)_CPS), \
			$(CP_$(cp)_PKG_INSTALLERS)) \
		$(foreach csharp, $($(TARGET_NAME)_CSHARPS), \
			$(CSHARP_$(csharp)_PKG_INSTALLERS)) \
	)))

$(TARGET_NAME)_SRC_FILES_UNIQUE_BINS := \
	$(call unique, $(foreach bin, $($(TARGET_NAME)_BINS),\
		$(BIN_$(bin)) $(BIN_$(bin))$(REAL_BINARY_MARKER)))

$(TARGET_NAME)_SRC_FILES_UNIQUE_CPS := \
	$(call unique, $(foreach cp, $($(TARGET_NAME)_CPS),\
		$(CP_$(cp))))

$(TARGET_NAME)_SRC_FILES_UNIQUE_CSHARPS := \
	$(call unique, $(foreach csharp, $($(TARGET_NAME)_CSHARPS),\
		$(CSHARP_$(csharp))))

$(TARGET_NAME)_SRC_FILES_UNIQUE_ANTS := \
	$(call unique, $(foreach ant, $($(TARGET_NAME)_ANTS),\
		$(ANT_$(ant)_INSTALL_DIR)))

$(TARGET_NAME)_SRC_FILES_UNIQUE_ALIENS := \
	$(call unique, $(foreach alien, $($(TARGET_NAME)_ALIENS),\
		$(ALIEN_$(alien)_TARGETS))) \
	$(call unique, $(foreach alien, $($(TARGET_NAME)_ALIENS),\
		$(ALIEN_$(alien)_OTHER_TARGETS)))

$(TARGET_NAME)_SRC_FILES_UNIQUE_SYMBOLS := \
	$(call unique, $(foreach sym, $($(TARGET_NAME)_SYMBOLS),\
		$(BIN_$(sym)_SYMS)))

$(TARGET_NAME)_SRC_FILES_BASE := \
	$(call unique, $($(TARGET_NAME)_SRC_FILES_UNIQUE_DEP_LIBS) \
		$($(TARGET_NAME)_SRC_FILES_UNIQUE_INSTALLERS) \
		$($(TARGET_NAME)_SRC_FILES_UNIQUE_BINS) \
		$($(TARGET_NAME)_SRC_FILES_UNIQUE_CPS) \
		$($(TARGET_NAME)_SRC_FILES_UNIQUE_CSHARPS) \
		$($(TARGET_NAME)_SRC_FILES_UNIQUE_ANTS) \
		$($(TARGET_NAME)_SRC_FILES_UNIQUE_ALIENS) \
		$($(TARGET_NAME)_SRC_FILES_UNIQUE_SYMBOLS))

$(TARGET_NAME)_SRC_FILES_FILTER_OUT := \
	$(call unique, $(foreach cp, $($(TARGET_NAME)_FILTER_OUT_CPS),\
		$(CP_$(cp))))

$(TARGET_NAME)_SRC_FILES := \
		$(filter $($(TARGET_NAME)_SRC_DIR)/%,\
			$(filter-out $($(TARGET_NAME)_SRC_FILES_FILTER_OUT),\
				$($(TARGET_NAME)_SRC_FILES_BASE)))

$(TARGET_NAME)_BUILD_PATH := $(BUILD_ROOT)/installers/$(INSTALL_NAME)

$(TARGET_NAME)_DEST_DIR := $($(TARGET_NAME)_BUILD_PATH)/$($(TARGET_NAME)_OUTPUT_DIR_ROOT)

$(TARGET_NAME)_FILES := $(patsubst $($(TARGET_NAME)_SRC_DIR)/%,$($(TARGET_NAME)_DEST_DIR)/%,$($(TARGET_NAME)_SRC_FILES))

$(TARGET_NAME) := $(INSTALL_ROOT)/installers/$($(TARGET_NAME)_NAME)

$(TARGET_NAME)_STATIC_FILES_OUT := $(addprefix $($(TARGET_NAME)_DEST_DIR)/,$($(TARGET_NAME)_STATIC_FILES))

$($(TARGET_NAME)_STATIC_FILES_OUT): $($(TARGET_NAME)_DEST_DIR)/%: $($(TARGET_NAME)_STATIC_SRC_DIR)/%
	$(COPY_CMD)


$($(TARGET_NAME)_FILES): $($(TARGET_NAME)_DEST_DIR)/%: $($(TARGET_NAME)_SRC_DIR)/%
	$(COPY_CMD)

$($(TARGET_NAME)): TARGET_NAME := $(TARGET_NAME)
$($(TARGET_NAME)): $($(TARGET_NAME)_FILES) $($(TARGET_NAME)_STATIC_FILES_OUT)
	@$(call MKDIR,$(@D))
	$(call $(TARGET_NAME)_BUILD_CMD,$($(TARGET_NAME)_BUILD_PATH))

.PHONY: $(TARGET_NAME)
$(TARGET_NAME): $($(TARGET_NAME))
