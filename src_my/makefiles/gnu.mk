# The interaction of ccache and distcc is slightly obscure.  Each one,
# in isolation, just has to be prepended onto the compiler command.
# To use both, however, you must just prepend ccache and put distcc
# instead into the environment variable CCACHE_PREFIX.  Otherwise
# ccache tries to treat distcc as the compiler (eg, hash its checksum,
# invoke it with -E, etc.).
CC = $(if $(USE_CCACHE),$(CCACHE) ,\
       $(if $(USE_DISTCC),$(DISTCC) ) )$(DEV_TOOLS)/bin/gcc
CXX_NO_CCACHE = $(if $(USE_DISTCC),$(DISTCC) )$(DEV_TOOLS)/bin/g++
CXX = $(if $(USE_CCACHE),$(CCACHE) $(DEV_TOOLS)/bin/g++, $(CXX_NO_CCACHE))

# ccache is configured using environment variables rather than command
# line flags.  Thus we must set and export any relevant items.
CCACHE_DIR ?= /home/aaan/ccache
#CCACHE_DIR ?= /net/devfiler.us.oracle.com/export/public-ccache/public-ccache
export CCACHE_DIR
CCACHE_UMASK = 000
export CCACHE_UMASK
# Note that ccache doesn't like an empty CCACHE_PREFIX, so we disable distcc by
# setting CCACHE_PREFIX to env(1), which just runs the command.
CCACHE_PREFIX = $(if $(USE_DISTCC),$(DISTCC),/usr/bin/env)
export CCACHE_PREFIX
# by default ccache uses CCACHE_DIR for temp; a local dir avoids NFS traffic
CCACHE_TEMPDIR=$(if $(TMPDIR),$(TMPDIR),/tmp)/ccache
export CCACHE_TEMPDIR
# gcc -g -E inserts some absolute pathnames into files, causing cache misses
CCACHE_BASEDIR=$(shell $(PWD))
export CCACHE_BASEDIR

AR := $(DEV_TOOLS)/bin/ar
OBJCOPY := $(DEV_TOOLS)/bin/objcopy

PGO_ROOT := $(OUT_BASE)/$(TARGET_PLATFORM)$(BUILD_MODE)/pgo-data/empty
ifneq (,$(MAKE_PGO)$(USE_PGO))
 ifneq (,$(USE_DISTCC))
   $(error distcc incompatible with PGO))
 endif
 PGO := $(if $(MAKE_PGO),--profile-generate,-Wcoverage-mismatch --profile-use)=$(PGO_ROOT)
endif

LINK_LIB_ARG = -l$(1)
LINK_PATH_ARG = -L$(1)
# always try to load so's from the libdir
TOOL_LDFLAGS = -Wl,-rpath,'$$ORIGIN/../${LIBDIRNAME}' $(if $(LTCG),-flto,) $(PGO)
# always use libdl
TOOL_LDLIBS = -ldl -lpthread

CFLAGS_x86_64 = -m64 -march=core2 -mtune=corei7
LDFLAGS_x86_64 = $(CFLAGS_x86_64)

CFLAGS_sparcv9 = -m64 -D__sparcv9
LDFLAGS_sparcv9 = $(CFLAGS_sparcv9)

CFLAGS_CPU = $(CFLAGS_$(TARGET_CPU))
LDFLAGS_CPU = $(LDFLAGS_$(TARGET_CPU))

UC_ALLOCATOR_PKG = $(shell echo $(ALLOCATOR_PKG) | $(TR) a-z A-Z)

# -MF and -MT should not be needed, but distcc 2.18.3 fails with -MMD otherwise
CFLAGS_BASE = -pipe -pthread -MMD -MP -MF $(@:.$(O)=.$(D)) -MT $@ \
  $(if $(ALLOCATOR_PKG),-D$(UC_ALLOCATOR_PKG) -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free) \
  $(if $(WERROR),-Werror) $(if $(LTCG),-flto,) -D__USE_XOPEN2K8 -D_GNU_SOURCE \
  -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS \
  $(if $(UNCOMPRESSED_DEBUG),,-Wa,--compress-debug-sections) \
  -fno-strict-aliasing

CXXFLAGS_BASE = -std=c++1y

# linker options for using gold
ifneq (,$(USE_GOLD))
TOOL_LDLIBS += -fuse-ld=gold $(if $(UNCOMPRESSED_DEBUG),,-Wl,--compress-debug-sections,zlib)
ifneq (,$(GOLD_INCREMENTAL))
TOOL_LDLIBS += -Wl,--incremental
endif
ifneq (,$(GOLD_THREADS))
TOOL_LDLIBS += -Wl,--threads -Wl,--thread-count,$(GOLD_THREADS)
endif
endif

# Referenced, indirectly, in the definition of CFLAGS in
# makefiles/c_cpp_patterns.mk.
# n.b. Warray-bound is turned off because it seems to spuriously
# trigger and the local suppression doesn't work well for templates.
CFLAGS_WARNLEVEL = \
 -DGCC_PRAGMA_DIAGNOSTIC \
 -fdiagnostics-show-option \
 -Wall \
 -Wignored-qualifiers \
 -Winit-self \
 -Wmain \
 -Wempty-body \
 -Wlogical-op \
 -Wvla \
 -Wpointer-arith \
 -Wno-format \
 -Wno-deprecated-declarations \
 -Wno-endif-labels \
 -Wno-unused-function \
 -Wno-array-bounds \
 -Wno-maybe-uninitialized \
 -Wno-misleading-indentation
# -Wconversion

# Referenced, indirectly, in the definition of CXXFLAGS in
# makefiles/c_cpp_patterns.mk.
CXXFLAGS_WARNLEVEL = \
 -Wsign-promo \
 -Wno-invalid-offsetof \
 -Wnon-virtual-dtor \
 -Wno-unused-local-typedefs

CFLAGS_OPT = $(if $(OPT),-O3,-O0)
CFLAGS_CHECKED = -g -fno-var-tracking-assignments $(if $(CHECKED),-D_DEBUG,-DNDEBUG)

DISTRIB_CFLAGS := $(foreach i,$(subst :,$(space),$(DISTRIB_INC_DIRS)),-isystem $(i))

# Only these variables will be set in the compiler's environment.  This makes
# the compile consistent, particularly when the generated shell script is
# used directly.
COMPILE_ENV_NAMES := $(filter HOME DISTCC_% CCACHE_%,$(.VARIABLES))
COMPILE_ENV := $(foreach i,$(COMPILE_ENV_NAMES),$(i)="$($(i))") PATH="$(DEV_TOOLS)/bin"

#
# First arg:   name of variable for compiler
# Second arg:  name of FLAGS variable
#
COMPILE_COMMAND = env -i $(subst ",\",$(COMPILE_ENV)) $(COMPILE_PREFIX) $($(1)) $(if $($(TARGET_NAME)_FOR_SHLIB),-fPIC) \
	-o $@ $($(2)FLAGS) $(FLAVOR_CFLAGS) $(PLATFORM_CFLAGS) $(IFLAGS) \
	$($(TARGET_NAME)_OTHER_$(2)FLAGS) $(DISTRIB_CFLAGS) $(PGO) -c $<

COMPILE_SCRIPT_FILE = $(@D)/$(<F).compile.sh

# name of symbols files
ifneq (,$(SEPARATE_SYMBOLS))
# for release, extract debug symbols to a separate file
# reference: http://sourceware.org/gdb/current/onlinedocs/gdb_16.html#SEC154
define EXTRACT_SYMBOLS
	$(OBJCOPY) --only-keep-debug $(if $(UNCOMPRESSED_DEBUG),,--compress-debug-sections) $(1) $(1).dbg
	$(OBJCOPY) --strip-debug $(1)
	$(OBJCOPY) --add-gnu-debuglink=$(1).dbg $(1)
endef
SYMBOLS = $(1).dbg
else
define EXTRACT_SYMBOLS
endef
SYMBOLS =
endif

# Write a helper script to compile just one file 
define WRITE_COMPILE_SCRIPT_FILE
	@(echo "#!/bin/bash"; \
		echo "set -e"; \
		echo "cd $(ABS_SRC_ROOT)"; \
		echo "rm -f $@"; \
		echo "$(1)") \
			> $(COMPILE_SCRIPT_FILE)
	@$(call CHMOD_X, $(COMPILE_SCRIPT_FILE))
endef

define WRITE_JSON
	@(echo '{' ; \
		echo '"directory": "$(ABS_SRC_ROOT)",' ; \
		echo '"command": "$(subst ",\",$(strip $(call COMPILE_COMMAND,$(1),$(2))))",' ; \
		echo '"file": "$(subst src/../,,$(ABS_SRC_ROOT)/$<)"' ; \
		echo '},')
endef

define C_OR_CXX_BUILD_CMD
	@$(call MKDIR,$(@D))
	@$(if $(PGO),$(call MKDIR,$(PGO_ROOT)))
	@$(call RM_F,$@)
	@$(if $(JSON),$(call WRITE_JSON,$(1),$(2)))
	@$(call WRITE_COMPILE_SCRIPT_FILE,$(call COMPILE_COMMAND,$(1),$(2)))
	@$(if $(call quieten,$(PRINT_C_CXX_CMDS)),,echo $(call COMPILE_COMMAND,$(1),$(2)))
	@$(COMPILE_SCRIPT_FILE)
endef

define STATIC_LINK_CMD
	@$(call MKDIR,$(@D))
	@$(call RM_F,$@)
	$(call quieten,$(PRINT_STATIC_LINK_CMDS))$(AR) rcs $@ $(1)
endef

define SHARED_LINK_CMD
	@$(call MKDIR,$(@D))
	@$(call RM_F,$@)
	$(call quieten,$(PRINT_SHARED_LINK_CMDS))$(CXX) -shared $(LDFLAGS) -o $@ $(1) $(FLAVOR_LDLIBS) $(PLATFORM_LDLIBS) $(LDLIBS)
	$(call quieten,$(PRINT_SHARED_LINK_CMDS))$(call EXTRACT_SYMBOLS,$@)
endef

#
# Need to send -E (--export-dynamic) to the linker so that perl modules that we
# load up can find their symbols.
#
define EXECUTABLE_LINK_CMD
	@$(call MKDIR,$(@D))
	@$(call RM_F,$@)
	$(call quieten,$(PRINT_EXE_LINK_CMDS))env -i LIBRARY_PATH="$(DISTRIB_LIB_DIRS)" $(COMPILE_ENV) $(LINK_PREFIX) \
$(CXX_NO_CCACHE) -o $@ $(LDFLAGS) $(1) $(FLAVOR_LDLIBS) $(PLATFORM_LDLIBS) $(LDLIBS)
	$(call quieten,$(PRINT_EXE_LINK_CMDS))$(call EXTRACT_SYMBOLS,$@)
endef

# need to pull in some libs (libstdc++, libgcc_s) from devtools
# we want the devtools variants, not the system-specific ones!
LIB_DIRS=$(DEV_TOOLS)/$(LIBDIR)

TOOLCHAIN_INSTALLERS = CP_gcc

CP_gcc_SRC_DIR = $(DEV_TOOLS)/$(LIBDIRNAME)
CP_gcc_SRC_FILENAMES = libgcc_s.so.1 libstdc++.so.6 libatomic.so.1
CP_gcc_DEST_DIR = $(INSTALL_ROOT)/$(LIBDIRNAME)
