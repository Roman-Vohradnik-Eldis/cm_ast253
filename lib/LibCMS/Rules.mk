
# Predpokladame, ze zadny projekt nepouziva LibCMS a LibCMS4 najednou.
# V Rules.mk se totiz obe jmenuji stejne. Uzivatel napise, ze chce
# 'libCMS', a o nic vic se nestara. Pokud by chtel zjistit, ktera verze
# se v projektu pouziva, tak v CMSHeader.hh:
#
#  LibCMS ma definovano ELDIS_CMS_VERSION
#  LibCMS4 ma definovano CMS4_VERSION
#
TARGET_LIBS=libCMS
libCMS_SOURCES=*.c
libCMS_PKGS=glib-2.0 gthread-2.0
