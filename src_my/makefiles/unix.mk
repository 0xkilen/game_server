ifneq (,$(wildcard /etc/lsb-release))
DISTRIB_ID=$(shell \
	/usr/bin/awk \
		'{if (sub(/^DISTRIB_ID=/, "")) {print tolower($$0)}}' \
		/etc/lsb-release)
else
ifneq (,$(wildcard /etc/redhat-release))
DISTRIB_ID=rhel
endif
endif

ifneq (,$(DISTRIB_ID))
$(shell echo 'Linux distribution:   $(DISTRIB_ID)' 1>&2)
-include makefiles/$(DISTRIB_ID).mk
endif

# tool location

$(call warn_if_from_env,DEV_TOOLS)
DEV_TOOLS ?= /usr/dev_tools/trunk
$(call ensure_path_exists,DEV_TOOLS)
SHELL = $(DEV_TOOLS)/bin/bash

$(call warn_if_from_env,PERL_HOME)
PERL_HOME ?= /usr/dev_tools/perl-88830
$(call ensure_path_exists,PERL_HOME)
PERL = PERLLIB= PERL5LIB=$(PERL_HOME)/lib $(PERL_HOME)/bin/perl

PYTHON = env - $(DEV_TOOLS)/python/bin/python

# standardized paths
PATH = /bin:$(DEV_TOOLS)/bin
LD_LIBRARY_PATH=$(DEV_TOOLS)/$(DEV_TOOLSLIBDIRNAME)
export PATH
export LD_LIBRARY_PATH

MKDIR = /bin/mkdir -p $(1)
RM_F = $(if $(1),/bin/rm -f $(1),true)
RM_RF = $(if $(1),/bin/rm -rf $(1),true)
CCACHE ?= /usr/bin/ccache
CP_R = /bin/cp -rL $(1) $(2)
CHMOD_X = /bin/chmod a+x $(1)
DATE = /bin/date
DISTCC ?= /usr/bin/distcc
FIND = /usr/bin/find
CAT = /bin/cat /dev/null
MV_F = /bin/mv -f $(1) $(2)
LN_S = $(RM_F) $(3)$(2) && /bin/ln -sfn $(1) $(3)$(2)
PWD = /bin/pwd
AWK = $(DEV_TOOLS)/bin/gawk
SED = $(DEV_TOOLS)/bin/sed
ECHO = $(DEV_TOOLS)/bin/echo
GZIP = $(DEV_TOOLS)/bin/gzip -f $(1)
GUNZIP = $(DEV_TOOLS)/bin/gunzip $(1)
UNZIP_D = /usr/bin/unzip $(1) -d $(2)
$(call warn_if_from_env,SVNVERSION)
SVNVERSION ?= $(DEV_TOOLS)/bin/svnversion
GIT ?= /usr/bin/git
UNAME = /bin/uname
FIND_MKS = $(patsubst ./%,%,$(shell $(FIND) . -name \*.mk | $(SORT) --ignore-case))
TIMESTAMP = $(DATE)
XSLTPROC ?= $(DEV_TOOLS)/bin/xsltproc
MD5SUM = $(DEV_TOOLS)/bin/md5sum
SORT = $(DEV_TOOLS)/bin/sort
TAR = $(DEV_TOOLS)/bin/tar
XARGS ?= $(DEV_TOOLS)/bin/xargs
PATCH = $(DEV_TOOLS)/bin/patch
GDB ?= $(DEV_TOOLS)/bin/gdb
TR ?= $(DEV_TOOLS)/bin/tr

# N.b. older versions of doxygen can be somewhat flakey with complex C++, so best to
# use the latest stable release from www.doxygen.org.  Version 1.8.10 seems to work fine.
DOXYGEN ?= $(DEV_TOOLS)/bin/doxygen
DOT ?= $(DEV_TOOLS)/bin/dot

O := o
D := d
A := a
STATIC_LIB = lib$(1).a
SHARED_LIB = lib$(1).so
EXECUTABLE = $(1)

SHLIB_INSTALL_DIRNAME = $(LIBDIRNAME)

TOOLCHAIN_CHOICE := gnu

