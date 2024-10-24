#ifndef CHUNKLIB_HPP
#define CHUNKLIB_HPP

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <ncurses.h>
#include <regex>
#include <sstream>
#include <limits>

class ChunkyStatic {
public:
  ChunkyStatic(const std::string& filePath, size_t pageScrollAmount = 10)
  : filePath(filePath), pageScrollAmount(pageScrollAmount), chunkCounter(0), currentLineOffset(0) {
    fileStream.open(filePath);
    if (!fileStream.is_open()) {
      throw std::runtime_error("Error: Could not open file " + filePath);
    }
    initializeNcurses();
  }

  ~ChunkyStatic() {
    if (fileStream.is_open()) {
      fileStream.close();
    }
    endwin();
  }

  void processFile(size_t chunkSize) {
    chunkSizeLimit = chunkSize;
    displayChunks();
  }

  void processFile(const std::string& chunkSizeStr) {
    chunkSizeLimit = stringToSizeT(chunkSizeStr);
    displayChunks();
  }

private:
  std::ifstream fileStream;
  std::string filePath;
  size_t chunkCounter;
  size_t currentLineOffset;
  size_t pageScrollAmount;
  size_t chunkSizeLimit;

  std::vector<std::string> currentChunk;
  std::string lastSearchPattern;
  size_t lastFoundLineOffset = 0;

  void initializeNcurses() {
    initscr();
    start_color();
    init_pair(1, COLOR_BLACK, COLOR_YELLOW);
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    clear();
  }

  std::vector<std::string> readChunk(size_t chunkSize) {
    std::vector<std::string> chunk;
    chunk.reserve(chunkSize);
    std::string line;
    while (chunk.size() < chunkSize && std::getline(fileStream, line)) {
      chunk.push_back(std::move(line));
    }
    return chunk;
  }

  void displayChunks() {
    int ch, maxY, maxX;
    size_t chunkIndex = 0;
    currentLineOffset = 0;
    getmaxyx(stdscr, maxY, maxX);

    while (true) {
      fileStream.clear();
      fileStream.seekg(chunkIndex * chunkSizeLimit, std::ios::beg);
      currentChunk = readChunk(chunkSizeLimit);

      if (!currentChunk.empty()) {
        displayChunk(chunkIndex, maxY, maxX);
      }
      refresh();
      ch = getch();

      handleUserInput(ch, chunkIndex, maxY, maxX);
      if (ch == 'q') break;
    }
  }

  void displayChunk(size_t chunkIndex, int maxY, int maxX) {
    clear();
    mvprintw(0, 0, "--- Start of Chunk %lu ---", chunkIndex + 1);


    if (currentLineOffset >= currentChunk.size()) {
      currentLineOffset = 0;  
    }

    size_t linesToDisplay = std::min(static_cast<size_t>(maxY - 2), currentChunk.size() - currentLineOffset);

    for (size_t i = 0; i < linesToDisplay; ++i) {
      const std::string& line = currentChunk[currentLineOffset + i];
      if (!lastSearchPattern.empty() && std::regex_search(line, std::regex(lastSearchPattern))) {
        highlightSearch(line, lastSearchPattern, i + 1);
      } else {
        mvprintw(i + 1, 0, "%s", line.c_str());
      }
    }

    size_t endChunkLine = linesToDisplay + 1;
    if (endChunkLine < maxY - 1) {
      mvprintw(endChunkLine, 0, "--- End of Chunk %lu ---", chunkIndex + 1);
    }
  }

  void handleUserInput(int ch, size_t& chunkIndex, int maxY, int maxX) {
    if (ch == KEY_LEFT && chunkIndex > 0) {
      chunkIndex--;
      currentLineOffset = 0;
    } else if (ch == KEY_RIGHT && !currentChunk.empty()) {
      chunkIndex++;
      currentLineOffset = 0;
    } else if (ch == KEY_UP && currentLineOffset > 0) {
      currentLineOffset--;
    } else if (ch == KEY_DOWN && currentLineOffset + maxY - 2 < currentChunk.size()) {
      currentLineOffset++;
    } else if (ch == KEY_PPAGE && currentLineOffset >= pageScrollAmount) {
      currentLineOffset -= pageScrollAmount;
    } else if (ch == KEY_NPAGE && currentLineOffset + maxY - 2 < currentChunk.size()) {
      currentLineOffset += pageScrollAmount;
    } else if (ch == '/') {
      searchRegex(chunkIndex, maxY);
    } else if (ch == 'n') {
      findNextRegex(chunkIndex, maxY);
    } else if (ch == '?') {
      jumpToChunk(chunkIndex, maxY, maxX);
    }
  }

  void jumpToChunk(size_t& chunkIndex, int maxY, int maxX) {
    echo();
    char chunkNumberStr[10];
    mvprintw(maxY - 1, 0, "Enter chunk number: ");
    getstr(chunkNumberStr);
    noecho();

    size_t chunkNumber;
    try {
      chunkNumber = std::stoull(chunkNumberStr) - 1;  


      fileStream.clear(); 
      fileStream.seekg(0, std::ios::end); 
      size_t fileSize = fileStream.tellg(); 
      size_t totalChunks = fileSize / (chunkSizeLimit * sizeof(std::string));


      if (chunkNumber <= totalChunks) {
        chunkIndex = chunkNumber;
        currentLineOffset = 0;  
      } else {
        mvprintw(maxY - 1, 0, "Invalid chunk number. Press any key to continue.");
        getch();
      }
    } catch (...) {
      mvprintw(maxY - 1, 0, "Invalid chunk number. Press any key to continue.");
      getch();
    }
  }

  size_t stringToSizeT(const std::string& str) {
    try {
      return std::stoull(str);
    } catch (...) {
      throw std::runtime_error("Error: Invalid chunk size");
    }
  }

  void searchRegex(size_t& chunkIndex, int maxY) {
    echo();
    char searchPattern[256];
    mvprintw(maxY - 1, 0, "Enter regex pattern: ");
    getstr(searchPattern);
    noecho();

    lastSearchPattern = searchPattern;
    lastFoundLineOffset = currentLineOffset;
    findNextRegex(chunkIndex, maxY);
  }

  void findNextRegex(size_t& chunkIndex, int maxY) {
    if (lastSearchPattern.empty()) {
      mvprintw(maxY - 1, 0, "No previous search pattern. Press any key to continue.");
      getch();
      return;
    }

    std::regex pattern(lastSearchPattern);
    bool found = false;

    while (!found && fileStream.good()) {
      currentChunk = readChunk(chunkSizeLimit);
      for (size_t j = lastFoundLineOffset + 1; j < currentChunk.size(); ++j) {
        if (std::regex_search(currentChunk[j], pattern)) {
          chunkIndex++;
          currentLineOffset = j;
          found = true;
          break;
        }
      }
      lastFoundLineOffset = 0;
    }

    if (!found) {
      mvprintw(maxY - 1, 0, "Pattern not found. Press any key to continue.");
      getch();
    }
  }

  void highlightSearch(const std::string& line, const std::string& pattern, int y) {
    std::regex regexPattern(pattern);
    std::sregex_iterator iter(line.begin(), line.end(), regexPattern);
    std::sregex_iterator end;

    int x = 0;
    std::string remainingLine = line;

    while (iter != end) {
      std::smatch match = *iter;
      size_t startPos = match.position();
      size_t matchLen = match.length();

      if (startPos > remainingLine.size()) {
        break; 
      }

      mvprintw(y, x, "%s", remainingLine.substr(0, startPos).c_str());
      x += startPos;

      if (startPos + matchLen > remainingLine.size()) {
        matchLen = remainingLine.size() - startPos;
      }

      attron(COLOR_PAIR(1));
      mvprintw(y, x, "%s", remainingLine.substr(startPos, matchLen).c_str());
      attroff(COLOR_PAIR(1));
      x += matchLen;
      remainingLine = remainingLine.substr(startPos + matchLen);
      iter++;
    }

    mvprintw(y, x, "%s", remainingLine.c_str());
  }

};

#endif

