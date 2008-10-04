

CXX = g++
CXXFLAGS += -O -g -Wall

BUILD_DIR = objs/
SRC_DIR = src/

HEADERS_SRC := $(wildcard $(SRC_DIR)*.h) $(wildcard $(SRC_DIR)*.hpp)
BUILD_SRC := $(wildcard $(SRC_DIR)*.cpp)

BUILD_OBJS = $(patsubst $(SRC_DIR)%.cpp,$(BUILD_DIR)%.o,$(BUILD_SRC))
BUILT_OBJS := $(wildcard $(BUILD_DIR)*.o)
BUILD_DEPS = $(patsubst $(BUILD_DIR)%.o,$(BUILD_DIR)%.d,$(BUILT_OBJS))

BUILD_EXE = holdem
RELEASE_DIR = bin
	
all:
	make tags
	echo "Building project"
	make $(RELEASE_DIR)/$(BUILD_EXE)

test:
	echo $(BUILD_OBJS)

clean:
	rm $(BUILD_OBJS)
	rm $(BUILD_DIR)/*.d
	#rm $(BUILD_EXE)

depends:
	make $(BUILD_DEPS)


-include $(BUILD_DEPS)

tags: $(BUILD_SRC) $(HEADERS_SRC)
	ctags $^


$(BUILD_DIR)%.d: $(SRC_DIR)%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -MM -MF $@ -MT $(BUILD_DIR)$*.o $<

$(RELEASE_DIR)/$(BUILD_EXE): $(BUILD_OBJS)
	@mkdir -p $(RELEASE_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^
	
	
$(BUILD_DIR)%.o: $(SRC_DIR)%.cpp
	@mkdir -p $(BUILD_DIR)
	@echo
	@echo "Rebuilding $@ [$?]:"
	$(CXX) $(CXXFLAGS) -c -o $@ $< 




