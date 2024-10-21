#include "../include/clicky.hpp"
#include "../include/chunklib.hpp"

#include <string>

int main(int argc, char* argv[])
{
  clicky cli;

  size_t size = 1000;

  cli.add_argument("file", "f", true, "file");
  cli.add_argument("size", "s", false, "chunk size");
  cli.add_flag("start", "S", "the output will start on the first chunk");
  cli.add_flag("middle", "M", "the output will start on the middle chunk");
  cli.add_flag("last", "L", "the output will start on the last chunk");
  cli.parse(argc,argv);

  try {
    if(!cli.argument("file").empty())
    {
      ChunkLib chunky(cli.argument("file"));
      chunky.processFile(size);
      

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
