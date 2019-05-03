OPT = 1
CHECKED =
INSTRUMENTATION_STYLE = systemtap
FLAVOR_CFLAGS = -DEXDLOGGING=1

ifeq ($(TARGET_OS),solaris)
$(error systemtap not supported on Solaris)
endif

ifeq ($(TARGET_OS),windows)
$(error systemtap not supported on Windows)
endif

$(shell echo 'After building this flavor, run the dgraph using:' 1>&2)
$(shell echo ' stap src/makefiles/flavors/systemtap/systemtap.stp -c "$$ENDECA_ROOT/bin/dgraph --xlog dgraph.log [other params...]" > stap.log' 1>&2)
$(shell echo 'then use' 1>&2)
$(shell echo ' sort --merge dgraph.log stap.log | src/makefiles/flavors/systemtap/annotate.awk' 1>&2)
$(shell echo 'to analyze the output' 1>&2)

OPT = 1
CHECKED =
INSTRUMENTATION_STYLE = systemtap
FLAVOR_CFLAGS = -DEXDLOGGING=1

ifeq ($(TARGET_OS),solaris)
$(error systemtap not supported on Solaris)
endif

ifeq ($(TARGET_OS),windows)
$(error systemtap not supported on Windows)
endif

$(shell echo 'After building this flavor, run the dgraph using:' 1>&2)
$(shell echo ' stap src/makefiles/flavors/systemtap/systemtap.stp -c "$$ENDECA_ROOT/bin/dgraph --xlog dgraph.log [other params...]" > stap.log' 1>&2)
$(shell echo 'then use' 1>&2)
$(shell echo ' sort --merge dgraph.log stap.log | src/makefiles/flavors/systemtap/annotate.awk' 1>&2)
$(shell echo 'to analyze the output' 1>&2)

