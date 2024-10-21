CXX = g++
CXXFLAGS = -std=c++20 -Wall -lcurl

ROOT_DIR = .
LIBS_DIR = $(ROOT_DIR)/libs
SRC_DIR = $(ROOT_DIR)/src
HEADERS = ./include
BUILD = dist/compiled
DIST = dist/build

EXEC = $(DIST)/chunky

LIBS := $(wildcard $(LIBS_DIR)/*.cpp)
SRCS := $(wildcard $(SRC_DIR)/*.cpp)

OBJS := $(patsubst $(LIBS_DIR)/%.cpp, $(BUILD)/%.o, $(LIBS)) \
        $(patsubst $(SRC_DIR)/%.cpp, $(BUILD)/%.o, $(SRCS))

all: create_dirs $(EXEC)

create_dirs:
	mkdir -p $(BUILD) && mkdir -p $(DIST)

clean:
	rm -rf dist/

$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(EXEC) -lcurl 

$(BUILD)/%.o: $(LIBS_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -I$(HEADERS) -c $< -o $@

$(BUILD)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -I$(HEADERS) -c $< -o $@

check:
	$(CXX) $(CXXFLAGS) -I$(HEADERS) $(SRCS) $(LIBS) -o /dev/null -lcurl
	@echo "Check complete: No errors found."

dist: create_dirs
	tar -czvf $(DIST)/linux.tar.gz $(SRC_DIR) $(HEADERS) $(LIBS_DIR) Makefile

distcheck: dist
	@mkdir -p /tmp/nrpm_distcheck
	@tar -xzf $(DIST)/linux.tar.gz -C /tmp/nrpm_distcheck
	@cd /tmp/nrpm_distcheck && $(MAKE) && $(MAKE) check
	@rm -rf /tmp/nrpm_distcheck
	@echo "Distcheck complete: Distribution builds and passes check successfully."
