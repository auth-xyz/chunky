#ifndef CHUNKLIB_HPP
#define CHUNKLIB_HPP

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

class ChunkLib {
public:
    // Constructor that accepts the file path
    ChunkLib(const std::string& filePath) : filePath(filePath) {
        fileStream.open(filePath);

        if (!fileStream.is_open()) {
            throw std::runtime_error("Error: Could not open file " + filePath);
        }
    }

    // Destructor for cleanup
    ~ChunkLib() {
        cleanup();
    }

    // Overload 1: Process file with chunk size as a size_t
    void processFile(size_t chunkSize) {
        processFileInternal(chunkSize);
    }

    // Overload 2: Process file with chunk size as a string
    void processFile(const std::string& chunkSizeStr) {
        size_t chunkSize = convertToSizeT(chunkSizeStr);
        processFileInternal(chunkSize);
    }

private:
    std::ifstream fileStream;  // Stream for reading the file
    std::string filePath;      // Store the file path

    // Common internal function for processing the file in chunks
    void processFileInternal(size_t chunkSize) {
        while (fileStream.good()) {
            auto chunk = readChunk(chunkSize);

            if (chunk.empty()) {
                break;
            }

            outputChunk(chunk);
        }
    }

    // Helper function to read a chunk of X lines
    std::vector<std::string> readChunk(size_t chunkSize) {
        std::vector<std::string> chunk;
        std::string line;

        // Read up to chunkSize lines from the file
        for (size_t i = 0; i < chunkSize && std::getline(fileStream, line); ++i) {
            chunk.push_back(line);
        }

        return chunk;
    }

    // Output or process the chunk (for now, just printing it to console)
    void outputChunk(const std::vector<std::string>& chunk) {
        for (const auto& line : chunk) {
            std::cout << line << std::endl;
        }
    }

    // Cleanup function to close the file and release resources
    void cleanup() {
        if (fileStream.is_open()) {
            fileStream.close();
        }
    }

    // Helper function to convert a string to size_t
    size_t convertToSizeT(const std::string& str) {
        try {
            size_t chunkSize = std::stoull(str);  // Convert string to size_t
            return chunkSize;
        } catch (const std::invalid_argument& e) {
            throw std::runtime_error("Error: Invalid chunk size string '" + str + "'");
        } catch (const std::out_of_range& e) {
            throw std::runtime_error("Error: Chunk size value out of range");
        }
    }
};

#endif // CHUNKLIB_HPP

