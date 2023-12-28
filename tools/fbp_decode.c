#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <Fbp.h>

#define MAX_CHARS 4096
#define FBP_STATE_NAME Fbp_Default

char const* HELP =
"encode [opts] byte_string\n"
"  decode the given bytes using the included dictionary\n"
"\n"
"Opts:\n"
"  -h          show this message\n"
"  -s          byte separator\n"
"                default: ', '\n"
"  -P          do not expect hex prefix, '0x' before each value\n"
"  -d          name of the dictionary to use for encoding\n"
"                default: 'Default'\n"
"\n"
"Positional:\n"
"  byte_string string to decompress (stdin if not given)\n"
"              accepts up to 4095 characters\n"
"";

int main(int argc, char* argv[]) {
    int ch;
    size_t i;
    bool prefix = true;
    char const* sep = ", ";
    char const* dictName = "Default";
    uint8_t src[MAX_CHARS];
    char dst[MAX_CHARS];

    while ((ch = getopt(argc, argv, "hs:Pd:")) != -1) {
        switch (ch) {
            case 'h':
                printf("%s\n", HELP);
                return 0;
            case 's':
                sep = optarg;
                break;
            case 'P':
                prefix = false;
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
        if (fgets(dst, MAX_CHARS, stdin) == 0) {
            int err = ferror(stdin);
            fprintf(stderr, "read error: %02x (%d)\n", err, err);
            return err;
        }
    } else {
        strncpy(dst, argv[optind], MAX_CHARS);
        dst[MAX_CHARS-1] = '\0';
    }

    unsigned len = strlen(dst) + 1;
    unsigned seplen = strlen(sep);
    char const* ptr = &(dst[0]);
    i = 0;
    while (ptr <= (&(dst[0]) + len)) {
        if (prefix && (sscanf("0x%02x", &src[i]) == 1)) {
            i += 1;
            ptr += 4;
        } else if (sscanf("%02x", &src[i]) == 1) {
            ptr += 2;
            i += 1;
        } else {
            fprintf(stderr, "input parse failure\n");
            return -1;
        }

        if (i == MAX_CHARS) {
            src[MAX_CHARS-1] = 0;
            break;
        } else if (strncmp(sep, ptr, seplen) != 0) {
            src[i] = 0;
            break;
        } else {
            ptr += seplen;
        }
    }

    int result = FixedBytePair_decode(FBP_STATE_NAME, dst, MAX_CHARS, src);
    if (result < 0) {
        fprintf(stderr, "decoding error: %02x (%d)\n", result, result);
        return result;
    }

    unsigned srclen = strlen(src) + 1;
    unsigned dstlen = strlen(dst) + 1;
    float ratio = 100.0 - (((float) dstlen / (float) srclen) * 100.0);

    for (i = 0)
    fprintf(stdout, )
    fprintf(stdout, "expanded by %0.2f%%, %u -> %u characters\n", ratio, srclen, dstlen);
    fprintf(stdout, "%s\n", dst);

    return 0;
}