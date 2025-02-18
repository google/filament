// Copyright 2024 The langsvr Authors
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

#include "langsvr/content_stream.h"

#include <sstream>
#include <string>

#include "langsvr/reader.h"
#include "langsvr/writer.h"
#include "src/utils/replace_all.h"

namespace langsvr {

namespace {
static constexpr std::string_view kContentLength = "Content-Length: ";

Result<SuccessType> Match(Reader& reader, std::string_view str) {
    auto got = reader.String(str.size());
    if (got != Success) {
        return got.Failure();
    }
    if (got.Get() != str) {
        std::stringstream err;
        err << "expected '" << str << "' got '" << got.Get() << "'";
        return Failure{err.str()};
    }
    return Success;
}

}  // namespace

Result<std::string> ReadContent(Reader& reader) {
    if (auto match = Match(reader, kContentLength); match != Success) {
        return match.Failure();
    }
    uint64_t len = 0;
    while (true) {
        char c = 0;
        if (auto read = reader.Read(reinterpret_cast<std::byte*>(&c), sizeof(c));
            read != sizeof(c)) {
            return Failure{"end of stream while parsing content length"};
        }
        if (c >= '0' && c <= '9') {
            len = len * 10 + static_cast<uint64_t>(c - '0');
            continue;
        }
        if (c == '\r') {
            break;
        }
        return Failure{"invalid content length value"};
    }

    auto got = reader.String(3);
    if (got != Success) {
        return got.Failure();
    }
    if (got.Get() != "\n\r\n") {
        auto fmt = [](std::string s) {
            s = ReplaceAll(s, "\n", "␊");
            s = ReplaceAll(s, "\r", "␍");
            return s;
        };

        std::stringstream err;
        err << "expected '␍␊␍␊' got '␍" << fmt(got.Get()) << "'";
        return Failure{err.str()};
    }

    return reader.String(len);
}

Result<SuccessType> WriteContent(Writer& writer, std::string_view content) {
    std::stringstream ss;
    ss << kContentLength << content.length() << "\r\n\r\n" << content;
    return writer.String(ss.str());
}

}  // namespace langsvr
