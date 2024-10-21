#ifndef CHUNKLIB_HPP
#define CHUNKLIB_HPP

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <ncurses.h>
#include <regex>

class ChunkLib {
public:
  ChunkLib(const std::string& filePath, size_t pageScrollAmount = 10)
  : filePath(filePath), chunkCounter(0), currentLineOffset(0), pageScrollAmount(pageScrollAmount), lastSearchPattern(""), lastFoundChunkIndex(0), lastFoundLineOffset(0) {
    fileStream.open(filePath);
    if (!fileStream.is_open()) {
      throw std::runtime_error("Error: Could not open file " + filePath);
    }
    initscr();
    start_color();
    init_pair(1, COLOR_BLACK, COLOR_YELLOW); 
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    clear();
  }

  ~ChunkLib() {
    if (fileStream.is_open()) {
      fileStream.close();
    }
    endwin();
  }

  void processFile(size_t chunkSize) {
    processFileInternal(chunkSize);
  }

  void processFile(const std::string& chunkSizeStr) {
    processFileInternal(convertToSizeT(chunkSizeStr));
  }

private:
  std::ifstream fileStream;
  std::string filePath;
  size_t chunkCounter;
  size_t currentLineOffset;
  size_t pageScrollAmount;
  std::string lastSearchPattern;
  size_t lastFoundChunkIndex;
  size_t lastFoundLineOffset;
  std::vector<std::vector<std::string>> chunks;

  void processFileInternal(size_t chunkSize) {
    while (fileStream.good()) {
      auto chunk = readChunk(chunkSize);
      if (chunk.empty()) break;
      chunks.push_back(std::move(chunk));
      ++chunkCounter;
    }
    displayChunks();
  }

  std::vector<std::string> readChunk(size_t chunkSize) {
    std::vector<std::string> chunk;
    chunk.reserve(chunkSize);
    std::string line;
    for (size_t i = 0; i < chunkSize && std::getline(fileStream, line); ++i) {
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
      clear();
      if (chunkIndex < chunks.size()) {
        displayChunk(chunkIndex, maxY, maxX);
      }
      refresh();
      ch = getch();

      if (ch == KEY_LEFT && chunkIndex > 0) {
        chunkIndex--;
        currentLineOffset = 0;
      } else if (ch == KEY_RIGHT && chunkIndex < chunks.size() - 1) {
        chunkIndex++;
        currentLineOffset = 0;
      } else if (ch == KEY_UP && currentLineOffset > 0) {
        currentLineOffset--;
      } else if (ch == KEY_DOWN && currentLineOffset + maxY - 2 < chunks[chunkIndex].size()) {
        currentLineOffset++;
      } else if (ch == KEY_PPAGE && currentLineOffset >= pageScrollAmount) {
        currentLineOffset -= pageScrollAmount;
      } else if (ch == KEY_NPAGE && currentLineOffset + maxY + pageScrollAmount - 2 < chunks[chunkIndex].size()) {
        currentLineOffset += pageScrollAmount;
      } else if (ch == '/') {
        searchRegex(chunkIndex, maxY, maxX);
      } else if (ch == 'n') {
        findNextRegex(chunkIndex, maxY, maxX);
      } else if (ch == 'q') {
        break;
      }
    }
  }

  void displayChunk(size_t chunkIndex, int maxY, int maxX) {
    mvprintw(0, 0, "--- Start of Chunk %lu ---", chunkIndex + 1);

    size_t chunkSize = chunks[chunkIndex].size();
    size_t linesToDisplay = std::min(static_cast<size_t>(maxY - 2), chunkSize - currentLineOffset);

    for (size_t i = 0; i < linesToDisplay; ++i) {
      const std::string& line = chunks[chunkIndex][currentLineOffset + i];
      if (!lastSearchPattern.empty() && std::regex_search(line, std::regex(lastSearchPattern))) {
        highlightSearch(line, lastSearchPattern, i + 1);
      } else {
        mvprintw(i + 1, 0, "%s", line.c_str());
      }
    }

    mvprintw(maxY - 1, 0, "--- End of Chunk %lu (Press 'q' to quit) ---", chunkIndex + 1);
  }

  size_t convertToSizeT(const std::string& str) {
    size_t chunkSize;
    try {
      chunkSize = std::stoull(str);
    } catch (...) {
      throw std::runtime_error("Error: Invalid chunk size");
    }
    return chunkSize;
  }

  void searchRegex(size_t& chunkIndex, int maxY, int maxX) {
    echo(); 
    char searchPattern[256];
    mvprintw(maxY - 1, 0, "Enter regex pattern: ");
    getstr(searchPattern); 
    noecho(); 

    lastSearchPattern = searchPattern;
    lastFoundChunkIndex = chunkIndex;
    lastFoundLineOffset = currentLineOffset;

    findNextRegex(chunkIndex, maxY, maxX);
  }

  void findNextRegex(size_t& chunkIndex, int maxY, int maxX) {
    if (lastSearchPattern.empty()) {
      mvprintw(maxY - 1, 0, "No previous search pattern. Press any key to continue.");
      getch();
      return;
    }

    std::regex pattern(lastSearchPattern);
    bool found = false;

    for (size_t i = lastFoundChunkIndex; i < chunks.size() && !found; ++i) {
      for (size_t j = (i == lastFoundChunkIndex) ? lastFoundLineOffset + 1 : 0; j < chunks[i].size(); ++j) {
        if (std::regex_search(chunks[i][j], pattern)) {
          chunkIndex = i;
          currentLineOffset = j;
          lastFoundChunkIndex = i;
          lastFoundLineOffset = j;
          found = true;
          break;
        }
      }
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


      mvprintw(y, x, "%s", remainingLine.substr(0, startPos).c_str());
      x += startPos;


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

