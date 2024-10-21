CXX = g++
CXXFLAGS = -std=c++20 -Wall -lncurses

ROOT_DIR = .
SRC_DIR = $(ROOT_DIR)/src
HEADERS = ./include
BUILD = dist/compiled
DIST = dist/build

EXEC = $(DIST)/chunky
INSTALL_PATH = /usr/bin

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD)/%.o, $(SRCS))

all: create_dirs $(EXEC)

create_dirs:
	mkdir -p $(BUILD) && mkdir -p $(DIST)

clean:
	rm -rf dist/

$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(EXEC) -lncurses

$(BUILD)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -I$(HEADERS) -c $< -o $@

check:
	$(CXX) $(CXXFLAGS) -I$(HEADERS) $(SRCS) -o /dev/null -lncurses
	@echo "Check complete: No errors found."

dist: create_dirs
	tar -czvf $(DIST)/linux.tar.gz $(SRC_DIR) $(HEADERS) Makefile

distcheck: dist
	@mkdir -p /tmp/nrpm_distcheck
	@tar -xzf $(DIST)/linux.tar.gz -C /tmp/nrpm_distcheck
	@cd /tmp/nrpm_distcheck && $(MAKE) && $(MAKE) check
	@rm -rf /tmp/nrpm_distcheck
	@echo "Distcheck complete: Distribution builds and passes check successfully."

install: $(EXEC)
	install -m 755 $(EXEC) $(INSTALL_PATH)

uninstall:
	rm -f $(INSTALL_PATH)/chunky

.PHONY: all create_dirs clean check dist distcheck install uninstall

