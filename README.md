

# Setup

```sh
# from project root

# Make a file containing training data.
# You can use a comment character for notes (default '#')
vim Text.txt

# The following generates Dictionary.h and Dictionary.c
# which contain the compression dictionary. These files
# are used by the compress and decompress functions
# (see Compress.h/cpp and Decompress.h/c)
python3 -m pip install -r scripts/requirements.txt
python3 scripts/BuildDictionary.py Text.txt -d src/

# At this point you have everything you need in src/
# to integrate into your program.

# As an example, tools/CompressCli.cpp is provided.
# This program converts a given input txt file using
# a provided dictionary to create a c header of
# compressed strings (by line).
g++ \
    tools/CompressCli.cpp \
    src/Compress.cpp \
    src/Decompress.c \
    src/Dictionary.c \
    -I src/ \
    -o CompressCli

# You can also compile using cmake provided the output
# of BuildDictionary was placed in src/
# (as in the example above)
mkdir build
cd build
cmake .. && make

# either way you choose, you can run the tool with
CompressCli -i Text.txt compress
```

# Example Usage

```c
#include <Dictionary.h>
#include <Compress.h>
#include <Decompress.h>

// ...
char const* text = "This is my example text";
char compressed[255];
char decompressed[255];

// compression
printf("compressed bytes: ");
Dictionary_compress(compressed, 255, text);
for (uint8_t i = 0; i < strlen(compressed); ++i) {
    printf("%02hhx ", compressed[i]);
}
printf("\n");

// decompression
Dictionary_decompress(decompressed, 255, compressed);
printf("decompressed string: '%s'", decompressed);
```


# TODO

## Targeted

- Arduino targeted export to a single file  
    (Dictionary + decompress + compress, select what to include)
- user supplied tuples
- user supplied triples
- rename
- c compress implementation (low memory)
- python bindings
- return number of bytes written (int?)
- allow differing input length type (default size_t?)
- Cleanup CompressCli.cpp
- Performance investigation/code cleanup

## Unsure

- user supplied words?
- word support?
- add dictionary step to cmake?
