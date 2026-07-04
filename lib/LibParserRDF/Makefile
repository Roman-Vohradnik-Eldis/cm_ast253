
USE_LIB_MUPARSERX ?= 1

LIB := libParserRDF.a
RDF := ../LibRDF
CMS := ../LibCMS
CM := ../LibCM
DEPDIR := .d
CXX := g++
CXXDEPS := $(CXX)
LD := $(CXX)
AR := ar
MAKE := make --silent
RM := rm -rf
USED_PKGS=glib-2.0 gthread-2.0 muparserx

-include verbosity.mk

DEPFLAGS = -MM -MF $(DEPDIR)/$*.d
CXXFLAGS = -O2 -I$(CM) -I$(CMS) -I$(RDF) -I. -Wall -Wno-deprecated-declarations
LDFLAGS = -L.

ifeq ($(USE_LIB_MUPARSERX), 1)
CXXFLAGS+=-DUSE_LIB_MUPARSERX
endif

ifeq ($(CXX), clang++)
CXXFLAGS += -fcolor-diagnostics -ferror-limit=1
endif

ifeq (, $(shell pkg-config --libs muparserx 2>/dev/null))
$(error "Library dependency `muparserx' is not found ... please download it : `git@linux:sw/other/muparserx'")
endif

CXXFLAGS += $(shell pkg-config --cflags $(USED_PKGS) 2>/dev/null)
LDFLAGS += $(shell pkg-config --libs $(USED_PKGS) 2>/dev/null)

SRC = $(filter-out , $(wildcard *.cc))
DEP = $(SRC:.cc=.d)
OBJ = $(SRC:.cc=.o)


DEFAULT_GOAL:=all

.PHONY: clean all mrproper
.SUFFIXES:


all: $(LIB)

%.o: %.cc | $(DEPDIR)
	@$(CXXDEPS) $(DEPFLAGS) $(CXXFLAGS) $<
#	$(info $(CXXDEPS) $(DEPFLAGS) $(CXXFLAGS))
	$(CXX) $(CXXFLAGS) $< -c


$(LIB): $(OBJ)
	$(AR) rcs $@ $?

$(DEPDIR):
	@mkdir -p $(DEPDIR)

clean:
	$(RM) $(LIB) $(DEPDIR) $(OBJ)

distclean: clean
	$(RM) core tags *~ *.out *.db *.a

mrproper:
	make clean -C .
	make clean -C $(RDF)

-include $(addprefix $(DEPDIR)/, $(DEP))
