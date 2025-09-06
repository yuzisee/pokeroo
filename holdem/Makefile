
CXX = g++
CXXFLAGS_RELATED_HEADERS_OPT = -Os
CXXFLAGS_EXTREME_SPEEDUP = -O3 -funroll-loops -flto -march=native
CXXFLAGS_EXECUTABLE_CHAIN = -O -fpic -Wextra
#The x86 instruction set only allows for -fpic compatible code, but on other architectures we may need this flag to make sure dynamic linking is possible
CXXFLAGS += -g -Wall

EXT_SRC_DIRS = appsrc libsrc
SRC_DIRS = src $(EXT_SRC_DIRS)

HEADERS_SRC := $(wildcard $(patsubst %,%/*.h,$(SRC_DIRS)))
BUILD_SRC := $(wildcard $(patsubst %,%/*.cpp,$(SRC_DIRS)))


BUILD_DIR = objs
BIN_DIR = bin
LIB_DIR = lib


CORE_OBJS = $(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(wildcard src/*.cpp)) appsrc/stratManual.o

BUILD_OBJS = $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(notdir $(BUILD_SRC)))
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
	clang++ $(CXXFLAGS) -D RTTIASSERT -o bin/unittests_clang -Isrc $(wildcard src/*.cpp) unittests/main.cpp
	echo '::endgroup::'
	echo '────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────'
	date
	echo '::group::Compile bin/unittests_gcc_O3'
	g++ $(CXXFLAGS) -D RTTIASSERT -O3 -o bin/unittests_gcc_O3 -Isrc $(wildcard src/*.cpp) unittests/main.cpp
	echo '::endgroup::'
	echo '────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────'
	date
	echo ~/pokeroo-run/lib/holdemdb/ > unittests/holdemdb.ini
	cd unittests && ../bin/unittests_clang && ../bin/unittests_gcc_O3

# `-ffast-math` also? If you're not worried about floating point...
db:
	mkdir -v -p bin
	echo '::group::Compile bin/regenerate_opening_book_selftest'
	clang++ $(CXXFLAGS) $(CXXFLAGS_EXTREME_SPEEDUP) -o bin/regenerate_opening_book_selftest -Isrc -D PROGRESSUPDATE=140 unittests/regenerate_opening_book.cpp src/aiCache.cpp src/aiInformation.cpp src/ai.cpp src/engine.cpp src/engine_base.cpp src/inferentials.cpp src/holdemutil.cpp src/holdem2.cpp src/randomDeck.cpp
	echo '::endgroup::'
	date
	echo '::group::Compile bin/regenerate_opening_book (Same as above but with `-fno-exceptions -fno-rtti`)'
	clang++ $(CXXFLAGS) $(CXXFLAGS_EXTREME_SPEEDUP) -fno-exceptions -fno-rtti -o bin/regenerate_opening_book -Isrc -D PROGRESSUPDATE=140 unittests/regenerate_opening_book.cpp src/aiCache.cpp src/aiInformation.cpp src/ai.cpp src/engine.cpp src/engine_base.cpp src/inferentials.cpp src/holdemutil.cpp src/holdem2.cpp src/randomDeck.cpp
	echo '::endgroup::'
	mkdir -p /tmp/opening_book
ifneq ($(GITHUB_ACTIONS),true)
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
