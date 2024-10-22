#include "../include/chunkystatic.hpp"
#include "../include/chunkylive.hpp"
#include "../include/clicky.hpp"

#include <string>

int main(int argc, char* argv[])
{
  clicky cli;
  size_t chunkSize = 1000;

  cli.add_argument("file", "f", false, "file");
  cli.add_argument("size", "s", false, "chunk size");
  cli.parse(argc,argv);

  try {
    if(!cli.argument("file").empty())
    {
      ChunkyStatic chunky(cli.argument("file"));
      chunky.processFile(chunkSize);
      

      if(!cli.argument("size").empty())
      {
        chunky.processFile(cli.argument("size"));
      }
    }

  }
  catch (const std::exception& err) {
    std::cerr << err.what() << std::endl;
  }

  return 0;
}
