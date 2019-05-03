_PKG_boost_DIR = $(PREBUILTS_DIR)/boost-1_65_1-endeca1
PKG_boost_INC_DIRS = $(_PKG_boost_DIR)/include
PKG_boost_LIB_DIRS = $(_PKG_boost_DIR)/$(LIBDIRNAME)
PKG_boost_LIB_NAMES = boost_system boost_filesystem boost_program_options boost_thread boost_chrono boost_date_time boost_atomic boost_regex boost_log boost_log_setup
PKG_boost_INSTALLERS = CP_boost_libs
PKG_boost_FOR_SHLIB = 1
ALL_PACKAGES += boost

CP_boost_libs_SRC_DIR = $(_PKG_boost_DIR)/$(LIBDIRNAME)
CP_boost_libs_SRC_FILENAMES = libboost_system.so.1.65.1 \
	libboost_filesystem.so.1.65.1 \
	libboost_program_options.so.1.65.1 \
	libboost_thread.so.1.65.1 \
	libboost_chrono.so.1.65.1 \
	libboost_date_time.so.1.65.1 \
	libboost_atomic.so.1.65.1 \
	libboost_regex.so.1.65.1 \
	libboost_log.so.1.65.1 \
	libboost_log_setup.so.1.65.1
CP_boost_libs_DEST_DIR = $(INSTALL_ROOT)/$(LIBDIRNAME)
