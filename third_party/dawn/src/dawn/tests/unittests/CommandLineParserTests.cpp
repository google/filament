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

#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "absl/strings/str_split.h"
#include "dawn/utils/CommandLineParser.h"
#include "gtest/gtest.h"

namespace dawn {
namespace {

using CLP = utils::CommandLineParser;

std::vector<std::string_view> Split(const char* s) {
    return absl::StrSplit(s, ' ');
}

void ExpectSuccess(const CLP::ParseResult& result) {
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.errorMessage, "");
}

void ExpectError(const CLP::ParseResult& result, std::string_view message) {
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.errorMessage, message);
}

// Tests for BoolOption parsing
TEST(CommandLineParserTest, BoolParsing) {
    // Test parsing with nothing afterwards.
    {
        CLP opts;
        auto& opt = opts.AddBool("foo").ShortName('f');
        ExpectSuccess(opts.Parse({Split("-f")}));

        EXPECT_TRUE(opt.GetValue());
    }

    // Test parsing with another flag afterwards.
    {
        CLP opts;
        auto& opt = opts.AddBool("foo").ShortName('f');
        auto& optB = opts.AddBool("bar").ShortName('b');
        ExpectSuccess(opts.Parse(Split("-f -b")));

        EXPECT_TRUE(opt.GetValue());
        EXPECT_TRUE(optB.IsSet());
    }

    // Test parsing with garbage afterwards.
    {
        CLP opts;
        auto& opt = opts.AddBool("foo").ShortName('f');
        ExpectSuccess(opts.Parse(Split("-f garbage"), {.unknownIsError = false}));

        EXPECT_TRUE(opt.GetValue());
    }

    // Test parsing "true"
    {
        CLP opts;
        auto& opt = opts.AddBool("foo").ShortName('f');
        auto& optB = opts.AddBool("bar").ShortName('b');
        ExpectSuccess(opts.Parse(Split("-f true -b")));

        EXPECT_TRUE(opt.GetValue());
        EXPECT_TRUE(optB.IsSet());
    }

    // Test parsing "false"
    {
        CLP opts;
        auto& opt = opts.AddBool("foo").ShortName('f');
        auto& optB = opts.AddBool("bar").ShortName('b');
        ExpectSuccess(opts.Parse(Split("-f false -b")));

        EXPECT_FALSE(opt.GetValue());
        EXPECT_TRUE(optB.IsSet());
    }

    // Test parsing the option multiple times, with an explicit true argument.
    {
        CLP opts;
        opts.AddBool("foo").ShortName('f');
        ExpectError(opts.Parse({Split("-f --foo true")}),
                    "Failure while parsing \"foo\": cannot set multiple times with explicit "
                    "true/false arguments");
    }

    // Test parsing the option multiple times, with an explicit false argument.
    {
        CLP opts;
        opts.AddBool("foo").ShortName('f');
        ExpectError(opts.Parse({Split("-f --foo false")}),
                    "Failure while parsing \"foo\": cannot set multiple times with explicit "
                    "true/false arguments");
    }

    // Test parsing the option multiple times, with the implicit true argument.
    {
        CLP opts;
        auto& opt = opts.AddBool("foo").ShortName('f');
        ExpectSuccess(opts.Parse({Split("-f -f")}));

        EXPECT_TRUE(opt.GetValue());
    }

    // Test parsing the option multiple times, with the implicit true argument but conflicting
    // values
    {
        CLP opts;
        opts.AddBool("foo").ShortName('f');
        ExpectError(opts.Parse({Split("-f false --foo")}),
                    "Failure while parsing \"foo\": cannot be set to both true and false");
    }

    // Test the default value
    {
        CLP opts;
        auto& opt = opts.AddBool("foo");
        ExpectSuccess(opts.Parse(0, nullptr));

        EXPECT_FALSE(opt.IsSet());
        EXPECT_FALSE(opt.GetValue());
    }
}

// Tests for StringOption parsing.
TEST(CommandLineParserTest, StringParsing) {
    // Test with nothing afterwards
    {
        CLP opts;
        opts.AddString("foo").ShortName('f');
        ExpectError(opts.Parse({Split("-f")}), "Failure while parsing \"foo\": expected a value");
    }

    // Test parsing with another flag afterwards.
    {
        CLP opts;
        auto& opt = opts.AddString("foo").ShortName('f');
        auto& optB = opts.AddBool("bar").ShortName('b');
        ExpectSuccess(opts.Parse(Split("-f -b")));

        EXPECT_EQ(opt.GetValue(), "-b");
        EXPECT_FALSE(optB.IsSet());
    }

    // Test parsing with some data afterwards
    {
        CLP opts;
        auto& opt = opts.AddString("foo").ShortName('f');
        ExpectSuccess(opts.Parse(Split("-f supercalifragilisticexpialidocious")));

        EXPECT_EQ(opt.GetValue(), "supercalifragilisticexpialidocious");
    }

    // Test setting multiple times
    {
        CLP opts;
        opts.AddString("foo").ShortName('f');
        ExpectError(opts.Parse({Split("-f aa -f aa")}),
                    "Failure while parsing \"foo\": cannot be set multiple times");
    }

    // Test the default value
    {
        CLP opts;
        auto& opt = opts.AddString("foo");
        ExpectSuccess(opts.Parse(0, nullptr));

        EXPECT_FALSE(opt.IsSet());
        EXPECT_TRUE(opt.GetValue().empty());
    }
}

// Tests for StringListOption parsing.
TEST(CommandLineParserTest, StringListParsing) {
    // Test with nothing afterwards
    {
        CLP opts;
        opts.AddStringList("foo").ShortName('f');
        ExpectError(opts.Parse({Split("-f")}), "Failure while parsing \"foo\": expected a value");
    }

    // Test parsing with another flag afterwards.
    {
        CLP opts;
        auto& opt = opts.AddStringList("foo").ShortName('f');
        auto& optB = opts.AddBool("bar").ShortName('b');
        ExpectSuccess(opts.Parse(Split("-f -b")));

        EXPECT_EQ(opt.GetValue().size(), 1u);
        EXPECT_EQ(opt.GetValue()[0], "-b");
        EXPECT_FALSE(optB.IsSet());
    }

    // Test parsing with some data afterwards
    {
        CLP opts;
        auto& opt = opts.AddStringList("foo").ShortName('f');
        ExpectSuccess(opts.Parse(Split("-f sugar,butter,flour")));

        EXPECT_EQ(opt.GetValue().size(), 3u);
        EXPECT_EQ(opt.GetValue()[0], "sugar");
        EXPECT_EQ(opt.GetValue()[1], "butter");
        EXPECT_EQ(opt.GetValue()[2], "flour");
    }

    // Test passing the option multiple times, it should add to the list.
    {
        CLP opts;
        auto& opt = opts.AddStringList("foo").ShortName('f');
        ExpectSuccess(opts.Parse(Split("-f sugar -foo butter,flour")));

        EXPECT_EQ(opt.GetValue().size(), 3u);
        EXPECT_EQ(opt.GetValue()[0], "sugar");
        EXPECT_EQ(opt.GetValue()[1], "butter");
        EXPECT_EQ(opt.GetValue()[2], "flour");
    }

    // Test the default value
    {
        CLP opts;
        auto& opt = opts.AddStringList("foo");
        ExpectSuccess(opts.Parse(0, nullptr));

        EXPECT_FALSE(opt.IsSet());
        EXPECT_TRUE(opt.GetValue().empty());
    }
}

// Tests for EnumOption parsing.
enum class Cell {
    Pop,
    Six,
    Squish,
    Uhuh,
    Cicero,
    Lipschitz,
};
TEST(CommandLineParserTest, EnumParsing) {
    std::vector<std::pair<std::string_view, Cell>> conversions = {{
        {"pop", Cell::Pop}, {"six", Cell::Six}, {"uh-uh", Cell::Uhuh},
        // others left as an exercise to the reader.
    }};

    // Test with nothing afterwards
    {
        CLP opts;
        opts.AddEnum<Cell>(conversions, "foo").ShortName('f');
        ExpectError(opts.Parse({Split("-f")}), "Failure while parsing \"foo\": expected a value");
    }

    // Test parsing with another flag afterwards.
    {
        CLP opts;
        opts.AddEnum<Cell>(conversions, "foo").ShortName('f');
        opts.AddBool("bar").ShortName('b');
        ExpectError(opts.Parse({Split("-f -b")}),
                    "Failure while parsing \"foo\": unknown value \"-b\". Expected one of pop, "
                    "six, uh-uh.");
    }

    // Test parsing a correct enum value.
    {
        CLP opts;
        auto& opt = opts.AddEnum<Cell>(conversions, "foo").ShortName('f');
        auto& optB = opts.AddBool("bar").ShortName('b');
        ExpectSuccess(opts.Parse({Split("-f six -b")}));

        EXPECT_EQ(opt.GetValue(), Cell::Six);
        EXPECT_TRUE(optB.IsSet());
    }

    // Test setting multiple times
    {
        CLP opts;
        opts.AddEnum<Cell>(conversions, "foo").ShortName('f');
        ExpectError(opts.Parse({Split("-f six -f six")}),
                    "Failure while parsing \"foo\": cannot be set multiple times");
    }

    // Test the default value
    {
        CLP opts;
        auto& opt = opts.AddEnum<Cell>(conversions, "foo").Default(Cell::Uhuh);
        ExpectSuccess(opts.Parse(0, nullptr));

        EXPECT_FALSE(opt.IsSet());
        EXPECT_EQ(opt.GetValue(), Cell::Uhuh);
    }
}

// Various tests for the handling of long and short names.
TEST(CommandLineParserTest, LongAndShortNames) {
    // An option can be referenced by both a long and short name.
    {
        CLP opts;
        auto& opt = opts.AddStringList("foo").ShortName('f');
        ExpectSuccess(opts.Parse(Split("-f sugar -foo butter,flour")));

        EXPECT_EQ(opt.GetValue().size(), 3u);
        EXPECT_EQ(opt.GetValue()[0], "sugar");
        EXPECT_EQ(opt.GetValue()[1], "butter");
        EXPECT_EQ(opt.GetValue()[2], "flour");
    }

    // An option without a short name cannot be referenced with it.
    {
        CLP opts;
        opts.AddStringList("foo");
        ExpectError(opts.Parse(Split("-f sugar -foo butter,flour")), "Unknown option \"f\"");
    }

    // Regression test for two options having no short name.
    {
        CLP opts;
        opts.AddStringList("foo");
        opts.AddStringList("bar");
        ExpectSuccess(opts.Parse(0, nullptr));
    }
}

// Tests for option names not being recognized.
TEST(CommandLineParserTest, UnknownOption) {
    // An empty arg is not a known option.
    {
        CLP opts;
        ExpectError(opts.Parse(Split("")), "Unknown option \"\"");
        ExpectSuccess(opts.Parse(Split(""), {.unknownIsError = false}));
    }

    // A - is not a known option.
    {
        CLP opts;
        ExpectError(opts.Parse(Split("-")), "Unknown option \"\"");
        ExpectSuccess(opts.Parse(Split("-"), {.unknownIsError = false}));
    }

    // A -- is not a known option.
    {
        CLP opts;
        ExpectError(opts.Parse(Split("--")), "Unknown option \"\"");
        ExpectSuccess(opts.Parse(Split("--"), {.unknownIsError = false}));
    }

    // An unknown short name option.
    {
        CLP opts;
        ExpectError(opts.Parse(Split("-f")), "Unknown option \"f\"");
        ExpectError(opts.Parse(Split("-f=")), "Unknown option \"f\"");
        ExpectSuccess(opts.Parse(Split("-f"), {.unknownIsError = false}));
        ExpectSuccess(opts.Parse(Split("-f="), {.unknownIsError = false}));
    }

    // An unknown long name option.
    {
        CLP opts;
        ExpectError(opts.Parse(Split("-foo")), "Unknown option \"foo\"");
        ExpectError(opts.Parse(Split("-foo=")), "Unknown option \"foo\"");
        ExpectSuccess(opts.Parse(Split("-foo"), {.unknownIsError = false}));
        ExpectSuccess(opts.Parse(Split("-foo="), {.unknownIsError = false}));
    }
}

// Tests for options being set with =
TEST(CommandLineParserTest, EqualSeparator) {
    // Test that using an = separator works and lets other arguments be consumed.
    {
        CLP opts;
        auto& opt = opts.AddStringList("foo").ShortName('f');
        ExpectSuccess(opts.Parse(Split("-f=sugar -foo butter,flour")));

        EXPECT_EQ(opt.GetValue().size(), 3u);
        EXPECT_EQ(opt.GetValue()[0], "sugar");
        EXPECT_EQ(opt.GetValue()[1], "butter");
        EXPECT_EQ(opt.GetValue()[2], "flour");
    }

    // Test that if the part after the = is not consumed there is an error.
    {
        CLP opts;
        opts.AddBool("foo").ShortName('f');
        ExpectError(opts.Parse({Split("-f=garbage")}),
                    "Argument \"garbage\" was not valid for option \"foo\"");
    }
}

// Test that the argc/argv version skips the command name.
TEST(CommandLineParserTest, ArgvArgcSkipCommandName) {
    CLP opts;
    auto& optA = opts.AddBool("a");
    auto& optB = opts.AddBool("b");
    auto& optC = opts.AddBool("c");

    const char* argv[] = {"-a", "-b", "-c"};
    ExpectSuccess(opts.Parse(3, argv));
    ASSERT_FALSE(optA.IsSet());
    ASSERT_TRUE(optB.IsSet());
    ASSERT_TRUE(optC.IsSet());
}

// Tests for the generation of the help strings for the options.
TEST(CommandLineParserTest, PrintHelp) {
    // A test with a few options, checks that they are sorted.
    {
        CLP opts;
        opts.AddString("foo");
        opts.AddString("bar");
        opts.AddString("baz");

        std::stringstream s;
        opts.PrintHelp(s);
        EXPECT_EQ(s.str(), R"(--bar <value>
--baz <value>
--foo <value>
)");
    }

    // A test with a custom parameter name.
    {
        CLP opts;
        opts.AddString("foo").Parameter("bar");

        std::stringstream s;
        opts.PrintHelp(s);
        EXPECT_EQ(s.str(), R"(--foo <bar>
)");
    }

    // A test with a boolean value that doesn't get its parameter printed.
    {
        CLP opts;
        opts.AddBool("foo", "enable fooing");

        std::stringstream s;
        opts.PrintHelp(s);
        EXPECT_EQ(s.str(), R"(--foo  enable fooing
)");
    }

    // A test for the parameter of enum options.
    {
        std::vector<std::pair<std::string_view, Cell>> conversions = {{
            {"pop", Cell::Pop},
            {"six", Cell::Six},
            {"uh-uh", Cell::Uhuh},
        }};

        CLP opts;
        opts.AddEnum(conversions, "cell", "which story to get");

        std::stringstream s;
        opts.PrintHelp(s);
        EXPECT_EQ(s.str(), R"(--cell <pop|six|uh-uh>  which story to get
)");
    }

    // A test for short name handling.
    {
        CLP opts;
        opts.AddString("foo").ShortName('f');

        std::stringstream s;
        opts.PrintHelp(s);
        EXPECT_EQ(s.str(), R"(--foo <value>
-f  alias for --foo
)");
    }
}

// Tests that AddHelp() add the correct option.
TEST(CommandLineParserTest, HelpOption) {
    CLP opts;
    CLP::BoolOption& helpOpt = opts.AddHelp();

    std::stringstream s;
    opts.PrintHelp(s);
    EXPECT_EQ(s.str(), R"(--help  Shows the help
-h  alias for --help
)");

    ExpectSuccess(opts.Parse(Split("-h")));
    EXPECT_TRUE(helpOpt.GetValue());
}

}  // anonymous namespace
}  // namespace dawn
