# if vsvarsall.bat has been called, a variable pointing to the base
# of the tools has been set.
MSDEV_HOME := $(subst \,/,$(VSINSTALLDIR))/
# if not, there is a variable that points to a subdirectory of the
# tools
ifeq (/,$(MSDEV_HOME))
  MSDEV_HOME := $(call abs_dir,'$(subst \,/,$(VS120COMNTOOLS))../..')
endif
MSDEV_HOME := $(subst //,/,$(MSDEV_HOME))/
export MSDEV_HOME

# likewise for the sdks dir
MS_SDKS := $(subst \,/,$(WindowsSdkDir))/
ifeq (/,$(MS_SDKS))
  MS_SDKS := C:/Program Files (x86)/Windows Kits/8.1/
endif
MS_SDKS := $(subst //,/,$(MS_SDKS))
export MS_SDKS

# This is where we get wsdl.exe
MS_WINSDK_EXE_PATH_X64 := $(subst \,/,$(WINDOWSSDK_EXECUTABLEPATH_X64))/
ifeq (/,$(MS_WINSDK_EXE_PATH_X64))
  MS_WINSDK_EXE_PATH_X64 := C:/Program Files (x86)/Microsoft SDKs/Windows/v8.1A/bin/NETFX 4.5.1 Tools/x64/
endif
MS_WINSDK_EXE_PATH_X64 := $(subst //,/,$(MS_WINSDK_EXE_PATH_X64))
export MS_WINSDK_EXE_PATH_X64

# export wsdl.exe
WSDL_BIN := $(MS_WINSDK_EXE_PATH_X64)/wsdl.exe
export WSDL_BIN

# set base address for .NET tools
FrameworkDir := $(SystemRoot)/Microsoft.NET/Framework

ifeq ($(TARGET_CPU),x86_64)
 MSDEV_PLATFORM := /amd64
 FrameworkDir := $(FrameworkDir)64
endif
MSDEV_BIN := $(MSDEV_HOME)VC/bin$(MSDEV_PLATFORM)
MSDEV_COMMON := $(MSDEV_HOME)Common7
MSDEV_COMMON_BIN := $(MSDEV_COMMON)/Tools/bin

# Add the .NET SDK Bin directory to path
MSDEV_SDK_VER := v3.5
MSDEV_SDK_BIN := $(MSDEV_HOME)SDK/$(MSDEV_SDK_VER)/bin

# some DLLs are loaded from here; add to path
PATH += ;$(MSDEV_COMMON)/IDE;$(MSDEV_BIN);$(MSDEV_SDK_BIN);

# Set up standardized compiler INCLUDE and LIB paths
# only include CRT and SDK; all other libraries must be explicitly configured.
# These are taken from the defaults in Visual Studio 12.0. To find them,
# - open a C++ project;
# - select the project
# - click the property manager tab
# - choose a Microsoft.Cpp.Win32.User node in the tree
# - right-click it and choose Propties from the context menu
# - choose the VC++ Directories line
# - look at Include Directories. The value is
#    $(VC_IncludePath);$(WindowsSDK_IncludePath);
# Look at the macros involved:
# $(VC_IncludePath) = C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\include;C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\atlmfc\include
# $(WindowsSDK_IncludePath) = C:\Program Files (x86)\Windows Kits\8.1\Include\um;C:\Program Files (x86)\Windows Kits\8.1\Include\shared;C:\Program Files (x86)\Windows Kits\8.1\Include\winrt
#
# These can be expressed more compactly as
#
# $(VC_IncludePath) = $(MSDEV_HOME)VC\include;$(MSDEV_HOME)VC\atlmfc\include
# $(WindowsSDK_IncludePath) = $(MS_SDKS)Include\um;$(MS_SDKS)Include\shared;$(MS_SDKS)Include\winrt
#
# Similarly for libs, except that now the platform matters, so we have an _x64:
# $(VC_LibraryPath_x64) = C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\lib\amd64;C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\atlmfc\lib\amd64
# $(WindowsSDK_LibraryPath_x64) = C:\Program Files (x86)\Windows Kits\8.1\lib\winv6.3\um\x64
#
# and more compactly
#
# $(VC_LibraryPath_x64) = $(MSDEV_HOME)VC\lib\amd64;$(MSDEV_HOME)VC\atlmfc\lib\amd64
# $(WindowsSDK_LibraryPath_x64) = $(MS_SDKS)lib\winv6.3\um\x64


VC_IncludePath := $(MSDEV_HOME)VC\include;$(MSDEV_HOME)VC\atlmfc\include
WindowsSDK_IncludePath := $(MS_SDKS)Include\um;$(MS_SDKS)Include\shared;$(MS_SDKS)Include\winrt

INCLUDE := $(subst /,\,$(VC_IncludePath);$(WindowsSDK_IncludePath))
export INCLUDE

VC_LibraryPath_x64 = $(MSDEV_HOME)VC\lib\amd64;$(MSDEV_HOME)VC\atlmfc\lib\amd64
WindowsSDK_LibraryPath_x64 = $(MS_SDKS)lib\winv6.3\um\x64

LIB := $(subst /,\,$(VC_LibraryPath_x64);$(WindowsSDK_LibraryPath_x64))
export LIB

CC := "cl.exe"
CXX := $(CC)

LINK_PATH_ARG = -libpath:$(1)
LINK_LIB_ARG = $(1).lib

# standard libraries
TOOL_LDLIBS = kernel32.lib user32.lib advapi32.lib shell32.lib ole32.lib \
 oleaut32.lib uuid.lib ws2_32.lib psapi.lib version.lib odbc32.lib

# standard linker options
TOOL_LDFLAGS = $(if $(LTCG),-ltcg) $(if $(DISABLE_PDB),,-debug) -STACK:8388608 -nologo -incremental:no \
 -ignore:4221 \
 -subsystem:console -largeaddressaware -nodefaultlib:msvcrt$(if $(OPT),d,) \
 $(if $(WERROR),-WX) -opt:ref,icf

# standard options for all modes
# VS2013 provides "alignof" as "__alignof".
# VS2013 does not provide noexcept. However throw() has always had the noexcept behavior.
# To macroize C++ keywords, which is illegal under the standard, it is necessary to prevent inclusion of the header that checks this, by defining _XKEYCHECK_H.
CFLAGS_CPP11 = -Dalignof=__alignof -Dnoexcept=throw\(\) -D_XKEYCHECK_H
CFLAGS_BASE = $(CFLAGS_EXTRA) -nologo -EHsc -FD -D_MBCS -DNOMINMAX -DWIN32_LEAN_AND_MEAN $(CFLAGS_CPP11) -fp:precise -bigobj $(if $(filter tcmalloc,$(ALLOCATOR_PKG)),-DTCMALLOC) $(if $(WERROR),-WX)

# Rationale:  http://wiki.endeca.com/confluence/display/Eng/The+case+against+(certain)+Windows+compiler+warnings
CFLAGS_WARNLEVEL = -W3 -WL -wd4244 -wd4996 -wd4267 -wd4355 -wd4146 -wd4503

CFLAGS_OPT = $(if $(OPT),$(if $(LTCG),-GL) -O2,-Od $(if $(CHECKED),-RTC1))
CFLAGS_CHECKED = $(if $(CHECKED),$(if $(DISABLE_PDB),,-Zi) -D_DEBUG -GS -MDd, -GS- -DNDEBUG -D_SECURE_SCL=0 -Z7 -MD)

# First arg:   name of variable for compiler
# Second arg:  name of FLAGS variable
COMPILE_COMMAND = $(COMPILE_PREFIX) $($(1)) -Fo$@ \
		-Fd$(basename $@).pdb \
		$($(2)FLAGS) $(PLATFORM_CFLAGS) $(IFLAGS) $(FLAVOR_CFLAGS) $($(TARGET_NAME)_OTHER_$(2)FLAGS) \
		-showIncludes -c $< \
	| $(AWK) -f makefiles/cl-deps.awk depfile="$(BUILD_ROOT)/$*.$(D)" target="$@"

COMPILE_BAT_SCRIPT_FILE = $(@D)/$(<F).compile.bat
COMPILE_SH_SCRIPT_FILE = $(@D)/$(<F).compile.sh

ifneq ($(OPT),)
 VC_DEBUG_LIB =
else
 VC_DEBUG_LIB = d
endif

CXXFLAGS_BASE = -TP -DBOOST_HAS_TR1

VC_MAKE_PDB = $(patsubst %.dll,%,$(patsubst %.exe,%,$(1)))$(2).pdb

# names of symbol files
ifneq (,$(SEPARATE_SYMBOLS))
 VC_PDB_OUT = -pdb:$(call VC_MAKE_PDB,$(1),_full) -pdbstripped:$(call VC_MAKE_PDB,$(1),)
 SYMBOLS = $(call VC_MAKE_PDB,$(1),) $(call VC_MAKE_PDB,$(1),_full)
else
 VC_PDB_OUT = -pdb:$(call VC_MAKE_PDB,$(1),_full)
 SYMBOLS = $(call VC_MAKE_PDB,$(1),_full)
endif

# Write a helper script to compile just one file
define WRITE_COMPILE_SH_SCRIPT_FILE
	(echo "$(1)" > $(COMPILE_SH_SCRIPT_FILE))
endef

define WRITE_COMPILE_BAT_SCRIPT_FILE
	(echo "@setlocal"; \
		echo "@set PATH=$(PATH)"; \
		echo "@set INCLUDE=$(INCLUDE)"; \
		echo "@set LIB=$(LIB)"; \
		echo "@pushd $(call CSL,$(ABS_SRC_ROOT))"; \
		echo "@if exist $(call CSL,$(@)) del $(call CSL,$(@))"; \
		echo "$(SHELL) $(COMPILE_SH_SCRIPT_FILE)"; \
		echo "@popd") \
			> $(COMPILE_BAT_SCRIPT_FILE)
endef

define C_OR_CXX_BUILD_CMD
	@$(call MKDIR,$(@D))
	@$(call RM_F,$@)
	$(call quieten,$(PRINT_C_CXX_CMDS))$(call WRITE_COMPILE_SH_SCRIPT_FILE,$(call COMPILE_COMMAND,$(1),$(2)))
	$(call quieten,$(PRINT_C_CXX_CMDS))$(call WRITE_COMPILE_BAT_SCRIPT_FILE)
	$(call quieten,$(PRINT_C_CXX_CMDS))$(call COMPILE_COMMAND,$(1),$(2))
endef

# append a list of words to a file, one per line
# $(1)=filename $(2)=list
APPEND_FILE = $(shell (for f in $(2); do echo $${f}; done) >> $(1))

# write a word list to a file. this is complicated because it's necessary to
# avoid passing extremely long lists to APPEND_FILE, because of shell line length limitations
# $(1)=filename $(2)=list
LIST_OUT = $(shell echo -n > $(1))$(call map20,$(2),APPEND_FILE,$(1))

OBJLIST = $($(1)_OUT_DIR)/$(notdir $(2)).objlist
MANIFEST = $($(1)_OUT_DIR)/$(notdir $(2)).manifest

# convert slashes
CSL = $(subst /,\\\,$(1))

# Undefine VS_UNICODE_OUTPUT so that MS build tools send their output to
# stdout, rather than a special "unicode pipe"
# See http://forums.microsoft.com/MSDN/ShowPost.aspx?PostID=390232&SiteID=1
# For some reason, we need this ifneq, or else builds from the command line
# (rather than through the IDE) don't work right.
ifneq (,$(VS_UNICODE_OUTPUT))
	   unexport VS_UNICODE_OUTPUT
endif

define STATIC_LINK_CMD
	$(call quieten,$(PRINT_STATIC_LINK_CMDS))$(shell $(call MKDIR,$(@D)))
	$(call quieten,$(PRINT_STATIC_LINK_CMDS))$(shell $(call RM_F,$(call OBJLIST,$(TARGET_NAME),$@)))
	$(call quieten,$(PRINT_STATIC_LINK_CMDS))$(call LIST_OUT,$(call OBJLIST,$(TARGET_NAME),$@),$(1:.dll=.lib))
	$(call quieten,$(PRINT_STATIC_LINK_CMDS))"$(MSDEV_BIN)/lib.exe" -NOLOGO $(if $(LTCG),-LTCG) -OUT:$@ @$(call CSL,$(call OBJLIST,$(TARGET_NAME),$@))
endef

# We use -fixed:no so that relocations are emitted, which enables
# instrumentation by call graph profilers such as Vtune. Note that it's the
# default setting for DLLs -- produced by the SHARED_LINK_CMD -- but we
# specify it to be explicitly clear.
define SHARED_LINK_CMD
	$(call quieten,$(PRINT_SHARED_LINK_CMDS))$(shell $(call MKDIR,$(@D)))
	$(call quieten,$(PRINT_SHARED_LINK_CMDS))$(shell $(call RM_F,$(call OBJLIST,$(TARGET_NAME),$@)))
	$(call quieten,$(PRINT_SHARED_LINK_CMDS))$(call LIST_OUT,$(call OBJLIST,$(TARGET_NAME),$@),$(1:.dll=.lib))
	$(call quieten,$(PRINT_SHARED_LINK_CMDS))"$(MSDEV_BIN)/link.exe" -dll -out:$@ -implib:$(@:%.dll=%.lib) $(call VC_PDB_OUT,$@) \
		-fixed:no -manifest -manifestfile:$(call MANIFEST,$(TARGET_NAME),$@) \
		$(addprefix -def:,$(wildcard $($(TARGET_NAME)_SRC_DIR)/*.def)) \
		$(LDFLAGS) @$(call CSL,$(call OBJLIST,$(TARGET_NAME),$@)) $(FLAVOR_LDLIBS) $(PLATFORM_LDLIBS) $(LDLIBS)
	$(call quieten,$(PRINT_SHARED_LINK_CMDS))"$(MS_SDKS)Bin/x64/mt.exe" -nologo -manifest $(call MANIFEST,$(TARGET_NAME),$@) \
		-outputresource:$@\;#2
endef

define EXECUTABLE_LINK_CMD
	$(call quieten,$(PRINT_EXE_LINK_CMDS))$(shell $(call MKDIR,$(@D)))
	$(call quieten,$(PRINT_EXE_LINK_CMDS))$(shell $(call RM_F,$(call OBJLIST,$(TARGET_NAME),$@)))
	$(call quieten,$(PRINT_EXE_LINK_CMDS))$(call LIST_OUT,$(call OBJLIST,$(TARGET_NAME),$@),$(1:.dll=.lib))
	$(call quieten,$(PRINT_EXE_LINK_CMDS))"$(MSDEV_BIN)/link.exe" -out:$@ $(LDFLAGS) $(call VC_PDB_OUT,$@) \
		-fixed:no -manifest -manifestfile:$(call MANIFEST,$(TARGET_NAME),$@) \
		@$(call CSL,$(call OBJLIST,$(TARGET_NAME),$@)) $(FLAVOR_LDLIBS) $(PLATFORM_LDLIBS) $(LDLIBS)
	$(call quieten,$(PRINT_EXE_LINK_CMDS))$(if $($(TARGET_NAME)_KEEP_UNMANIFESTED), $(call CP_R,$@,$(patsubst %.exe,%-nomanifest.exe,$@)))
	$(call quieten,$(PRINT_EXE_LINK_CMDS))"$(MS_SDKS)Bin/x64/mt.exe" -nologo -manifest $(call MANIFEST,$(TARGET_NAME),$@) \
		-outputresource:$@\;#1
endef

