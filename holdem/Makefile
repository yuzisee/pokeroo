
CXX = g++
# [!NOTE]
# As far as we can tell for now, the executable built by g++ runs about 25% faster.
# Meanwhile, we are investigating whether clang++ or g++ is more *accurate* also
CXX_REGENERATE_DB = g++
CXXFLAGS_RELATED_HEADERS_OPT = -Os
CXXFLAGS_EXTREME_SPEEDUP = -O3 -funroll-loops -march=native
CXXFLAGS_EXECUTABLE_CHAIN = -g -O -fpic -Wextra -Wno-parentheses
CXXFLAGS_LIVEDEBUG = -g -D RTTIASSERT
#The x86 instruction set only allows for -fpic compatible code, but on other architectures we may need this flag to make sure dynamic linking is possible
CXXFLAGS += -Wall
# [!NOTE]
# We manually override with `-Wno-unused-parameter` below in some spots

EXT_SRC_DIRS = appsrc libsrc
SRC_DIRS = src $(EXT_SRC_DIRS)

HEADERS_SRC := $(wildcard $(patsubst %,%/*.h,$(SRC_DIRS)))
BUILD_SRC := $(wildcard $(patsubst %,%/*.cpp,$(SRC_DIRS)))


BUILD_DIR = objs
BIN_DIR = bin
LIB_DIR = lib


CORE_OBJS = $(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(wildcard src/*.cpp)) appsrc/stratManual.o

BUILT_OBJS := $(wildcard $(BUILD_DIR)/*.o)
BUILD_DEPS = $(patsubst $(BUILD_DIR)/%.o,$(BUILD_DIR)/%.d,$(BUILT_OBJS))


STATIC_OBJ_CPP = $(BUILD_DIR)/holdem.a

#Naming your shared libraries: http://www.faqs.org/docs/Linux-HOWTO/Program-Library-HOWTO.html#SHARED-LIBRARIES
APP = $(BIN_DIR)/holdem.$(shell uname -m)
DYNAMIC_LINK_LIB_C = $(LIB_DIR)/libholdem.so
DYNAMIC_SO_LIB_C = $(DYNAMIC_LINK_LIB_C).1

#For easier access to libholdem.so, we may eventually try http://tldp.org/LDP/LGNET/issue55/tag/5.html

FINAL_TARGETS = $(DYNAMIC_LINK_LIB_C) $(DYNAMIC_SO_LIB_C) $(APP)


vpath %.cpp $(SRC_DIRS)


all:
	make tags
	echo "Building project"
	make $(FINAL_TARGETS)

test:
	echo $(SRC_DIRS)
	echo $(HEADERS_SRC)
	echo $(BUILD_SRC)
	echo $(CORE_OBJS)
	mkdir -v -p bin
	date
	echo '::group::Compile bin/unittests_clang'
	clang++ $(CXXFLAGS) $(CXXFLAGS_LIVEDEBUG) -fprofile-instr-generate -fcoverage-mapping -Isrc -o bin/unittests_clang $(wildcard src/*.cpp) unittests/main.cpp
	echo '::endgroup::'
	echo '────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────'
	date
	echo '::group::Compile bin/unittests_gcc_O3'
	g++ $(CXXFLAGS) $(CXXFLAGS_LIVEDEBUG) $(CXXFLAGS_EXTREME_SPEEDUP) -Isrc -o bin/unittests_gcc_O3 $(wildcard src/*.cpp) unittests/main.cpp
	echo '::endgroup::'
	echo '────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────'
	date
	echo ~/pokeroo-run/lib/holdemdb/ > unittests/holdemdb.ini
	mkdir -p /tmp/code_coverage.llvm
	cd unittests && LLVM_PROFILE_FILE="/tmp/code_coverage.llvm/unittests.profraw" ../bin/unittests_clang && ../bin/unittests_gcc_O3

REGENERATE_OPENING_BOOK_SRC = unittests/regenerate_opening_book.cpp src/aiCache.cpp src/aiInformation.cpp src/ai.cpp src/engine.cpp src/engine_base.cpp src/inferentials.cpp src/holdemutil.cpp src/holdem2.cpp src/randomDeck.cpp

# `-ffast-math` also? If you're not worried about floating point...
# It manifests in the final result only within https://github.com/yuzisee/pokeroo/pull/16#discussion_r2336961785
# but for now we'll leave it off since having an accurate DB seems preferred since the true runtime of reading from disk is super fast anyway
db:
	mkdir -v -p bin
	echo '::group::Compile bin/regenerate_opening_book_selftest (perf can be used for profiling)'
	clang++ $(CXXFLAGS) $(CXXFLAGS_EXTREME_SPEEDUP) -flto -g -fno-omit-frame-pointer -o bin/regenerate_opening_book_selftest -Isrc -D PROGRESSUPDATE=140 $(REGENERATE_OPENING_BOOK_SRC)
	echo '::endgroup::'
	date
	echo '::group::Compile bin/regenerate_opening_book (full speed, i.e `-fno-exceptions -fno-rtti` and no profiling)'
	$(CXX_REGENERATE_DB) $(CXXFLAGS) $(CXXFLAGS_EXTREME_SPEEDUP) -flto -fno-exceptions -fno-rtti -o bin/regenerate_opening_book -Isrc -D PROGRESSUPDATE=140  $(REGENERATE_OPENING_BOOK_SRC)
	echo '::endgroup::'
	date
	echo '::group::Compile bin/regenerate_opening_book_profiling (`g++ -pg` instead of clang++) use gprof to inspect'
	g++ $(CXXFLAGS) $(CXXFLAGS_EXTREME_SPEEDUP) -g -pg -o bin/regenerate_opening_book_profiling -Isrc -D PROGRESSUPDATE=140 -ffast-math $(REGENERATE_OPENING_BOOK_SRC)
	echo '::endgroup::'
ifneq ($(GITHUB_ACTIONS),true)
	mkdir -p /tmp/opening_book
	HOLDEMDB_PATH=/tmp/opening_book bin/regenerate_opening_book 1
endif
	date

clean:
	rm -R $(BUILD_DIR)

depends:
	make $(BUILD_DEPS)

# Each of our `.d` files also lists the header files it depended on.
# This `include` ensures that if any `*.h` file changes, any relevant `*.cpp` files will also be rebuilt.
-include $(BUILD_DEPS)
# [!TIP]
# Remember that these builds will be triggered even if you `make test` or `make db` etc.

tags: $(BUILD_SRC) $(HEADERS_SRC)
	ctags $^

# TODO(from joseph): Perhaps we should also have a `tournament` target that builds `sim/supergame.cpp`

$(BUILD_DIR)/%.d: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_RELATED_HEADERS_OPT) -MM -MF $@ -MT $(BUILD_DIR)/$*.o $<


$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@echo
	@echo "Rebuilding $@ [$?]:"
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_EXECUTABLE_CHAIN) -Wno-unused-parameter -c -o $@ $<

appsrc/stratManual.o: appsrc/stratManual.cpp
	@echo "appsrc/stratManual.cpp is needed only for the actual appsrc/testDriver.cpp entrypoint $@ [$<]:"
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_EXECUTABLE_CHAIN) -Wno-unused-parameter -c -o $@ $<

$(STATIC_OBJ_CPP): $(CORE_OBJS)
	@mkdir -p $(dir $@)
	ar rcs $@ $^

#
#  FINAL TARGET RULES
#

$(APP): $(BUILD_DIR)/testDriver.o $(STATIC_OBJ_CPP)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_EXECUTABLE_CHAIN) -o $@ $^

$(DYNAMIC_SO_LIB_C): $(BUILD_DIR)/interfaceC.o $(STATIC_OBJ_CPP)
	@mkdir -p $(dir $@)
ifeq ($(shell uname -s),Darwin)
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_EXECUTABLE_CHAIN) -shared -o $@ -Wl,-install_name,libholdem.so $^
else
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_EXECUTABLE_CHAIN) -shared -o $@ -Wl,-soname,libholdem.so $^
endif

$(DYNAMIC_LINK_LIB_C): $(DYNAMIC_SO_LIB_C)
	cp -v $< $@
