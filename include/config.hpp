#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <fstream>
#include <map>
#include <string>
#include <sstream>
#include <sys/stat.h>
#include <pwd.h>
#include <unistd.h>

struct KeyBindings {
    int quit = 'q';
    int nextChunk = KEY_RIGHT;
    int prevChunk = KEY_LEFT;
    int scrollDown = KEY_DOWN;
    int scrollUp = KEY_UP;
    int scrollDownAlt = 'j';
    int scrollUpAlt = 'k';
    int pageDown = KEY_NPAGE;
    int pageUp = KEY_PPAGE;
    int pageDownAlt = ' ';
    int pageUpAlt = 'b';
    int search = '/';
    int findNext = 'n';
    int jumpToChunk = '?';
    int gotoLine = 'g';
};

struct ColorScheme {
    bool enableIndentColors = true;
    int baseColorPair = 2;      // Color pair for base text (depth 0)
    int indent1ColorPair = 3;   // Color pair for 1st indent level
    int indent2ColorPair = 4;   // Color pair for 2nd indent level
    int indent3ColorPair = 5;   // Color pair for 3rd+ indent levels
    int highlightColorPair = 1; // Search highlight
};

class Config {
private:
    std::string configPath;
    std::map<std::string, std::string> settings;

    std::string getConfigPath() {
        const char* home = getenv("HOME");
        if (!home) {
            struct passwd* pw = getpwuid(getuid());
            home = pw->pw_dir;
        }
        return std::string(home) + "/.config/chunky/config";
    }

    void ensureConfigDir() {
        const char* home = getenv("HOME");
        if (!home) {
            struct passwd* pw = getpwuid(getuid());
            home = pw->pw_dir;
        }
        std::string configDir = std::string(home) + "/.config/chunky";
        mkdir(configDir.c_str(), 0755);
    }

    void createDefaultConfig() {
        ensureConfigDir();
        std::ofstream file(configPath);
        if (file.is_open()) {
            file << "# Chunky Viewer Configuration\n\n"
                 << "# Display Settings\n"
                 << "wrap_lines=false\n"
                 << "allow_scrolling=true\n"
                 << "enable_indent_colors=true\n\n"
                 << "# Key Bindings (use KEY_* for special keys or single character)\n"
                 << "# Available special keys: KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT,\n"
                 << "#                        KEY_PPAGE (Page Up), KEY_NPAGE (Page Down)\n"
                 << "key_quit=q\n"
                 << "key_next_chunk=KEY_RIGHT\n"
                 << "key_prev_chunk=KEY_LEFT\n"
                 << "key_scroll_down=KEY_DOWN\n"
                 << "key_scroll_up=KEY_UP\n"
                 << "key_scroll_down_alt=j\n"
                 << "key_scroll_up_alt=k\n"
                 << "key_page_down=KEY_NPAGE\n"
                 << "key_page_up=KEY_PPAGE\n"
                 << "key_page_down_alt= \n"  // space
                 << "key_page_up_alt=b\n"
                 << "key_search=/\n"
                 << "key_find_next=n\n"
                 << "key_jump_chunk=?\n"
                 << "key_goto_line=g\n";
            file.close();
        }
    }

    void loadConfig() {
        std::ifstream file(configPath);
        if (!file.is_open()) {
            createDefaultConfig();
            file.open(configPath);
        }

        std::string line;
        while (std::getline(file, line)) {
            // Skip comments and empty lines
            if (line.empty() || line[0] == '#') continue;

            size_t pos = line.find('=');
            if (pos != std::string::npos) {
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);
                
                // Trim whitespace
                key.erase(0, key.find_first_not_of(" \t"));
                key.erase(key.find_last_not_of(" \t") + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);
                
                settings[key] = value;
            }
        }
    }

    int parseKey(const std::string& value) {
        if (value.empty()) return 0;
        
        // Special keys
        if (value == "KEY_UP") return KEY_UP;
        if (value == "KEY_DOWN") return KEY_DOWN;
        if (value == "KEY_LEFT") return KEY_LEFT;
        if (value == "KEY_RIGHT") return KEY_RIGHT;
        if (value == "KEY_PPAGE") return KEY_PPAGE;
        if (value == "KEY_NPAGE") return KEY_NPAGE;
        
        // Single character
        return value[0];
    }

    bool parseBool(const std::string& value) {
        return value == "true" || value == "1" || value == "yes";
    }

public:
    bool wrapLines;
    bool allowScrolling;
    KeyBindings keys;
    ColorScheme colors;

    Config() : configPath(getConfigPath()) {
        loadConfig();

        // Load settings
        wrapLines = parseBool(getSetting("wrap_lines", "false"));
        allowScrolling = parseBool(getSetting("allow_scrolling", "true"));
        colors.enableIndentColors = parseBool(getSetting("enable_indent_colors", "true"));

        // Load key bindings
        keys.quit = parseKey(getSetting("key_quit", "q"));
        keys.nextChunk = parseKey(getSetting("key_next_chunk", "KEY_RIGHT"));
        keys.prevChunk = parseKey(getSetting("key_prev_chunk", "KEY_LEFT"));
        keys.scrollDown = parseKey(getSetting("key_scroll_down", "KEY_DOWN"));
        keys.scrollUp = parseKey(getSetting("key_scroll_up", "KEY_UP"));
        keys.scrollDownAlt = parseKey(getSetting("key_scroll_down_alt", "j"));
        keys.scrollUpAlt = parseKey(getSetting("key_scroll_up_alt", "k"));
        keys.pageDown = parseKey(getSetting("key_page_down", "KEY_NPAGE"));
        keys.pageUp = parseKey(getSetting("key_page_up", "KEY_PPAGE"));
        keys.pageDownAlt = parseKey(getSetting("key_page_down_alt", " "));
        keys.pageUpAlt = parseKey(getSetting("key_page_up_alt", "b"));
        keys.search = parseKey(getSetting("key_search", "/"));
        keys.findNext = parseKey(getSetting("key_find_next", "n"));
        keys.jumpToChunk = parseKey(getSetting("key_jump_chunk", "?"));
        keys.gotoLine = parseKey(getSetting("key_goto_line", "g"));
    }

    std::string getSetting(const std::string& key, const std::string& defaultValue = "") {
        auto it = settings.find(key);
        return (it != settings.end()) ? it->second : defaultValue;
    }
};

#endif // CONFIG_HPP
