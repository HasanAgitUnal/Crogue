PROGRAM := crogue
RUNFLAGS :=
SOLCONFIG := -DSOL_LUA_VERSION=504
CC := gcc
CXX := g++
CPPFLAGS := -Iinclude -I/usr/include/lua5.4 -MMD -MP -Llibs -rdynamic -g $(SOLCONFIG)
CFLAGS := -lstdc++
CXXFLAGS :=
LDFLAGS := -lncursesw -llua5.4
CXXFILES := $(wildcard src/*.cpp)
CFILES := $(wildcard src/*.c)
OBJFILES := $(patsubst src/%.cpp,build/%.o,$(CXXFILES)) $(patsubst src/%.c,build/%.o,$(CFILES))
DEPS := $(OBJFILES:.o=.d)

default: build

all: clean default

buildstart:
	@mkdir -p build
	@printf "[ \033[34mBUILD\033[0m ] CC: $(CC)\n"
	@printf "[ \033[34mBUILD\033[0m ] CXX: $(CXX)\n"
	@printf "[ \033[34mBUILD\033[0m ] CFLAGS: $(CFLAGS)\n"
	@printf "[ \033[34mBUILD\033[0m ] CXXFLAGS: $(CXXFLAGS)\n"
	@printf "[ \033[34mBUILD\033[0m ] CPPFLAGS: $(CPPFLAGS)\n"
	@printf "[ \033[34mBUILD\033[0m ] LDFLAGS: $(LDFLAGS)\n"
	@printf "[ \033[34mBUILD\033[0m ] started\n"

build: buildstart build/$(PROGRAM)
	@printf "[ \033[34mBUILD\033[0m ] finished\n"

dbuild: CPPFLAGS += -DDEBUG
dbuild: build

build/$(PROGRAM): $(OBJFILES)
	@sleep 0.1
	@printf "[ \033[34mBUILD\033[0m ] [ \033[35mAPP\033[0m ] *.o -> $@\n"
	@$(CXX) $(OBJFILES) -o $@ $(LDFLAGS)

build/%.o: src/%.cpp
	@sleep 0.1
	@printf "[ \033[34mBUILD\033[0m ] [ \033[31mCXX\033[0m ] $< -> $@\n"
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
	@printf "[ \033[34mBUILD\033[0m ] [ \033[36mCXX\033[0m ] $< compiled\n"

build/%.o: src/%.c
	@sleep 0.1
	@printf "[ \033[34mBUILD\033[0m ] [ \033[33mC\033[0m   ] $< -> $@\n"
	@$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
	@printf "[ \033[34mBUILD\033[0m ] [ \033[32mC\033[0m   ] $< compiled\n"

run: build
	@printf "[  \033[32mRUN\033[0m  ] started\n"
	@./build/$(PROGRAM) $(RUNFLAGS); printf "[  \033[32mRUN\033[0m  ] finished with status: $$?\n"

drun: dbuild run

log: dbuild
	@printf "[  \033[36mLOG\033[0m  ] Viewing:\n"
	@-tail -n 0 -F build/debug.log

cbin:
	@printf "[ \033[93mCLEAN\033[0m ] [ \033[92mBIN\033[0m ] started\n"
	@rm -f build/*.o build/*.d build/$(PROGRAM)	
	@printf "[ \033[93mCLEAN\033[0m ] [ \033[92mBIN\033[0m ] finished\n"

clog:
	@printf "[ \033[93mCLEAN\033[0m ] [ \033[94mLOG\033[0m ] started\n"
	@rm -f build/debug.log
	@printf "[ \033[93mCLEAN\033[0m ] [ \033[94mLOG\033[0m ] finished\n"

clean: cbin clog

-include $(DEPS)

.PHONY: default all build dbuild run drun log clean buildstart
