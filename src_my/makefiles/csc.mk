# this defines the version of .NET that is in use
DOTNET_HOME := $(subst \,/,$(FrameworkDir))/v2.0.50727

# set up standardized compiler LIBPATH
# only include system assemblies; all other must be explicitly configured.
LIBPATH := $(subst /,\,$(DOTNET_HOME))
export LIBPATH

TOOL_CSCFLAGS_BASE = -nologo -noconfig -unsafe- -warnaserror- \
	-filealign:4096 -nostdlib- -warn:1 -fullpaths -debug+ -nowarn:1699
ifeq ($(OPT),)
# debug
TOOL_CSCFLAGS = $(TOOL_CSCFLAGS_BASE) -debug:full -optimize- -checked+
else
# release
TOOL_CSCFLAGS = $(TOOL_CSCFLAGS_BASE) -debug:pdbonly -optimize+ -checked-
endif

TOOL_CSC_REFERENCES = System.Data System System.Web System.Xml

# change a list of assembly references to csc parameters
CSC_REFERENCE_ARGS = $(addprefix -r:,$(addsuffix .dll,$(1)))

# return name with proper suffix
CSC_SUFFIX = $(if $(filter exe,$(1)),.exe,.dll)

# name of symbols files
CSC_RESUFFIX = $(patsubst %.dll,%,$(patsubst %.exe,%,$(1)))$(2)
CS_SYMBOLS = $(call CSC_RESUFFIX,$(1),_full.pdb)

# convert slashes
CSL = $(subst /,\\\,$(1))

define CSC_CMD
	@$(call MKDIR,$(@D))
	@$(call MKDIR,$($(TARGET_NAME)_OUT_DIR))
	@$(call RM_F,$@)
	"$(DOTNET_HOME)/csc.exe" -target:$($(TARGET_NAME)_TYPE) -out:$(call CSL,$($(TARGET_NAME))) \
		-doc:$(call CSL,$(call CSC_RESUFFIX,$($(TARGET_NAME)),.xml)) \
		-pdb:$(call CSL,$(call CS_SYMBOLS,$($(TARGET_NAME)))) \
	 	$($(TARGET_NAME)_CSCFLAGS) \
	 	$(call CSL,$(call map,CSC_REFERENCE_ARGS,$(TOOL_CSC_REFERENCES) $($(TARGET_NAME)_REFERENCES))) \
		$(call CSL,$(addprefix -r:,$(call deref,$(addprefix CSHARP_,$($(TARGET_NAME)_USE_PACKAGES))))) \
	 	$(call CSL,$(1)) \
		$($(TARGET_NAME)_OTHER_INPUTS)
endef

