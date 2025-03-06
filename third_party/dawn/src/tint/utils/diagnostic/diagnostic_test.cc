// Copyright 2020 The Dawn & Tint Authors
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

#include "src/tint/utils/diagnostic/formatter.h"

#include "gtest/gtest.h"
#include "src/tint/utils/diagnostic/diagnostic.h"

namespace tint::diag {
namespace {

TEST(DiagListTest, CtorInitializerList) {
    Diagnostic err_a, err_b;
    err_a.severity = Severity::Error;
    err_b.severity = Severity::Warning;
    List list{err_a, err_b};
    EXPECT_EQ(list.Count(), 2u);
}

TEST(DiagListTest, CtorVectorRef) {
    Diagnostic err_a, err_b;
    err_a.severity = Severity::Error;
    err_b.severity = Severity::Warning;
    List list{Vector{err_a, err_b}};
    EXPECT_EQ(list.Count(), 2u);
}

TEST(DiagListTest, OwnedFilesShared) {
    auto file = std::make_shared<Source::File>("path", "content");

    List list_a, list_b;
    {
        Diagnostic diag{};
        diag.source = Source{Source::Range{{0, 0}}, file.get()};
        list_a.Add(std::move(diag));
    }

    list_b = list_a;

    ASSERT_EQ(list_b.Count(), list_a.Count());
    EXPECT_EQ(list_b.begin()->source.file, file.get());
}

}  // namespace
}  // namespace tint::diag
