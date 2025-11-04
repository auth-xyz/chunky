CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -O2
LDFLAGS = -lncurses

SRCS = $(wildcard src/*.cpp)
OBJS = $(SRCS:src/%.cpp=dist/compiled/%.o)
EXEC = dist/build/chunky

.PHONY: all clean install uninstall

all: $(EXEC)

$(EXEC): $(OBJS) | dist/build
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

dist/compiled/%.o: src/%.cpp | dist/compiled
	$(CXX) $(CXXFLAGS) -Iinclude -c $< -o $@

dist/compiled dist/build:
	@mkdir -p $@

clean:
	@rm -rf dist/

install: $(EXEC)
	@install -m 755 $(EXEC) /usr/bin/chunky

uninstall:
	@rm -f /usr/bin/chunky
