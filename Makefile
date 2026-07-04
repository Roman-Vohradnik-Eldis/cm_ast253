#default is 1
USE_LIB_PARSER_RDF ?= 1

APPNAME := bin/Decode253
LIBCM := lib/LibCM
LIBCMS := lib/LibCMS
LIBRDF := lib/LibRDF
LIBPARSERRDF := lib/LibParserRDF
LIBVER := lib/LibVER
LIBS=LibCMS LibRDF LibCM LibParserRDF

DESTDIR :=
INSTALLDIR := /eldis/astmerge
DEPDIR = .d
CXX := g++
LD := $(CXX)
CXXDEPS := $(CXX)
MAKE := make --silent
RM := rm -rf

DEPFLAGS = -MM -MF $(DEPDIR)/$*.d 
USED_PKGS = glib-2.0 gthread-2.0
CXXFLAGS =  -ggdb -O2 -Wall -Wno-deprecated-declarations
CXXFLAGS +=  -DUSE_LIB_PARSER_RDF=$(USE_LIB_PARSER_RDF)
CXXFLAGS += -I$(LIBCMS) -I$(LIBRDF) -I. -I$(LIBVER) $(GLIB_CXXFLAGS)
LDFLAGS = -lm -lz  -lpthread  -L$(LIBVER)
SECONDEXPLIBS = $(LIBRDF)/libRDF.a $(LIBCMS)/libCMS.a

ifeq ($(CXX),clang++)
CXXFLAGS += -fcolor-diagnostics -ferror-limit=1
endif

-include verbosity.mk

SRC = main.cc
LIBDEP = $(wildcard $(dir $@)/*.cc $(dir $@)/*.hh)

DEP = $(SRC:.cc=.d)
OBJ = $(SRC:.cc=.o)
ifeq ($(USE_LIB_PARSER_RDF), 1)
OBJ += $(LIBCM)/libCM.a
OBJ += $(LIBPARSERRDF)/libParserRDF.a
endif
OBJ += $(LIBCMS)/libCMS.a $(LIBRDF)/libRDF.a
OBJ += $(LIBVER)/version.o

ifeq ($(USE_LIB_PARSER_RDF), 1)
USED_PKGS += muparserx
LDFLAGS += -L$(LIBCM) -L$(LIBPARSERRDF)
CXXFLAGS += -I$(LIBCM) -I$(LIBPARSERRDF)
CXXFLAGS+=-DUSE_LIB_MUPARSERX
SECONDEXPLIBS += $(LIBCM)/libCM.a $(LIBPARSERRDF)/libParserRDF.a
endif

LDFLAGS += -L$(LIBCMS) -L$(LIBRDF)
CXXFLAGS += $(shell pkg-config --cflags $(USED_PKGS) 2>/dev/null)
LDFLAGS += $(shell pkg-config --libs $(USED_PKGS) 2>/dev/null)

all: checklibs $(APPNAME)

checklibs:
ifeq ($(USE_LIB_PARSER_RDF), 1)
ifeq ($(shell pkg-config muparserx --exists; echo $$?),1)
$(error "Library dependency `muparserx' is not found ... please download it : `git@linux:sw/other/muparserx'")
endif
endif

.PHONY: clean strip all mrproper

%.o: %.cc | $(DEPDIR)
	@$(CXXDEPS) $(DEPFLAGS) $(CXXFLAGS) $<
	$(CXX) $(CXXFLAGS) $< -c

$(APPNAME): $(SECONDEXPLIBS) $(OBJ)
	$(LD) $(LDFLAGS) $(OBJ) -o $(APPNAME)

.FORCE:
$(LIBVER)/version.o: status
	$(MAKE) -C $(dir $@) VERSIONDIR=$(LIBVER) PROJDIR=$(CURDIR)

##.SECONDEXPANSION:
#$(SECONDEXPLIBS):
## $$(LIBDEP)
#	$(MAKE) -C $(dir $@) VERBOSE=$(VERBOSE) USE_LIB_MUPARSERX=$(USE_LIB_PARSER_RDF)
## USE_LIB_MUPARSERX=$(USE_LIB_PARSER_RDF) tento parametr ... je potreba pouze pro LibCM, ale ostatnim asi nebude vadit

$(LIBRDF)/libRDF.a:
	$(MAKE) -C $(dir $@) VERBOSE=$(VERBOSE)

$(LIBCMS)/libCMS.a:
	$(MAKE) -C $(dir $@) VERBOSE=$(VERBOSE)

$(LIBCM)/libCM.a:
	$(MAKE) -C $(dir $@) VERBOSE=$(VERBOSE) USE_LIB_MUPARSERX=$(USE_LIB_PARSER_RDF)

$(LIBPARSERRDF)/libParserRDF.a:
	$(MAKE) -C $(dir $@) VERBOSE=$(VERBOSE) USE_LIB_MUPARSERX=$(USE_LIB_PARSER_RDF)




$(DEPDIR)::
	@mkdir -p $(DEPDIR)

status:
	@echo LIBVERCHECK; $(LIBVER)/version.sh --status --report-changes

clean:
	$(RM) $(APPNAME) $(DEPDIR) $(OBJ)

distclean: clean
	$(RM) ./test_files/ *.bin *.out core tags *~

#$(LIBS):
#	$(info make -C $@)
#	$(MAKE) --silent -C lib/$@ VERBOSE=$(VERBOSE) USE_LIB_MUPARSERX=$(USE_LIB_PARSER_RDF)
## USE_LIB_MUPARSERX=$(USE_LIB_PARSER_RDF) tento parametr ... je potreba pouze pro LibCM, ale ostatnim asi nebude vadit

mrproper:
	make clean -C .
	make clean -C $(LIBRDF)
	make clean -C $(LIBCMS)
	make clean -C $(LIBCM)
	make clean -C $(LIBPARSERRDF)
	make clean -C $(LIBVER)

strip:
	strip --strip-unneeded $(APPNAME)

-include $(patsubst %, $(DEPDIR)/%.d,$(basename $(SRC)))


install: $(APPNAME)
	@echo "INSTALL ALL"
	@mkdir -p $(DESTDIR)/$(INSTALLDIR)/bin $(DESTDIR)/$(INSTALLDIR)/etc $(DESTDIR)/$(INSTALLDIR)/samples
	@(cd bin && for i in * ; do  echo "Installing bin/$$i" ; cp -rfp $$i $(DESTDIR)/$(INSTALLDIR)/bin; done; cd ..)
	@(cd etc && for i in * ; do  echo "Installing etc/$$i" ; cp -rfp $$i $(DESTDIR)/$(INSTALLDIR)/etc; done; cd ..)
	@(cd samples && for i in * ; do  echo "Installing samples/$$i" ; cp -rfp $$i $(DESTDIR)/$(INSTALLDIR)/samples; done; cd ..)

#uninstall:
#	@echo "UNINSTALL $(APPNAME)"
#	@rm -rf $(DESTDIR)/$(INSTALLDIR)
