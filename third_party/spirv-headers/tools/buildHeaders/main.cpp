// SPDX-FileCopyrightText: 2014-2024 The Khronos Group Inc.
// SPDX-License-Identifier: MIT
// 
// MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
// KHRONOS STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS
// SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT
//    https://www.khronos.org/registry/

//#include <fstream>
#include <string>
#include <algorithm>

#include "jsonToSpirv.h"
#include "header.h"

// Command-line options
enum TOptions {
    EOptionNone                       = 0x000,
    EOptionPrintHeader                = 0x008,
};

std::string jsonPath;
int Options;
spv::TLanguage Language;

void Usage()
{
    printf("Usage: spirv option [file]\n"
           "\n"
           "  -h <language> print header for given language to stdout, from one of:\n"
           "     C      - C99 header\n"
           "     C++    - C++03 or greater header (also accepts C++03)\n"
           "     C++11  - C++11 or greater header\n"
           "     JSON   - JSON format data\n"
           "     Lua    - Lua module\n"
           "     Python - Python module (also accepts Py)\n"
           "     C#     - C# module (also accepts CSharp)\n"
           "     D      - D module\n"
           "     Beef   - Beef module\n"
           "  -H print header in all supported languages to files in current directory\n"
           );
}

std::string tolower_s(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

bool ProcessArguments(int argc, char* argv[])
{
    argc--;
    argv++;
    for (; argc >= 1; argc--, argv++) {
        if (argv[0][0] == '-') {
            switch (argv[0][1]) {
            case 'H':
                Options |= EOptionPrintHeader;
                Language = spv::ELangAll;
                break;
            case 'h': {
                if (argc < 2)
                    return false;

                Options |= EOptionPrintHeader;
                const std::string language(tolower_s(argv[1]));

                if (language == "c") {
                    Language = spv::ELangC;
                } else if (language == "c++" || language == "c++03") {
                    Language = spv::ELangCPP;
                } else if (language == "c++11") {
                    Language = spv::ELangCPP11;
                } else if (language == "json") {
                    Language = spv::ELangJSON;
                } else if (language == "lua") {
                    Language = spv::ELangLua;
                } else if (language == "python" || language == "py") {
                    Language = spv::ELangPython;
                } else if (language == "c#" || language == "csharp") {
                    Language = spv::ELangCSharp;
                } else if (language == "d") {
                    Language = spv::ELangD;
                } else if (language == "beef") {
                    Language = spv::ELangBeef;
                } else
                    return false;

                return true;
            }
            default:
                return false;
            }
        } else {
            jsonPath = std::string(argv[0]);
        }
    }

    return true;
}

int main(int argc, char* argv[])
{
    if (argc < 2 || ! ProcessArguments(argc, argv)) {
        Usage();
        return 1;
    }

    spv::jsonToSpirv(jsonPath, (Options & EOptionPrintHeader) != 0);
    if (Options & EOptionPrintHeader)
        spv::PrintHeader(Language, std::cout);

    return 0;
}
