LIB := libCMS.a
DEPDIR := .d

CC := gcc
LD := $(CC)
AR := ar
RM := rm -rf

DEPFLAGS = -MM -MF $(DEPDIR)/$*.d
CFLAGS = -g -ggdb -O2 -fpic -Wall -Wundef
CFLAGS += -Wno-deprecated-declarations
CFLAGS += -Wno-nonnull-compare
CFLAGS += -I.
CFLAGS += $(shell pkg-config --cflags glib-2.0)

SRC = $(filter-out , $(wildcard *.c))
DEP = $(SRC:.c=.d)
OBJ = $(SRC:.c=.o)

DEFAULT_GOAL := all

all: $(LIB)

%.o: %.c | $(DEPDIR)
	@$(CC) $(DEPFLAGS) $(CFLAGS) $<
	$(CC) $(CFLAGS) $< -c

$(LIB): $(OBJ)
	$(AR) rcs $(LIB) $(OBJ)

$(DEPDIR):
	@mkdir -p $(DEPDIR)

clean:
	$(RM) $(LIB) $(DEPDIR) $(OBJ)

distclean: clean
	$(RM) core tags *~ *.out *.db *.a

-include $(addprefix $(DEPDIR)/, $(DEP))
