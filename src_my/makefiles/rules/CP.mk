TARGET_NAME := CP_$(CP_NAME)

$(call ensure_defined,$(TARGET_NAME)_SRC_DIR)
$(call ensure_defined,$(TARGET_NAME)_DEST_DIR)

# Force evaluation of these now, so we don't repeatedly evaluate them later
$(TARGET_NAME)_SRC_FILENAMES := $($(TARGET_NAME)_SRC_FILENAMES)
$(TARGET_NAME)_BUILT_FILENAMES := $($(TARGET_NAME)_BUILT_FILENAMES)

ifeq (,$(strip $($(TARGET_NAME)_SRC_FILENAMES)$($(TARGET_NAME)_BUILT_FILENAMES)))
$(error CP_$(CP_NAME) contains no filenames to copy)
endif

$(TARGET_NAME)_FROMSRC := $(addprefix $($(TARGET_NAME)_DEST_DIR)/, $($(TARGET_NAME)_SRC_FILENAMES))
$(TARGET_NAME)_FROMSRC_TARGETS := $(filter-out $(CP_ALREADY_SEEN), $($(TARGET_NAME)_FROMSRC))
CP_ALREADY_SEEN := $(CP_ALREADY_SEEN) $($(TARGET_NAME)_FROMSRC)

$(TARGET_NAME)_FROMBUILT := $(addprefix $($(TARGET_NAME)_DEST_DIR)/, $($(TARGET_NAME)_BUILT_FILENAMES))
$(TARGET_NAME)_FROMBUILT_TARGETS := $(filter-out $(CP_ALREADY_SEEN), $($(TARGET_NAME)_FROMBUILT))
CP_ALREADY_SEEN := $(CP_ALREADY_SEEN) $($(TARGET_NAME)_FROMBUILT)

define COPY_CMD
	@$(call MKDIR,$(@D))
	@$(call RM_RF,$@)
	$(call quieten,$(PRINT_CP_CMDS))$(call CP_R,$<,$@)
endef

define GUNZIP_CMD
	@$(call MKDIR,$(@D))
	@$(call RM_RF,$@)
	@$(call CP_R,$<,$@.gz)
	$(call GUNZIP,$@.gz)
endef

ifneq (,$($(TARGET_NAME)_GUNZIP))
$(TARGET_NAME)_FROMSRC_ZIP := $(filter %.gz,$($(TARGET_NAME)_FROMSRC))
$(TARGET_NAME)_FROMSRC := $(filter-out %.gz,$($(TARGET_NAME)_FROMSRC))
$(TARGET_NAME)_FROMSRC_ZIP := $(patsubst %.gz,%,$($(TARGET_NAME)_FROMSRC_ZIP))

$($(TARGET_NAME)_FROMSRC_ZIP): $($(TARGET_NAME)_DEST_DIR)/%: $($(TARGET_NAME)_SRC_DIR)/%.gz
	$(GUNZIP_CMD)
endif

$($(TARGET_NAME)_FROMSRC_TARGETS): $($(TARGET_NAME)_DEST_DIR)/%: $($(TARGET_NAME)_SRC_DIR)/%
	$(COPY_CMD)

$($(TARGET_NAME)_FROMBUILT_TARGETS): $($(TARGET_NAME)_DEST_DIR)/%: $(BUILD_ROOT)/$($(TARGET_NAME)_SRC_DIR)/%
	$(COPY_CMD)

$(TARGET_NAME) := $($(TARGET_NAME)_FROMSRC) $($(TARGET_NAME)_FROMBUILT) $($(TARGET_NAME)_FROMSRC_ZIP)

.PHONY: $(TARGET_NAME)
$(TARGET_NAME): $($(TARGET_NAME))
