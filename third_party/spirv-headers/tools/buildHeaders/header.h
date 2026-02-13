// SPDX-FileCopyrightText: 2014-2024 The Khronos Group Inc.
// SPDX-License-Identifier: MIT
// 
// MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
// KHRONOS STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS
// SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT
//    https://www.khronos.org/registry/

//
// Print headers for SPIR-V in several languages.
//

#pragma once
#ifndef header
#define header

#include <iostream>

namespace spv {
    // Languages supported
    enum TLanguage {
        ELangC,        // C
        ELangCPP,      // C++03
        ELangCPP11,    // C++11
        ELangJSON,     // JSON
        ELangLua,      // Lua
        ELangPython,   // Python
        ELangCSharp,   // CSharp
        ELangD,        // D
        ELangBeef,     // Beef

        ELangAll,      // print headers in all languages to files
    };

    // Generate header for requested language
    void PrintHeader(TLanguage, std::ostream&);
} // namespace spv

#endif // header
