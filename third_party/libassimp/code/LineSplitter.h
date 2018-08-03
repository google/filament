/*
Open Asset Import Library (assimp)
----------------------------------------------------------------------

Copyright (c) 2006-2017, assimp team

All rights reserved.

Redistribution and use of this software in source and binary forms,
with or without modification, are permitted provided that the
following conditions are met:

* Redistributions of source code must retain the above
  copyright notice, this list of conditions and the
  following disclaimer.

* Redistributions in binary form must reproduce the above
  copyright notice, this list of conditions and the
  following disclaimer in the documentation and/or other
  materials provided with the distribution.

* Neither the name of the assimp team, nor the names of its
  contributors may be used to endorse or promote products
  derived from this software without specific prior
  written permission of the assimp team.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

----------------------------------------------------------------------
*/

/** @file  LineSplitter.h
 *  @brief LineSplitter, a helper class to iterate through all lines
 *    of a file easily. Works with StreamReader.
 */
#ifndef INCLUDED_LINE_SPLITTER_H
#define INCLUDED_LINE_SPLITTER_H

#include <stdexcept>

#include "StreamReader.h"
#include "ParsingUtils.h"

namespace Assimp {

// ------------------------------------------------------------------------------------------------
/** Usage:
@code
for(LineSplitter splitter(stream);splitter;++splitter) {

    if (*splitter == "hi!") {
       ...
    }
    else if (splitter->substr(0,5) == "hello") {
       ...
       // access the third token in the line (tokens are space-separated)
       if (strtol(splitter[2]) > 5) { .. }
    }

    std::cout << "Current line is: " << splitter.get_index() << std::endl;
}
@endcode
*/
// ------------------------------------------------------------------------------------------------
class LineSplitter {
public:
    typedef size_t line_idx;

    // -----------------------------------------
    /** construct from existing stream reader
    note: trim is *always* assumed true if skyp_empty_lines==true
    */
    LineSplitter(StreamReaderLE& stream, bool skip_empty_lines = true, bool trim = true)
    : idx( 0 )
    , stream(stream)
    , swallow()
    , skip_empty_lines(skip_empty_lines)
    , trim(trim) {
        cur.reserve(1024);
        operator++();

        idx = 0;
    }

    ~LineSplitter() {
        // empty
    }

public:

    // -----------------------------------------
    /** pseudo-iterator increment */
    LineSplitter& operator++() {
        if(swallow) {
            swallow = false;
            return *this;
        }
        if (!*this) {
            throw std::logic_error("End of file, no more lines to be retrieved.");
        }
        char s;
        cur.clear();
        while(stream.GetRemainingSize() && (s = stream.GetI1(),1)) {
            if (s == '\n' || s == '\r') {
                if (skip_empty_lines) {
                    while (stream.GetRemainingSize() && ((s = stream.GetI1()) == ' ' || s == '\r' || s == '\n'));
                    if (stream.GetRemainingSize()) {
                        stream.IncPtr(-1);
                    }
                }
                else {
                    // skip both potential line terminators but don't read past this line.
                    if (stream.GetRemainingSize() && (s == '\r' && stream.GetI1() != '\n')) {
                        stream.IncPtr(-1);
                    }
                    if (trim) {
                        while (stream.GetRemainingSize() && ((s = stream.GetI1()) == ' ' || s == '\t'));
                        if (stream.GetRemainingSize()) {
                            stream.IncPtr(-1);
                        }
                    }
                }
                break;
            }
            cur += s;
        }
        ++idx;
        return *this;
    }

    // -----------------------------------------
    LineSplitter& operator++(int) {
        return ++(*this);
    }

    // -----------------------------------------
    /** get a pointer to the beginning of a particular token */
    const char* operator[] (size_t idx) const {
        const char* s = operator->()->c_str();

        SkipSpaces(&s);
        for(size_t i = 0; i < idx; ++i) {

            for(;!IsSpace(*s); ++s) {
                if(IsLineEnd(*s)) {
                    throw std::range_error("Token index out of range, EOL reached");
                }
            }
            SkipSpaces(&s);
        }
        return s;
    }

    // -----------------------------------------
    /** extract the start positions of N tokens from the current line*/
    template <size_t N>
    void get_tokens(const char* (&tokens)[N]) const {
        const char* s = operator->()->c_str();

        SkipSpaces(&s);
        for(size_t i = 0; i < N; ++i) {
            if(IsLineEnd(*s)) {

                throw std::range_error("Token count out of range, EOL reached");

            }
            tokens[i] = s;

            for(;*s && !IsSpace(*s); ++s);
            SkipSpaces(&s);
        }
    }

    // -----------------------------------------
    /** member access */
    const std::string* operator -> () const {
        return &cur;
    }

    std::string operator* () const {
        return cur;
    }

    // -----------------------------------------
    /** boolean context */
    operator bool() const {
        return stream.GetRemainingSize()>0;
    }

    // -----------------------------------------
    /** line indices are zero-based, empty lines are included */
    operator line_idx() const {
        return idx;
    }

    line_idx get_index() const {
        return idx;
    }

    // -----------------------------------------
    /** access the underlying stream object */
    StreamReaderLE& get_stream() {
        return stream;
    }

    // -----------------------------------------
    /** !strcmp((*this)->substr(0,strlen(check)),check) */
    bool match_start(const char* check) {
        const size_t len = strlen(check);

        return len <= cur.length() && std::equal(check,check+len,cur.begin());
    }


    // -----------------------------------------
    /** swallow the next call to ++, return the previous value. */
    void swallow_next_increment() {
        swallow = true;
    }

private:
    LineSplitter( const LineSplitter & );
    LineSplitter &operator = ( const LineSplitter & );

private:
    line_idx idx;
    std::string cur;
    StreamReaderLE& stream;
    bool swallow, skip_empty_lines, trim;
};

}
#endif // INCLUDED_LINE_SPLITTER_H
