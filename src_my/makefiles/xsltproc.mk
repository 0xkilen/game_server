ifeq ($(TARGET_OS),windows)
define XSLTPROC_gen
	@$(call MKDIR,$(@D))
	$(call quieten,$(PRINT_XSLTPROC_CMDS))$(XSLTPROC) $(XSLTPROC_FLAGS) --output $(subst /,\,$@) $(if $(strip $(XSLTPROC_PATH)),--path $(subst $(space),:,$(strip $(XSLTPROC_PATH)))) $(subst /,\,$(1)) $(subst /,\,$(2))
endef
else
define XSLTPROC_gen
	@$(call MKDIR,$(@D))
	$(call quieten,$(PRINT_XSLTPROC_CMDS))$(XSLTPROC) $(XSLTPROC_FLAGS) --output $@ --path "$(XSLTPROC_PATH)" $(1) $(2)
endef
endif

XSLTPROC_CMD = $(call XSLTPROC_gen,$(word 1,$(filter %.xsl,$^)),$(word 1,$(filter %.xml,$^)))

