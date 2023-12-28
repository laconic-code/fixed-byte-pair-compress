
# Fixed Byte Pair Compression

Fixed Byte Pair compression builds a static dictionary of symbols, tuples of
symbols, and triples of symbols that occur commonly in a training ascii text
set. This dictionary is used to compress a string of text into a smaller
representation. The algorithm provided performs only a single pass in an attempt
to balance speed and complexity.

Fixed Byte Pair compression is best used for small strings which are non-ideal
for more comprehensive general purpose compression algorithms.

## Dictionary Codes

| Code | Value |
| ---- | ----- |
| 0x00 | End of compressed string signifier. chosen to match '\0'.
| 0x01 | String Literal. (max length of 255 bytes)
| 0x02 | Character Literal.
|    * | Character Single, Tuple, or Triple.

Codes 0x03-0xFF are determine using `scripts/BuildDictionary.py` and a text file
representative of typical input (or the input itself).

## Dictionary Structure

The dictionary is built as a byte array such that terminating null characters
are not necessary. This is done to further reduce the size complexity of the
dictionary at the expense of more run/compile time calculations.

The dictionary starts with single characters, then tuples, and finally triples.
The starting index and byte offset within the dictionary of each section is
determined prior to compile time. Additionally, the codes within each section
are sorted from most common to least common occurance.

```c
uint8_t Dictionary[253] =  {
    // 1 character codes
        // most common 1 character
        'a',
        // ...
        // least common 1 character
        'w',
    // 2 character codes
        // most common 2 characters
        'ab',
        // ...
        // least common 2 characters
        'we',
    // 3 character codes
        // most common 3 characters
        'abc',
        // ...
        // least common 3 characters
        'wel'
};
```

## Decoding

Decoding is fairly simple:
- string literal: read the next byte to get the number of literal bytes to copy
  then do so.
- character literal: copy the next byte.
- single/tuple/triple: determine offset in the dictionary then copy 1/2/3 bytes.

```c
if (code == 0) {
    // end of input
    *destination = '\0'

} else if (code == 1) {
    // string literal
    len = *(source + 1);
    memcpy(destination, source + 2, len);

} else if (code == 2) {
    // character literal
    destination = *(source + 1);

} elseif (code <= DICT_2C_START_IDX) {
    // dictionary entry, 1 character
    idx_delta = (code - RESERVED_CODES) - DICT_1C_START_IDX;
    offset = DICT_1C_START_OFFSET + (1 * idx_delta);
    memcpy(destination, Dictionary + offset, 1);

} else if (code <= DICT_3C_START_IDX) {
    // dictionary entry, 2 characters
    idx_delta = (code - RESERVED_CODES) - DICT_2C_START_IDX;
    offset = DICT_2C_START_OFFSET + (2 * idx_delta);
    memcpy(destination, Dictionary + offset, 2);

} else {
    // dictionary entry, 3 characters
    idx_delta = (code - RESERVED_CODES) - DICT_3C_START_IDX;
    offset = DICT_3C_START_OFFSET + (3 * idx_delta);
    memcpy(destination, Dictionary + offset, 3);
}
```

## Encoding

Encoding is more complex, especially given the low memory constraints. This
process could be performed faster if a lookup table of strings to codes was
built (removing several linear searches) but this library strives to keep
memory usage low. This choice feels justified given compression is likely to
occur less often (likely on a more powerful host computer) than decompression
(on an embedded platform).

The logic itself is involved so it will be summarized here:
1. attempt to find any match (stop on first match):
    1. try matching `[source+0, source+2]` characters to a triple, iterating
       from the first triple in the dictionary to the last triple.
    2. try matching `[source+0, source+1]` characters to a tuple, iterating from
       the first tuple in the dictionary to the last tuple.
    3. try matching `[source+0]` character to a single, iterating from the first
       single in the dictionary to the last single.
2. if a match was found
    1. if a literal is being processed, write the literal and clear literal mark
    2. write the associated match code
3. if a match was NOT found
    1. if a literal is being processed, mark this character for later
    2. else start a new literal

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
${CXX} \
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

# Examples

```txt
# Example text training data
# doubles as the data we want to compress

This is a single line of compressible text
The tools don't include newlines as compressible text

Empty lines are ignores
```

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

# Arduino Specific Instructions

This library was originally built for a game on the arduboy platform powered
by an ATmega32u4. An embedded version of the code is provided for single file,
single dictionary usage.

```sh
python3 scripts/BuildEmbedded.py SampleText.txt
mv Fbp.h Dictionary.h Encoder.c ${ArduinoProjectDir}/
```

The encoder works as:
```sh
# build encoder
cd ${ArduinoProjectDir}/
${CC} Encoder.c -o encoder -I.

# remove so arduino compiler doesn't get confused
rm Encoder.c

# encode text
./encoder "MyCompressedString" "Compress this string" >> Text.h

cat Text.h
const uint8_t* MyCompressedString PROGMEM = { 0x1a, /* ... , */ 0x00 };
```

Usage is as:
```c
#include <Fbp.h>
#include <Text.h>

// ...
void myDecode() {
    char* dst[64];
    uint8_t const* compressed = pgm_read_byte(&(MY_COMPRESSED_STRING));
    Fbp_decode(dst, 64, src);

    // do something w/ decoded string in dst
}
```

More complex use cases will use the normal library build routines but
will need to merge headers and source files into a single file and likely
write their own encoder (or use the provided python).

The flags `-D FBP_ENCODE` and `-D FBP_REGISTER` should `_NOT_` be provided
when compiling for the arduino (though will be necessary for building the
encoder python module).

# Further Reading and Alternatives

For further reading and alternatives, see:
- [Wikipedia: Byte Pair Encoding](https://en.wikipedia.org/wiki/Byte_pair_encoding)
- [Wikibooks: Dictionary Compression](https://en.wikibooks.org/wiki/Data_Compression/Dictionary_compression#non-adaptive_dictionary_compression)
- [SMAZ compression library](https://github.com/antirez/smaz)
- [shoco compression library](https://ed-von-schleck.github.io/shoco/)
