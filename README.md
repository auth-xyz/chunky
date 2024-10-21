-----

<p align="center">
	<img width="200" src="https://github.com/auth-xyz/assets/blob/main/logos/chunky.png?raw=true" alt="chunky logo">
</p>

-----

### chunky
**NOTE: THIS PROJECT HAS NOTHING TO DO WITH [Chunky](https://github.com/chunky-dev/chunky)**

**chunky** is a tool I wrote to assist a friend on an issue he was having, we both needed a way to read giant log files in a way that wouldn't lag the whole computer or crash it for that case. So I wrote chunky.


-----

- `What is chunky?`:
	chunky is, in basic terms, a `cat`, a way to output a file in the console, but it divides the files in chunks and outputs those chunks for the user, decreasing the load to the CPU and the Memory.

- `How is it useful?`:
	It depends on your usecase, if you're just trying to output files, `chunky` isn't your tool. It's made to be used with large files, so it's pretty much useless with small files.

- `What are chunks?`:
    Chunks, as its definition states, is a "substatial amount" of what? Lines. The default is `1000` lines per chunk, but you can change that with the `--size` argument as shown in the usage.

- `Why?`:
    why not?

----

### Setting up

The basic setup, is first installing `ncurses` on your machine, I'll let you do the searching for that.


```bash
$ git clone https://github.com/auth-xyz/chunky
$ cd chunky/
$ make && make install 
```

----

### Usage 

So, for moving in the chunky interface:tm:, you use the arrow keys in your keyboard. Up and Down for, you guessed it, moving up and down, while the left and right keys are for changing chunks. (PageUp and PageDown also work.)
For using chunky itself, heres how:


```bash
chunky --file path/to/file # default chunk size is 1000 lines
chunky -f file.txt -s / --size 100 # changes the chunk size to 100 

Flags:
  --help, -h    : Display this help message (default: false)

Arguments:
  --size, -s    : chunk size (optional)
  --file, -f    : file (required)

```

----

### Contributing

If you think my code is shit, it's because it most likely is, and if you have any advice for me on how to improve my code, please reach out on discord <actually.auth>, or create an issue on this repo.



[logo]: https://github.com/auth-xyz/assets/blob/main/logos/chunky.png?raw=true
