_PKG_cppunit_DIR = $(PREBUILTS_DIR)/cppunit-1.10.2-endeca1
PKG_cppunit_INC_DIRS = $(_PKG_cppunit_DIR)/include
PKG_cppunit_LIB_DIRS = $(_PKG_cppunit_DIR)/$(LIBDIRNAME)
PKG_cppunit_LIB_NAMES = cppunit
PKG_cppunit_INSTALLERS = CP_cppunit

CP_cppunit_SRC_DIR = $(_PKG_cppunit_DIR)/$(LIBDIRNAME)
CP_cppunit_SRC_FILENAMES = $(notdir $(wildcard $(CP_cppunit_SRC_DIR)/*))
CP_cppunit_DEST_DIR = $(INSTALL_ROOT)/$(LIBDIRNAME)
