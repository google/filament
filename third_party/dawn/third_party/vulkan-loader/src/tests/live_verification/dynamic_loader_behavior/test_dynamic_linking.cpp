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
    std::cout << do_logic() << "\n";
#if defined(PRINT_OUTPUT_C)
    LibraryWrapper dynamic_library_c{std::string("./libdynamic_library_c.") + LIB_EXT};
    InitFunction init = dynamic_library_c.get_symbol(INIT_FUNCTION_NAME);
    if (init == nullptr) return 1;
    init();
    DoLogicFunction do_logic = dynamic_library_c.get_symbol(DO_LOGIC_FUNCTION_NAME);
    if (do_logic == nullptr || do_logic() != 'C') return 2;

    do_logic = reinterpret_cast<DoLogicFunction>(dlsym(RTLD_NEXT, DO_LOGIC_FUNCTION_NAME));
    if (do_logic == nullptr || do_logic() != 'A') return 3;
    std::cout << "Success\n";
#endif
    return 0;
}