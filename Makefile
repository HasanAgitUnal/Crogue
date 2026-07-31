COPY_COMPILE_COMMANDS ?= no
LOG_ENABLED ?= 1
APPNAME := $(shell grep 'set(APPNAME' CMakeLists.txt | cut -d' ' -f2 | tr -d ')')
RUNFLAGS ?=
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
	@sleep 0.3
	@mkdir -p $(BUILD_DIR)
	@printf -- "--------------------------------------------------------------\n"
	@printf -- " APPNAME                : $(APPNAME)\n"
	@printf -- " RUNFLAGS               : $(RUNFLAGS)\n"
	@printf -- " COPY_COMPILE_COMMANDS  : $(COPY_COMPILE_COMMANDS)\n"
	@printf -- " CMAKE_FLAGS            : $(CMAKE_FLAGS)\n"
	@printf -- "--------------------------------------------------------------\n"
	@if [ ! -f $(BUILD_DIR)/Makefile ]; then \
		printf -- "-- Running CMake\n";\
		cd $(BUILD_DIR) && cmake $(CMAKE_FLAGS) ..; \
	fi
	@$(MAKE) -C $(BUILD_DIR)

dbuild: BUILD_TYPE = -DCMAKE_BUILD_TYPE=Debug
dbuild: build

BEFORE_RUN :=
AFTER_RUN :=
ifeq ($(LOG_ENABLED), 1)
	BEFORE_RUN := printf -- "--- START ---\n" >> $(LOGFILE)
	AFTER_RUN := printf -- "--- END ---\n" >> $(LOGFILE)
endif

run: build
	@$(BEFORE_RUN)
	@printf -- "-- Running $(APPNAME) with flags: $(RUNFLAGS)\n"
	@./$(BUILD_DIR)/bin/$(APPNAME) $(RUNFLAGS); \
	printf -- "-- $(APPNAME) finished with status: $$?\n"
	@$(AFTER_RUN)

drun: dbuild run

test: CMAKE_TARGET_FLAGS = -DCMAKE_BUILD_TYPE=Debug -DTEST_ENABLED=ON
test: build
	@cd $(BUILD_DIR) && ctest --output-on-failure

clean:
	@rm -rf $(BUILD_DIR)
	@printf -- "-- Cleaned build directory\n"

ifeq ($(LOG_ENABLED), 1)
log:
	@-tail -n 10 -F $(LOGFILE) 2>/dev/null

else
log:
	@printf -- "-- Logging is not enabled!!\n-- Enable for single run with LOG_ENABLED=1 option.\n"

endif

format:
	@command -v clang-format >/dev/null 2>&1 || { printf -- '-- clang-format not found\n'; exit 1; }
	@for dir in src include test; do \
		if [ -d "$$dir" ]; then \
			find "$$dir" -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.cc" -o -name "*.hpp" -o -name "*.cxx" -o -name "*.hxx" -o -name "*.ixx" \) -exec clang-format -i -style=file {} \; ; \
		fi; \
	done

.PHONY: all build dbuild run drun test clean log format
