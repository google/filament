// Copyright 2022 The Dawn & Tint Authors
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

#include "src/tint/utils/text/unicode.h"

#include <cstdint>
#include <ios>
#include <string>
#include <string_view>
#include <vector>

#include "gmock/gmock.h"
#include "src/tint/utils/text/string.h"

TINT_BEGIN_DISABLE_WARNING(UNSAFE_BUFFER_USAGE);

/// Helper for constructing a CodePoint
#define C(x) CodePoint(x)

namespace tint {
namespace {

struct CodePointCase {
    CodePoint code_point;
    bool is_xid_start;
    bool is_xid_continue;
};

static std::ostream& operator<<(std::ostream& out, CodePointCase c) {
    return out << c.code_point;
}

struct CodePointAndWidth {
    CodePoint code_point;
    size_t width;
};

bool operator==(const CodePointAndWidth& a, const CodePointAndWidth& b) {
    return a.code_point == b.code_point && a.width == b.width;
}

static std::ostream& operator<<(std::ostream& out, CodePointAndWidth cpw) {
    return out << "code_point: " << cpw.code_point << ", width: " << cpw.width;
}

}  // namespace

////////////////////////////////////////////////////////////////////////////////
// CodePoint character set tests
////////////////////////////////////////////////////////////////////////////////
namespace {

class CodePointTest : public testing::TestWithParam<CodePointCase> {};

TEST_P(CodePointTest, CharacterSets) {
    auto param = GetParam();
    EXPECT_EQ(param.code_point.IsXIDStart(), param.is_xid_start);
    EXPECT_EQ(param.code_point.IsXIDContinue(), param.is_xid_continue);
}

INSTANTIATE_TEST_SUITE_P(
    CodePointTest,
    CodePointTest,
    ::testing::ValuesIn({
        CodePointCase{C(' '), /* start */ false, /* continue */ false},
        CodePointCase{C('\t'), /* start */ false, /* continue */ false},
        CodePointCase{C('\n'), /* start */ false, /* continue */ false},
        CodePointCase{C('\r'), /* start */ false, /* continue */ false},
        CodePointCase{C('!'), /* start */ false, /* continue */ false},
        CodePointCase{C('"'), /* start */ false, /* continue */ false},
        CodePointCase{C('#'), /* start */ false, /* continue */ false},
        CodePointCase{C('$'), /* start */ false, /* continue */ false},
        CodePointCase{C('%'), /* start */ false, /* continue */ false},
        CodePointCase{C('&'), /* start */ false, /* continue */ false},
        CodePointCase{C('\\'), /* start */ false, /* continue */ false},
        CodePointCase{C('/'), /* start */ false, /* continue */ false},
        CodePointCase{C('('), /* start */ false, /* continue */ false},
        CodePointCase{C(')'), /* start */ false, /* continue */ false},
        CodePointCase{C('*'), /* start */ false, /* continue */ false},
        CodePointCase{C(','), /* start */ false, /* continue */ false},
        CodePointCase{C('-'), /* start */ false, /* continue */ false},
        CodePointCase{C('/'), /* start */ false, /* continue */ false},
        CodePointCase{C('`'), /* start */ false, /* continue */ false},
        CodePointCase{C('@'), /* start */ false, /* continue */ false},
        CodePointCase{C('^'), /* start */ false, /* continue */ false},
        CodePointCase{C('['), /* start */ false, /* continue */ false},
        CodePointCase{C(']'), /* start */ false, /* continue */ false},
        CodePointCase{C('|'), /* start */ false, /* continue */ false},
        CodePointCase{C('('), /* start */ false, /* continue */ false},
        CodePointCase{C(','), /* start */ false, /* continue */ false},
        CodePointCase{C('}'), /* start */ false, /* continue */ false},
        CodePointCase{C('a'), /* start */ true, /* continue */ true},
        CodePointCase{C('b'), /* start */ true, /* continue */ true},
        CodePointCase{C('c'), /* start */ true, /* continue */ true},
        CodePointCase{C('x'), /* start */ true, /* continue */ true},
        CodePointCase{C('y'), /* start */ true, /* continue */ true},
        CodePointCase{C('z'), /* start */ true, /* continue */ true},
        CodePointCase{C('A'), /* start */ true, /* continue */ true},
        CodePointCase{C('B'), /* start */ true, /* continue */ true},
        CodePointCase{C('C'), /* start */ true, /* continue */ true},
        CodePointCase{C('X'), /* start */ true, /* continue */ true},
        CodePointCase{C('Y'), /* start */ true, /* continue */ true},
        CodePointCase{C('Z'), /* start */ true, /* continue */ true},
        CodePointCase{C('_'), /* start */ false, /* continue */ true},
        CodePointCase{C('0'), /* start */ false, /* continue */ true},
        CodePointCase{C('1'), /* start */ false, /* continue */ true},
        CodePointCase{C('2'), /* start */ false, /* continue */ true},
        CodePointCase{C('8'), /* start */ false, /* continue */ true},
        CodePointCase{C('9'), /* start */ false, /* continue */ true},
        CodePointCase{C('0'), /* start */ false, /* continue */ true},

        // First in XID_Start
        CodePointCase{C(0x00041), /* start */ true, /* continue */ true},
        // Last in XID_Start
        CodePointCase{C(0x3134a), /* start */ true, /* continue */ true},

        // Random selection from XID_Start, using the interval's first
        CodePointCase{C(0x002ee), /* start */ true, /* continue */ true},
        CodePointCase{C(0x005ef), /* start */ true, /* continue */ true},
        CodePointCase{C(0x009f0), /* start */ true, /* continue */ true},
        CodePointCase{C(0x00d3d), /* start */ true, /* continue */ true},
        CodePointCase{C(0x00d54), /* start */ true, /* continue */ true},
        CodePointCase{C(0x00e86), /* start */ true, /* continue */ true},
        CodePointCase{C(0x00edc), /* start */ true, /* continue */ true},
        CodePointCase{C(0x01c00), /* start */ true, /* continue */ true},
        CodePointCase{C(0x01c80), /* start */ true, /* continue */ true},
        CodePointCase{C(0x02071), /* start */ true, /* continue */ true},
        CodePointCase{C(0x02dd0), /* start */ true, /* continue */ true},
        CodePointCase{C(0x0a4d0), /* start */ true, /* continue */ true},
        CodePointCase{C(0x0aac0), /* start */ true, /* continue */ true},
        CodePointCase{C(0x0ab5c), /* start */ true, /* continue */ true},
        CodePointCase{C(0x0ffda), /* start */ true, /* continue */ true},
        CodePointCase{C(0x11313), /* start */ true, /* continue */ true},
        CodePointCase{C(0x1ee49), /* start */ true, /* continue */ true},

        // Random selection from XID_Start, using the interval's last
        CodePointCase{C(0x00710), /* start */ true, /* continue */ true},
        CodePointCase{C(0x00b83), /* start */ true, /* continue */ true},
        CodePointCase{C(0x00b9a), /* start */ true, /* continue */ true},
        CodePointCase{C(0x00ec4), /* start */ true, /* continue */ true},
        CodePointCase{C(0x01081), /* start */ true, /* continue */ true},
        CodePointCase{C(0x012be), /* start */ true, /* continue */ true},
        CodePointCase{C(0x02107), /* start */ true, /* continue */ true},
        CodePointCase{C(0x03029), /* start */ true, /* continue */ true},
        CodePointCase{C(0x03035), /* start */ true, /* continue */ true},
        CodePointCase{C(0x0aadd), /* start */ true, /* continue */ true},
        CodePointCase{C(0x10805), /* start */ true, /* continue */ true},
        CodePointCase{C(0x11075), /* start */ true, /* continue */ true},
        CodePointCase{C(0x1d4a2), /* start */ true, /* continue */ true},
        CodePointCase{C(0x1e7fe), /* start */ true, /* continue */ true},
        CodePointCase{C(0x1ee27), /* start */ true, /* continue */ true},
        CodePointCase{C(0x2b738), /* start */ true, /* continue */ true},

        // Random selection from XID_Continue, using the interval's first
        CodePointCase{C(0x16ac0), /* start */ false, /* continue */ true},
        CodePointCase{C(0x00dca), /* start */ false, /* continue */ true},
        CodePointCase{C(0x16f4f), /* start */ false, /* continue */ true},
        CodePointCase{C(0x0fe00), /* start */ false, /* continue */ true},
        CodePointCase{C(0x00ec8), /* start */ false, /* continue */ true},
        CodePointCase{C(0x009be), /* start */ false, /* continue */ true},
        CodePointCase{C(0x11d47), /* start */ false, /* continue */ true},
        CodePointCase{C(0x11d50), /* start */ false, /* continue */ true},
        CodePointCase{C(0x0a926), /* start */ false, /* continue */ true},
        CodePointCase{C(0x0aac1), /* start */ false, /* continue */ true},
        CodePointCase{C(0x00f18), /* start */ false, /* continue */ true},
        CodePointCase{C(0x11145), /* start */ false, /* continue */ true},
        CodePointCase{C(0x017dd), /* start */ false, /* continue */ true},
        CodePointCase{C(0x0aaeb), /* start */ false, /* continue */ true},
        CodePointCase{C(0x11173), /* start */ false, /* continue */ true},
        CodePointCase{C(0x00a51), /* start */ false, /* continue */ true},

        // Random selection from XID_Continue, using the interval's last
        CodePointCase{C(0x00f84), /* start */ false, /* continue */ true},
        CodePointCase{C(0x10a3a), /* start */ false, /* continue */ true},
        CodePointCase{C(0x1e018), /* start */ false, /* continue */ true},
        CodePointCase{C(0x0a827), /* start */ false, /* continue */ true},
        CodePointCase{C(0x01abd), /* start */ false, /* continue */ true},
        CodePointCase{C(0x009d7), /* start */ false, /* continue */ true},
        CodePointCase{C(0x00b6f), /* start */ false, /* continue */ true},
        CodePointCase{C(0x0096f), /* start */ false, /* continue */ true},
        CodePointCase{C(0x11146), /* start */ false, /* continue */ true},
        CodePointCase{C(0x10eac), /* start */ false, /* continue */ true},
        CodePointCase{C(0x00f39), /* start */ false, /* continue */ true},
        CodePointCase{C(0x1e136), /* start */ false, /* continue */ true},
        CodePointCase{C(0x00def), /* start */ false, /* continue */ true},
        CodePointCase{C(0x0fe34), /* start */ false, /* continue */ true},
        CodePointCase{C(0x009c8), /* start */ false, /* continue */ true},
        CodePointCase{C(0x00fbc), /* start */ false, /* continue */ true},

        // Random code points that are one less than an interval of XID_Start
        CodePointCase{C(0x003f6), /* start */ false, /* continue */ false},
        CodePointCase{C(0x005ee), /* start */ false, /* continue */ false},
        CodePointCase{C(0x009ef), /* start */ false, /* continue */ true},
        CodePointCase{C(0x00d3c), /* start */ false, /* continue */ true},
        CodePointCase{C(0x00d53), /* start */ false, /* continue */ false},
        CodePointCase{C(0x00e85), /* start */ false, /* continue */ false},
        CodePointCase{C(0x00edb), /* start */ false, /* continue */ false},
        CodePointCase{C(0x01bff), /* start */ false, /* continue */ false},
        CodePointCase{C(0x02070), /* start */ false, /* continue */ false},
        CodePointCase{C(0x02dcf), /* start */ false, /* continue */ false},
        CodePointCase{C(0x0a4cf), /* start */ false, /* continue */ false},
        CodePointCase{C(0x0aabf), /* start */ false, /* continue */ true},
        CodePointCase{C(0x0ab5b), /* start */ false, /* continue */ false},
        CodePointCase{C(0x0ffd9), /* start */ false, /* continue */ false},
        CodePointCase{C(0x11312), /* start */ false, /* continue */ false},
        CodePointCase{C(0x1ee48), /* start */ false, /* continue */ false},

        // Random code points that are one more than an interval of XID_Continue
        CodePointCase{C(0x00060), /* start */ false, /* continue */ false},
        CodePointCase{C(0x00a4e), /* start */ false, /* continue */ false},
        CodePointCase{C(0x00a84), /* start */ false, /* continue */ false},
        CodePointCase{C(0x00cce), /* start */ false, /* continue */ false},
        CodePointCase{C(0x00eda), /* start */ false, /* continue */ false},
        CodePointCase{C(0x00f85), /* start */ false, /* continue */ false},
        CodePointCase{C(0x01b74), /* start */ false, /* continue */ false},
        CodePointCase{C(0x01c38), /* start */ false, /* continue */ false},
        CodePointCase{C(0x0fe30), /* start */ false, /* continue */ false},
        CodePointCase{C(0x11174), /* start */ false, /* continue */ false},
        CodePointCase{C(0x112eb), /* start */ false, /* continue */ false},
        CodePointCase{C(0x115de), /* start */ false, /* continue */ false},
        CodePointCase{C(0x1172c), /* start */ false, /* continue */ false},
        CodePointCase{C(0x11a3f), /* start */ false, /* continue */ false},
        CodePointCase{C(0x11c37), /* start */ false, /* continue */ false},
        CodePointCase{C(0x11d92), /* start */ false, /* continue */ false},
        CodePointCase{C(0x1e2af), /* start */ false, /* continue */ false},
    }));

}  // namespace

////////////////////////////////////////////////////////////////////////////////
// DecodeUTF8 valid tests
////////////////////////////////////////////////////////////////////////////////
namespace utf8_tests {

struct UTF8Case {
    std::vector<uint8_t> string;
    std::vector<CodePointAndWidth> code_points;
};

static std::ostream& operator<<(std::ostream& out, UTF8Case c) {
    for (size_t i = 0; i < c.string.size(); i++) {
        if (i > 0) {
            out << ", ";
        }
        out << "0x" << std::hex << std::setfill('0') << std::setw(2) << c.string[i];
    }
    return out;
}

class UTF8Test : public testing::TestWithParam<UTF8Case> {};

TEST_P(UTF8Test, Decode) {
    auto param = GetParam();

    const uint8_t* data = reinterpret_cast<const uint8_t*>(param.string.data());
    const size_t len = param.string.size();

    std::vector<CodePointAndWidth> got;
    size_t offset = 0;
    while (offset < len) {
        auto [code_point, width] = utf8::Decode(data + offset, len - offset);
        if (width == 0) {
            FAIL() << "Decode() failed at byte offset " << offset;
        }
        offset += width;
        got.emplace_back(CodePointAndWidth{code_point, width});
    }

    EXPECT_THAT(got, ::testing::ElementsAreArray(param.code_points));
}

TEST_P(UTF8Test, Encode) {
    auto param = GetParam();

    Slice<const uint8_t> str{reinterpret_cast<const uint8_t*>(param.string.data()),
                             param.string.size()};
    for (auto codepoint : param.code_points) {
        EXPECT_EQ(utf8::Encode(codepoint.code_point, nullptr), codepoint.width);

        uint8_t encoded[4];
        size_t len = utf8::Encode(codepoint.code_point, encoded);
        ASSERT_EQ(len, codepoint.width);
        EXPECT_THAT(Slice<const uint8_t>(encoded, len),
                    ::testing::ElementsAreArray(str.Truncate(len)));
        str = str.Offset(len);
    }
}

INSTANTIATE_TEST_SUITE_P(AsciiLetters,
                         UTF8Test,
                         ::testing::ValuesIn({
                             UTF8Case{{'a'}, {{C('a'), 1}}},
                             UTF8Case{{'a', 'b', 'c'}, {{C('a'), 1}, {C('b'), 1}, {C('c'), 1}}},
                             UTF8Case{{'d', 'e', 'f'}, {{C('d'), 1}, {C('e'), 1}, {C('f'), 1}}},
                             UTF8Case{{'g', 'h'}, {{C('g'), 1}, {C('h'), 1}}},
                             UTF8Case{{'i', 'j'}, {{C('i'), 1}, {C('j'), 1}}},
                             UTF8Case{{'k', 'l', 'm'}, {{C('k'), 1}, {C('l'), 1}, {C('m'), 1}}},
                             UTF8Case{{'n', 'o', 'p'}, {{C('n'), 1}, {C('o'), 1}, {C('p'), 1}}},
                             UTF8Case{{'q', 'r'}, {{C('q'), 1}, {C('r'), 1}}},
                             UTF8Case{{'s', 't', 'u'}, {{C('s'), 1}, {C('t'), 1}, {C('u'), 1}}},
                             UTF8Case{{'v', 'w'}, {{C('v'), 1}, {C('w'), 1}}},
                             UTF8Case{{'x', 'y', 'z'}, {{C('x'), 1}, {C('y'), 1}, {C('z'), 1}}},
                             UTF8Case{{'A'}, {{C('A'), 1}}},
                             UTF8Case{{'A', 'B', 'C'}, {{C('A'), 1}, {C('B'), 1}, {C('C'), 1}}},
                             UTF8Case{{'D', 'E', 'F'}, {{C('D'), 1}, {C('E'), 1}, {C('F'), 1}}},
                             UTF8Case{{'G', 'H'}, {{C('G'), 1}, {C('H'), 1}}},
                             UTF8Case{{'I', 'J'}, {{C('I'), 1}, {C('J'), 1}}},
                             UTF8Case{{'K', 'L', 'M'}, {{C('K'), 1}, {C('L'), 1}, {C('M'), 1}}},
                             UTF8Case{{'N', 'O', 'P'}, {{C('N'), 1}, {C('O'), 1}, {C('P'), 1}}},
                             UTF8Case{{'Q', 'R'}, {{C('Q'), 1}, {C('R'), 1}}},
                             UTF8Case{{'S', 'T', 'U'}, {{C('S'), 1}, {C('T'), 1}, {C('U'), 1}}},
                             UTF8Case{{'V', 'W'}, {{C('V'), 1}, {C('W'), 1}}},
                             UTF8Case{{'X', 'Y', 'Z'}, {{C('X'), 1}, {C('Y'), 1}, {C('Z'), 1}}},
                         }));

INSTANTIATE_TEST_SUITE_P(AsciiNumbers,
                         UTF8Test,
                         ::testing::ValuesIn({
                             UTF8Case{{'0', '1', '2'}, {{C('0'), 1}, {C('1'), 1}, {C('2'), 1}}},
                             UTF8Case{{'3', '4', '5'}, {{C('3'), 1}, {C('4'), 1}, {C('5'), 1}}},
                             UTF8Case{{'6', '7', '8'}, {{C('6'), 1}, {C('7'), 1}, {C('8'), 1}}},
                             UTF8Case{{'9'}, {{C('9'), 1}}},
                         }));

INSTANTIATE_TEST_SUITE_P(AsciiSymbols,
                         UTF8Test,
                         ::testing::ValuesIn({
                             UTF8Case{{'!', '"', '#'}, {{C('!'), 1}, {C('"'), 1}, {C('#'), 1}}},
                             UTF8Case{{'$', '%', '&'}, {{C('$'), 1}, {C('%'), 1}, {C('&'), 1}}},
                             UTF8Case{{'\'', '(', ')'}, {{C('\''), 1}, {C('('), 1}, {C(')'), 1}}},
                             UTF8Case{{'*', ',', '-'}, {{C('*'), 1}, {C(','), 1}, {C('-'), 1}}},
                             UTF8Case{{'/', '`', '@'}, {{C('/'), 1}, {C('`'), 1}, {C('@'), 1}}},
                             UTF8Case{{'^', '\\', '['}, {{C('^'), 1}, {C('\\'), 1}, {C('['), 1}}},
                             UTF8Case{{']', '_', '|'}, {{C(']'), 1}, {C('_'), 1}, {C('|'), 1}}},
                             UTF8Case{{'{', '}'}, {{C('{'), 1}, {C('}'), 1}}},
                         }));

INSTANTIATE_TEST_SUITE_P(
    AsciiSpecial,
    UTF8Test,
    ::testing::ValuesIn({
        UTF8Case{{}, {}},
        UTF8Case{{' ', '\t', '\n'}, {{C(' '), 1}, {C('\t'), 1}, {C('\n'), 1}}},
        UTF8Case{{'\a', '\b', '\f'}, {{C('\a'), 1}, {C('\b'), 1}, {C('\f'), 1}}},
        UTF8Case{{'\n', '\r', '\t'}, {{C('\n'), 1}, {C('\r'), 1}, {C('\t'), 1}}},
        UTF8Case{{'\v'}, {{C('\v'), 1}}},
    }));

INSTANTIATE_TEST_SUITE_P(Hindi,
                         UTF8Test,
                         ::testing::ValuesIn({UTF8Case{
                             // नमस्ते दुनिया
                             {
                                 0xe0, 0xa4, 0xa8, 0xe0, 0xa4, 0xae, 0xe0, 0xa4, 0xb8, 0xe0,
                                 0xa5, 0x8d, 0xe0, 0xa4, 0xa4, 0xe0, 0xa5, 0x87, 0x20, 0xe0,
                                 0xa4, 0xa6, 0xe0, 0xa5, 0x81, 0xe0, 0xa4, 0xa8, 0xe0, 0xa4,
                                 0xbf, 0xe0, 0xa4, 0xaf, 0xe0, 0xa4, 0xbe,
                             },
                             {
                                 {C(0x0928), 3},  // न
                                 {C(0x092e), 3},  // म
                                 {C(0x0938), 3},  // स
                                 {C(0x094d), 3},  // ् //
                                 {C(0x0924), 3},  // त
                                 {C(0x0947), 3},  // े //
                                 {C(' '), 1},
                                 {C(0x0926), 3},  // द
                                 {C(0x0941), 3},  // ु //
                                 {C(0x0928), 3},  // न
                                 {C(0x093f), 3},  // ि //
                                 {C(0x092f), 3},  // य
                                 {C(0x093e), 3},  // ा //
                             },
                         }}));

INSTANTIATE_TEST_SUITE_P(Mandarin,
                         UTF8Test,
                         ::testing::ValuesIn({UTF8Case{
                             // 你好世界
                             {
                                 0xe4,
                                 0xbd,
                                 0xa0,
                                 0xe5,
                                 0xa5,
                                 0xbd,
                                 0xe4,
                                 0xb8,
                                 0x96,
                                 0xe7,
                                 0x95,
                                 0x8c,
                             },
                             {
                                 {C(0x4f60), 3},  // 你
                                 {C(0x597d), 3},  // 好
                                 {C(0x4e16), 3},  // 世
                                 {C(0x754c), 3},  // 界
                             },
                         }}));

INSTANTIATE_TEST_SUITE_P(Japanese,
                         UTF8Test,
                         ::testing::ValuesIn({UTF8Case{
                             // こんにちは世界
                             {
                                 0xe3, 0x81, 0x93, 0xe3, 0x82, 0x93, 0xe3, 0x81, 0xab, 0xe3, 0x81,
                                 0xa1, 0xe3, 0x81, 0xaf, 0xe4, 0xb8, 0x96, 0xe7, 0x95, 0x8c,
                             },
                             {
                                 {C(0x3053), 3},  // こ
                                 {C(0x3093), 3},  // ん
                                 {C(0x306B), 3},  // に
                                 {C(0x3061), 3},  // ち
                                 {C(0x306F), 3},  // は
                                 {C(0x4E16), 3},  // 世
                                 {C(0x754C), 3},  // 界
                             },
                         }}));

INSTANTIATE_TEST_SUITE_P(Korean,
                         UTF8Test,
                         ::testing::ValuesIn({UTF8Case{
                             // 안녕하세요 세계
                             {
                                 0xec, 0x95, 0x88, 0xeb, 0x85, 0x95, 0xed, 0x95, 0x98, 0xec, 0x84,
                                 0xb8, 0xec, 0x9a, 0x94, 0x20, 0xec, 0x84, 0xb8, 0xea, 0xb3, 0x84,
                             },
                             {
                                 {C(0xc548), 3},  // 안
                                 {C(0xb155), 3},  // 녕
                                 {C(0xd558), 3},  // 하
                                 {C(0xc138), 3},  // 세
                                 {C(0xc694), 3},  // 요
                                 {C(' '), 1},     //
                                 {C(0xc138), 3},  // 세
                                 {C(0xacc4), 3},  // 계
                             },
                         }}));

INSTANTIATE_TEST_SUITE_P(Emoji,
                         UTF8Test,
                         ::testing::ValuesIn({UTF8Case{
                             // 👋🌎
                             {
                                 0xf0,
                                 0x9f,
                                 0x91,
                                 0x8b,
                                 0xf0,
                                 0x9f,
                                 0x8c,
                                 0x8e,
                             },
                             {
                                 {C(0x1f44b), 4},  // 👋
                                 {C(0x1f30e), 4},  // 🌎
                             },
                         }}));

INSTANTIATE_TEST_SUITE_P(Random,
                         UTF8Test,
                         ::testing::ValuesIn({UTF8Case{
                             // Øⓑꚫ쁹Ǵ𐌒岾🥍ⴵ㍨又ᮗ
                             {
                                 0xc3, 0x98, 0xe2, 0x93, 0x91, 0xea, 0x9a, 0xab, 0xec,
                                 0x81, 0xb9, 0xc7, 0xb4, 0xf0, 0x90, 0x8c, 0x92, 0xe5,
                                 0xb2, 0xbe, 0xf0, 0x9f, 0xa5, 0x8d, 0xe2, 0xb4, 0xb5,
                                 0xe3, 0x8d, 0xa8, 0xe5, 0x8f, 0x88, 0xe1, 0xae, 0x97,
                             },
                             {
                                 {C(0x000d8), 2},  // Ø
                                 {C(0x024d1), 3},  // ⓑ
                                 {C(0x0a6ab), 3},  // ꚫ
                                 {C(0x0c079), 3},  // 쁹
                                 {C(0x001f4), 2},  // Ǵ
                                 {C(0x10312), 4},  // 𐌒
                                 {C(0x05cbe), 3},  // 岾
                                 {C(0x1f94d), 4},  // 🥍
                                 {C(0x02d35), 3},  // ⴵ
                                 {C(0x03368), 3},  // ㍨
                                 {C(0x053c8), 3},  // 又
                                 {C(0x01b97), 3},  // ᮗ
                             },
                         }}));

////////////////////////////////////////////////////////////////////////////////
// DecodeUTF8 invalid tests
////////////////////////////////////////////////////////////////////////////////
class DecodeUTF8InvalidTest : public testing::TestWithParam<std::vector<uint8_t>> {};

TEST_P(DecodeUTF8InvalidTest, Invalid) {
    auto [code_point, width] = utf8::Decode(GetParam().data(), GetParam().size());
    EXPECT_EQ(code_point, CodePoint(0));
    EXPECT_EQ(width, 0u);
}

INSTANTIATE_TEST_SUITE_P(Invalid,
                         DecodeUTF8InvalidTest,
                         ::testing::ValuesIn(std::vector<std::vector<uint8_t>>{
                             {0x80, 0x80, 0x80, 0x80},  // 10000000
                             {0x81, 0x80, 0x80, 0x80},  // 10000001
                             {0x8f, 0x80, 0x80, 0x80},  // 10001111
                             {0x90, 0x80, 0x80, 0x80},  // 10010000
                             {0x91, 0x80, 0x80, 0x80},  // 10010001
                             {0x9f, 0x80, 0x80, 0x80},  // 10011111
                             {0xa0, 0x80, 0x80, 0x80},  // 10100000
                             {0xa1, 0x80, 0x80, 0x80},  // 10100001
                             {0xaf, 0x80, 0x80, 0x80},  // 10101111
                             {0xb0, 0x80, 0x80, 0x80},  // 10110000
                             {0xb1, 0x80, 0x80, 0x80},  // 10110001
                             {0xbf, 0x80, 0x80, 0x80},  // 10111111
                             {0xc0, 0x80, 0x80, 0x80},  // 11000000
                             {0xc1, 0x80, 0x80, 0x80},  // 11000001
                             {0xf5, 0x80, 0x80, 0x80},  // 11110101
                             {0xf6, 0x80, 0x80, 0x80},  // 11110110
                             {0xf7, 0x80, 0x80, 0x80},  // 11110111
                             {0xf8, 0x80, 0x80, 0x80},  // 11111000
                             {0xfe, 0x80, 0x80, 0x80},  // 11111110
                             {0xff, 0x80, 0x80, 0x80},  // 11111111

                             {0xd0},              // 2-bytes, missing second byte
                             {0xe8, 0x8f},        // 3-bytes, missing third byte
                             {0xf4, 0x8f, 0x8f},  // 4-bytes, missing fourth byte

                             {0xd0, 0x7f},              // 2-bytes, second byte's MSB unset
                             {0xe8, 0x7f, 0x8f},        // 3-bytes, second byte's MSB unset
                             {0xe8, 0x8f, 0x7f},        // 3-bytes, third byte's MSB unset
                             {0xf4, 0x7f, 0x8f, 0x8f},  // 4-bytes, second byte's MSB unset
                             {0xf4, 0x8f, 0x7f, 0x8f},  // 4-bytes, third byte's MSB unset
                             {0xf4, 0x8f, 0x8f, 0x7f},  // 4-bytes, fourth byte's MSB unset

                             {0xd0, 0xff},              // 2-bytes, second byte's second-MSB set
                             {0xe8, 0xff, 0x8f},        // 3-bytes, second byte's second-MSB set
                             {0xe8, 0x8f, 0xff},        // 3-bytes, third byte's second-MSB set
                             {0xf4, 0xff, 0x8f, 0x8f},  // 4-bytes, second byte's second-MSB set
                             {0xf4, 0x8f, 0xff, 0x8f},  // 4-bytes, third byte's second-MSB set
                             {0xf4, 0x8f, 0x8f, 0xff},  // 4-bytes, fourth byte's second-MSB set

                             {0xe0, 0x9d, 0x81},  // Value out of range for 3-byte
                         }));

}  // namespace utf8_tests

////////////////////////////////////////////////////////////////////////////////
// DecodeUTF16 valid tests
////////////////////////////////////////////////////////////////////////////////
namespace utf16_tests {

struct UTF16Case {
    std::vector<uint16_t> string;
    std::vector<CodePointAndWidth> code_points;
};

static std::ostream& operator<<(std::ostream& out, UTF16Case c) {
    for (size_t i = 0; i < c.string.size(); i++) {
        if (i > 0) {
            out << ", ";
        }
        out << "0x" << std::hex << std::setfill('0') << std::setw(4) << c.string[i];
    }
    return out;
}

class UTF16Test : public testing::TestWithParam<UTF16Case> {};

TEST_P(UTF16Test, Decode) {
    auto param = GetParam();

    const uint16_t* data = reinterpret_cast<const uint16_t*>(param.string.data());
    const size_t len = param.string.size();

    std::vector<CodePointAndWidth> got;
    size_t offset = 0;
    while (offset < len) {
        auto [code_point, width] = utf16::Decode(data + offset, len - offset);
        if (width == 0) {
            FAIL() << "Decode() failed at byte offset " << offset;
        }
        offset += width;
        got.emplace_back(CodePointAndWidth{code_point, width});
    }

    EXPECT_THAT(got, ::testing::ElementsAreArray(param.code_points));
}

TEST_P(UTF16Test, Encode) {
    auto param = GetParam();

    Slice<const uint16_t> str{reinterpret_cast<const uint16_t*>(param.string.data()),
                              param.string.size()};
    for (auto codepoint : param.code_points) {
        EXPECT_EQ(utf16::Encode(codepoint.code_point, nullptr), codepoint.width);

        uint16_t encoded[2];
        size_t len = utf16::Encode(codepoint.code_point, encoded);
        ASSERT_EQ(len, codepoint.width);
        EXPECT_THAT(Slice<const uint16_t>(encoded, len),
                    ::testing::ElementsAreArray(str.Truncate(len)));
        str = str.Offset(len);
    }
}

INSTANTIATE_TEST_SUITE_P(AsciiLetters,
                         UTF16Test,
                         ::testing::ValuesIn({
                             UTF16Case{{'a'}, {{C('a'), 1}}},
                             UTF16Case{{'a', 'b', 'c'}, {{C('a'), 1}, {C('b'), 1}, {C('c'), 1}}},
                             UTF16Case{{'d', 'e', 'f'}, {{C('d'), 1}, {C('e'), 1}, {C('f'), 1}}},
                             UTF16Case{{'g', 'h'}, {{C('g'), 1}, {C('h'), 1}}},
                             UTF16Case{{'i', 'j'}, {{C('i'), 1}, {C('j'), 1}}},
                             UTF16Case{{'k', 'l', 'm'}, {{C('k'), 1}, {C('l'), 1}, {C('m'), 1}}},
                             UTF16Case{{'n', 'o', 'p'}, {{C('n'), 1}, {C('o'), 1}, {C('p'), 1}}},
                             UTF16Case{{'q', 'r'}, {{C('q'), 1}, {C('r'), 1}}},
                             UTF16Case{{'s', 't', 'u'}, {{C('s'), 1}, {C('t'), 1}, {C('u'), 1}}},
                             UTF16Case{{'v', 'w'}, {{C('v'), 1}, {C('w'), 1}}},
                             UTF16Case{{'x', 'y', 'z'}, {{C('x'), 1}, {C('y'), 1}, {C('z'), 1}}},
                             UTF16Case{{'A'}, {{C('A'), 1}}},
                             UTF16Case{{'A', 'B', 'C'}, {{C('A'), 1}, {C('B'), 1}, {C('C'), 1}}},
                             UTF16Case{{'D', 'E', 'F'}, {{C('D'), 1}, {C('E'), 1}, {C('F'), 1}}},
                             UTF16Case{{'G', 'H'}, {{C('G'), 1}, {C('H'), 1}}},
                             UTF16Case{{'I', 'J'}, {{C('I'), 1}, {C('J'), 1}}},
                             UTF16Case{{'K', 'L', 'M'}, {{C('K'), 1}, {C('L'), 1}, {C('M'), 1}}},
                             UTF16Case{{'N', 'O', 'P'}, {{C('N'), 1}, {C('O'), 1}, {C('P'), 1}}},
                             UTF16Case{{'Q', 'R'}, {{C('Q'), 1}, {C('R'), 1}}},
                             UTF16Case{{'S', 'T', 'U'}, {{C('S'), 1}, {C('T'), 1}, {C('U'), 1}}},
                             UTF16Case{{'V', 'W'}, {{C('V'), 1}, {C('W'), 1}}},
                             UTF16Case{{'X', 'Y', 'Z'}, {{C('X'), 1}, {C('Y'), 1}, {C('Z'), 1}}},
                         }));

INSTANTIATE_TEST_SUITE_P(AsciiNumbers,
                         UTF16Test,
                         ::testing::ValuesIn({
                             UTF16Case{{'0', '1', '2'}, {{C('0'), 1}, {C('1'), 1}, {C('2'), 1}}},
                             UTF16Case{{'3', '4', '5'}, {{C('3'), 1}, {C('4'), 1}, {C('5'), 1}}},
                             UTF16Case{{'6', '7', '8'}, {{C('6'), 1}, {C('7'), 1}, {C('8'), 1}}},
                             UTF16Case{{'9'}, {{C('9'), 1}}},
                         }));

INSTANTIATE_TEST_SUITE_P(AsciiSymbols,
                         UTF16Test,
                         ::testing::ValuesIn({
                             UTF16Case{{'!', '"', '#'}, {{C('!'), 1}, {C('"'), 1}, {C('#'), 1}}},
                             UTF16Case{{'$', '%', '&'}, {{C('$'), 1}, {C('%'), 1}, {C('&'), 1}}},
                             UTF16Case{{'\'', '(', ')'}, {{C('\''), 1}, {C('('), 1}, {C(')'), 1}}},
                             UTF16Case{{'*', ',', '-'}, {{C('*'), 1}, {C(','), 1}, {C('-'), 1}}},
                             UTF16Case{{'/', '`', '@'}, {{C('/'), 1}, {C('`'), 1}, {C('@'), 1}}},
                             UTF16Case{{'^', '\\', '['}, {{C('^'), 1}, {C('\\'), 1}, {C('['), 1}}},
                             UTF16Case{{']', '_', '|'}, {{C(']'), 1}, {C('_'), 1}, {C('|'), 1}}},
                             UTF16Case{{'{', '}'}, {{C('{'), 1}, {C('}'), 1}}},
                         }));

INSTANTIATE_TEST_SUITE_P(
    AsciiSpecial,
    UTF16Test,
    ::testing::ValuesIn({
        UTF16Case{{}, {}},
        UTF16Case{{' ', '\t', '\n'}, {{C(' '), 1}, {C('\t'), 1}, {C('\n'), 1}}},
        UTF16Case{{'\a', '\b', '\f'}, {{C('\a'), 1}, {C('\b'), 1}, {C('\f'), 1}}},
        UTF16Case{{'\n', '\r', '\t'}, {{C('\n'), 1}, {C('\r'), 1}, {C('\t'), 1}}},
        UTF16Case{{'\v'}, {{C('\v'), 1}}},
    }));

INSTANTIATE_TEST_SUITE_P(Hindi,
                         UTF16Test,
                         ::testing::ValuesIn({UTF16Case{
                             // नमस्ते दुनिया
                             {
                                 0x0928,
                                 0x092e,
                                 0x0938,
                                 0x094d,
                                 0x0924,
                                 0x0947,
                                 0x0020,
                                 0x0926,
                                 0x0941,
                                 0x0928,
                                 0x093f,
                                 0x092f,
                                 0x093e,
                             },
                             {
                                 {C(0x0928), 1},  // न
                                 {C(0x092e), 1},  // म
                                 {C(0x0938), 1},  // स
                                 {C(0x094d), 1},  // ् //
                                 {C(0x0924), 1},  // त
                                 {C(0x0947), 1},  // े //
                                 {C(' '), 1},
                                 {C(0x0926), 1},  // द
                                 {C(0x0941), 1},  // ु //
                                 {C(0x0928), 1},  // न
                                 {C(0x093f), 1},  // ि //
                                 {C(0x092f), 1},  // य
                                 {C(0x093e), 1},  // ा //
                             },
                         }}));

INSTANTIATE_TEST_SUITE_P(Mandarin,
                         UTF16Test,
                         ::testing::ValuesIn({UTF16Case{
                             // 你好世界
                             {0x4f60, 0x597d, 0x4e16, 0x754c},
                             {
                                 {C(0x4f60), 1},  // 你
                                 {C(0x597d), 1},  // 好
                                 {C(0x4e16), 1},  // 世
                                 {C(0x754c), 1},  // 界
                             },
                         }}));

INSTANTIATE_TEST_SUITE_P(Japanese,
                         UTF16Test,
                         ::testing::ValuesIn({UTF16Case{
                             // こんにちは世界
                             {
                                 0x3053,
                                 0x3093,
                                 0x306b,
                                 0x3061,
                                 0x306f,
                                 0x4e16,
                                 0x754c,
                             },
                             {
                                 {C(0x3053), 1},  // こ
                                 {C(0x3093), 1},  // ん
                                 {C(0x306B), 1},  // に
                                 {C(0x3061), 1},  // ち
                                 {C(0x306F), 1},  // は
                                 {C(0x4E16), 1},  // 世
                                 {C(0x754C), 1},  // 界
                             },
                         }}));

INSTANTIATE_TEST_SUITE_P(Korean,
                         UTF16Test,
                         ::testing::ValuesIn({UTF16Case{
                             // 안녕하세요 세계
                             {
                                 0xc548,
                                 0xb155,
                                 0xd558,
                                 0xc138,
                                 0xc694,
                                 0x0020,
                                 0xc138,
                                 0xacc4,
                             },
                             {
                                 {C(0xc548), 1},  // 안
                                 {C(0xb155), 1},  // 녕
                                 {C(0xd558), 1},  // 하
                                 {C(0xc138), 1},  // 세
                                 {C(0xc694), 1},  // 요
                                 {C(' '), 1},     //
                                 {C(0xc138), 1},  // 세
                                 {C(0xacc4), 1},  // 계
                             },
                         }}));

INSTANTIATE_TEST_SUITE_P(Emoji,
                         UTF16Test,
                         ::testing::ValuesIn({UTF16Case{
                             // 👋🌎
                             {0xd83d, 0xdc4b, 0xd83c, 0xdf0e},
                             {
                                 {C(0x1f44b), 2},  // 👋
                                 {C(0x1f30e), 2},  // 🌎
                             },
                         }}));

INSTANTIATE_TEST_SUITE_P(Random,
                         UTF16Test,
                         ::testing::ValuesIn({UTF16Case{
                             // Øⓑꚫ쁹Ǵ𐌒岾🥍ⴵ㍨又ᮗ
                             {
                                 0x00d8,
                                 0x24d1,
                                 0xa6ab,
                                 0xc079,
                                 0x01f4,
                                 0xd800,
                                 0xdf12,
                                 0x5cbe,
                                 0xd83e,
                                 0xdd4d,
                                 0x2d35,
                                 0x3368,
                                 0x53c8,
                                 0x1b97,
                             },
                             {
                                 {C(0x000d8), 1},  // Ø
                                 {C(0x024d1), 1},  // ⓑ
                                 {C(0x0a6ab), 1},  // ꚫ
                                 {C(0x0c079), 1},  // 쁹
                                 {C(0x001f4), 1},  // Ǵ
                                 {C(0x10312), 2},  // 𐌒
                                 {C(0x05cbe), 1},  // 岾
                                 {C(0x1f94d), 2},  // 🥍
                                 {C(0x02d35), 1},  // ⴵ
                                 {C(0x03368), 1},  // ㍨
                                 {C(0x053c8), 1},  // 又
                                 {C(0x01b97), 1},  // ᮗ
                             },
                         }}));

////////////////////////////////////////////////////////////////////////////////
// DecodeUTF16 invalid tests
////////////////////////////////////////////////////////////////////////////////
class DecodeUTF16InvalidTest : public testing::TestWithParam<std::vector<uint16_t>> {};

TEST_P(DecodeUTF16InvalidTest, Invalid) {
    auto [code_point, width] = utf16::Decode(GetParam().data(), GetParam().size());
    EXPECT_EQ(code_point, CodePoint(0));
    EXPECT_EQ(width, 0u);
}
INSTANTIATE_TEST_SUITE_P(Invalid,
                         DecodeUTF16InvalidTest,
                         ::testing::ValuesIn(std::vector<std::vector<uint16_t>>{
                             {0xdc00},          // surrogate, end-of-stream
                             {0xdc00, 0x0040},  // surrogate, non-surrogate
                         }));

}  // namespace utf16_tests
}  // namespace tint

TINT_END_DISABLE_WARNING(UNSAFE_BUFFER_USAGE);
