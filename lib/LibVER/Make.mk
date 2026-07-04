
.SUFFIXES:
.PHONY: all build_first
.DEFAULT_GOAL:=all

## To be filled from subdirectories ...
ALL_TARGET_BINS:=
ALL_TARGET_LIBS:=
ALL_TARGET_SHARED_LIBS:=
ALL_GENERATED_SOURCES:=

## VERBOSITY
V?=0
VERBOSE:=$(or $(VERBOSE),$(V))
ifeq ($(VERBOSE),0)
AT:=@
else
AT:=
endif

BUILD_DIR?=.
BIN_DIR?=$(BUILD_DIR)/bin
PKG_CONFIG?=pkg-config

CC0?=$(CC)
NVCC?=nvcc

INFO_ARCH=$(shell $(CC0) -dumpmachine)
INFO_HOSTNAME=$(shell hostname)
INFO_KERNEL_VERSION=$(shell uname --kernel-version)
INFO_COMPILER_VERSION=$(CC0):$(shell $(CC0) -dumpversion)
INFO_OS_PRETTY_NAME=$(shell grep PRETTY_NAME /etc/os-release | cut -d\" -f2)

# $1 = bin_name, $2 = directory
#
# Note: We ignore libraries which have empty HEADERS_DIRECTORY
define get_compilation_flags
-I$2/$($(1)_INCLUDE) \
$$(foreach d,$$(foreach l,$$($(1)_ALL_LIBS),$$($$(l)_HEADERS_DIRECTORY)),-I$$(d)) \
$$(if $$(strip $$($(1)_ALL_PKGS)),$$(shell $(PKG_CONFIG) --cflags $$($(1)_ALL_PKGS)),)\
-DPRODUCT_NAME="\"$1\""
endef

# $1 = bin_name, $2 = directory
# $@ = relative_path/executable_name
# $< = relative_path/source1.o relative_path/source2.o ...
# We use $+ which does not remove duplicate arguments.
define get_linker_args
$$+\
$$(LDFLAGS)\
$$($(1)_LDFLAGS)\
$$(if $$(strip  $$($(1)_ALL_PKGS)),$$(shell $(PKG_CONFIG) --libs $$($(1)_ALL_PKGS)),)\
-o $$@\
$$($(1)_ADD_LIBS)
endef

# $1 = bin_name, $2 = directory
# $@ = relative_path/executable_name
# $< = relative_path/source1.o relative_path/source2.o ...
# We use $+ which does not remove duplicate arguments.
define get_shared_lib_args
-shared\
-Wl,--whole-archive\
$$^\
-Wl,--no-whole-archive\
$$(if $$(strip  $$($(1)_ALL_PKGS)),$$(shell $(PKG_CONFIG) --libs $$($(1)_ALL_PKGS)),)\
-o $$@\
$$(LDFLAGS)\
$$($(1)_LDFLAGS)
endef

# $1 = bin_name, $2 = directory.
define print_verbose_info
ifeq ($(VERBOSE),2)
	@cat $(BUILD_DIR)/project_makefile.txt
	@cat $(BUILD_DIR)/$(2)/project_info_$(1).txt
endif
endef

# $1 = bin_name, $2 = directory, $3 = type (C,CC,CPP,CU...)
# $@ = relative_path/source_name.o
# $< = relative_path/source_name.XXX
define generate_compile_source_command
	@printf "\e[32mBuild $(3) $$@\e[0m\n"
$(call print_verbose_info,$(1),$(2))
	@mkdir -p $(BUILD_DIR)/$(2)/$(DEPDIR)
	@mkdir -p $$(dir $$@)
	$(AT) $$($(1)_COMPILER_$(3)) -c -MMD -MP -MF $$(@:.o=.d) $$($(1)_COMPILER_FLAGS) $$($(1)_FLAGS_$(3)) -o $$@ $$<
endef

# $1 = bin_name (from TARGET_BINS), $2 = directory (of Rules.mk), $3 = type (C,CC,CPP,CU...)
# $@ = relative_path/depdir/source_name.cmd
# $< = relative_path/source_name.cc (or .c)
# Note: It is possible to use jq instead of sed.
define generate_build_command
	@mkdir -p $$(dir $$@)
	@{\
	printf '{\n';\
	printf '  "directory":"%s",\n' '$(CURDIR)';\
	printf '  "file":"%s",\n' '$$<';\
	printf '  "arguments": [\n';\
	printf "%s\n" $$($(1)_COMPILER_$(3)) -c $$($(1)_COMPILER_FLAGS) $$($(1)_FLAGS_$(3)) -o $$(basename $$<).o $$<\
	| sed -e 's/["\]/\\\0/g' -e '1s/^/    "/' -e '2,$$$$s/^/   ,"/' -e 's/$$$$/"/';\
	printf '  ]\n';\
	printf '}\n';\
	} > $$@.tmp
	@if ! cmp -s $$@.tmp $$@; then mv $$@.tmp $$@; fi
endef

# $1 = bin_name, $2 = directory.
define make_shared_lib
	@printf "\e[32mSHARED LIB $$@\e[0m\n"
$(call print_verbose_info,$(1),$(2))
	@mkdir -p $(BIN_DIR)
	$(AT)$$($(1)_LD) $(call get_shared_lib_args,$(1),$(2))
endef

# $1 = bin_name, $2 = directory.
define make_link
	@printf "\e[32mLink $$@\e[0m\n"
$(call print_verbose_info,$(1),$(2))
	@mkdir -p $(BIN_DIR)
	$(AT)$$($(1)_LD) $$($(1)_LINKER_ARGS)
endef

# $1 = bin_name, $2 = directory.
define make_archive
	@printf "\e[32mArch $$@\e[0m\n"
$(call print_verbose_info,$(1),$(2))
	@mkdir -p $(BIN_DIR)
	$(AT)$(AR) rcs $$@ $$($(1)_ALL_OBJECTS)
endef

# set_target_variables $1 = bin_name, $2 = directory
define set_target_variables
$(1)_ROOT=$(2)
$(1)_ALL_LIBS=$$($(1)_LIBS) $$(foreach l,$$($(1)_LIBS),$$($$(l)_ALL_LIBS))
$(1)_ALL_SOURCES=$(wildcard $(addprefix $(2)/,$($(1)_SOURCES))) $(addprefix $(2)/,$($(1)_ADDSOURCES))
$(1)_ALL_PKGS=$$($(1)_PKGS) $$(foreach l,$$($(1)_ALL_LIBS),$$($$(l)_PKGS))
$(1)_CUSTOM_OBJECTS=$$(addprefix $(BUILD_DIR)/,$(addprefix $(2)/,$($(1)_OBJECTS)))
$(1)_CC?=$(CC)
$(1)_CXX?=$(CXX)
$(1)_NVCC?=$(NVCC)
$(1)_LD?=$(CXX) # Tohle nevim, jestli je uplne dobre

$(1)_C_SOURCES=$$(filter %.c,$$($(1)_ALL_SOURCES))
$(1)_CC_SOURCES=$$(filter %.cc,$$($(1)_ALL_SOURCES))
$(1)_CPP_SOURCES=$$(filter %.cpp,$$($(1)_ALL_SOURCES))
$(1)_CU_SOURCES=$$(filter %.cu,$$($(1)_ALL_SOURCES))

$(1)_C_CMDS=$$(foreach src,$$($(1)_C_SOURCES),$(BUILD_DIR)/$$(src:.c=.cmd))
$(1)_CC_CMDS=$$(foreach src,$$($(1)_CC_SOURCES),$(BUILD_DIR)/$$(src:.cc=.cmd))
$(1)_CPP_CMDS=$$(foreach src,$$($(1)_CPP_SOURCES),$(BUILD_DIR)/$$(src:.cpp=.cmd))
$(1)_CU_CMDS=$$(foreach src,$$($(1)_CU_SOURCES),$(BUILD_DIR)/$$(src:.cu=.cmd))

$(1)_GENERATED_OBJECTS=$$($(1)_GENSOURCES:.cc=.o)
$(1)_C_OBJECTS=$$(addprefix $(BUILD_DIR)/,$$($(1)_C_SOURCES:.c=.o))
$(1)_CC_OBJECTS=$$(addprefix $(BUILD_DIR)/,$$($(1)_CC_SOURCES:.cc=.o))
$(1)_CPP_OBJECTS=$$(addprefix $(BUILD_DIR)/,$$($(1)_CPP_SOURCES:.cpp=.o))
$(1)_CU_OBJECTS=$$(addprefix $(BUILD_DIR)/,$$($(1)_CU_SOURCES:.cu=.o))

$(1)_COMPILER_C=$$($(1)_CC)
$(1)_COMPILER_CC=$$($(1)_CXX)
$(1)_COMPILER_CPP=$$($(1)_CXX)
$(1)_COMPILER_CU=$$($(1)_NVCC)

$(1)_FLAGS_C=$$(CXXFLAGS) $$($(1)_CXXFLAGS)
$(1)_FLAGS_CC=$$(CXXFLAGS) $$($(1)_CXXFLAGS)
$(1)_FLAGS_CPP=$$(CXXFLAGS) $$($(1)_CXXFLAGS)
$(1)_FLAGS_CU=$$(CUFLAGS) $$($(1)_CUFLAGS)

$(1)_ALL_OBJECTS=$$($(1)_GENERATED_OBJECTS) $$($(1)_C_OBJECTS) \
$$($(1)_CC_OBJECTS) $$($(1)_CPP_OBJECTS) $$($(1)_CU_OBJECTS) $$($(1)_CUSTOM_OBJECTS)

$(1)_COMPILER_FLAGS=$(call get_compilation_flags,$(1),$(2))
$(1)_LINKER_ARGS=$(call get_linker_args,$(1),$(2))
endef

# make_project_info $1 = bin_name, $2 = directory
define make_project_info
$(BUILD_DIR)/project_rules.txt: $(BUILD_DIR)/$(2)/project_info_$(1).txt
$(BUILD_DIR)/$(2)/project_info_$(1).txt: FORCE_BUILD
	@mkdir -p $(BUILD_DIR)/$(2)
	@printf "%s\n" \
		 "$(1)_ROOT=$$($(1)_ROOT)" \
		 "$(1)_INCLUDE=$$($(1)_INCLUDE)" \
		 "$(1)_CC=$$($(1)_CC)" \
		 "$(1)_CXX=$$($(1)_CXX)" \
		 "$(1)_LD=$$($(1)_LD)" \
		 "$(1)_LIBS=$$(sort $$($(1)_ALL_LIBS))" \
		 "$(1)_PKGS=$$(sort $$($(1)_ALL_PKGS))" \
		 "$(1)_CXXFLAGS=$$($(1)_CXXFLAGS)" \
		 "$(1)_LDFLAGS=$$($(1)_LDFLAGS)" \
		 "$(1)_GENSOURCES=$$($(1)_GENSOURCES)" \
		 "$(1)_SOURCES=$$($(1)_ALL_SOURCES)" \
		 > $$@.tmp
	@if ! cmp -s $$@.tmp $$@; then mv $$@.tmp $$@; fi
endef

# make_compilation_rules $1 = bin_name, $2 = directory
define make_compilation_rules
$$($(1)_GENERATED_OBJECTS): %.o: %.cc $(BUILD_DIR)/$(2)/project_info_$(1).txt $(BUILD_DIR)/project_makefile.txt | build_first
	$(call generate_compile_source_command,$(1),$(2),CC)

# Generate build rules
$$($(1)_C_OBJECTS): $(BUILD_DIR)/$(2)/%.o: $(2)/%.c $(BUILD_DIR)/$(2)/project_info_$(1).txt $(BUILD_DIR)/project_makefile.txt | build_first
	$(call generate_compile_source_command,$(1),$(2),C)
$$($(1)_CC_OBJECTS): $(BUILD_DIR)/$(2)/%.o: $(2)/%.cc $(BUILD_DIR)/$(2)/project_info_$(1).txt $(BUILD_DIR)/project_makefile.txt | build_first
	$(call generate_compile_source_command,$(1),$(2),CC)
$$($(1)_CPP_OBJECTS): $(BUILD_DIR)/$(2)/%.o: $(2)/%.cpp $(BUILD_DIR)/$(2)/project_info_$(1).txt $(BUILD_DIR)/project_makefile.txt | build_first
	$(call generate_compile_source_command,$(1),$(2),CPP)
$$($(1)_CU_OBJECTS): $(BUILD_DIR)/$(2)/%.o: $(2)/%.cu $(BUILD_DIR)/$(2)/project_info_$(1).txt $(BUILD_DIR)/project_makefile.txt | build_first
	$(call generate_compile_source_command,$(1),$(2),CU)

# Generate compile_commands.json rules
$$($(1)_C_CMDS): $(BUILD_DIR)/%.cmd: %.c $(BUILD_DIR)/$(2)/project_info_$(1).txt $(BUILD_DIR)/project_makefile.txt
	$(call generate_build_command,$(1),$(2),C)
$$($(1)_CC_CMDS): $(BUILD_DIR)/%.cmd: %.cc $(BUILD_DIR)/$(2)/project_info_$(1).txt $(BUILD_DIR)/project_makefile.txt
	$(call generate_build_command,$(1),$(2),CC)
$$($(1)_CPP_CMDS): $(BUILD_DIR)/%.cmd: %.cpp $(BUILD_DIR)/$(2)/project_info_$(1).txt $(BUILD_DIR)/project_makefile.txt
	$(call generate_build_command,$(1),$(2),CPP)
$$($(1)_CU_CMDS): $(BUILD_DIR)/%.cmd: %.cu $(BUILD_DIR)/$(2)/project_info_$(1).txt $(BUILD_DIR)/project_makefile.txt
	$(call generate_build_command,$(1),$(2),CU)

$(BUILD_DIR)/compile_commands.json: $$($(1)_C_CMDS) $$($(1)_CC_CMDS) $$($(1)_CPP_CMDS) $$($(1)_CU_CMDS)
endef

# make_shared_libs_rule: $1 = bin_name, $2 = directory
#
define make_shared_lib_rule
$(call make_compilation_rules,$(1),$(2))
.PHONY: $(1)
$(1): compile_commands $(BIN_DIR)/$(1).so
$(BIN_DIR)/$(1).so: $$($(1)_ALL_OBJECTS) $$(foreach l,$$($(1)_ALL_LIBS),$(BUILD_DIR)/$$($$(l)_ROOT)/$$(l).a) $($(1)_DEPS)
	$(call make_shared_lib,$(1),$(2))
$$(foreach src,$$($(1)_ALL_SOURCES),$$(eval -include $(BUILD_DIR)/$$(basename $$(src)).d))
$(call make_project_info,$(1),$(2))
endef

# make_bin_rule: $1 = bin_name, $2 = directory
#
# Note: We added '.PHONY: bin_name', and so the user can write `make bin_name`
# instead of `make bin/bin_name`.
define make_bin_rule
$(call make_compilation_rules,$(1),$(2))
.PHONY: $(1)
$(1): compile_commands $(BIN_DIR)/$(1)
$(BIN_DIR)/$(1): $$($(1)_ALL_OBJECTS) $$(foreach l,$$($(1)_ALL_LIBS),$(BUILD_DIR)/$$($$(l)_ROOT)/$$(l).a) $($(1)_DEPS)
	$(call make_link,$(1),$(2))
$$(foreach src,$$($(1)_ALL_SOURCES),$$(eval -include $(BUILD_DIR)/$$(basename $$(src)).d))
$(call make_project_info,$(1),$(2))
endef

# $1 = lib_name, $2 = directory
# Note: When we delete a source file, project_info_$1 changes, and the library is rebuilt
define make_lib_rule
$(call make_compilation_rules,$(1),$(2))
$(1)_HEADERS_DIRECTORY=$(2)/$($(1)_INCLUDE)
.PHONY: $(1)
$(1): $(BUILD_DIR)/$(2)/$(1).a
$(BUILD_DIR)/$(2)/$(1).a: $$($(1)_ALL_OBJECTS) $(BUILD_DIR)/$(2)/project_info_$(1).txt
	@mkdir -p $(BUILD_DIR)/$(2)
	$(make_archive)
$$(foreach src,$$($(1)_ALL_SOURCES),$$(eval -include $(BUILD_DIR)/$$(basename $$(src)).d))
$(call make_project_info,$(1),$(2))
endef

# $1 = directory
define load_rules_mk
# 1. Clean local variables
TARGET_BINS:=
TARGET_LIBS:=
TARGET_SHARED_LIBS:=
GENERATED_SOURCES:=
TARGET_SUBDIR:=$1
# 2. Fill them from user settings
include $(1)/Rules.mk
ALL_TARGET_LIBS+=$$(TARGET_LIBS)
ALL_TARGET_BINS+=$$(TARGET_BINS)
ALL_TARGET_SHARED_LIBS+=$$(TARGET_SHARED_LIBS)
ALL_GENERATED_SOURCES+=$$(GENERATED_SOURCES)
$$(foreach t,$$(TARGET_LIBS), $$(eval $$(call set_target_variables,$$(t),$(1))))
$$(foreach t,$$(TARGET_BINS), $$(eval $$(call set_target_variables,$$(t),$(1))))
$$(foreach t,$$(TARGET_SHARED_LIBS), $$(eval $$(call set_target_variables,$$(t),$(1))))
TEST_ROOT:=$$($(1)_ROOT)
TARGET_SUBDIR:=
endef

######################################################################

# First, collect all targets and fill their attributes (SOURCES, CXXFLAGS...)
$(foreach dir,$(TARGET_SUBDIRS), $(eval $(call load_rules_mk,$(dir))))

ALL_PKGS:=$(sort $(foreach t,$(ALL_TARGET_BINS) $(ALL_TARGET_LIBS), $($(t)_PKGS)))

# ifneq ($(ALL_PKGS),)
# ifneq (Yes, $(shell $(PKG_CONFIG) --exists --print-errors $(ALL_PKGS) && echo Yes))
# $(error "Build failed: Cannot find required packages")
# endif
# endif

# When all attributes are loaded, generate build rules
$(foreach target,$(ALL_TARGET_BINS), $(eval $(call make_bin_rule,$(target),$($(target)_ROOT))))
$(foreach target,$(ALL_TARGET_LIBS), $(eval $(call make_lib_rule,$(target),$($(target)_ROOT))))
$(foreach target,$(ALL_TARGET_SHARED_LIBS), $(eval $(call make_shared_lib_rule,$(target),$($(target)_ROOT))))

all: $(BUILD_DIR)/compile_commands.json $(BUILD_DIR)/project_rules.txt $(BUILD_DIR)/project_makefile.txt

$(BUILD_DIR)/project_targets.txt: FORCE_BUILD
	@mkdir -p $(BUILD_DIR)
	@printf "%s=%s\n" \
		"TARGET_BINS" "$(ALL_TARGET_BINS)" \
		"TARGET_LIBS" "$(ALL_TARGET_LIBS)" \
		"TARGET_SHARED_LIBS" "$(ALL_TARGET_SHARED_LIBS)" \
		> $@.tmp
	@if ! cmp -s $@.tmp $@; then printf "\e[32mUpdating $@\e[0m\n"; mv $@.tmp $@; fi

$(BUILD_DIR)/project_makefile.txt: FORCE_BUILD
	@mkdir -p $(BUILD_DIR)
	@printf "%s=%s\n" \
		"CXXFLAGS" "$(CXXFLAGS)" \
		"LDFLAGS" "$(LDFLAGS)" \
		"HOSTNAME" "$(INFO_HOSTNAME)" \
		"KERNEL_VERSION" "$(INFO_KERNEL_VERSION)" \
		"COMPILER_VERSION" "$(INFO_COMPILER_VERSION)" \
		"ARCH" "$(INFO_ARCH)" \
		"OS_PRETTY_NAME" "$(INFO_OS_PRETTY_NAME)" \
		> $@.tmp
	@if ! cmp -s $@.tmp $@; then printf "\e[32mUpdating $@\e[0m\n"; mv $@.tmp $@; fi

$(BUILD_DIR)/project_rules.txt:
	@printf "\e[32mUpdating $@\e[0m\n"
	@cat $^ > $@

$(BUILD_DIR)/compile_commands.json:
	@printf "\e[32mUpdating $@\e[0m\n"
	@{ printf "["; cat /dev/null $^; printf "]\n"; } | sed 's/^{/,{/' > $@

compile_commands: $(BUILD_DIR)/compile_commands.json

.PHONY: compile_commands

$(BUILD_DIR)/ls-files:  $(ALL_GENERATED_SOURCES) FORCE_BUILD
	@printf "Creating ls-files\n"
	@mkdir -p $(BUILD_DIR)
	@{ \
		git ls-files --recurse-submodules; \
		printf "%s\n" $(ALL_GENERATED_SOURCES); \
	} | sort > $@

tarball: $(BUILD_DIR)/ls-files
	$(if $(TARBALL_DEST),,$(error Please set TARBALL_DEST))
	tar -zcf $(TARBALL_DEST) --transform='s/^/$(basename $(notdir $(TARBALL_DEST)))\//' --files-from=$(BUILD_DIR)/ls-files

rsync: $(BUILD_DIR)/ls-files
	$(if $(RSYNC_DEST),,$(error Please set RSYNC_DEST))
	rsync -ac --chown=root:root --files-from=$(BUILD_DIR)/ls-files . $(RSYNC_DEST)

.PHONY: rsync tarball FORCE_BUILD

FORCE_BUILD:
