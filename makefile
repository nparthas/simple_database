CXX ?= g++

# path #
SRC_PATH = src
BUILD_PATH = build
BIN_PATH = $(BUILD_PATH)/bin
TEST_PATH = test

# executable # 
BIN_NAME = simpledb 

# extensions #
SRC_EXT = cpp
TEST_EXT = py

# code lists #
# Find all source files in the source directory, sorted by
# most recently modified
SOURCES = $(shell find $(SRC_PATH) -name '*.$(SRC_EXT)' | sort -k 1nr | cut -f2-)
# Set the object file names, with the source directory stripped
# from the path, and the build path prepended in its place
OBJECTS = $(SOURCES:$(SRC_PATH)/%.$(SRC_EXT)=$(BUILD_PATH)/%.o)
# Set the dependency files that will be used to add header dependencies
DEPS = $(OBJECTS:.o=.d)

# find the basename of all .py files in the test directory, use for testing
TEST_SOURCES = $(shell find $(TEST_PATH) -name '*.$(TEST_EXT)' -exec basename {} ';')

# flags #
COMPILE_FLAGS = -std=c++11 -W -Wall -pedantic -Wextra -Werror -g
# COMPILE_FLAGS += -Wno-pointer-arith # temporary
INCLUDES = -I include/ -I /usr/local/include -I include/project/
# Space-separated pkg-config libraries used by this project
LIBS =

.PHONY: default_target
default_target: release

.PHONY: release
release: export CXXFLAGS := $(CXXFLAGS) $(COMPILE_FLAGS)
release: dirs
	@$(MAKE) all

.PHONY: dirs
dirs:
	@echo "Creating directories"
	@mkdir -p $(dir $(OBJECTS))
	@mkdir -p $(BIN_PATH)

.PHONY: clean
clean:
	@echo "Deleting $(BIN_NAME) symlink"
	@$(RM) $(BIN_NAME)
	@echo "Deleting directories"
	@$(RM) -r $(BUILD_PATH)
	@$(RM) -r $(BIN_PATH)

# checks the executable and symlinks to the output
.PHONY: all
all: $(BIN_PATH)/$(BIN_NAME)
	@echo "Making symlink: $(BIN_NAME) -> $<"
	@$(RM) $(BIN_NAME)
	@ln -s $(BIN_PATH)/$(BIN_NAME) $(BIN_NAME)

# @echo "$(TEST_SOURCES)"
# $(shell export $PATH = pwd/$(TEST_PATH):$$PATH; python3 -m unittest -v $(TEST_SOURCES))
# export PATH = $(shell pwd)/$(TEST_PATH):$$PATH;
# export PATH = $(shell pwd)/$(TEST_PATH):$$PATH
# @echo $(shell pwd)/$(TEST_PATH)
# export PATH=$(shell pwd)/$(TEST_PATH):$$PATH; $(shell python3 -m unittest $(TEST_SOURCES))
.PHONY: test
test: release
	cd $(TEST_PATH) && python3 -m unittest $(TEST_SOURCES)

# Creation of the executable
$(BIN_PATH)/$(BIN_NAME): $(OBJECTS)
	@echo "Linking: $@"
	$(CXX) $(OBJECTS) -o $@

# Add dependency files, if they exist
-include $(DEPS)

# Source file rules
# After the first compilation they will be joined with the rules from the
# dependency files to provide header dependencies
$(BUILD_PATH)/%.o: $(SRC_PATH)/%.$(SRC_EXT)
	@echo "Compiling: $< -> $@"
	$(CXX) $(CXXFLAGS) $(INCLUDES) -MP -MMD -c $< -o $@

