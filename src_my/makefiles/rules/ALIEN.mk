# The user must provide rules for any ALIEN entities in RULES.mk files

# this allows someone to do:
# make ALIEN_my-alien-target
# and run their ALIEN entity alone

TARGET_NAME := ALIEN_$(ALIEN_NAME)

.PHONY: $(TARGET_NAME)
$(TARGET_NAME): $($(TARGET_NAME))

