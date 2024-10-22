#ifndef CHUNKYLIVE_HPP
#define CHUNKYLIVE_HPP

#include <ncurses.h>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>


template<typename T>
class ThreadSafeQueue {
public:
  ThreadSafeQueue() = default;
  ~ThreadSafeQueue() = default;

  void push(T value) {
    std::lock_guard<std::mutex> lock(mtx);
    queue.push(std::move(value));
    cond_var.notify_one();
  }

  std::optional<T> try_pop() {
    std::lock_guard<std::mutex> lock(mtx);
    if (queue.empty()) {
      return std::nullopt;
    }
    T value = std::move(queue.front());
    queue.pop();
    return value;
  }

  T wait_and_pop() {
    std::unique_lock<std::mutex> lock(mtx);
    cond_var.wait(lock, [this] { return !queue.empty(); });
    T value = std::move(queue.front());
    queue.pop();
    return value;
  }

  bool empty() const {
    std::lock_guard<std::mutex> lock(mtx);
    return queue.empty();
  }

private:
  mutable std::mutex mtx;
  std::queue<T> queue;
  std::condition_variable cond_var;
};


class StreamProcessor {
public:
  StreamProcessor(std::function<void(const std::string&)> process_chunk_func)
  : process_chunk(std::move(process_chunk_func)), stop_flag(false) {}


  void start_processing() {
    reader_thread = std::thread(&StreamProcessor::read_from_stdin, this);
    process_input();
  }


  void stop_processing() {
    stop_flag = true;
    if (reader_thread.joinable()) {
      reader_thread.join();
    }
  }

  ~StreamProcessor() {
    stop_processing();
  }

private:
  ThreadSafeQueue<std::string> queue;
  std::function<void(const std::string&)> process_chunk;
  std::thread reader_thread;
  std::atomic<bool> stop_flag;

  void read_from_stdin() {
    std::string line;
    while (!stop_flag && std::getline(std::cin, line)) {
      if (!line.empty()) {
        queue.push(line);
      }
    }
  }

  void process_input() {
    while (!stop_flag) {
      std::optional<std::string> chunk = queue.try_pop();
      if (chunk.has_value()) {
        process_chunk(chunk.value());
      } else {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
    }
  }
};


class ChunkyLive {
public:
  ChunkyLive(size_t pageScrollAmount = 10)
  : chunkCounter(0), currentLineOffset(0), pageScrollAmount(pageScrollAmount) {
    initscr();
    start_color();
    init_pair(1, COLOR_BLACK, COLOR_YELLOW);
    init_pair(2, COLOR_WHITE, COLOR_BLACK); 
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    clear();


    processor = new StreamProcessor([this](const std::string& chunk) {
      handle_chunk(chunk);
    });
  }

  ~ChunkyLive() {
    processor->stop_processing();
    delete processor;
    endwin();
  }


  void start() {
    processor->start_processing();
    display();
  }

private:
  size_t chunkCounter;
  size_t currentLineOffset;
  size_t pageScrollAmount;
  std::vector<std::string> displayedChunks;  
  StreamProcessor* processor;


  void handle_chunk(const std::string& chunk) {
    if (chunk.empty()) return;

    displayedChunks.push_back(chunk);  
    refresh_display();  
  }


  void display() {
    while (true) {
      int ch = getch();
      switch (ch) {
        case KEY_DOWN:
          scroll_down();
          break;
        case KEY_UP:
          scroll_up();
          break;
        case 'q':
          return;  
        default:
          break;
      }
    }
  }


  void refresh_display() {
    clear();
    size_t displayStart = currentLineOffset;
    size_t displayEnd = std::min(displayStart + pageScrollAmount, displayedChunks.size());

    for (size_t i = displayStart; i < displayEnd; ++i) {
      attron(COLOR_PAIR(2));  
      mvprintw(i - displayStart, 0, "%s", displayedChunks[i].c_str());
      attroff(COLOR_PAIR(2));
    }

    refresh();
  }


  void scroll_down() {
    if (currentLineOffset + pageScrollAmount < displayedChunks.size()) {
      currentLineOffset += pageScrollAmount;
      refresh_display();
    }
  }


  void scroll_up() {
    if (currentLineOffset > 0) {
      currentLineOffset -= pageScrollAmount;
      refresh_display();
    }
  }
};

#endif //Mango 

