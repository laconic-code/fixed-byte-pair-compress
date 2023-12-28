#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <Fbp.h>

#define MAX_CHARS 4096
#define FBP_STATE_NAME Fbp_Default

char const* HELP =
"encode [opts] string\n"
"  encode the given string using the included dictionary\n"
"\n"
"Opts:\n"
"  -h          show this message\n"
"  -n <name>   c variable name\n"
"                default: 'String'\n"
"  -p          variable postfix (ex 'PROGMEM')\n"
"                default: ''\n"
"  -d          name of the dictionary to use for encoding\n"
"                default: 'Default'\n"
"\n"
"Positional:\n"
"  string      string to compress (stdin if not given)\n"
"              accepts up to 4095 characters\n"
"";

int main(int argc, char* argv[]) {
    int ch;
    size_t i;
    char const* varname = "String";
    char const* postfix = "";
    char const* dictName = "Default";
    uint8_t dst[MAX_CHARS];
    char src[MAX_CHARS];

    while ((ch = getopt(argc, argv, "hn:p:")) != -1) {
        switch (ch) {
            case 'h':
                printf("%s\n", HELP);
                return 0;
            case 'n':
                varname = optarg;
                break;
            case 'p':
                postfix = optarg;
                break;
            case 'd':
                dictName = optarg;
                break;
            default:
                fprintf(stderr, "invalid option / missing param. see help\n");
                return -1;
        }
    }

    if (argc - optind == 0) {
        if (fgets(src, MAX_CHARS, stdin) == 0) {
            int err = ferror(stdin);
            fprintf(stderr, "read error: %02x (%d)\n", err, err);
            return err;
        }
    } else {
        strncpy(src, argv[optind], MAX_CHARS);
        src[MAX_CHARS-1] = '\0';
    }

    int result = FixedBytePair_encode(FBP_STATE_NAME, dst, MAX_CHARS, src);
    if (result < 0) {
        fprintf(stderr, "encoding error: %02x (%d)\n", result, result);
        return result;
    }

    unsigned srclen = strlen(src) + 1;
    unsigned dstlen = strlen(dst) + 1;
    float ratio = 100.0 - (((float) dstlen / (float) srclen) * 100.0);

    fprintf(stdout, "// '%s'\n", text);
    fprintf(stdout, "// compressed by %0.2f%%, %u -> %u characters\n", ratio, srclen, dstlen);
    fprintf(stdout, "const char %s[] %s = { ", varname, postfix);
    for (i = 0; i < dstlen; ++i) {
        fprintf(stdout, "0x%02x", dst[i] & 0x000000ff);
        if (i != (dstlen - 1)) {
            fprintf(stdout, ", ");
        }
    }
    fprintf(stdout, "0x00 };\n");

    return 0;
}