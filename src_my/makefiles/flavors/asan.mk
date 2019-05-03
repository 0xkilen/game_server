OPT =
CHECKED =
ALLOCATOR_PKG = 
INSTRUMENTATION_STYLE = asan
FLAVOR_CFLAGS = -DEXIT_CLEANUP -fsanitize=address -DASAN
FLAVOR_LDFLAGS = -fsanitize=address
RUN_PREFIX= ASAN_OPTIONS=check_initialization_order=true
