# This flavor tells EVE to use new/delete instead of mmap in its memory pools,
# for better memory checking with valgrind.

include makefiles/flavors/valgrind.mk

FLAVOR_CFLAGS += -DEVE_HEAP_ALLOC
CLANG_FLAGS += -DEVE_HEAP_ALLOC
INSTRUMENTATION_STYLE = valgrind-eve-heap-alloc
