// Copyright 2025 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_CORE_IR_VALIDATOR_TEST_H_
#define SRC_TINT_LANG_CORE_IR_VALIDATOR_TEST_H_

#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "src/tint/lang/core/builtin_value.h"
#include "src/tint/lang/core/io_attributes.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::core {

namespace type {
class Type;
}  // namespace type

namespace ir {

class Function;

class IR_ValidatorTest : public IRTestHelper {
  public:
    /// Builds and returns a basic 'compute' entry point function, named @p name
    Function* ComputeEntryPoint(const std::string& name = "f");

    /// Builds and returns a basic 'fragment' entry point function, named @p name
    Function* FragmentEntryPoint(const std::string& name = "f");

    /// Builds and returns a basic 'vertex' entry point function, named @p name
    Function* VertexEntryPoint(const std::string& name = "f");

    /// Adds to a function an input param named @p name of type @p type, and decorated with @p
    /// builtin
    void AddBuiltinParam(Function* func,
                         const std::string& name,
                         BuiltinValue builtin,
                         const core::type::Type* type);

    /// Adds to a function an return value of type @p type with attributes @p attr.
    /// If there is an already existing non-structured return, both values are moved into a
    /// structured return using @p name as the name.
    /// If there is an already existing structured return, then this ICEs, since that is beyond the
    /// scope of this implementation.
    void AddReturn(Function* func,
                   const std::string& name,
                   const core::type::Type* type,
                   const IOAttributes& attr = {});

    /// Adds to a function an return value of type @p type, and decorated with @p builtin.
    /// See @ref AddReturn for more details
    void AddBuiltinReturn(Function* func,
                          const std::string& name,
                          BuiltinValue builtin,
                          const core::type::Type* type);
};

}  // namespace ir
}  // namespace tint::core

#endif  // SRC_TINT_LANG_CORE_IR_VALIDATOR_TEST_H_
