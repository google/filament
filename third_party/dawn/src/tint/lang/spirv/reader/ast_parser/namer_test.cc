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

#include "src/tint/lang/spirv/reader/ast_parser/namer.h"

#include "gmock/gmock.h"
#include "src/tint/lang/core/builtin_fn.h"
#include "src/tint/utils/text/string_stream.h"

namespace tint::spirv::reader::ast_parser {
namespace {

using ::testing::Eq;

class SpvNamerTest : public testing::Test {
  public:
    SpvNamerTest() : fail_stream_(&success_, &errors_) {}

    /// @returns the accumulated diagnostic strings
    std::string error() { return errors_.str(); }

  protected:
    StringStream errors_;
    bool success_ = true;
    FailStream fail_stream_;
};

TEST_F(SpvNamerTest, SanitizeEmpty) {
    EXPECT_THAT(Namer::Sanitize(""), Eq("empty"));
}

TEST_F(SpvNamerTest, SanitizeLeadingUnderscore) {
    EXPECT_THAT(Namer::Sanitize("_"), Eq("x_"));
}

TEST_F(SpvNamerTest, SanitizeLeadingDigit) {
    EXPECT_THAT(Namer::Sanitize("7zip"), Eq("x7zip"));
}

TEST_F(SpvNamerTest, SanitizeOkChars) {
    EXPECT_THAT(Namer::Sanitize("_abcdef12345"), Eq("x_abcdef12345"));
}

TEST_F(SpvNamerTest, SanitizeNonIdentifierChars) {
    EXPECT_THAT(Namer::Sanitize("a:1.2'f\n"), "a_1_2_f_");
}

TEST_F(SpvNamerTest, NoFailureToStart) {
    Namer namer(fail_stream_);
    EXPECT_TRUE(success_);
    EXPECT_TRUE(error().empty());
}

TEST_F(SpvNamerTest, FailLogsError) {
    Namer namer(fail_stream_);
    const bool converted_result = namer.Fail() << "st. johns wood";
    EXPECT_FALSE(converted_result);
    EXPECT_EQ(error(), "st. johns wood");
    EXPECT_FALSE(success_);
}

TEST_F(SpvNamerTest, NoNameRecorded) {
    Namer namer(fail_stream_);

    EXPECT_FALSE(namer.HasName(12));
    EXPECT_TRUE(success_);
    EXPECT_TRUE(error().empty());
}

TEST_F(SpvNamerTest, FindUnusedDerivedName_NoRecordedName) {
    Namer namer(fail_stream_);
    EXPECT_THAT(namer.FindUnusedDerivedName("eleanor"), Eq("eleanor"));
    // Prove that it wasn't registered when first found.
    EXPECT_THAT(namer.FindUnusedDerivedName("eleanor"), Eq("eleanor"));
}

TEST_F(SpvNamerTest, FindUnusedDerivedName_HasRecordedName) {
    Namer namer(fail_stream_);
    namer.Register(12, "rigby");
    EXPECT_THAT(namer.FindUnusedDerivedName("rigby"), Eq("rigby_1"));
}

TEST_F(SpvNamerTest, FindUnusedDerivedName_HasMultipleConflicts) {
    Namer namer(fail_stream_);
    namer.Register(12, "rigby");
    namer.Register(13, "rigby_1");
    namer.Register(14, "rigby_3");
    // It picks the first non-conflicting suffix.
    EXPECT_THAT(namer.FindUnusedDerivedName("rigby"), Eq("rigby_2"));
}

TEST_F(SpvNamerTest, IsRegistered_NoRecordedName) {
    Namer namer(fail_stream_);
    EXPECT_FALSE(namer.IsRegistered("abbey"));
}

TEST_F(SpvNamerTest, IsRegistered_RegisteredById) {
    Namer namer(fail_stream_);
    namer.Register(1, "abbey");
    EXPECT_TRUE(namer.IsRegistered("abbey"));
}

TEST_F(SpvNamerTest, IsRegistered_RegisteredByDerivation) {
    Namer namer(fail_stream_);
    const auto got = namer.MakeDerivedName("abbey");
    EXPECT_TRUE(namer.IsRegistered("abbey"));
    EXPECT_EQ(got, "abbey");
}

TEST_F(SpvNamerTest, MakeDerivedName_NoRecordedName) {
    Namer namer(fail_stream_);
    EXPECT_THAT(namer.MakeDerivedName("eleanor"), Eq("eleanor"));
    // Prove that it was registered when first found.
    EXPECT_THAT(namer.MakeDerivedName("eleanor"), Eq("eleanor_1"));
}

TEST_F(SpvNamerTest, MakeDerivedName_HasRecordedName) {
    Namer namer(fail_stream_);
    namer.Register(12, "rigby");
    EXPECT_THAT(namer.MakeDerivedName("rigby"), Eq("rigby_1"));
}

TEST_F(SpvNamerTest, MakeDerivedName_HasMultipleConflicts) {
    Namer namer(fail_stream_);
    namer.Register(12, "rigby");
    namer.Register(13, "rigby_1");
    namer.Register(14, "rigby_3");
    // It picks the first non-conflicting suffix.
    EXPECT_THAT(namer.MakeDerivedName("rigby"), Eq("rigby_2"));
}

TEST_F(SpvNamerTest, RegisterWithoutId_Once) {
    Namer namer(fail_stream_);

    const std::string n("abbey");
    EXPECT_FALSE(namer.IsRegistered(n));
    EXPECT_TRUE(namer.RegisterWithoutId(n));
    EXPECT_TRUE(namer.IsRegistered(n));
    EXPECT_TRUE(success_);
    EXPECT_TRUE(error().empty());
}

TEST_F(SpvNamerTest, RegisterWithoutId_Twice) {
    Namer namer(fail_stream_);

    const std::string n("abbey");
    EXPECT_FALSE(namer.IsRegistered(n));
    EXPECT_TRUE(namer.RegisterWithoutId(n));
    // Fails on second attempt.
    EXPECT_FALSE(namer.RegisterWithoutId(n));
    EXPECT_FALSE(success_);
    EXPECT_EQ(error(), "internal error: name already registered: abbey");
}

TEST_F(SpvNamerTest, RegisterWithoutId_ConflictsWithIdRegisteredName) {
    Namer namer(fail_stream_);

    const std::string n("abbey");
    EXPECT_TRUE(namer.Register(1, n));
    EXPECT_TRUE(namer.IsRegistered(n));
    // Fails on attempt to register without ID.
    EXPECT_FALSE(namer.RegisterWithoutId(n));
    EXPECT_FALSE(success_);
    EXPECT_EQ(error(), "internal error: name already registered: abbey");
}

TEST_F(SpvNamerTest, Register_Once) {
    Namer namer(fail_stream_);

    const uint32_t id = 9;
    EXPECT_FALSE(namer.HasName(id));
    const bool save_result = namer.Register(id, "abbey road");
    EXPECT_TRUE(save_result);
    EXPECT_TRUE(namer.HasName(id));
    EXPECT_EQ(namer.GetName(id), "abbey road");
    EXPECT_TRUE(success_);
    EXPECT_TRUE(error().empty());
}

TEST_F(SpvNamerTest, Register_TwoIds) {
    Namer namer(fail_stream_);

    EXPECT_FALSE(namer.HasName(8));
    EXPECT_FALSE(namer.HasName(9));
    EXPECT_TRUE(namer.Register(8, "abbey road"));
    EXPECT_TRUE(namer.Register(9, "rubber soul"));
    EXPECT_TRUE(namer.HasName(8));
    EXPECT_TRUE(namer.HasName(9));
    EXPECT_EQ(namer.GetName(9), "rubber soul");
    EXPECT_EQ(namer.GetName(8), "abbey road");
    EXPECT_TRUE(success_);
    EXPECT_TRUE(error().empty());
}

TEST_F(SpvNamerTest, Register_FailsDueToIdReuse) {
    Namer namer(fail_stream_);

    const uint32_t id = 9;
    EXPECT_TRUE(namer.Register(id, "abbey road"));
    EXPECT_FALSE(namer.Register(id, "rubber soul"));
    EXPECT_TRUE(namer.HasName(id));
    EXPECT_EQ(namer.GetName(id), "abbey road");
    EXPECT_FALSE(success_);
    EXPECT_FALSE(error().empty());
}

TEST_F(SpvNamerTest, SuggestSanitizedName_TakeSuggestionWhenNoConflict) {
    Namer namer(fail_stream_);

    EXPECT_TRUE(namer.SuggestSanitizedName(1, "father"));
    EXPECT_THAT(namer.GetName(1), Eq("father"));
}

TEST_F(SpvNamerTest, SuggestSanitizedName_RejectSuggestionWhenConflictOnSameId) {
    Namer namer(fail_stream_);

    namer.Register(1, "lennon");
    EXPECT_FALSE(namer.SuggestSanitizedName(1, "mccartney"));
    EXPECT_THAT(namer.GetName(1), Eq("lennon"));
}

TEST_F(SpvNamerTest, SuggestSanitizedName_SanitizeSuggestion) {
    Namer namer(fail_stream_);

    EXPECT_TRUE(namer.SuggestSanitizedName(9, "m:kenzie"));
    EXPECT_THAT(namer.GetName(9), Eq("m_kenzie"));
}

TEST_F(SpvNamerTest, SuggestSanitizedName_GenerateNewNameWhenConflictOnDifferentId) {
    Namer namer(fail_stream_);

    namer.Register(7, "rice");
    EXPECT_TRUE(namer.SuggestSanitizedName(9, "rice"));
    EXPECT_THAT(namer.GetName(9), Eq("rice_1"));
}

TEST_F(SpvNamerTest, GetMemberName_EmptyStringForUnvisitedStruct) {
    Namer namer(fail_stream_);
    EXPECT_THAT(namer.GetMemberName(1, 2), Eq(""));
}

TEST_F(SpvNamerTest, GetMemberName_EmptyStringForUnvisitedMember) {
    Namer namer(fail_stream_);
    namer.SuggestSanitizedMemberName(1, 2, "mother");
    EXPECT_THAT(namer.GetMemberName(1, 0), Eq(""));
}

TEST_F(SpvNamerTest, SuggestSanitizedMemberName_TakeSuggestionWhenNoConflict) {
    Namer namer(fail_stream_);
    EXPECT_TRUE(namer.SuggestSanitizedMemberName(1, 2, "mother"));
    EXPECT_THAT(namer.GetMemberName(1, 2), Eq("mother"));
}

TEST_F(SpvNamerTest, SuggestSanitizedMemberName_TakeSanitizedSuggestion) {
    Namer namer(fail_stream_);
    EXPECT_TRUE(namer.SuggestSanitizedMemberName(1, 2, "m:t%er"));
    EXPECT_THAT(namer.GetMemberName(1, 2), Eq("m_t_er"));
}

TEST_F(
    SpvNamerTest,
    SuggestSanitizedMemberName_TakeSuggestionWhenNoConflictAfterSuggestionForLowerMember) {  // NOLINT
    Namer namer(fail_stream_);
    EXPECT_TRUE(namer.SuggestSanitizedMemberName(1, 7, "mother"));
    EXPECT_THAT(namer.GetMemberName(1, 2), Eq(""));
    EXPECT_TRUE(namer.SuggestSanitizedMemberName(1, 2, "mary"));
    EXPECT_THAT(namer.GetMemberName(1, 2), Eq("mary"));
}

TEST_F(SpvNamerTest, SuggestSanitizedMemberName_RejectSuggestionIfConflictOnMember) {
    Namer namer(fail_stream_);
    EXPECT_TRUE(namer.SuggestSanitizedMemberName(1, 2, "mother"));
    EXPECT_FALSE(namer.SuggestSanitizedMemberName(1, 2, "mary"));
    EXPECT_THAT(namer.GetMemberName(1, 2), Eq("mother"));
}

TEST_F(SpvNamerTest, Name_GeneratesNameIfNoneRegistered) {
    Namer namer(fail_stream_);
    EXPECT_THAT(namer.Name(14), Eq("x_14"));
}

TEST_F(SpvNamerTest, Name_GeneratesNameWithoutConflict) {
    Namer namer(fail_stream_);
    namer.Register(42, "x_14");
    EXPECT_THAT(namer.Name(14), Eq("x_14_1"));
}

TEST_F(SpvNamerTest, Name_ReturnsRegisteredName) {
    Namer namer(fail_stream_);
    namer.Register(14, "hello");
    EXPECT_THAT(namer.Name(14), Eq("hello"));
}

TEST_F(SpvNamerTest, ResolveMemberNamesForStruct_GeneratesRegularNamesOnItsOwn) {
    Namer namer(fail_stream_);
    namer.ResolveMemberNamesForStruct(2, 4);
    EXPECT_THAT(namer.GetMemberName(2, 0), Eq("field0"));
    EXPECT_THAT(namer.GetMemberName(2, 1), Eq("field1"));
    EXPECT_THAT(namer.GetMemberName(2, 2), Eq("field2"));
    EXPECT_THAT(namer.GetMemberName(2, 3), Eq("field3"));
}

TEST_F(SpvNamerTest, ResolveMemberNamesForStruct_ResolvesConflictBetweenSuggestedNames) {
    Namer namer(fail_stream_);
    namer.SuggestSanitizedMemberName(2, 0, "apple");
    namer.SuggestSanitizedMemberName(2, 1, "apple");
    namer.ResolveMemberNamesForStruct(2, 2);
    EXPECT_THAT(namer.GetMemberName(2, 0), Eq("apple"));
    EXPECT_THAT(namer.GetMemberName(2, 1), Eq("apple_1"));
}

TEST_F(SpvNamerTest, ResolveMemberNamesForStruct_FillsUnsuggestedGaps) {
    Namer namer(fail_stream_);
    namer.SuggestSanitizedMemberName(2, 1, "apple");
    namer.SuggestSanitizedMemberName(2, 2, "core");
    namer.ResolveMemberNamesForStruct(2, 4);
    EXPECT_THAT(namer.GetMemberName(2, 0), Eq("field0"));
    EXPECT_THAT(namer.GetMemberName(2, 1), Eq("apple"));
    EXPECT_THAT(namer.GetMemberName(2, 2), Eq("core"));
    EXPECT_THAT(namer.GetMemberName(2, 3), Eq("field3"));
}

TEST_F(SpvNamerTest, ResolveMemberNamesForStruct_GeneratedNameAvoidsConflictWithSuggestion) {
    Namer namer(fail_stream_);
    namer.SuggestSanitizedMemberName(2, 0, "field1");
    namer.ResolveMemberNamesForStruct(2, 2);
    EXPECT_THAT(namer.GetMemberName(2, 0), Eq("field1"));
    EXPECT_THAT(namer.GetMemberName(2, 1), Eq("field1_1"));
}

TEST_F(SpvNamerTest, ResolveMemberNamesForStruct_TruncatesOutOfBoundsSuggestion) {
    Namer namer(fail_stream_);
    namer.SuggestSanitizedMemberName(2, 3, "sitar");
    EXPECT_THAT(namer.GetMemberName(2, 3), Eq("sitar"));
    namer.ResolveMemberNamesForStruct(2, 2);
    EXPECT_THAT(namer.GetMemberName(2, 0), Eq("field0"));
    EXPECT_THAT(namer.GetMemberName(2, 1), Eq("field1"));
    EXPECT_THAT(namer.GetMemberName(2, 3), Eq(""));
}

using SpvNamerReservedWordTest = ::testing::TestWithParam<std::string>;

TEST_P(SpvNamerReservedWordTest, ReservedWordsAreUsed) {
    bool success;
    StringStream errors;
    FailStream fail_stream(&success, &errors);
    Namer namer(fail_stream);
    const std::string reserved = GetParam();
    // Since it's reserved, it's marked as used, and we can't register an ID
    EXPECT_THAT(namer.FindUnusedDerivedName(reserved), Eq(reserved + "_1"));
}

INSTANTIATE_TEST_SUITE_P(SpirvASTParserTest_ReservedWords,
                         SpvNamerReservedWordTest,
                         ::testing::ValuesIn(std::vector<std::string>{
                             // Please keep this list sorted.
                             "array",      "as",          "asm",
                             "bf16",       "binding",     "block",
                             "bool",       "break",       "builtin",
                             "case",       "cast",        "compute",
                             "const",      "continue",    "default",
                             "discard",    "do",          "else",
                             "elseif",     "entry_point", "enum",
                             "f16",        "f32",         "fallthrough",
                             "false",      "fn",          "for",
                             "fragment",   "i16",         "i32",
                             "i64",        "i8",          "if",
                             "image",      "import",      "in",
                             "let",        "location",    "loop",
                             "mat2x2",     "mat2x3",      "mat2x4",
                             "mat3x2",     "mat3x3",      "mat3x4",
                             "mat4x2",     "mat4x3",      "mat4x4",
                             "offset",     "out",         "override",
                             "premerge",   "private",     "ptr",
                             "regardless", "return",      "set",
                             "storage",    "struct",      "switch",
                             "true",       "type",        "typedef",
                             "u16",        "u32",         "u64",
                             "u8",         "uniform",     "uniform_constant",
                             "unless",     "using",       "var",
                             "vec2",       "vec3",        "vec4",
                             "vertex",     "void",        "while",
                             "workgroup",
                         }));

using SpvNamerBuiltinFunctionTest = ::testing::TestWithParam<const char*>;

TEST_P(SpvNamerBuiltinFunctionTest, BuiltinFunctionsAreUsed) {
    bool success;
    StringStream errors;
    FailStream fail_stream(&success, &errors);
    Namer namer(fail_stream);
    const std::string builtin_fn = GetParam();
    // Since it's a builtin function, it's marked as used, and we can't register an ID.
    EXPECT_THAT(namer.FindUnusedDerivedName(builtin_fn), Eq(builtin_fn + "_1"));
}

INSTANTIATE_TEST_SUITE_P(SpirvASTParserTest_BuiltinFunctions,
                         SpvNamerBuiltinFunctionTest,
                         ::testing::ValuesIn(core::kBuiltinFnStrings));

}  // namespace
}  // namespace tint::spirv::reader::ast_parser
