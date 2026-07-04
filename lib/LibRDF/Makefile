LIB := libRDF.a
DEPDIR := .d

CXX := g++
LD := $(CXX)
AR := ar
MAKE := make --silent
RM := rm -rf

DEPFLAGS = -MM -MF $(DEPDIR)/$*.d
CXXFLAGS = -std=c++11 -g -ggdb -O2 -fpic -Wall -Wundef
CXXFLAGS += -Wno-deprecated-declarations
CXXFLAGS += -Wno-nonnull-compare
CXXFLAGS += -I.

ifeq ($(shell test -e ../LibCMS4 && echo -n yes),yes)
  CXXFLAGS += -I../LibCMS4
else
  CXXFLAGS += -I../LibCMS $(shell pkg-config --cflags glib-2.0)
endif

CXXFLAGS += -DPROJ_VERSION=$(shell pkg-config --modversion proj | cut -d. -f1)

SRC = $(filter-out , $(wildcard *.cc))
DEP = $(SRC:.cc=.d)
OBJ = $(SRC:.cc=.o)

DEFAULT_GOAL := all

all: $(LIB)

%.o: %.cc | $(DEPDIR)
	@$(CXX) $(DEPFLAGS) $(CXXFLAGS) $<
	$(CXX) $(CXXFLAGS) $< -c

$(LIB): $(OBJ)
	$(AR) rcs $(LIB) $(OBJ)

$(DEPDIR):
	@mkdir -p $(DEPDIR)

clean:
	$(RM) $(LIB) $(DEPDIR) $(OBJ)

distclean: clean
	$(RM) core tags *~ *.out *.db *.a

-include $(addprefix $(DEPDIR)/, $(DEP))
