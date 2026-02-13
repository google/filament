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

#ifndef SRC_DAWN_NATIVE_OBJECTCONTENTHASHER_H_
#define SRC_DAWN_NATIVE_OBJECTCONTENTHASHER_H_

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "dawn/common/HashUtils.h"

namespace dawn::native {

// ObjectContentHasher records a hash that can be used as a key to lookup a cached object in a
// cache.
class ObjectContentHasher {
  public:
    // Record calls the appropriate record function based on the type.
    template <typename T, typename... Args>
    void Record(const T& value, const Args&... args) {
        RecordImpl<T, Args...>::Call(this, value, args...);
    }

    size_t GetContentHash() const;

  private:
    template <typename T, typename... Args>
    struct RecordImpl {
        static constexpr void Call(ObjectContentHasher* recorder,
                                   const T& value,
                                   const Args&... args) {
            HashCombine(&recorder->mContentHash, value, args...);
        }
    };

    template <typename T>
    struct RecordImpl<T*> {
        static constexpr void Call(ObjectContentHasher* recorder, T* obj) {
            // Calling Record(objPtr) is not allowed. This check exists to only prevent such
            // mistakes.
            static_assert(obj == nullptr);
        }
    };

    template <typename T>
    struct RecordImpl<std::vector<T>> {
        static constexpr void Call(ObjectContentHasher* recorder, const std::vector<T>& vec) {
            recorder->RecordIterable<std::vector<T>>(vec);
        }
    };

    template <typename T, size_t N>
    struct RecordImpl<std::array<T, N>> {
        static constexpr void Call(ObjectContentHasher* recorder, const std::array<T, N>& array) {
            recorder->RecordIterable<std::array<T, N>>(array);
        }
    };

    template <typename T, typename E>
    struct RecordImpl<std::map<T, E>> {
        static constexpr void Call(ObjectContentHasher* recorder, const std::map<T, E>& map) {
            recorder->RecordIterable<std::map<T, E>>(map);
        }
    };

    template <typename IteratorT>
    constexpr void RecordIterable(const IteratorT& iterable) {
        for (auto it = iterable.begin(); it != iterable.end(); ++it) {
            Record(*it);
        }
    }

    template <typename T, typename E>
    struct RecordImpl<std::pair<T, E>> {
        static constexpr void Call(ObjectContentHasher* recorder, const std::pair<T, E>& pair) {
            recorder->Record(pair.first);
            recorder->Record(pair.second);
        }
    };

    size_t mContentHash = 0;
};

template <>
struct ObjectContentHasher::RecordImpl<std::string> {
    static constexpr void Call(ObjectContentHasher* recorder, const std::string& str) {
        recorder->RecordIterable<std::string>(str);
    }
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_OBJECTCONTENTHASHER_H_
