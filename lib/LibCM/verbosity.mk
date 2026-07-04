VERBOSE ?= 0

ifdef CXX
	ACTUAL := $(CXX)
	CXX_0 = @echo "CXX $<"; $(ACTUAL)
	CXX_1 = $(ACTUAL)
	CXX = $(CXX_$(VERBOSE))
	CXXDEPS = $(CXX_1)
endif

ifdef CC
	ACTUAL := $(CC)
	CC_0 = @echo "CC $<"; $(ACTUAL)
	CC_1 = $(ACTUAL)
	CC = $(CC_$(VERBOSE))
	CCDEPS = $(CC_1)
endif

ifdef LD
	ACTUAL := $(LD)
	LD_0 = @echo "LD $@"; $(ACTUAL)
	LD_1 = $(ACTUAL)
	LD = $(LD_$(VERBOSE))
endif

ifdef MAKE
	ACTUAL_MAKE := $(MAKE)
	MAKE_0 = @echo "MAKE $(dir $@)"; $(ACTUAL_MAKE)
	MAKE_1 = $(ACTUAL_MAKE)
	MAKE = $(MAKE_$(VERBOSE)) VERBOSE=$(VERBOSE)
endif

ifdef AR
	ACTUAL_AR := $(AR)
	AR_0 = @echo "AR $@"; $(ACTUAL_AR)
	AR_1 = $(ACTUAL_AR)
	AR = $(AR_$(VERBOSE))
endif
