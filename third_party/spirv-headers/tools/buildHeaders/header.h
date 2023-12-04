// Copyright (c) 2014-2019 The Khronos Group Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and/or associated documentation files (the "Materials"),
// to deal in the Materials without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Materials, and to permit persons to whom the
// Materials are furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Materials.
//
// MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS KHRONOS
// STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS SPECIFICATIONS AND
// HEADER INFORMATION ARE LOCATED AT https://www.khronos.org/registry/
//
// THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM,OUT OF OR IN CONNECTION WITH THE MATERIALS OR THE USE OR OTHER DEALINGS
// IN THE MATERIALS.

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
