#
# Depend on Variable Contents
#

# output of $(shell) seems to be treated as a single value, not a whitespece-separated list
# until it is assigned to a variable, so we need to use this temporary variable to enable the
# use of $(firstword)
MD5SUM_INTERNAL = $(shell echo '$(1)'|$(MD5SUM))

# $(firstword) is used to pull out just the MD5SUM value
MD5SUM_VAR = $(firstword $(call MD5SUM_INTERNAL,$(1)))

HASH_DIR = $(BUILD_ROOT)/hashes

# hash files are of the form VARNAME-hash-MD5SUM, so whenever the MD5SUM value changes then
# the target needs to get rebuilt
DEPEND_ON_VAR = $(HASH_DIR)/$(1)-hash-$(call MD5SUM_VAR,$($(1)))

# this pattern rule cleans out hash files representing old values of this variable, then
# creates a new file to indicate that the target is up-to-date until the MD5SUM of the
# variable changes again
$(HASH_DIR)/%:
	$(call MKDIR,$(HASH_DIR))
	$(call RM_F,$(filter %-hash-,$(subst -hash-,-hash- ,$@))*)
	touch $@
