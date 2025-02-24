/*
 * Copyright (c) 2022 The Khronos Group Inc.
 * Copyright (c) 2022 Valve Corporation
 * Copyright (c) 2022 LunarG, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and/or associated documentation files (the "Materials"), to
 * deal in the Materials without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Materials, and to permit persons to whom the Materials are
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice(s) and this permission notice shall be included in
 * all copies or substantial portions of the Materials.
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE MATERIALS OR THE
 * USE OR OTHER DEALINGS IN THE MATERIALS.
 *
 * Author: Charles Giessen <charles@lunarg.com>
 */

#include "dynamic_library.h"

int main() {
    {
        LibraryWrapper dynamic_library_a{std::string("./libdynamic_library_a.") + LIB_EXT};
        LibraryWrapper dynamic_library_b{std::string("./libdynamic_library_b.") + LIB_EXT};

        DoLogicFunction do_logic = nullptr;
        do_logic = dynamic_library_a.get_symbol(DO_LOGIC_FUNCTION_NAME);
        if (do_logic == nullptr || do_logic() != 'A') return 1;
        do_logic = dynamic_library_b.get_symbol(DO_LOGIC_FUNCTION_NAME);
        if (do_logic == nullptr || do_logic() != 'B') return 2;
    }
    {
        LibraryWrapper dynamic_library_c{std::string("./libdynamic_library_c.") + LIB_EXT};
        InitFunction init = dynamic_library_c.get_symbol(INIT_FUNCTION_NAME);
        if (init == nullptr) return 3;
        init();
        DoLogicFunction do_logic = dynamic_library_c.get_symbol(DO_LOGIC_FUNCTION_NAME);
        if (do_logic == nullptr || do_logic() != 'C') return 4;
        // should fail because RTLD_NEXT on linux only is for dynamically *linked* libraries
        do_logic = reinterpret_cast<DoLogicFunction>(dlsym(RTLD_NEXT, DO_LOGIC_FUNCTION_NAME));
        if (do_logic != nullptr) return 5;
    }
    std::cout << "Success\n";
    return 0;
}
