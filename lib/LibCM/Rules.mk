
TARGET_LIBS=libCM
libCM_SOURCES=*.cc
libCM_LIBS=libCMS
libCM_PKGS=glib-2.0

ifeq ($(USE_LIB_ZSTD),1)
libCM_PKGS+=libzstd
libCM_CXXFLAGS=-DUSE_LIB_ZSTD
endif

ifeq ($(USE_LIB_MUPARSERX),1)
libCM_PKGS+=muparserx
libCM_CXXFLAGS+=-DUSE_LIB_MUPARSERX
endif
