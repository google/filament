// Copyright 2020 The Dawn & Tint Authors  //
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

#include "src/tint/utils/containers/scope_stack.h"

#include "gtest/gtest.h"
#include "src/tint/lang/wgsl/program/program_builder.h"

namespace tint {
namespace {

class ScopeStackTest : public ProgramBuilder, public testing::Test {};

TEST_F(ScopeStackTest, Get) {
    ScopeStack<Symbol, uint32_t> s;
    Symbol a(1, ID(), "1");
    Symbol b(3, ID(), "3");
    s.Push();
    s.Set(a, 5u);
    s.Set(b, 10u);

    EXPECT_EQ(s.Get(a), 5u);
    EXPECT_EQ(s.Get(b), 10u);

    s.Push();

    s.Set(a, 15u);
    EXPECT_EQ(s.Get(a), 15u);
    EXPECT_EQ(s.Get(b), 10u);

    s.Pop();
    EXPECT_EQ(s.Get(a), 5u);
    EXPECT_EQ(s.Get(b), 10u);
}

TEST_F(ScopeStackTest, Get_MissingSymbol) {
    ScopeStack<Symbol, uint32_t> s;
    Symbol sym(1, ID(), "1");
    EXPECT_EQ(s.Get(sym), 0u);
}

TEST_F(ScopeStackTest, Set) {
    ScopeStack<Symbol, uint32_t> s;
    Symbol a(1, ID(), "1");
    Symbol b(2, ID(), "2");

    EXPECT_EQ(s.Set(a, 5u), 0u);
    EXPECT_EQ(s.Get(a), 5u);

    EXPECT_EQ(s.Set(b, 10u), 0u);
    EXPECT_EQ(s.Get(b), 10u);

    EXPECT_EQ(s.Set(a, 20u), 5u);
    EXPECT_EQ(s.Get(a), 20u);

    EXPECT_EQ(s.Set(b, 25u), 10u);
    EXPECT_EQ(s.Get(b), 25u);
}

TEST_F(ScopeStackTest, Clear) {
    ScopeStack<Symbol, uint32_t> s;
    Symbol a(1, ID(), "1");
    Symbol b(2, ID(), "2");

    EXPECT_EQ(s.Set(a, 5u), 0u);
    EXPECT_EQ(s.Get(a), 5u);

    s.Push();

    EXPECT_EQ(s.Set(b, 10u), 0u);
    EXPECT_EQ(s.Get(b), 10u);

    s.Push();

    s.Clear();
    EXPECT_EQ(s.Get(a), 0u);
    EXPECT_EQ(s.Get(b), 0u);
}

}  // namespace
}  // namespace tint
