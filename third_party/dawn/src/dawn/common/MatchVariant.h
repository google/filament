// Copyright 2024 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_COMMON_MATCHVARIANT_H_
#define SRC_DAWN_COMMON_MATCHVARIANT_H_

#include <variant>

namespace dawn {

// This is the `Overloaded` template in chromium/src/base/functional/Overloaded.h.
// std::visit() needs to be called with a functor object, such as
//
//  struct Visitor {
//    std::string operator()(const PackageA& source) {
//      return "PackageA";
//    }
//
//    std::string operator()(const PackageB& source) {
//      return "PackageB";
//    }
//  };
//
//  std::variant<PackageA, PackageB> var = PackageA();
//  return std::visit(Visitor(), var);
//
// `Overloaded` enables the above code to be written as:
//
//  std::visit(
//     Overloaded{
//         [](const PackageA& pack) { return "PackageA"; },
//         [](const PackageB& pack) { return "PackageB"; },
//     }, var);
//
// Note: Overloads must be implemented for all the variant options. Otherwise, there will be a
// compilation error.
//
// This struct inherits operator() method from all its base classes. Introduces operator() method
// from all its base classes into its definition.
template <typename... Callables>
struct Overloaded : Callables... {
    using Callables::operator()...;
};

// Uses template argument deduction so that the `Overloaded` struct can be used without specifying
// its template argument. This allows anonymous lambdas passed into the `Overloaded` constructor.
template <typename... Callables>
Overloaded(Callables...) -> Overloaded<Callables...>;

// With this template we can simplify the call of std::visit(Overloaded{...}, variant).
template <typename Variant, typename... Callables>
auto MatchVariant(const Variant& v, Callables... args) {
    return std::visit(Overloaded{args...}, v);
}

}  // namespace dawn

#endif
