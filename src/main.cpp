#include "../include/chunky.hpp"
#include <getopt.h>
#include <unistd.h>

void printUsage(const char* progName) {
    std::cerr << "Usage: " << progName << " [OPTIONS]\n"
              << "\nOptions:\n"
              << "  -f, --file FILE       Read from file instead of stdin\n"
              << "  -c, --chunk-size N    Lines per chunk (default: 1000)\n"
              << "  -h, --help            Show this help message\n"
              << "\nControls:\n"
              << "  q          Quit\n"
              << "  ← →        Previous/Next chunk\n"
              << "  ↑ ↓ j k    Scroll up/down\n"
              << "  PgUp/PgDn  Page up/down\n"
              << "  Space/b    Page down/up\n"
              << "  /          Search (regex)\n"
              << "  n          Find next match\n"
              << "  ?          Jump to chunk number\n"
              << "  g          Go to line number\n"
              << "\nExamples:\n"
              << "  cat largefile.txt | " << progName << "\n"
              << "  " << progName << " --file largefile.txt\n"
              << "  " << progName << " -f largefile.txt -c 500\n";
}

int main(int argc, char* argv[]) {
    std::string filename;
    size_t chunkSize = 1000;

    static struct option longOptions[] = {
        {"file", required_argument, 0, 'f'},
        {"chunk-size", required_argument, 0, 'c'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "f:c:h", longOptions, nullptr)) != -1) {
        switch (opt) {
            case 'f':
                filename = optarg;
                break;
            case 'c':
                try {
                    chunkSize = std::stoull(optarg);
                    if (chunkSize == 0) {
                        std::cerr << "Error: Chunk size must be greater than 0\n";
                        return 1;
                    }
                } catch (...) {
                    std::cerr << "Error: Invalid chunk size\n";
                    return 1;
                }
                break;
            case 'h':
                printUsage(argv[0]);
                return 0;
            default:
                printUsage(argv[0]);
                return 1;
        }
    }

    try {
        std::unique_ptr<InputSource> input;

        if (!filename.empty()) {
            input = std::make_unique<FileInput>(filename);
        } else if (!isatty(STDIN_FILENO)) {
            input = std::make_unique<StdinInput>();
        } else {
            std::cerr << "Error: No input provided. Use --file or pipe input via stdin.\n";
            printUsage(argv[0]);
            return 1;
        }

        ChunkyViewer viewer(std::move(input), chunkSize);
        viewer.run();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
