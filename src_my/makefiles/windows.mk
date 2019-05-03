DEV_TOOLS_PREFIX ?= T:/
DEV_TOOLS_DIRECTORY ?= trunk-808306

$(call warn_if_from_env,DEV_TOOLS)

DEV_TOOLS ?= $(DEV_TOOLS_PREFIX)$(DEV_TOOLS_DIRECTORY)
$(call ensure_path_exists,DEV_TOOLS)

CYGWIN_HOME ?= $(DEV_TOOLS)/cygwin
SHELL := $(CYGWIN_HOME)/bin/sh.exe

$(call warn_if_from_env,PERL_HOME)
PERL_HOME := $(call normalize_path_or_default,$(PERL_HOME),c:/data/perl/5.8.3)
$(call ensure_path_exists,PERL_HOME)
PERLLIB=
PERL5LIB= $(PERL_HOME)/lib
PERL= $(PERL_HOME)/bin/perl.exe
export PERLLIB
export PERL5LIB

PURIFY_DIR ?= $(subst \,/,$(ProgramFiles))/Rational/PurifyPlus

PYTHON = C:/Python27/python

# Quick quiz: Windows environment variables are not case sensitive;
# Make variables are case sensitive. If the variable "Path" is in
# the (not case sensitive) environment, and exported implicitly by make,
# and the variable "PATH" is exported by make, what does a process
# started by Make see as its path?
# Answer: whichever gets set in the sub process' environment last.
# Therefore, unexport all other variants of PATH to get predictable behavior.
unexport path Path pAth paTh patH PAth PaTh PatH pATh pAtH paTH pATH PaTH PAtH PATh
PATH += ;$(dir $(ComSpec));$(CYGWIN_HOME)/bin;$(DEV_TOOLS)/bin;$(DEV_TOOLS)/$(DEV_TOOLSLIBDIRNAME);$(JAVA_HOME)/bin;

define RM_F_SOME
	$(if $(2),$(shell echo $(CYGWIN_HOME)/bin/rm -f $(2)))
	$(if $(2),$(shell $(CYGWIN_HOME)/bin/rm -f $(2)))
endef
define RM_RF_SOME
	$(if $(2),$(shell echo $(CYGWIN_HOME)/bin/rm -rf $(2)))
	$(if $(2),$(shell $(CYGWIN_HOME)/bin/rm -rf $(2)))
endef

MKDIR = $(CYGWIN_HOME)/bin/mkdir -p $(1)
RM_F = $(call map20,$(1),RM_F_SOME)
RM_RF = $(call map20,$(1),RM_RF_SOME)
CP_R = $(CYGWIN_HOME)/bin/cp -rL $(1) $(2)
CAT = $(CYGWIN_HOME)/bin/cat /dev/null
DATE = $(CYGWIN_HOME)/bin/date
MV_F = $(CYGWIN_HOME)/bin/mv -f $(1) $(2)
FIND = $(DEV_TOOLS)/bin/find
LN_S = $(DEV_TOOLS)/bin/junction $(3)$(2) $(3)$(1)
AWK = $(CYGWIN_HOME)/bin/gawk
GZIP = $(CYGWIN_HOME)/bin/gzip -f $(1)
GUNZIP = $(CYGWIN_HOME)/bin/gzip -d $(1)
UNZIP_D = $(CYGWIN_HOME)/bin/unzip $(1) -d $(2)
$(call warn_if_from_env,SVNVERSION)
SVNVERSION ?= $(DEV_TOOLS)/bin/svnversion
UNAME = $(CYGWIN_HOME)/bin/uname $(1)
FIND_MKS = $(patsubst ./%,%,$(shell makefiles/windows/wfindf.exe . "*.mk"))
TIMESTAMP = $(DATE) +%s
XSLTPROC ?= $(DEV_TOOLS)/bin/xsltproc
SED = $(CYGWIN_HOME)/bin/sed
ECHO = $(CYGWIN_HOME)/bin/echo
MD5SUM = $(CYGWIN_HOME)/bin/md5sum
XARGS ?= $(CYGWIN_HOME)/bin/xargs
PATCH = $(CYGWIN_HOME)/bin/patch
TAR = $(DEV_TOOLS)/bin/tar

# override for Windows
find_pattern = '$(addprefix *.,$(addsuffix ;,$(1)))'
find_src_recursive = $(call print_if,$(PRINT_FILE_SEARCH),Search for patterns $(2) in $(1)) $(patsubst $(1)/%,%,$(shell makefiles/windows/wfindf.exe /f '$(1)' $(call find_pattern,$(2))))


O := obj
D := d
A := lib
STATIC_LIB = $(1).lib
SHARED_LIB = $(1).dll
EXECUTABLE = $(1).exe

SHLIB_INSTALL_DIRNAME = bin

TOOLCHAIN_CHOICE ?= vc12

#ALLOCATOR_PKG = tcmalloc

WINDOWS_VERSION := $(shell cmd /c ver)
IS_OS_SUPPORTED ?= $(if $(filter %Version\ 6.1.%,$(WINDOWS_VERSION)),YES)
ifneq ($(IS_OS_SUPPORTED),YES)
$(error Only Windows 2008 is supported.  If you really want to continue, add a line saying IS_OS_SUPPORTED=YES to your trunk/src/Makefile.local or in your environment)
endif

PLATFORM_CFLAGS = -DWIN32=1
# Default FD_SETSIZE on Windows is 64 -- this limits the number of sockets
# we can select() on. Bump this to 1024 to match linux, as we exceed 64
# connections in some of our perf tests. Do this on the compiler command line
# because this must be defined consistently before any includes of winsock*.h
PLATFORM_CFLAGS += -DFD_SETSIZE=1024
ifeq ($(TARGET_CPU),x86_64)
 PLATFORM_CFLAGS += -DWIN64=1
endif
