$(call ensure_defined,$(TARGET_NAME)_SRC_DIR)

#
# This file has the common C and C++ definitions, and pulls in the specific
# rules for each
#

# If this was a recursive variable, evaluate it now once and for all
$(TARGET_NAME)_SRC := $($(TARGET_NAME)_SRC)
$(TARGET_NAME)_GENERATED_HEADERS := $($(TARGET_NAME)_GENERATED_HEADERS)
$(TARGET_NAME)_OTHER_INC_DIRS := $($(TARGET_NAME)_OTHER_INC_DIRS)

$(TARGET_NAME)_OBJS := \
	$(addprefix $($(TARGET_NAME)_OUT_DIR)/, \
		$(addsuffix .$(O), \
			$(basename $($(TARGET_NAME)_SRC))))
ALL_OBJS += $($(TARGET_NAME)_OBJS)

# If a source file doesn't exist in $(FOO_SRC_DIR), assume a rule exists to
# generate it in the build directory
$(TARGET_NAME)_GENERATED_SRC := \
	$(foreach s,$($(TARGET_NAME)_SRC), \
		$(if $(wildcard $(s:%=$($(TARGET_NAME)_SRC_DIR)/%)),,$(s)))

$(TARGET_NAME)_INC_DIRS := $(strip $(call unique, \
	$($(TARGET_NAME)_OTHER_INC_DIRS) \
	$(if $($(TARGET_NAME)_GENERATED_SRC),$($(TARGET_NAME)_OUT_DIR)) \
	$(foreach pkg,$($(TARGET_NAME)_USE_PACKAGES), $(PKG_$(pkg)_INC_DIRS))))

$(TARGET_NAME)_DEPS := $($(TARGET_NAME)_OBJS:$(O)=$(D))
ALL_DEPS += $($(TARGET_NAME)_DEPS)

#
# Remember our target name when doing the objects' build commands
#
$($(TARGET_NAME)_OBJS): TARGET_NAME := $(TARGET_NAME)

#
# Make the generated source before compiling anybody, in case we are also
# generating header files, for example with bison -d.
#
# This is an order-only dependency, not a regular one; for details see:
#   http://www.gnu.org/software/make/manual/html_node/Prerequisite-Types.html
#
# XXX I (Alan) think that this can lead to a source file getting compiled
# against an out-of-date generated header.  I think the proper rule is actually
# for objects with no corresponding depfile to fully depend on all generated
# files they might include:
#   $(foreach f,$($($(TARGET_NAME)_OBJS)),$(if $(wildcard $(f:.$(O)=.$(D))),,$(f))):  <all generated headers>
#
$($(TARGET_NAME)_OBJS): | $(addprefix $($(TARGET_NAME)_OUT_DIR)/,$($(TARGET_NAME)_GENERATED_SRC))
$($(TARGET_NAME)_OBJS): | $(addprefix $($(TARGET_NAME)_OUT_DIR)/,$($(TARGET_NAME)_GENERATED_HEADERS)) $(foreach pkg,$($(TARGET_NAME)_USE_PACKAGES), $(PKG_$(pkg)_GENERATED_HEADERS))

#
# The actual rules to do the compile come from patterns:
#
include makefiles/c_cpp_patterns.mk
