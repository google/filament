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

#ifndef SRC_TINT_UTILS_CONTAINERS_SCOPE_STACK_H_
#define SRC_TINT_UTILS_CONTAINERS_SCOPE_STACK_H_

#include <utility>

#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/containers/vector.h"
#include "src/tint/utils/macros/compiler.h"

namespace tint {

/// Used to store a stack of scope information.
/// The stack starts with a global scope which can not be popped.
template <class K, class V>
class ScopeStack {
  public:
    ScopeStack() { Push(); }

    /// Push a new scope on to the stack
    void Push() {
        depth_++;
        if (DAWN_LIKELY(stack_.Length() >= depth_)) {
            Top().Clear();
        } else {
            stack_.Push({});
        }
    }

    /// Pop the scope off the top of the stack
    void Pop() {
        if (depth_ > 1) {
            depth_--;
        }
    }

    /// Assigns the value into the top most scope of the stack.
    /// @param key the key of the value
    /// @param val the value
    /// @returns the old value if there was an existing key at the top of the
    /// stack, otherwise the zero initializer for type T.
    V Set(const K& key, V val) {
        if (auto el = Top().Get(key)) {
            std::swap(val, *el);
            return val;
        }
        Top().Add(key, val);
        return {};
    }

    /// Retrieves a value from the stack
    /// @param key the key to look for
    /// @returns the value, or the zero initializer if the value was not found
    V Get(const K& key) const {
        for (size_t i = depth_; i > 0; i--) {
            if (auto val = stack_[i - 1].Get(key)) {
                return *val;
            }
        }

        return V{};
    }

    /// Return the top scope of the stack.
    /// @returns the top scope of the stack
    const Hashmap<K, V, 4>& Top() const { return stack_[depth_ - 1]; }

    /// Return the top scope of the stack.
    /// @returns the top scope of the stack
    Hashmap<K, V, 4>& Top() { return stack_[depth_ - 1]; }

    /// Clear the scope stack.
    void Clear() {
        depth_ = 1;
        stack_.Resize(1);
        stack_.Front().Clear();
    }

  private:
    Vector<Hashmap<K, V, 4>, 8> stack_;
    /// The active count in stack. We don't push and pop the stack to avoid frequent re-allocations
    /// of the hashmaps.
    size_t depth_ = 0;
};

}  // namespace tint

#endif  // SRC_TINT_UTILS_CONTAINERS_SCOPE_STACK_H_
