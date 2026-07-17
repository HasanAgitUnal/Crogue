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
	@sleep 0.5
	@printf -- "-- COPY_COMPILE_COMMANDS  : $(COPY_COMPILE_COMMANDS)\n"
	@printf -- "-- APPNAME                : $(APPNAME)\n"
	@printf -- "-- RUNFLAGS               : $(RUNFLAGS)\n"
	@printf -- "-- CMAKE_FLAGS            : $(CMAKE_FLAGS)\n"
	@printf -- "-- Running CMake\n"
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake $(CMAKE_FLAGS) ..
	@$(MAKE) -C $(BUILD_DIR)

dbuild: BUILD_TYPE = -DCMAKE_BUILD_TYPE=Debug
dbuild: build

run: build
	@printf -- "-- Running $(APPNAME) with flags: $(RUNFLAGS)\n"
	@./$(BUILD_DIR)/bin/$(APPNAME) $(RUNFLAGS); \
	printf -- "-- $(APPNAME) finished with status: $$?\n"

drun: dbuild run

test: CMAKE_TARGET_FLAGS = -DCMAKE_BUILD_TYPE=Debug -DTEST_ENABLED=ON
test: build
	@cd $(BUILD_DIR) && ctest --output-on-failure

clean:
	@rm -rf $(BUILD_DIR)
	@printf -- "-- Cleaned build directory\n"

log:
	@-tail -n 10 -F $(LOGFILE)

format:
	@command -v clang-format >/dev/null 2>&1 || { printf -- '-- clang-format not found\n'; exit 1; }
	@for dir in src include test; do \
		if [ -d "$$dir" ]; then \
			find "$$dir" -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.cc" -o -name "*.hpp" -o -name "*.cxx" -o -name "*.hxx" -o -name "*.ixx" \) -exec clang-format -i -style=file {} \; ; \
		fi; \
	done

.PHONY: all build dbuild run drun test clean log format
