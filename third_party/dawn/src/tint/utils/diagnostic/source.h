
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

#ifndef SRC_TINT_UTILS_DIAGNOSTIC_SOURCE_H_
#define SRC_TINT_UTILS_DIAGNOSTIC_SOURCE_H_

#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include "src/tint/utils/rtti/traits.h"
#include "src/tint/utils/text/string_stream.h"

namespace tint {

/// Source describes a range of characters within a source file.
class Source {
  public:
    /// FileContent describes the content of a source file encoded using UTF-8.
    class FileContent {
      public:
        /// Constructs the FileContent with the given file content.
        /// @param data the file contents
        explicit FileContent(std::string_view data);

        /// Copy constructor
        /// @param rhs the FileContent to copy
        FileContent(const FileContent& rhs);

        /// Destructor
        ~FileContent();

        /// The original un-split file content
        const std::string data;
        /// #data split by lines
        const std::vector<std::string_view> lines;
    };

    /// File describes a source file, including path and content.
    class File {
      public:
        /// Constructs the File with the given file path and content.
        /// @param p the path for this file
        /// @param c the file contents
        inline File(const std::string& p, std::string_view c) : path(p), content(c) {}

        /// Copy constructor
        File(const File&) = default;

        /// Move constructor
        File(File&&) = default;

        /// Destructor
        ~File();

        /// file path
        const std::string path;
        /// file content
        const FileContent content;
    };

    /// Location holds a 1-based line and column index.
    class Location {
      public:
        /// the 1-based line number. 0 represents no line information.
        uint32_t line = 0;
        /// the 1-based column number in utf8-code units (bytes).
        /// 0 represents no column information.
        uint32_t column = 0;

        /// Returns true if `this` location is lexicographically less than `rhs`
        /// @param rhs location to compare against
        /// @returns true if `this` < `rhs`
        inline bool operator<(const Source::Location& rhs) const {
            return std::tie(line, column) < std::tie(rhs.line, rhs.column);
        }

        /// Returns true if `this` location is lexicographically less than or equal to `rhs`
        /// @param rhs location to compare against
        /// @returns true if `this` <= `rhs`
        inline bool operator<=(const Source::Location& rhs) const {
            return std::tie(line, column) <= std::tie(rhs.line, rhs.column);
        }

        /// Returns true if `this` location is lexicographically greater than `rhs`
        /// @param rhs location to compare against
        /// @returns true if `this` > `rhs`
        inline bool operator>(const Source::Location& rhs) const {
            return std::tie(line, column) > std::tie(rhs.line, rhs.column);
        }

        /// Returns true if `this` location is lexicographically greater than or equal to `rhs`
        /// @param rhs location to compare against
        /// @returns true if `this` >= `rhs`
        inline bool operator>=(const Source::Location& rhs) const {
            return std::tie(line, column) >= std::tie(rhs.line, rhs.column);
        }

        /// Returns true of `this` location is equal to `rhs`
        /// @param rhs location to compare against
        /// @returns true if `this` == `rhs`
        inline bool operator==(const Location& rhs) const {
            return line == rhs.line && column == rhs.column;
        }

        /// Returns true of `this` location is not equal to `rhs`
        /// @param rhs location to compare against
        /// @returns true if `this` != `rhs`
        inline bool operator!=(const Location& rhs) const { return !(*this == rhs); }
    };

    /// Range holds a Location interval described by [begin, end).
    class Range {
      public:
        /// Constructs a zero initialized Range.
        inline Range() = default;

        /// Constructs a zero-length Range starting at `loc`
        /// @param loc the start and end location for the range
        inline constexpr explicit Range(const Location& loc) : begin(loc), end(loc) {}

        /// Constructs the Range beginning at `b` and ending at `e`
        /// @param b the range start location
        /// @param e the range end location
        inline constexpr Range(const Location& b, const Location& e) : begin(b), end(e) {}

        /// Return a column-shifted Range
        /// @param n the number of UTF-8 codepoint to shift by
        /// @returns a Range with a #begin and #end column shifted by `n`
        inline Range operator+(uint32_t n) const {
            return Range{{begin.line, begin.column + n}, {end.line, end.column + n}};
        }

        /// Returns true of `this` range is not equal to `rhs`
        /// @param rhs range to compare against
        /// @returns true if `this` != `rhs`
        inline bool operator==(const Range& rhs) const {
            return begin == rhs.begin && end == rhs.end;
        }

        /// Returns true of `this` range is equal to `rhs`
        /// @param rhs range to compare against
        /// @returns true if `this` == `rhs`
        inline bool operator!=(const Range& rhs) const { return !(*this == rhs); }

        /// @param content the file content that this range belongs to
        /// @returns the length of the range in UTF-8 codepoints, treating all line-break sequences
        /// as a single code-point.
        /// @see https://www.w3.org/TR/WGSL/#blankspace-and-line-breaks for the definition of a line
        /// break.
        size_t Length(const FileContent& content) const;

        /// The location of the first UTF-8 codepoint in the range.
        Location begin;
        /// The location of one-past the last UTF-8 codepoint in the range.
        Location end;
    };

    /// Constructs the Source with an zero initialized Range and null File.
    inline Source() : range() {}

    /// Constructs the Source with the Range `rng` and a null File
    /// @param rng the source range
    inline explicit Source(const Range& rng) : range(rng) {}

    /// Constructs the Source with the Range `loc` and a null File
    /// @param loc the start and end location for the source range
    inline explicit Source(const Location& loc) : range(Range(loc)) {}

    /// Constructs the Source with the Range `rng` and File `file`
    /// @param rng the source range
    /// @param f the source file
    inline Source(const Range& rng, File const* f) : range(rng), file(f) {}

    /// @returns a Source that points to the begin range of this Source.
    inline Source Begin() const { return Source(Range{range.begin}, file); }

    /// @returns a Source that points to the end range of this Source.
    inline Source End() const { return Source(Range{range.end}, file); }

    /// Return a column-shifted Source
    /// @param n the number of characters to shift by
    /// @returns a Source with the range's columns shifted by `n`
    inline Source operator+(uint32_t n) const { return Source(range + n, file); }

    /// Returns true of `this` Source is lexicographically less than `rhs`
    /// @param rhs source to compare against
    /// @returns true if `this` < `rhs`
    inline bool operator<(const Source& rhs) {
        if (file != rhs.file) {
            return false;
        }
        return range.begin < rhs.range.begin;
    }

    /// Helper function that returns the range union of two source locations. The
    /// `start` and `end` locations are assumed to refer to the same source file.
    /// @param start the start source of the range
    /// @param end the end source of the range
    /// @returns the combined source
    inline static Source Combine(const Source& start, const Source& end) {
        return Source(Source::Range(start.range.begin, end.range.end), start.file);
    }

    /// range is the span of text this source refers to in #file
    Range range;
    /// file is the optional source content this source refers to
    const File* file = nullptr;
};

/// @param source the input Source
/// @returns a string that describes given source location
std::string ToString(const Source& source);

/// Writes the Source::Location to the stream.
/// @param out the stream to write to
/// @param loc the location to write
/// @returns out so calls can be chained
template <typename STREAM, typename = traits::EnableIfIsOStream<STREAM>>
auto& operator<<(STREAM& out, const Source::Location& loc) {
    return out << loc.line << ":" << loc.column;
}

/// Writes the Source::Range to the stream.
/// @param out the stream to write to
/// @param range the range to write
/// @returns out so calls can be chained
template <typename STREAM, typename = traits::EnableIfIsOStream<STREAM>>
auto& operator<<(STREAM& out, const Source::Range& range) {
    return out << "[" << range.begin << ", " << range.end << "]";
}

/// Writes the Source to the stream.
/// @param out the stream to write to
/// @param source the source to write
/// @returns out so calls can be chained
template <typename STREAM, typename = traits::EnableIfIsOStream<STREAM>>
auto& operator<<(STREAM& out, const Source& source) {
    return out << ToString(source);
}

/// Writes the Source::FileContent to the stream.
/// @param out the stream to write to
/// @param content the file content to write
/// @returns out so calls can be chained
template <typename STREAM, typename = traits::EnableIfIsOStream<STREAM>>
auto& operator<<(STREAM& out, const Source::FileContent& content) {
    return out << content.data;
}

}  // namespace tint

#endif  // SRC_TINT_UTILS_DIAGNOSTIC_SOURCE_H_
