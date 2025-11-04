<p align="center">
<a href="https://git.io/typing-svg">
<img src="https://readme-typing-svg.herokuapp.com?font=JetBrains+Mono&weight=800&pause=1000&color=FFFFFF&background=0A0A25&center=true&vCenter=true&width=330&lines=chunky" alt="Typing SVG" />
</a>
<br/>
<img src="https://img.shields.io/badge/LANGUAGE-C-88C0D0?style=for-the-badge&labelColor=0A0A25&color=88C0D0"/>
<img src="https://img.shields.io/badge/LIBRARY-NCURSES-88C0D0?style=for-the-badge&labelColor=0A0A25&color=88C0D0"/>
<img src="https://img.shields.io/badge/LICENSE-MIT-88C0D0?style=for-the-badge&labelColor=0A0A25&color=88C0D0"/>
</p>

---

> [!NOTE]
> **This project has nothing to do with [Chunky (the rendering tool)](https://github.com/chunky-dev/chunky)**

---

## Overview

**Chunky** is a terminal-based file viewer designed for navigating massive log files without freezing your system.  
It breaks files into manageable chunks and provides an interactive interface for efficient browsing, searching, and navigation.

---

## Why Chunky?

| Problem | Solution |
| ------- | -------- |
| Opening giant log files crashes your editor | Chunky loads files in chunks, keeping memory usage minimal |
| Can't navigate large outputs effectively | Interactive navigation with search, jump-to-line, and regex support |
| Need to process stdin streams | Full stdin support with piping |

---

## Features

- **📦 Chunked Loading** — Process files in configurable line chunks (default: 1000 lines)
- **🔍 Regex Search** — Find patterns across your entire file
- **⚡ Lightweight** — Minimal CPU and memory footprint
- **⌨️ Vi-style Navigation** — Familiar keybindings for power users
- **📊 Stdin Support** — Works seamlessly with pipes and command output

---

## Structure
```
.
├── src/           # Source code
├── Makefile       # Build configuration
└── README.md      # This file
```

---

## Installation

### Dependencies

Install **ncurses** for your platform:
```bash
# Arch Linux
sudo pacman -S ncurses

# Ubuntu/Debian
sudo apt install libncurses-dev

# Fedora
sudo dnf install ncurses-devel

# macOS
brew install ncurses
```

### Build & Install
```bash
git clone https://github.com/auth-xyz/chunky.git
cd chunky/
make && sudo make install
```

---

## Usage
```
Usage: chunky [OPTIONS]

Options:
  -f, --file FILE       Read from file instead of stdin
  -c, --chunk-size N    Lines per chunk (default: 1000)
  -h, --help            Show this help message

Controls:
  q          Quit
  ← →        Previous/Next chunk
  ↑ ↓ j k    Scroll up/down
  PgUp/PgDn  Page up/down
  Space/b    Page down/up
  /          Search (regex)
  n          Find next match
  ?          Jump to chunk number
  g          Go to line number
```

---

## Examples

**View a large file:**
```bash
chunky --file /var/log/syslog
chunky -f application.log -c 500
```

**Pipe command output:**
```bash
cat largefile.txt | chunky
journalctl | chunky
docker logs container_name | chunky
```

**Search within a log:**
```bash
chunky -f debug.log
# Press '/' and enter your regex pattern
# Press 'n' to jump to next match
```

---

## Contributing

Found a bug? Have a feature request? Want to improve the code?

- **Discord:** `@actually.auth`
- **Issues:** [Open an issue](https://github.com/auth-xyz/chunky/issues)
- **Pull Requests:** Always welcome

---

## License

Licensed under the **MIT License**.  
You are free to use, modify, and redistribute this software.

---

<p align="center">
<sub>Built with ☕ for dealing with massive log files</sub>
</p>
