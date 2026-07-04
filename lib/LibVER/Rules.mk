
## Expand immediately
LIBVER_DIR:=$(TARGET_SUBDIR)
LIBVER_BUILD_DIR:=$(BUILD_DIR)/$(TARGET_SUBDIR)

TARGET_LIBS=libVER
libVER_SOURCES=project_version.cc
libVER_ADDSOURCES=generated_git_info.cc

## Generated at build time
libVER_GENSOURCES=$(LIBVER_BUILD_DIR)/generated_project_info.cc

## Include in source tarball
GENERATED_SOURCES=$(LIBVER_DIR)/generated_git_info.cc

## Rebuild when Makefile, Rules.mk, or command line options are changed
$(LIBVER_BUILD_DIR)/generated_project_info.cc: $(BUILD_DIR)/project_makefile.txt  $(BUILD_DIR)/project_rules.txt $(BUILD_DIR)/project_targets.txt
	@mkdir -p $(LIBVER_BUILD_DIR)
	@$(LIBVER_DIR)/create_project_info $(BUILD_DIR) $@

## Rebuild when git status changes
$(LIBVER_DIR)/generated_git_info.cc: LIBVER_FORCE_BUILD
	@$(LIBVER_DIR)/create_git_info . $@

LIBVER_FORCE_BUILD:
.PHONY: LIBVER_FORCE_BUILD
