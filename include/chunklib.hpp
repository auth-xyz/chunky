#ifndef CHUNKLIB_HPP
#define CHUNKLIB_HPP

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <ncurses.h>

class ChunkLib {
public:
    ChunkLib(const std::string& filePath) : filePath(filePath), chunkCounter(0), currentLineOffset(0) {
        fileStream.open(filePath);
        if (!fileStream.is_open()) {
            throw std::runtime_error("Error: Could not open file " + filePath);
        }
        initscr();
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
            mvprintw(i + 1, 0, "%s", chunks[chunkIndex][currentLineOffset + i].c_str());
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
};

#endif // CHUNKLIB_HPP

