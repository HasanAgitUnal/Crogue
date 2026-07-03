COPY_COMPILE_COMMANDS ?= no
APPNAME := $(shell grep 'set(APPNAME' CMakeLists.txt | cut -d' ' -f2 | tr -d ')')
RUNFLAGS :=
LOGFILE := build/debug.log
BUILD_DIR := build
BUILD_TYPE = -DCMAKE_BUILD_TYPE=Release
CMAKE_TARGET_FLAGS = $(BUILD_TYPE)
CMAKE_FLAGS = $(CMAKE_TARGET_FLAGS) -Wno-dev

all: clean build compile_commands

compile_commands:
ifeq ($(COPY_COMPILE_COMMANDS),yes)
	@cp -f $(BUILD_DIR)/compile_commands.json . 2>/dev/null || true
endif

build:
	@printf -- "-- APPNAME: $(APPNAME)\n"
	@printf -- "-- CMAKE_FLAGS: $(CMAKE_FLAGS)\n"
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake $(CMAKE_FLAGS) ..
	@$(MAKE) -C $(BUILD_DIR)

dbuild: BUILD_TYPE = -DCMAKE_BUILD_TYPE=Debug
dbuild: build

run: build
	@printf "[ \033[32mRUN\033[0m ] running with flags: $(RUNFLAGS)\n"
	@./$(BUILD_DIR)/bin/$(APPNAME) $(RUNFLAGS); \
	printf "[ \033[32mRUN\033[0m ] finished with status: $$?\n"

drun: dbuild run

test: CMAKE_TARGET_FLAGS = -DCMAKE_BUILD_TYPE=Debug -DTEST_ENABLED=ON
test: build
	@cd $(BUILD_DIR) && ctest --output-on-failure

clean:
	@rm -rf $(BUILD_DIR)
	@printf "[ \033[93mCLEAN\033[0m ] finished\n"

log:
	@-tail -n 1 -F $(LOGFILE)

.PHONY: all build dbuild run drun test clean log
