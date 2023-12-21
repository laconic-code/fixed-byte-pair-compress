#include <stdio.h>
#include <string.h>
#include <Dictionary.h>
#include <Compress.h>
#include <Decompress.h>

int main(int argc, char* argv[]) {
    char const* text;
    char buff1[255];
    char buff2[255];

    if (argc != 2) {
        printf("Example <string>\n", argv[0]);
        return 0;
    }

    text = argv[1];
    printf("input string: '%s'\n", text);

    // compress
    printf("compressed bytes: ");
    Dictionary_compress(buff1, 255, text);
    for (uint8_t i = 0; i <= strlen(buff1); ++i) {
        // be sure to include the \0 terminator in output
        printf("%02hhx ", buff1[i]);
    }
    printf("\n");

    float textLen = strlen(text) + 1;
    float compLen = strlen(buff1) + 1;
    float ratio = 100.0 - ((compLen / textLen) * 100.0);
    printf("compressed by %0.2f%%\n", ratio);

    // reverse compression
    memset(buff2, 0, 255);
    Dictionary_decompress(buff2, 255, buff1);
    printf("decompressed string: '%s'\n", buff2);

    return 0;
}