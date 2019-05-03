OPT =
CHECKED =
ALLOCATOR_PKG = 

FLAVOR_CFLAGS = -DEXIT_CLEANUP -DVALGRIND 

ifneq ($(TARGET_OS),linux)
$(error valgrind only supported on linux)
endif

INSTRUMENTATION_STYLE = valgrind

# ene.copytocd.conf depends on this suffix being _bin to copy the correct
# executables for nightly valgrind tests
REAL_BINARY_MARKER=_bin

$(call warn_if_from_env,VALGRIND_SUPPRESSION_FILES)
VALGRIND_SUPPRESSION_FILES?=$(notdir $(wildcard ./suppressions/*.supp))

$(call warn_if_from_env,VALGRIND_XML_OUT)
VALGRIND_XML_OUT=--xml=no

# Use --xml-file if we are using --xml=yes otherwise use --log-file.
VALGRIND_FILE_TYPE_OUT=--xml-file
ifeq ($(VALGRIND_XML_OUT),--xml=no)
VALGRIND_FILE_TYPE_OUT=--log-file
endif

$(call warn_if_from_env,VALGRIND_CONST_OPTS)
VALGRIND_CONST_OPTS ?= \
	--num-callers=50 \
	--leak-resolution=high \
	--leak-check=full \
	--error-limit=no \
	--gen-suppressions=all \
	-q

#
# Usage:  $(call VALGRIND_OPTS,$(TARGET_NAME))
#
$(call warn_if_from_env,VALGRIND_OPTS)
VALGRIND_OPTS ?= \
	$(VALGRIND_CONST_OPTS) \
	$(VALGRIND_FILE_TYPE_OUT)=$($(1)_BASENAME)_valgrind.%p \
	$(if $($(1)_TRACE_CHILDREN),\
		--trace-children=yes --child-silent-after-fork=yes,\
		--trace-children=no) \
	$(foreach file,$(VALGRIND_SUPPRESSION_FILES),--suppressions=\$$SCRIPT_DIR/$(file)) \
	$(VALGRIND_XML_OUT)

#
# Usage:  $(call BUILD_WRAPPER_SCRIPT,$(TARGET_NAME))
#
BUILD_WRAPPER_SCRIPT=\
$(if $($(1)_VALGRIND_EXCLUDE),\
(echo "\#!/bin/sh"; \
echo "SCRIPT_DIR=\`/usr/bin/dirname \$$0\`"; \
echo "\$$SCRIPT_DIR/$($(1)_BASENAME)$(REAL_BINARY_MARKER) \$$*";) > $($(1)),\
(echo "\#!/bin/sh"; \
echo "SCRIPT_DIR=\`/usr/bin/dirname \$$0\`"; \
echo "export LD_LIBRARY_PATH=\$$LD_LIBRARY_PATH:\$$SCRIPT_DIR/../lib64:\$$SCRIPT_DIR/../lib;"; \
echo 'VALGRIND="$${VALGRIND:-$(VALGRIND)}"'; \
echo "exec \$$VALGRIND \\"; \
echo "	$(call VALGRIND_OPTS,$(1)) \\"; \
echo "	\$$SCRIPT_DIR/$($(1)_BASENAME)$(REAL_BINARY_MARKER) \$$*";) > $($(1)))
