#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <algorithm>
#include "Compress.h"
#include "Dictionary.h"

#define MODE_UNDEFINED 0
#define MODE_COMPRESS 1
#define MODE_DECOMPRESS 2
#define LINE_MAX_CHARS 255

char const* HELP =
"dictionary [opts] <mode> [file]\n"
"  compress or decompress strings using dictionary compression\n"
"\n"
"Opts:\n"
"  -h      show this message\n"
"  -n      variable name to use\n"
"            default: 'String'\n"
"  -p      store in PROGMEM\n"
"  -c      comment string, ignore lines starting w/ this\n"
"            default: \"#\"\n"
"  -i      input file path\n"
"            default: stdin\n"
"  -o      output file path\n"
"            default: stdout\n"
"\n"
"Positional:\n"
"  mode    one of: 'compress', 'decompress'\n"
"            required\n"
"";

int main(int argc, char** argv) {
    int ch;
    std::istream* input = &std::cin;
    std::ifstream infile;
    char const* ifname = "stdin";

    std::ostream* output = &std::cout;
    std::ofstream outfile;
    char const* ofname = "stout";

    int mode = MODE_UNDEFINED;
    char const* varName = "String";
    bool progmem = false;
    char const* commentStr = "#";

    char buffBuild[LINE_MAX_CHARS];

    while ((ch = getopt(argc, argv, "hn:pc:i:o:")) != -1) {
        switch (ch) {
            case 'h':
                printf("%s\n", HELP);
                return 0;
            case 'n':
                varName = optarg;
                break;
            case 'p':
                progmem = true;
                break;
            case 'c':
                commentStr = optarg;
                break;
            case 'o':
                ofname = optarg;
                outfile = std::ofstream(ofname, std::ios::out);
                output = &outfile;
                if (output->fail()) {
                    fprintf(stderr, "[Error] Failed to open '%s'\n", ofname);
                    return -1;
                }
                break;
            case 'i':
                ifname = optarg;
                infile = std::ifstream(ifname, std::ios::in);
                input = &infile;
                if (input->fail()) {
                    fprintf(stderr, "[Error] Failed to open '%s'\n", ifname);
                    return -1;
                }
                break;
            default:
                fprintf(stderr, "invalid option / missing param. see help\n");
                return -1;
        }
    }

    if ((argc - optind) == 0) {
        fprintf(stderr, "[Error] Missing argument <mode>\n");
        return -1;
    } else {
        if (strcmp(argv[optind], "compress") == 0) {
            mode = MODE_COMPRESS;
        } else if (strcmp(argv[optind], "decompress") == 0) {
            mode = MODE_DECOMPRESS;
        } else {
            fprintf(stderr, "[Error] Unknown mode '%s'\n", argv[optind]);
        }
    }
    optind += 1;

    if (mode == MODE_COMPRESS) {
        unsigned lineNo = 0;
        unsigned count = 0;
        size_t totalBytes = 0;
        size_t compressedBytes = 0;

        while (true) {
            lineNo += 1;
            std::string line;
            std::getline(*input, line);

            if (input->eof()) {
                // done
                break;
            }

            if (input->fail()) {
                fprintf(stderr,
                    "[Error] %s:%u - File I/O error\n",
                    ifname, lineNo);
                return -1;
            }

            if (line.size() > LINE_MAX_CHARS) {
                fprintf(stderr,
                    "[Warning]: %s:%u - line > %u characters\n",
                    ifname, lineNo, LINE_MAX_CHARS);
            }

            if (line.empty() || std::all_of(line.begin(), line.end(), isspace)) {
                // empty line
                continue;
            }

            if (line.find(commentStr) == 0) {
                // comment
                continue;
            }

            Dictionary_compress(
                buffBuild, LINE_MAX_CHARS, line.c_str());

            unsigned len = strlen(buffBuild);
            float ratio = 100.0 - (((float) len / (float) line.size()) * 100.0);

            totalBytes += line.size() + 1;
            compressedBytes += len + 1;

            *output
                << "// '" << line.c_str() << "'\n"
                << "// Compressed " << std::setprecision(2) << ratio << "%,"
                    << " " << line.size() << " -> " << len << " characters\n"
                << "const char " << varName << "_" << count << "[] "
                    << (progmem ? "PROGMEM " : "") << "= { ";

            for (size_t i = 0; i <= len; ++i) {
                // be sure to include the \0 terminator in output
                if (i > 0) {
                    *output << ", ";
                }
                *output << std::hex << std::setw(2) << "0x" << (buffBuild[i] & 0x000000ff);
            }
            *output << std::setw(0) << std::dec << " };\n";

            count += 1;
        }
        *output << "\n";

        float ratio = 100.0 - (((float) compressedBytes / (float) totalBytes) * 100.0);
        *output
            << "\n"
            << "// total compression: " << ratio << "%"
            << ", " << totalBytes << " -> " << compressedBytes << " bytes\n";

        compressedBytes += DICTIONARY_BYTES;
        ratio = 100.0 - (((float) compressedBytes / (float) totalBytes) * 100.0);
        *output
            << "// w/ dictionary: " << ratio << "%"
            << ", " << totalBytes << " -> " << compressedBytes << " bytes\n";

        *output
            << "\n"
            << "const char* const " << varName << "s[] "
                << (progmem ? "PROGMEM " : "") << "= {\n";
        for (unsigned i = 0; i < count; ++i) {
            if (i != 0) {
                *output << ",\n";
            }
            *output << "    " << varName << "_" << i;
        }
        *output
            << "\n"
            << "};\n"
            << "\n"
            << "constexpr uint8_t " << varName << "s_COUNT = sizeof("
                << varName << "s) / sizeof(" << varName << "s[0]);\n";

    } else if (mode == MODE_DECOMPRESS) {

    }

    return 0;
}