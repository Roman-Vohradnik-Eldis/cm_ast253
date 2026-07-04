OBJDIR ?= $(CURDIR)
VERSIONDIR ?= $(CURDIR)
PROJDIR ?= $(CURDIR)

CC := $(CROSS_COMPILE)gcc
CC0 := $(CC)
LD := $(CC)
ARCH ?= $(shell $(CC0) -dumpmachine)

CCFLAGS := -I. -std=c99 -Wall -ggdb

VERSIONS = $(shell cd $(PROJDIR); $(VERSIONDIR)/version.sh --version)
GIT_VERSIONS = $(shell cd $(PROJDIR); $(VERSIONDIR)/version.sh --git --version)

.PHONY: version.o

version.o: version.c version.h
	$(CC) $(CCFLAGS) $< -c -o $(OBJDIR)/version.o \
	-DVERSIONS="\"$(VERSIONS)\"" \
	-DGIT_VERSIONS="\"$(GIT_VERSIONS)\"" \
	-DOS_PRETTY_NAME="\"$(shell grep PRETTY_NAME /etc/os-release | cut -d\" -f2)\""

clean:
	$(RM) *.o
