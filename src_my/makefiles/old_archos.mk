OLD_ARCH_x86 := i86pc
OLD_ARCH_x86_64 := x86_64pc
OLD_ARCH_sparcv9 := sparcv9

OLD_OS_windows := win32
OLD_OS_linux := linux
OLD_OS_solaris := solaris

OLD_ARCH := $(OLD_ARCH_$(TARGET_CPU))
OLD_OS := $(OLD_OS_$(TARGET_OS))

OLD_ARCH_OS := $(OLD_ARCH)-$(OLD_OS)
OLD_ARCHOS_FLAVOR := $(if $(filter-out release,$(FLAVOR)),-$(FLAVOR))
