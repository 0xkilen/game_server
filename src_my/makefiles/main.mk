ifneq ($(filter endeca-unique,$(.FEATURES)),endeca-unique)
define MSG

This Makefile requires a special build of gmake implementing the
endeca-unique native function.  This is gmake $(MAKE_VERSION)
For example, try /usr/dev_tools/trunk-975955/bin/make
endef
$(error $(MSG))
endif

ifneq ($(MAKE_VERSION),3.82)
define MSG

This Makefile requires gmake 3.82, and this is gmake $(MAKE_VERSION)
For example, try /usr/dev_tools/trunk-975955/bin/make
endef
$(error $(MSG))
endif

#
# Include the user preferences file
#
-include Makefile.local

#
# Include print controls for compile and link
include makefiles/print_controls.mk

# RELEASE_VERSION can be specified on the make command
# line; this percolates down to the various modules' builds

# get the build start time

# Disable builtin rules
MAKEFLAGS += -r
.SUFFIXES:

# Remove possibly hosed output files
.DELETE_ON_ERROR:

# Clear out PATH and LD_LIBRARY_PATH to avoid getting
# contaminated by things in the user environment
PATH:=
export PATH
LD_LIBRARY_PATH:=
export LD_LIBRARY_PATH

include makefiles/make-support.mk

$(call warn_if_from_env,CXX)
$(call warn_if_from_env,CMD_ARGS)
$(call warn_if_from_env,CMD_PREFIX)

$(call warn_if_from_env,OUT_BASE)
OUT_BASE := $(call normalize_path_or_default,$(OUT_BASE),..)
ALL_OBJS :=
ALL_DEPS :=
UNIT_TEST_BINS :=
CLUSTER_TEST_BINS :=
ALL_PACKAGES :=
ALL_ENTITY_TYPES := BIN ANT SHLIB LIB CSHARP ALIEN CP

include makefiles/arch.mk
include makefiles/$(TARGET_OS).mk
include makefiles/flavor.mk
include makefiles/buildmode.mk
include makefiles/$(TOOLCHAIN_CHOICE).mk

.PHONY: default print-var build-var test unit-test-bins build-obj
.PHONY: clean clean-install compile doxygen love turkey help rocks easter-egg
.PHONY: installers clean-installers unit-test-objs production clobber
default:

$(shell echo 'Make target flavor:   $(FLAVOR)' 1>&2)
$(shell echo 'Make toolchain    :   $(TOOLCHAIN_CHOICE)' 1>&2)
$(call print_timestamp,"svnversion ",$(BASIC_TIMESTAMP_PRINT_FLAG))


PRODUCT_NAME := Unspecified
# Hack until we can get tr.exe (or some similar) to run reliably on Windows
PRODUCT_LCASE_NAME := unspecified
PRODUCT_VERSION := 9.9.9
VERSION := INTERNAL
ifneq (,$(wildcard ../.git))
BUILD := $(shell $(GIT) rev-parse --short=10 HEAD)-$(FLAVOR)
else
BUILD := $(subst $(space),_,$(subst :,-,$(shell $(SVNVERSION))))-$(FLAVOR)
endif
BUILD_VERSION := $(if $(USE_CCACHE),ccache,$(shell $(DATE) -u +%Y%m%d%H%M%S%Z)-r$(BUILD))

$(call warn_if_from_env,OUT_ROOT)
OUT_ROOT := $(call normalize_path_or_default,$(OUT_ROOT),$(OUT_BASE)/$(TARGET_PLATFORM)$(BUILD_MODE))

$(call warn_if_from_env,BUILD_ROOT)
BUILD_ROOT := $(call normalize_path_or_default,$(BUILD_ROOT),$(OUT_ROOT)/build$(BITS))
ABS_BUILD_ROOT := $(call abs_dir,$(BUILD_ROOT))

$(call warn_if_from_env,INSTALL_ROOT)
INSTALL_ROOT := $(call normalize_path_or_default,$(INSTALL_ROOT),$(OUT_ROOT)/install)
ABS_INSTALL_ROOT := $(call abs_dir,$(INSTALL_ROOT))

ABS_SRC_ROOT := $(call abs_dir,.)

$(call warn_if_from_env,ENDECA_ROOT)
ENDECA_ROOT := $(call normalize_path_or_default,$(ENDECA_ROOT),$(ABS_INSTALL_ROOT))
export ENDECA_ROOT

$(call warn_if_from_env,MDEX_INSTALL_ROOT)
MDEX_INSTALL_ROOT := $(call normalize_path_or_default,$(MDEX_INSTALL_ROOT),$(ABS_INSTALL_ROOT))
export MDEX_INSTALL_ROOT

$(call warn_if_from_env,MDEX_TEST_ROOT)
MDEX_TEST_ROOT := $(call normalize_path_or_default,$(MDEX_TEST_ROOT),$(ABS_INSTALL_ROOT))
export MDEX_TEST_ROOT

# Goals can either be names (like "clean") or paths. If it's a path, it 
# should start with OUT_ROOT.
$(foreach goal, $(MAKECMDGOALS), $(if $(call has_slashes,$(goal)), $(if $(filter-out $(OUT_ROOT)/%,$(goal)), $(warning NOTE: the goal should probably start with "$(OUT_ROOT)"))))

SHARED_PREBUILTS_DIR := /usr/dev_tools/prebuilts/shared
PREBUILTS_DIR := /usr/dev_tools/prebuilts/$(TARGET_PLATFORM)
ABS_PREBUILTS_DIR := $(call abs_dir,$(PREBUILTS_DIR))

export DEV_TOOLS

include makefiles/depend_on_var.mk
include makefiles/bison.mk
include makefiles/re2c.mk
include makefiles/xsltproc.mk
include makefiles/elua.mk
include makefiles/protobuf.mk
ifeq ($(TARGET_OS),windows)
include makefiles/rc.mk
include makefiles/mc.mk
include makefiles/midl.mk
include makefiles/csc.mk
endif

MAKE_OUT_DIR = $(BUILD_ROOT)/$(1)

define INCLUDE_ONE
SRC_DIR := $(patsubst %/,%,$(dir $(1)))
OUT_DIR := $(call MAKE_OUT_DIR,$$(SRC_DIR))
include $(1)
endef

# $(1):  filename to look for (e.g. VARS.mk)
# $(2):  list of filenames to look in (e.g. $(ALL_MKS))
INCLUDE_MKS = \
	$(foreach f,$(filter %/$(1),$(2)),$(eval $(call INCLUDE_ONE, $(f))))

#
# Create output directory, and add flavor name link
#
$(shell $(call MKDIR,$(BUILD_ROOT)))
IGNORE := $(shell $(call LN_S,$(TARGET_PLATFORM)$(BUILD_MODE),$(TARGET_PLATFORM)-$(FLAVOR),$(OUT_BASE)/))

# 
# create link to old-style ARCH_OS name to support legacy
# build and test systems.
#
include makefiles/old_archos.mk
IGNORE := $(shell $(call LN_S,$(TARGET_PLATFORM)$(BUILD_MODE)/install,$(OLD_ARCH_OS)$(OLD_ARCHOS_FLAVOR),$(OUT_BASE)/))

$(call print_timestamp,"finding mks",$(BASIC_TIMESTAMP_PRINT_FLAG))

#
# enumerate the *.mk files
# explicitly list the directories so that paths do not start with './'
#
ALL_MKS := $(FIND_MKS)

#
# The following mk files talk about stuff that doesn't exist in 64 bits.
# (or that particular 64-bit platform)
#
MK_EXCLUDES_x86_64 = \
	$(NULL)

#
# The following mk files talk about stuff that doesn't exist in 32 bits.
#
MK_EXCLUDES_$(TARGET_CPU) ?= \
	$(NULL)

#
# The following mk files talk about stuff that isn't built on Windows.
#
MK_EXCLUDES_windows = \
	$(NULL)

ALL_MKS := $(filter-out $(MK_EXCLUDES_$(TARGET_OS)) $(MK_EXCLUDES_$(TARGET_CPU)),$(ALL_MKS))

$(call print_timestamp,"vars       ",$(BASIC_TIMESTAMP_PRINT_FLAG))

#
# User variable definitions
#
$(call INCLUDE_MKS,VARS.mk,$(ALL_MKS))

INTROSPECT_TARGETS = $(patsubst $(1)_%_SRC_DIR,%,$(filter $(1)_%_SRC_DIR,$(.VARIABLES)))

$(call print_timestamp,"pkg default",$(BASIC_TIMESTAMP_PRINT_FLAG))

#
# Generate packages for user-defined libraries
#
ALL_LIBS := $(call INTROSPECT_TARGETS,LIB)
LIB_PKG_DEFAULTER := $(BUILD_ROOT)/LIB_PKG_DEFAULTS.mk
ifneq (,$(ALL_LIBS))
$(shell (for libname in $(ALL_LIBS); do \
	echo "PKG_$${libname}_INC_DIRS ?= \$$(call unique_by_rev,\$$(LIB_$${libname}_OTHER_INC_DIRS) \$$(call deref,\$$(patsubst %,PKG_%_INC_DIRS,\$$(LIB_$${libname}_USE_PACKAGES))))"; \
	echo "PKG_$${libname}_GENERATED_HEADERS ?= \$$(call unique,\$$(addprefix \$$(LIB_$${libname}_OUT_DIR)/,\$$(LIB_$${libname}_GENERATED_HEADERS)) \$$(call deref,\$$(patsubst %,PKG_%_GENERATED_HEADERS,\$$(LIB_$${libname}_USE_PACKAGES))))"; \
	echo "PKG_$${libname}_DEP_LIBS ?= \$$(call unique_by_rev,\$$(LIB_$${libname}) \$$(call deref,\$$(patsubst %,PKG_%_DEP_LIBS,\$$(LIB_$${libname}_USE_PACKAGES))))"; \
	echo "PKG_$${libname}_LIB_DIRS ?= \$$(call unique_by_rev,\$$(call deref,\$$(patsubst %,PKG_%_LIB_DIRS,\$$(LIB_$${libname}_USE_PACKAGES))))"; \
	echo "PKG_$${libname}_LIB_NAMES ?= \$$(call unique_by_rev,\$$(call deref,\$$(patsubst %,PKG_%_LIB_NAMES,\$$(LIB_$${libname}_USE_PACKAGES))))"; \
	echo "PKG_$${libname}_USE_PACKAGES ?= \$$(LIB_$${libname}_USE_PACKAGES)"; \
	echo "PKG_$${libname}_FOR_SHLIB ?= \$$(LIB_$${libname}_FOR_SHLIB)"; \
	echo "PKG_$${libname}_INSTALLERS ?= \$$(call unique,\$$(LIB_$${libname}_INSTALLERS) \$$(call deref,\$$(patsubst %,PKG_%_INSTALLERS,\$$(LIB_$${libname}_USE_PACKAGES))))"; \
	echo "ALL_PACKAGES += $${libname}"; \
	echo; \
	done) > $(LIB_PKG_DEFAULTER))

include $(LIB_PKG_DEFAULTER)
endif

ALL_SHLIBS := $(call INTROSPECT_TARGETS,SHLIB)
SHLIB_PKG_DEFAULTER := $(BUILD_ROOT)/SHLIB_PKG_DEFAULTS.mk
ifneq (,$(ALL_SHLIBS))
$(shell (for libname in $(ALL_SHLIBS); do \
	echo "PKG_$${libname}_INC_DIRS ?= \$$(call unique_by_rev,\$$(SHLIB_$${libname}_OTHER_INC_DIRS) \$$(call deref,\$$(patsubst %,PKG_%_INC_DIRS,\$$(SHLIB_$${libname}_USE_PACKAGES))))"; \
	echo "PKG_$${libname}_GENERATED_HEADERS ?= \$$(call unique,\$$(addprefix \$$(SHLIB_$${libname}_OUT_DIR)/,\$$(SHLIB_$${libname}_GENERATED_HEADERS)) \$$(call deref,\$$(patsubst %,PKG_%_GENERATED_HEADERS,\$$(SHLIB_$${libname}_USE_PACKAGES))))"; \
	echo "PKG_$${libname}_DEP_LIBS ?= \$$(call unique_by_rev,\$$(SHLIB_$${libname}) \$$(call deref,\$$(patsubst %,PKG_%_DEP_LIBS,\$$(SHLIB_$${libname}_USE_PACKAGES))))"; \
	echo "PKG_$${libname}_LIB_DIRS ?= \$$(call unique_by_rev,\$$(call deref,\$$(patsubst %,PKG_%_LIB_DIRS,\$$(SHLIB_$${libname}_USE_PACKAGES))))"; \
	echo "PKG_$${libname}_LIB_NAMES ?= \$$(call unique_by_rev,\$$(call deref,\$$(patsubst %,PKG_%_LIB_NAMES,\$$(SHLIB_$${libname}_USE_PACKAGES))))"; \
	echo "PKG_$${libname}_USE_PACKAGES ?= \$$(SHLIB_$${libname}_USE_PACKAGES)"; \
	echo "PKG_$${libname}_FOR_SHLIB ?= 1"; \
	echo "PKG_$${libname}_INSTALLERS ?= \$$(call unique,\$$(LIB_$${libname}_INSTALLERS) \$$(call deref,\$$(patsubst %,PKG_%_INSTALLERS,\$$(SHLIB_$${libname}_USE_PACKAGES))))"; \
	echo "ALL_PACKAGES += $${libname}"; \
	echo; \
	done) > $(SHLIB_PKG_DEFAULTER))

include $(SHLIB_PKG_DEFAULTER)
endif

ALL_CSHARPS := $(call INTROSPECT_TARGETS,CSHARP)
CSHARP_PKG_DEFAULTER := $(BUILD_ROOT)/CSHARP_PKG_DEFAULTS.mk
ifneq (,$(ALL_CSHARPS))
$(shell (for libname in $(ALL_CSHARPS); do \
	echo "PKG_$${libname}_DEP_LIBS ?= \$$(call unique_by_rev,\$$(CSHARP_$${libname}) \$$(call deref,\$$(patsubst %,PKG_%_DEP_LIBS,\$$(CSHARP_$${libname}_USE_PACKAGES))))"; \
	echo "PKG_$${libname}_USE_PACKAGES ?= \$$(CSHARP_$${libname}_USE_PACKAGES)"; \
	echo "ALL_PACKAGES += $${libname}"; \
	echo; \
	done) > $(CSHARP_PKG_DEFAULTER))

include $(CSHARP_PKG_DEFAULTER)
endif

ANT_PKG_DEFAULTER := $(BUILD_ROOT)/ANT_PKG_DEFAULTS.mk
ALL_ANTS := $(call INTROSPECT_TARGETS,ANT)
ifneq (,$(ALL_ANTS))
$(shell (for antname in $(ALL_ANTS); do \
	echo "PKG_$${antname}_DEP_LIBS ?= \$$(call unique_by_rev,\$$(ANT_$${antname}) \$$(call deref,\$$(patsubst %,PKG_%_DEP_LIBS,\$$(ANT_$${antname}_USE_PACKAGES))))"; \
	echo "ALL_PACKAGES += $${antname}"; \
	done) > $(ANT_PKG_DEFAULTER))

include $(ANT_PKG_DEFAULTER)
endif

CP_PKG_DEFAULTER := $(BUILD_ROOT)/CP_PKG_DEFAULTS.mk
ALL_CPS := $(call INTROSPECT_TARGETS,CP)
ifneq (,$(ALL_CPS))
$(shell (for cpname in $(ALL_CPS); do \
	echo "ALL_PACKAGES += $${cpname}"; \
	done) > $(CP_PKG_DEFAULTER))

include $(CP_PKG_DEFAULTER)
endif

#
# Validate that all USE_PACKAGES vars refer to packages that actually exist
#
$(call print_timestamp,"check deps ",$(BASIC_TIMESTAMP_PRINT_FLAG))

check_uses = $(if $(2),$(error $(1) uses non-existent package(s) $(2)))

$(foreach use,\
	$(filter $(foreach type,$(ALL_ENTITY_TYPES),$(type)_%_USE_PACKAGES),$(.VARIABLES)),\
	$(call check_uses,$(use:%_USE_PACKAGES=%),$(filter-out $(ALL_PACKAGES),$($(use))))\
	)

#
# Validate that shared libraries are constructed from shareable packages
#
$(call print_timestamp,"check shlib",$(BASIC_TIMESTAMP_PRINT_FLAG))

SHAREABLE = $(patsubst PKG_%_FOR_SHLIB,%,$(foreach pkg,$(filter PKG_%_FOR_SHLIB,$(.VARIABLES)),$(if $($(pkg)),$(pkg))))
$(foreach pkg,$(SHAREABLE),$(foreach lib,$(PKG_$(pkg)_USE_PACKAGES), \
 $(if $(PKG_$(lib)_FOR_SHLIB),,$(error package $(lib) must be FOR_SHLIB, because it is used by package $(pkg)))))

#
# Topologically sort the libraries according to their USE_PACKAGES
# relationships, so that the makefiles/rules/LIB.mk conversion of PKG_*
# variables from recursive to immediate maximally avoids redundant computation.
#
$(call print_timestamp,"sort libs  ",$(BASIC_TIMESTAMP_PRINT_FLAG))

LIB_SORT_INPUT := $(BUILD_ROOT)/lib_sort_input
$(shell echo > $(LIB_SORT_INPUT))

APPEND_LIB_SORT_INPUT = \
    $(call print_if,$(PRINT_TOPO_SORT_LINES),$(1) depends on $(LIB_$(1)_USE_PACKAGES)) \
	$(shell echo $(1) $(filter $(ALL_LIBS),$(LIB_$(1)_USE_PACKAGES)) \
		>> $(LIB_SORT_INPUT))

$(foreach lib,$(ALL_LIBS),$(call APPEND_LIB_SORT_INPUT,$(lib)))

ALL_LIBS := $(shell $(AWK) -f makefiles/topo_sort.awk $(LIB_SORT_INPUT))

#
# Set vars based on the user variables
#
$(call print_timestamp,"vars       ",$(BASIC_TIMESTAMP_PRINT_FLAG))

INCLUDE_TYPE=vars
ENTITY_TYPE=LIB
include makefiles/includer.mk

ENTITY_TYPE=SHLIB
include makefiles/includer.mk

ALL_BINS := $(call INTROSPECT_TARGETS,BIN)
ENTITY_TYPE=BIN
include makefiles/includer.mk

ENTITY_TYPE=CSHARP
include makefiles/includer.mk

# ant tests will be added to this variable
ALL_TEST_ANTS :=
ENTITY_TYPE=ANT
include makefiles/includer.mk

ALL_ALIENS := $(call INTROSPECT_TARGETS,ALIEN)
ENTITY_TYPE=ALIEN
include makefiles/includer.mk

ALL_CPS := $(call INTROSPECT_TARGETS,CP)
ENTITY_TYPE=CP
include makefiles/includer.mk

ALL_INSTALLS := $(patsubst INSTALL_%_BUILD_CMD,%,$(filter INSTALL_%_BUILD_CMD,$(.VARIABLES)))
ENTITY_TYPE=INSTALL
include makefiles/includer.mk

$(call print_timestamp,"rules      ",$(BASIC_TIMESTAMP_PRINT_FLAG))

#
# Generate rules based on the user variables
#
INCLUDE_TYPE=rules
$(call print_timestamp,"  lib      ",$(BASIC_TIMESTAMP_PRINT_FLAG))
ENTITY_TYPE=LIB
include makefiles/includer.mk

$(call print_timestamp,"  shlib    ",$(BASIC_TIMESTAMP_PRINT_FLAG))
ENTITY_TYPE=SHLIB
include makefiles/includer.mk

$(call print_timestamp,"  cp       ",$(BASIC_TIMESTAMP_PRINT_FLAG))
ENTITY_TYPE=CP
include makefiles/includer.mk

$(call print_timestamp,"  bin      ",$(BASIC_TIMESTAMP_PRINT_FLAG))
ENTITY_TYPE=BIN
include makefiles/includer.mk

$(call print_timestamp,"  csharp   ",$(BASIC_TIMESTAMP_PRINT_FLAG))
ENTITY_TYPE=CSHARP
include makefiles/includer.mk

$(call print_timestamp,"  ant      ",$(BASIC_TIMESTAMP_PRINT_FLAG))
ENTITY_TYPE=ANT
include makefiles/includer.mk

$(call print_timestamp,"  alien    ",$(BASIC_TIMESTAMP_PRINT_FLAG))
ENTITY_TYPE=ALIEN
include makefiles/includer.mk

$(call print_timestamp,"  install  ",$(BASIC_TIMESTAMP_PRINT_FLAG))
ENTITY_TYPE=INSTALL
include makefiles/includer.mk

$(call print_timestamp,"user rules ",$(BASIC_TIMESTAMP_PRINT_FLAG))

#
# User rule definitions
#
$(call INCLUDE_MKS,RULES.mk,$(ALL_MKS))

$(call print_timestamp,"incl .d    ",$(BASIC_TIMESTAMP_PRINT_FLAG))

#
# Automatically generated depfiles
#
ifneq (,$(ALL_DEPS))
-include $(ALL_DEPS)
endif

#
# Phony targets
#
# The ONLY_FOR_PREFIX and FILTERED_ stuff is for builds initiated in
# subdirectories
#

FILTERED_ROOT = $(BUILD_ROOT)$(if $(ONLY_FOR_PREFIX),/$(ONLY_FOR_PREFIX))
ALL_BUILD_NAMES = $(foreach type, \
	$(ALL_ENTITY_TYPES), \
	$(ALL_$(type)S:%=$(type)_%))

DEFAULT_BUILD_NAMES = $(foreach name,$(ALL_BUILD_NAMES),$(if $($(name)_ONLY_IF_NEEDED),,$(name)))

# 'test' runs all Ant test targets and all binaries registered in UNIT_TEST_BINS
# built at or under the current directory
test: \
	$(foreach anttest,$(ALL_TEST_ANTS),\
		$(if $(filter $(FILTERED_ROOT)/%,$(TEST_ANT_$(anttest))), \
			ant-test-$(anttest))) \
	$(foreach testbin,$(UNIT_TEST_BINS), \
		$(if $(filter $(ONLY_FOR_PREFIX)%,$(BIN_$(testbin)_SRC_DIR)), \
			run-$(testbin)))

unit-test-bins: \
	$(foreach testbin,$(UNIT_TEST_BINS), \
		$(if $(filter $(ONLY_FOR_PREFIX)%,$(BIN_$(testbin)_SRC_DIR)), \
			BIN_$(testbin)))

# 'default' builds all targets at or under the current directory and any
# installable things built from there
default: \
	$(filter $(INSTALL_ROOT)/%,\
		$(call deref, \
			$(foreach target,$(DEFAULT_BUILD_NAMES), \
				$(if $(filter $(ONLY_FOR_PREFIX)%,$($(target)_SRC_DIR)),$(target)) \
				$(if $(filter $(FILTERED_ROOT)%,$($(target)_SRC_DIR)),$(target)) \
				))) \
	$(filter $(FILTERED_ROOT)/%, \
		$(ALL_OBJS) $(call deref,$(DEFAULT_BUILD_NAMES)))

production: \
	$(filter-out \
		LIB% SHLIB% $(UNIT_TEST_BINS:%=BIN_%),\
		$(DEFAULT_BUILD_NAMES))

# 'clobber' is the nuclear option for cleaning; it removes all build output
# for this flavor using rm -rf
clobber:
	$(call RM_F,$(OUT_BASE)/$(TARGET_PLATFORM)-$(FLAVOR))
	$(call RM_F,$(OUT_BASE)/$(OLD_ARCH_OS)$(OLD_ARCHOS_FLAVOR))
	$(call RM_RF,$(OUT_ROOT))

# 'clean' removes all build output at or under the current directory, and
# any installed things that were built from there
# also, clean out all installer files
clean: clean-installers
	$(call RM_RF,$(FILTERED_ROOT) \
		$(filter $(INSTALL_ROOT)/%, \
			$(call deref, \
				$(foreach target,$(ALL_BUILD_NAMES), \
					$(if $(filter $(ONLY_FOR_PREFIX)%,$($(target)_SRC_DIR)), \
						$(target) $(target)_SYMS) $(target)$(REAL_BINARY_MARKER)))))

clean-install:
	$(call RM_RF,$(INSTALL_ROOT))

installers:	$(call deref, $(addprefix INSTALL_,$(ALL_INSTALLS)))

clean-installers:
	$(call RM_RF,$(INSTALL_ROOT)/installers $(BUILD_ROOT)/installers)

# compile object files, without linking
compile: $(filter $(FILTERED_ROOT)/%, $(ALL_OBJS))

# compile unit test object files
unit-test-objs: $(foreach bin,$(UNIT_TEST_BINS),compile-$(bin))

ifneq (,$(filter print-var,$(MAKECMDGOALS)))
ifneq (,$(VARNAME))
print-var:
	@echo '$(VARNAME)=$(subst ',",$($(VARNAME)))'
else
ALL_VARS_FILE = $(BUILD_ROOT)/ALL_VARS
$(shell $(call RM_F,$(ALL_VARS_FILE)))
ECHO_WORD = $(shell echo -n '$(subst ',",$(1)) ' >> $(ALL_VARS_FILE))
ALL_VARS := $(.VARIABLES)
BAD_VARS := .% ensure_defined %_CMD STATIC_LINK_CXX STATIC_LINK \
	abs_dir make_files_abs
GOOD_VARS := $(foreach v,$(filter-out $(BAD_VARS),$(ALL_VARS)),$(if $(filter-out automatic,$(origin $(v))),$(v)))
VAR_TARGETS := $(sort $(GOOD_VARS:%=print-var-%))
.PHONY: $(VAR_TARGETS)
$(VAR_TARGETS): print-var-%:
	@: $(call map,ECHO_WORD,$(*)=$($(*)))
	@echo >> $(ALL_VARS_FILE)

print-var: $(VAR_TARGETS)
	@$(CAT) $(ALL_VARS_FILE)

endif
endif

ifneq (,$(filter build-var,$(MAKECMDGOALS)))
build-var: $($(VARNAME))
endif

ifneq (,$(filter build-obj,$(MAKECMDGOALS)))
ifeq (,$(SRC))
$(error "The build-obj target requires the variable SRC to be set")
endif
endif

build-obj: $(FILTERED_ROOT)/$(basename $(SRC)).$(O)

# For unknown run-% targets, print a list of known targets
run-% :
	@echo $@ is not a valid run target. Try one of these: $(addprefix run-,$(sort $(ALL_BINS)))

DOXYGEN_DOC_DIR ?= $(call abs_dir,$(OUT_BASE))/doxygen_out

doxygen:
	cd $(ABS_SRC_ROOT)/.. ; env - DOXYGEN_PATH=$(DOXYGEN) DOT_PATH=$(DOT) DOC_DIR=$(DOXYGEN_DOC_DIR) ./makedoc.sh
	@echo If all went well, the doxygen output can be browsed at this URL:
	@echo   file://$(DOXYGEN_DOC_DIR)/index.html
	@echo if not, please see $(call abs_dir,$(OUT_BASE))/doxygen_warnings.txt for details

turkey:
	@$(CAT) makefiles/turkey

help:
	@$(CAT) makefiles/doc/help

rocks:
	@echo You bet it does.

easter-egg:
	@$(CAT) makefiles/easter-egg

love:
	@$(CAT) makefiles/love

include makefiles/dump_dep_graph.mk

$(call print_timestamp,"end parsing",$(BASIC_TIMESTAMP_PRINT_FLAG))
