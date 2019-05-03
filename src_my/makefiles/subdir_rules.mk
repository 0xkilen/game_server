#
# To be included from subdirectory Makefiles, with DIR and ROOT set
# appropriately.
#
.PHONY: build_from_top $(MAKECMDGOALS)
$(MAKECMDGOALS): build_from_top
build_from_top:
	$(MAKE) -C $(ROOT) ONLY_FOR_PREFIX=$(DIR) $(MAKECMDGOALS)
