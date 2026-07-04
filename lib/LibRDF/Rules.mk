

# Predpokladame, ze zadny projekt nepouziva LibRDF a LibRDF2 najednou.
# V Rules.mk se totiz obe jmenuji stejne. Uzivatel napise, ze chce
# 'libRDF', a o nic vic se nestara.
#
# LibRDF bude automaticky pouzivat LibCMS nebo LibCMS4, podle toho,
# ktera je v projektu.
#
TARGET_LIBS=libRDF

libRDF_SOURCES=*.cc
libRDF_LIBS=libCMS
libRDF_PKGS=proj

libRDF_CXXFLAGS=-DPROJ_VERSION=$(shell pkg-config --modversion proj | cut -d. -f1)

