#default is 0
GDB ?= 0
#default is 1
USE_LIB_MUPARSERX ?= 0

LIB := libCM.a
CMS := ../LibCMS 
DEPDIR := .d
CXX := g++ -std=c++11
CXXDEPS := $(CXX)
LD := $(CXX)
AR := ar
# MAKE := make --silent
RM := rm -rf
USED_PKGS=glib-2.0 gthread-2.0

-include verbosity.mk

DEPFLAGS = -MM -MF $(DEPDIR)/$*.d
CXXFLAGS = -I$(CMS) -I. -Wall -Wno-deprecated-declarations
ifeq ($(GDB), 1)
CXXFLAGS += -O0 -ggdb
else
CXXFLAGS += -O2
endif
CLDFLAGS = -L.

ifeq ($(CXX), clang++)
CXXFLAGS += -fcolor-diagnostics -ferror-limit=1
endif

#USE_LIB_MUPARSERX=$(shell cat .config.h 2>/dev/null | grep ^"\#define USE_LIB_MUPARSERX " | sed "s/\#define USE_LIB_MUPARSERX \(.*\)/\1/" )
#ifeq ($(USE_LIB_MUPARSERX), )
#USE_LIB_MUPARSERX = $(DEFAULT_USE_LIB_MUPARSERX)
# $(shell echo \#define USE_LIB_MUPARSERX $(USE_LIB_MUPARSERX) > .config.h.new )
# #endif
# ifneq ($(shell diff .config.h.new .config.h > /dev/null 2>/dev/null ; echo $$?),0)
# $(shell mv -f .config.h.new .config.h )
# else
# $(shell rm -f .config.h.new )
# endif

$(info  ... LibCM: USE_LIB_MUPARSERX=$(USE_LIB_MUPARSERX))

ifeq ($(USE_LIB_MUPARSERX), 1)
USED_PKGS += muparserx
CXXFLAGS+=-DUSE_LIB_MUPARSERX
endif


CXXFLAGS += $(shell pkg-config --cflags $(USED_PKGS) 2>/dev/null)
LDFLAGS += $(shell pkg-config --libs $(USED_PKGS) 2>/dev/null)

CXXFLAGS_LOCAL=-I$(CMS) -I. -Wno-deprecated-declarations $(shell pkg-config --cflags $(USED_PKGS) 2>/dev/null)

SRC = $(filter-out , $(wildcard *.cc))
DEP = $(SRC:.cc=.d)
OBJ = $(SRC:.cc=.o)


all: checklibs $(LIB)
#DEFAULT_GOAL:=all

.PHONY: clean all mrproper
.SUFFIXES:



checklibs:
ifeq ($(USE_LIB_MUPARSERX), 1)
ifeq ($(shell pkg-config muparserx --exists; echo $$?),1)
$(error "Library dependency `muparserx' is not found ... please download it : `git@linux:sw/other/muparserx'")
endif
endif

%.o: %.cc | $(DEPDIR)
	@$(CXXDEPS) $(DEPFLAGS) $(CXXFLAGS_LOCAL) $(CXXFLAGS) $<
#	$(info $(CXXDEPS) $(DEPFLAGS) $(CXXFLAGS))
	$(CXX) $(CXXFLAGS_LOCAL) $(CXXFLAGS) $< -c


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
	make clean -C $(CMS)

-include $(addprefix $(DEPDIR)/, $(DEP))
