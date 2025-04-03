// Copyright 2023 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/utils/reflection.h"

#include "src/tint/utils/math/math.h"
#include "src/tint/utils/text/string_stream.h"

namespace tint::reflection::detail {

Result<SuccessType> CheckAllFieldsReflected(VectorRef<ReflectedFieldInfo> fields,
                                            std::string_view class_name,
                                            size_t class_size,
                                            size_t class_align,
                                            bool class_is_castable) {
    size_t calculated_offset = class_is_castable ? sizeof(CastableBase) : 0;
    for (auto& field : fields) {
        calculated_offset = RoundUp(field.align, calculated_offset);
        if (calculated_offset < field.offset) {
            StringStream msg;
            msg << "TINT_REFLECT(" << class_name << ", ...) field mismatch at '" << field.name
                << "'.\n"
                   "Expected field offset of "
                << calculated_offset << " bytes, but field was at " << field.offset << " bytes";
            return Failure{msg.str()};
        }
        calculated_offset += field.size;
    }
    calculated_offset = RoundUp(class_align, calculated_offset);
    if (calculated_offset != class_size) {
        StringStream msg;
        msg << "TINT_REFLECT(" << class_name
            << ", ...) missing fields at end of class\n"
               "Expected class size of "
            << calculated_offset << " bytes, but class is " << class_size << " bytes";
        return Failure{msg.str()};
    }
    return Success;
}

}  // namespace tint::reflection::detail
