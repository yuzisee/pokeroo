
CXX = g++
CXXFLAGS += -O -g -Wall -fpic
#The x86 instruction set only allows for -fpic compatible code, but on other architectures we may need this flag to make sure dynamic linking is possible

EXT_SRC_DIRS = appsrc libsrc
SRC_DIRS = src $(EXT_SRC_DIRS)

HEADERS_SRC := $(wildcard $(patsubst %,%/*.h,$(SRC_DIRS)))
BUILD_SRC := $(wildcard $(patsubst %,%/*.cpp,$(SRC_DIRS)))
 

BUILD_DIR = objs
BIN_DIR = bin
LIB_DIR = lib


CORE_OBJS = $(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(wildcard src/*.cpp))

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

clean:
	rm -R $(BUILD_DIR)

depends:
	make $(BUILD_DEPS)


-include $(BUILD_DEPS)

tags: $(BUILD_SRC) $(HEADERS_SRC)
	ctags $^


$(BUILD_DIR)/%.d: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MM -MF $@ -MT $(BUILD_DIR)/$*.o $<


$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@echo
	@echo "Rebuilding $@ [$?]:"
	$(CXX) $(CXXFLAGS) -c -o $@ $< 

$(STATIC_OBJ_CPP): $(CORE_OBJS)
	@mkdir -p $(dir $@)
	ar rcs $@ $^

#
#  FINAL TARGET RULES
#  

$(APP): $(BUILD_DIR)/testDriver.o $(STATIC_OBJ_CPP)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(DYNAMIC_SO_LIB_C): $(BUILD_DIR)/interfaceC.o $(STATIC_OBJ_CPP)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -shared -o $@ -Wl,-soname,libholdem.so $^

$(DYNAMIC_LINK_LIB_C): $(DYNAMIC_SO_LIB_C)
	cp -v $< $@
