#
# broadly useful makefile functions
#

# $(endeca-unique ...) is an Endeca-specific builtin function added to "make"
# It needs to remove duplicates and be efficient.  The builtin $(sort ...) 
# function removes duplicates, but linking libraries (among other things)
# is order-dependent...
unique = $(endeca-unique $(1))
unique_by_rev = $(endeca-unique-reverse $(1))

deref = $(foreach v,$(1),$($(v)))

map = $(foreach v,$(2),$(call $(1),$(v)))

warn_if_from_env = $(if $(filter environment%,$(origin $(1))),\
	$(shell echo 'Warning:  using environment variable $(1)=$($(1))' 1>&2))

normalize_path_or_default = $(if $(1),$(subst \,/,$(1)),$(2))

ensure_defined = $(if $(filter undefined,$(origin $(1))),\
  $(error Please define $(1)))

# Ensure the variable given contains a path that exists
ensure_path_exists = $(if $(wildcard $($(1))),,$(error Cannot find required path $$($(1))=$($(1)))) 

filter_out_contains = $(foreach word,$(2),$(if $(findstring $(1),$(word)),,$(word)))

# Like findstring but returns the subset of list "find" that occurs in "text".
#	usage: $(call findany,find,text)
findany = $(strip $(foreach word,$(1),$(findstring $(word),$(2))))

# Like filter_out_contains but returns the subset of "list" excluding matches to any in list "find"
#	usage: $(call filter_out_any,find,list)
filter_out_any = $(foreach word,$(2),$(if $(call findany,$(1),$(word)),,$(word)))

# XXX This is unsatisfactory because it makes directories that might be the result of typos.
# n.b. the MSYS pwd magic to get a Windows path on Windows
abs_dir = $(subst \,/,$(shell $(call MKDIR,$(1)) && cd $(1) && $(if $(filter windows,$(TARGET_OS)),cygpath -w `pwd`,pwd)))
make_files_abs = $(foreach file,$(1),$(call abs_dir,$(dir $(file)))/$(notdir $(file)))

# true if the string has \ or / in it
has_slashes = $(findstring /,$(subst \,/,$(1)))

# Break list into smallish chunks and call a function on each
# $(call map20,<list>,<function name>,<function arg 1>)
map20 = $(if $(1),$(call map20,$(wordlist 21,9999,$(1)),$(2),$(3)) $(call $(2),$(3),$(wordlist 1,20,$(1))))

# $(call find_src,$(LIB_foo_SRC_DIR),c cc cpp)
# $(call find_src,$(LIB_foo_SRC_DIR),c cc cpp,subdir1 subdir2) 
find_src = $(patsubst $(1)/%,%,$(filter $(addprefix %.,$(2)),$(wildcard $(addprefix $(1)/,* $(addsuffix /*,$(3))))))

# $(call find_src_recursive,$(LIB_foo_SRC_DIR),c cc cpp)
find_pattern = '(' -name '*.$(word 1,$(1))' $(addprefix -o -name '*.,$(addsuffix ',$(wordlist 2,9999,$(1)))) ')'
find_src_recursive = $(call print_if,$(PRINT_FILE_SEARCH),Search for patterns $(2) in $(1)) $(patsubst $(1)/%,%,$(shell $(FIND) $(1) $(call find_pattern,$(2))))

# $(call find_test_code,$(LIB_foo_SRC_DIR))
# $(call find_test_code_recursive,$(LIB_foo_SRC_DIR))
# Finds all cpp files in all directories named "test" under $(LIB_foo_SRC_DIR);
# the recursive variant finds all cpp files in all directory trees rooted at
# each instance of "test" under $(LIB_foo_SRC_DIR).  For these macros to work
# correctly, there MUST NOT be a space after the comma.
find_test_code = $(patsubst $(1)/%,%,$(foreach td,$(shell $(FIND) $(1) -type d -name test),$(addprefix $(td)/,$(call find_src,$(td),cpp))))
find_test_code_recursive = $(patsubst $(1)/%,%,$(foreach td,$(shell $(FIND) $(1) -type d -name test),$(addprefix $(td)/,$(call find_src_recursive,$(td),cpp))))

# $(call find_proto_recursive,$(LIB_foo_SRC_DIR))
find_proto_recursive = $(patsubst $(1)/%.proto,%.pb.cc,$(shell $(FIND) $(1) -name *\.proto))

#
# Crazy kludge to get a variable containing a space, taken from the gmake
# documentation
#
empty :=
space := $(empty) $(empty)
lparen := (
rparen := )

escapeSpaces = $(subst $(space),\ ,$(1))
escapeParens = $(subst $(lparen),\$(lparen),$(subst $(rparen),\$(rparen),$(1)))

print_timestamp = $(if $(2),$(shell echo -n $(1) ' ' 1>&2 && $(TIMESTAMP) 1>&2),)


# This will tar and gzip the contents of the directory identified by $(1)
# into the file specified in $(2). The temporary tar file will be built
# in the directory of $(2). This means that the output $(2) cannot be specified
# to be inside the directory $(1), or else the command will fail.

# Note: as it stands, this command will create an empty ./ directory in the tarfile
# alongside the ./foo that is expected. This won't effect the extraction
# process, but looks strange when open with something like winRAR
define TGZ
$(TAR) -cv -f $(dir $(2))/$(notdir $(basename $(2))).tar -C $(1) .
$(call GZIP,$(dir $(2))/$(notdir $(basename $(2))).tar)
$(call MV_F,$(dir $(2))/$(notdir $(basename $(2))).tar.gz,$(2))
endef

define MD5TXT
$ (cd $(1) && $(MD5SUM) `$(FIND) . -type f` > jxwy_md5.txt)
$(call MV_F,$(1)/jxwy_md5.txt,$(dir $(2))/jxwy_md5.txt)
endef
