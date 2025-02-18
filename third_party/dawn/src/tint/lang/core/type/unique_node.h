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

#ifndef SRC_TINT_LANG_CORE_TYPE_UNIQUE_NODE_H_
#define SRC_TINT_LANG_CORE_TYPE_UNIQUE_NODE_H_

#include <functional>

#include "src/tint/lang/core/type/node.h"

namespace tint::core::type {

/// UniqueNode is the base class for objects that are de-duplicated by the Manager.
/// Deduplication is achieved by comparing a temporary object to the set of existing objects, using
/// Hash() and Equals(). If an existing object is found, then the pointer to that object is
/// returned, otherwise a new object is constructed, added to the Manager's set and returned.
class UniqueNode : public Castable<UniqueNode, Node> {
  public:
    /// Constructor
    /// @param hash the immutable hash for the node
    inline explicit UniqueNode(size_t hash) : unique_hash(hash) {}

    /// Destructor
    ~UniqueNode() override;

    /// @param other the other node to compare this node against
    /// @returns true if the this node is equal to @p other
    virtual bool Equals(const UniqueNode& other) const = 0;

    /// the immutable hash for the node
    const size_t unique_hash;
};

}  // namespace tint::core::type

namespace std {

/// std::hash specialization for tint::core::type::UniqueNode
template <>
struct hash<tint::core::type::UniqueNode> {
    /// @param node the unique node to obtain a hash from
    /// @returns the hash of the node
    size_t operator()(const tint::core::type::UniqueNode& node) const { return node.unique_hash; }
};

/// std::equal_to specialization for tint::core::type::UniqueNode
template <>
struct equal_to<tint::core::type::UniqueNode> {
    /// @param a the first unique node to compare
    /// @param b the second unique node to compare
    /// @returns true if the two nodes are equal
    bool operator()(const tint::core::type::UniqueNode& a,
                    const tint::core::type::UniqueNode& b) const {
        return &a == &b || a.Equals(b);
    }
};

}  // namespace std

#endif  // SRC_TINT_LANG_CORE_TYPE_UNIQUE_NODE_H_
