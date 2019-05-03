_PKG_tcmalloc_DIR = $(PREBUILTS_DIR)/gperftools-2.6.1-endeca1

ifeq (,$(TCMALLOC_HEAP_PROFILE))
_PKG_tcmalloc_LIB_NAMES = tcmalloc_minimal
else
_PKG_tcmalloc_LIB_NAMES = tcmalloc_and_profiler
endif

PKG_tcmalloc_INC_DIRS = $(_PKG_tcmalloc_DIR)/include
PKG_tcmalloc_LIB_DIRS = $(_PKG_tcmalloc_DIR)/$(LIBDIRNAME)

# static link on Linux
PKG_tcmalloc_LIB_NAMES = $(_PKG_tcmalloc_LIB_NAMES)

