DUMP_PKG_TYPES = BIN LIB SHLIB

DEP_SORT_INPUT := $(BUILD_ROOT)/dep_sort_input
$(shell echo > $(DEP_SORT_INPUT))

APPEND_DEP_SORT_INPUT = \
	$(shell echo $(1) $($(2)_$(1)_USE_PACKAGES) >> $(DEP_SORT_INPUT))

DEPS_FOR_PKG = \
	$(foreach pkgtype,$(DUMP_PKG_TYPES), \
		$(foreach pkg,$(ALL_$(pkgtype)S), \
			$(call APPEND_DEP_SORT_INPUT,$(pkg),$(pkgtype)))) \
	$(shell $(AWK) -f makefiles/topo_sort.awk root=$(1) $(DEP_SORT_INPUT))

ALL_PKGS = $(foreach pkgtype,$(DUMP_PKG_TYPES),$(ALL_$(pkgtype)S))

DUMP_PKG_FILE = $(BUILD_ROOT)/dep.dot

DUMP_PKG_USES = \
	$(shell echo \
		$(foreach pkgtype,$(DUMP_PKG_TYPES), \
			$(foreach pkg,$(filter-out $(ALLOCATOR_PKG),$($(pkgtype)_$(1)_USE_PACKAGES)),\"$(1)\" '->' \"$(pkg)\"';' )) \
		>> $(DUMP_PKG_FILE)) \

.PHONY: dump-dep-graph
dump-dep-graph:
	@echo Dependencies dumped to $(DUMP_PKG_FILE) \
	$(shell echo digraph \{ > $(DUMP_PKG_FILE)) \
	$(foreach pkg, \
		$(if $(DEP_GRAPH_ROOT), \
			$(call DEPS_FOR_PKG,$(DEP_GRAPH_ROOT)), \
			$(ALL_PKGS)), \
		$(call DUMP_PKG_USES,$(pkg))) \
	$(shell echo \} >> $(DUMP_PKG_FILE))
