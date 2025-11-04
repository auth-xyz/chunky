#ifndef CHUNKY_HPP
#define CHUNKY_HPP

#include <algorithm>
#include <cstring>
#include <deque>
#include <fstream>
#include <iostream>
#include <memory>
#include <ncurses.h>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

// Abstract input source interface
class InputSource {
public:
    virtual ~InputSource() = default;
    virtual bool getLine(std::string& line) = 0;
    virtual bool seekToLine(size_t lineNum) = 0;
    virtual bool isSeekable() const = 0;
    virtual size_t getCurrentLine() const = 0;
};

// File-based input (seekable)
class FileInput : public InputSource {
private:
    std::ifstream file;
    std::vector<std::streampos> lineIndex;
    size_t currentLine;
    bool indexBuilt;

    void buildIndex() {
        if (indexBuilt) return;
        
        file.clear();
        file.seekg(0, std::ios::beg);
        lineIndex.clear();
        lineIndex.push_back(0);
        
        std::string line;
        while (std::getline(file, line)) {
            lineIndex.push_back(file.tellg());
        }
        
        file.clear();
        indexBuilt = true;
    }

public:
    explicit FileInput(const std::string& path) : currentLine(0), indexBuilt(false) {
        file.open(path);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file: " + path);
        }
    }

    bool getLine(std::string& line) override {
        if (std::getline(file, line)) {
            currentLine++;
            return true;
        }
        return false;
    }

    bool seekToLine(size_t lineNum) override {
        if (!indexBuilt) {
            buildIndex();
        }

        if (lineNum >= lineIndex.size() - 1) {
            return false;
        }

        file.clear();
        file.seekg(lineIndex[lineNum]);
        currentLine = lineNum;
        return true;
    }

    bool isSeekable() const override { return true; }
    size_t getCurrentLine() const override { return currentLine; }
    
    size_t getTotalLines() {
        if (!indexBuilt) {
            buildIndex();
        }
        return lineIndex.empty() ? 0 : lineIndex.size() - 1;
    }
};

// Stdin input (non-seekable, caches read lines)
class StdinInput : public InputSource {
private:
    std::deque<std::string> cache;
    size_t currentLine;
    bool eof;

public:
    StdinInput() : currentLine(0), eof(false) {}

    bool getLine(std::string& line) override {
        if (currentLine < cache.size()) {
            line = cache[currentLine];
            currentLine++;
            return true;
        }

        if (eof) return false;

        if (std::getline(std::cin, line)) {
            cache.push_back(line);
            currentLine++;
            return true;
        }

        eof = true;
        return false;
    }

    bool seekToLine(size_t lineNum) override {
        // Read up to requested line if not cached
        while (lineNum >= cache.size() && !eof) {
            std::string line;
            if (std::getline(std::cin, line)) {
                cache.push_back(line);
            } else {
                eof = true;
                return false;
            }
        }

        if (lineNum < cache.size()) {
            currentLine = lineNum;
            return true;
        }

        return false;
    }

    bool isSeekable() const override { return true; }
    size_t getCurrentLine() const override { return currentLine; }
};

// Display manager
class Display {
private:
    int maxY, maxX;
    std::string searchPattern;
    
    void initNcurses() {
        initscr();
        start_color();
        init_pair(1, COLOR_BLACK, COLOR_YELLOW);
        cbreak();
        noecho();
        keypad(stdscr, TRUE);
        clear();
    }

public:
    Display() {
        initNcurses();
        getmaxyx(stdscr, maxY, maxX);
    }

    ~Display() {
        endwin();
    }

    int getMaxY() const { return maxY; }
    int getMaxX() const { return maxX; }

    void updateSize() {
        getmaxyx(stdscr, maxY, maxX);
    }

    void setSearchPattern(const std::string& pattern) {
        searchPattern = pattern;
    }

    const std::string& getSearchPattern() const {
        return searchPattern;
    }

    void clear() {
        ::clear();
    }

    void refresh() {
        ::refresh();
    }

    void drawLine(int y, const std::string& text, bool highlight = false) {
        if (y >= maxY) return;

        if (!highlight || searchPattern.empty()) {
            mvprintw(y, 0, "%s", text.c_str());
            return;
        }

        // Highlight matches
        try {
            std::regex pattern(searchPattern, std::regex::icase);
            auto searchStart = text.cbegin();
            auto searchEnd = text.cend();
            std::smatch match;
            int x = 0;

            while (std::regex_search(searchStart, searchEnd, match, pattern)) {
                // Print text before match
                std::string before(searchStart, searchStart + match.position());
                mvprintw(y, x, "%s", before.c_str());
                x += before.length();

                // Print match with highlight
                attron(COLOR_PAIR(1));
                mvprintw(y, x, "%s", match.str().c_str());
                attroff(COLOR_PAIR(1));
                x += match.length();

                searchStart += match.position() + match.length();
            }

            // Print remaining text
            std::string remaining(searchStart, searchEnd);
            mvprintw(y, x, "%s", remaining.c_str());
        } catch (const std::regex_error&) {
            mvprintw(y, 0, "%s", text.c_str());
        }
    }

    void drawStatus(const std::string& msg) {
        mvprintw(maxY - 1, 0, "%s", std::string(maxX, ' ').c_str());
        mvprintw(maxY - 1, 0, "%s", msg.c_str());
    }

    std::string prompt(const std::string& msg) {
        echo();
        mvprintw(maxY - 1, 0, "%s", std::string(maxX, ' ').c_str());
        mvprintw(maxY - 1, 0, "%s", msg.c_str());
        
        char buffer[256];
        getstr(buffer);
        noecho();
        
        return std::string(buffer);
    }

    int getKey() {
        return getch();
    }
};

// Main viewer class
class ChunkyViewer {
private:
    std::unique_ptr<InputSource> input;
    Display display;
    size_t linesPerChunk;
    size_t currentChunkStart;
    size_t scrollOffset;
    std::vector<std::string> currentChunk;

    void loadChunk(size_t startLine) {
        currentChunk.clear();
        
        if (!input->seekToLine(startLine)) {
            return;
        }

        std::string line;
        for (size_t i = 0; i < linesPerChunk && input->getLine(line); ++i) {
            currentChunk.push_back(std::move(line));
        }

        currentChunkStart = startLine;
        scrollOffset = 0;
    }

    void render() {
        display.clear();
        
        int screenLines = display.getMaxY() - 2;
        size_t displayStart = std::min(scrollOffset, currentChunk.size());
        size_t displayEnd = std::min(displayStart + screenLines, currentChunk.size());

        // Header
        std::ostringstream header;
        header << "--- Chunk starting at line " << (currentChunkStart + 1)
               << " (lines " << (currentChunkStart + displayStart + 1) 
               << "-" << (currentChunkStart + displayEnd) << ") ---";
        display.drawLine(0, header.str());

        // Content
        for (size_t i = displayStart; i < displayEnd; ++i) {
            display.drawLine(1 + (i - displayStart), currentChunk[i], true);
        }

        // Status line
        std::ostringstream status;
        status << "q:quit  ←→:chunk  ↑↓:scroll  PgUp/PgDn:page  /:search  n:next  ?:jump  g:goto";
        display.drawStatus(status.str());

        display.refresh();
    }

    void scrollDown() {
        int screenLines = display.getMaxY() - 2;
        if (scrollOffset + screenLines < currentChunk.size()) {
            scrollOffset++;
        } else if (currentChunkStart + currentChunk.size() < getTotalLinesEstimate()) {
            loadChunk(currentChunkStart + linesPerChunk);
        }
    }

    void scrollUp() {
        if (scrollOffset > 0) {
            scrollOffset--;
        } else if (currentChunkStart > 0) {
            size_t newStart = currentChunkStart >= linesPerChunk ? 
                currentChunkStart - linesPerChunk : 0;
            loadChunk(newStart);
            scrollOffset = currentChunk.size() - std::min(currentChunk.size(), 
                static_cast<size_t>(display.getMaxY() - 2));
        }
    }

    void pageDown() {
        int screenLines = display.getMaxY() - 2;
        if (scrollOffset + screenLines * 2 < currentChunk.size()) {
            scrollOffset += screenLines;
        } else {
            loadChunk(currentChunkStart + linesPerChunk);
        }
    }

    void pageUp() {
        int screenLines = display.getMaxY() - 2;
        if (scrollOffset >= static_cast<size_t>(screenLines)) {
            scrollOffset -= screenLines;
        } else if (currentChunkStart > 0) {
            size_t newStart = currentChunkStart >= linesPerChunk ? 
                currentChunkStart - linesPerChunk : 0;
            loadChunk(newStart);
        }
    }

    void nextChunk() {
        loadChunk(currentChunkStart + linesPerChunk);
    }

    void prevChunk() {
        if (currentChunkStart > 0) {
            size_t newStart = currentChunkStart >= linesPerChunk ? 
                currentChunkStart - linesPerChunk : 0;
            loadChunk(newStart);
        }
    }

    void search() {
        std::string pattern = display.prompt("Search (regex): ");
        if (!pattern.empty()) {
            display.setSearchPattern(pattern);
            findNext();
        }
    }

    void findNext() {
        if (display.getSearchPattern().empty()) {
            display.drawStatus("No search pattern set. Press any key.");
            display.refresh();
            display.getKey();
            return;
        }

        try {
            std::regex pattern(display.getSearchPattern(), std::regex::icase);
            
            // Search in current chunk from current position
            for (size_t i = scrollOffset + 1; i < currentChunk.size(); ++i) {
                if (std::regex_search(currentChunk[i], pattern)) {
                    scrollOffset = i;
                    return;
                }
            }

            // Search in subsequent chunks
            size_t searchStart = currentChunkStart + currentChunk.size();
            std::string line;
            
            input->seekToLine(searchStart);
            size_t lineOffset = 0;
            
            while (input->getLine(line)) {
                if (std::regex_search(line, pattern)) {
                    loadChunk(searchStart);
                    scrollOffset = lineOffset;
                    return;
                }
                
                lineOffset++;
                if (lineOffset >= linesPerChunk) {
                    searchStart += linesPerChunk;
                    lineOffset = 0;
                }
            }

            display.drawStatus("Pattern not found. Press any key.");
            display.refresh();
            display.getKey();
        } catch (const std::regex_error& e) {
            display.drawStatus(std::string("Regex error: ") + e.what() + ". Press any key.");
            display.refresh();
            display.getKey();
        }
    }

    void jumpToChunk() {
        std::string input = display.prompt("Jump to chunk number: ");
        try {
            size_t chunkNum = std::stoull(input);
            if (chunkNum > 0) {
                loadChunk((chunkNum - 1) * linesPerChunk);
            }
        } catch (...) {
            display.drawStatus("Invalid chunk number. Press any key.");
            display.refresh();
            display.getKey();
        }
    }

    void gotoLine() {
        std::string input = display.prompt("Go to line number: ");
        try {
            size_t lineNum = std::stoull(input);
            if (lineNum > 0) {
                size_t targetLine = lineNum - 1;
                size_t chunkStart = (targetLine / linesPerChunk) * linesPerChunk;
                loadChunk(chunkStart);
                scrollOffset = targetLine - chunkStart;
            }
        } catch (...) {
            display.drawStatus("Invalid line number. Press any key.");
            display.refresh();
            display.getKey();
        }
    }

    size_t getTotalLinesEstimate() {
        if (auto* fileInput = dynamic_cast<FileInput*>(input.get())) {
            return fileInput->getTotalLines();
        }
        return SIZE_MAX; // Unknown for stdin
    }

public:
    ChunkyViewer(std::unique_ptr<InputSource> src, size_t lines = 1000)
        : input(std::move(src)), linesPerChunk(lines), 
          currentChunkStart(0), scrollOffset(0) {}

    void run() {
        loadChunk(0);

        while (true) {
            render();
            int ch = display.getKey();

            switch (ch) {
                case 'q':
                case 'Q':
                    return;
                case KEY_RIGHT:
                    nextChunk();
                    break;
                case KEY_LEFT:
                    prevChunk();
                    break;
                case KEY_DOWN:
                case 'j':
                    scrollDown();
                    break;
                case KEY_UP:
                case 'k':
                    scrollUp();
                    break;
                case KEY_NPAGE: // Page Down
                case ' ':
                    pageDown();
                    break;
                case KEY_PPAGE: // Page Up
                case 'b':
                    pageUp();
                    break;
                case '/':
                    search();
                    break;
                case 'n':
                    findNext();
                    break;
                case '?':
                    jumpToChunk();
                    break;
                case 'g':
                    gotoLine();
                    break;
                case KEY_RESIZE:
                    display.updateSize();
                    break;
            }
        }
    }
};

#endif // CHUNKY_HPP
