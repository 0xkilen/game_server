#-*-make-*-
#
# Provide controls for printing information about the compile.
#
# This controls the printing of all commands in targets. The default is not to print commands.
# It works by adding @ at the beginning of the commands.
# 
PRINT_CMDS ?= $(PRINT_DEFAULT_GENERAL)
PRINT_C_CXX_CMDS ?= $(PRINT_DEFAULT_GENERAL)
PRINT_STATIC_LINK_CMDS ?= $(PRINT_DEFAULT_GENERAL)
PRINT_SHARED_LINK_CMDS ?= $(PRINT_DEFAULT_GENERAL)
PRINT_EXE_LINK_CMDS ?= $(PRINT_DEFAULT_GENERAL)
PRINT_XSLTPROC_CMDS ?= $(PRINT_DEFAULT_GENERAL)
PRINT_CLANG_CMDS ?= $(PRINT_DEFAULT_GENERAL)
PRINT_AST_PYTHON_CMDS ?= $(PRINT_DEFAULT_GENERAL)
PRINT_CP_CMDS ?= $(PRINT_DEFAULT_GENERAL)
PRINT_TIMESTAMPS ?= $(PRINT_DEFAULT_SPECIAL)
PRINT_FILE_SEARCH ?= $(PRINT_DEFAULT_GENERAL)
PRINT_TOPO_SORT_LINES ?= $(PRINT_DEFAULT_GENERAL)
PRINT_D_FILE_INCLUDES ?= $(PRINT_DEFAULT_GENERAL)


# Filter out anything matching 0 from the parameter concatenated with the global command printing variable. If something is left, command printing is ON, so
# do not provide @; otherwise, provide it.
quieten = $(if $(filter-out 0,$1 $(PRINT_CMDS)),,@)

#
# This turns on the printing of the name of C++ source files as they are compiled. 
# The default is to do this. Unlike suppressing the printing of commands, this works by
# inserting an additional print statement, so the command-line variable is used directly.
#
PRINT_CPP ?= $(PRINT_DEFAULT_SPECIAL)

# Same story, but print the name of libraries and executables as they're linked.
PRINT_LIB ?= $(PRINT_DEFAULT_SPECIAL)
PRINT_BIN ?= $(PRINT_DEFAULT_SPECIAL)

#
# Function for output if a flag is set.
#
print_if = $(if $(filter 0,$(1)),,$(shell echo $(2) $(3) $(4) 1>&2))

#
# Get default values from QUIET and VERBOSE
# The general value says that if neither is specified a 0 is used
# If QUIET, a 0 is used.
# if VERBOSE, a $(VERBOSE) is used.
PRINT_DEFAULT_GENERAL = $(if $(QUIET),0,$(if $(VERBOSE),$(VERBOSE),0))
#
# For the special print items
# Same as the general one, except that if neither quiet nor verbose is specified, use 1.
PRINT_DEFAULT_SPECIAL = $(if $(QUIET),0,$(if $(VERBOSE),$(VERBOSE),1))

#
# For timestamps: 
#    if PRINT_TIMESTAMPS is 0, print no timestamps
#    if anything but 0, print basic timestamps
#    if 2, print entity timestamps
BASIC_TIMESTAMP_PRINT_FLAG = $(if $(filter $(PRINT_TIMESTAMPS),0),,1)
ENTITY_TIMESTAMP_PRINT_FLAG = $(if $(filter $(PRINT_TIMESTAMPS),2),1,)



