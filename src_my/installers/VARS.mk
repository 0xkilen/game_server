INSTALL_jxwy_NAME := jxwy_server_full_COMMIT_TIME_rel.tar.gz
INSTALL_jxwy_OUTPUT_DIR_ROOT := jxwy
INSTALL_jxwy_BINS := jxwy_scened jxwy_logind jxwy_chatd jxwy_dbcached jxwy_gmd jxwy_log_serverd jxwy_mirrord jxwy_robot game_data merger analyse jxwy_routed jxwy_common_resourced
INSTALL_jxwy_CPS := config sqls managescripts
define INSTALL_jxwy_BUILD_CMD
$(call MD5TXT,$(1),$@)
$(call TGZ,$(1),$@)
endef

INSTALL_jxwy_analyse_NAME := jxwy_server_analyse.tar.gz
INSTALL_jxwy_analyse_OUTPUT_DIR_ROOT := jxwy
INSTALL_jxwy_analyse_BINS := analyse
INSTALL_jxwy_analyse_CPS := config
define INSTALL_jxwy_analyse_BUILD_CMD
$(call TGZ,$(1),$@)
endef

INSTALL_jxwy_merger_NAME := jxwy_server_merger.tar.gz
INSTALL_jxwy_merger_OUTPUT_DIR_ROOT := jxwy
INSTALL_jxwy_merger_BINS := merger
INSTALL_jxwy_merger_CPS := config
define INSTALL_jxwy_merger_BUILD_CMD
$(call TGZ,$(1),$@)
endef
