// Copyright 2023 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_CORE_CONSTANT_MANAGER_H_
#define SRC_TINT_LANG_CORE_CONSTANT_MANAGER_H_

#include <utility>

#include "src/tint/lang/core/constant/invalid.h"
#include "src/tint/lang/core/constant/value.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/utils/containers/unique_allocator.h"
#include "src/tint/utils/math/hash.h"

namespace tint::core::constant {
class Splat;

template <typename T>
class Scalar;
}  // namespace tint::core::constant

namespace tint::core::constant {

/// The constant manager holds a type manager and all the pointers to the known constant values.
class Manager final {
  public:
    /// Iterator is the type returned by begin() and end()
    using TypeIterator = BlockAllocator<Value>::ConstIterator;

    /// Constructor
    Manager();

    /// Move constructor
    Manager(Manager&&);

    /// Move assignment operator
    /// @param rhs the Manager to move
    /// @return this Manager
    Manager& operator=(Manager&& rhs);

    /// Destructor
    ~Manager();

    /// Wrap returns a new Manager created with the constants and types of `inner`.
    /// The Manager returned by Wrap is intended to temporarily extend the constants and types of an
    /// existing immutable Manager. As the copied constants and types are owned by `inner`, `inner`
    /// must not be destructed or assigned while using the returned Manager.
    /// TODO(crbug.com/tint/460) - Evaluate whether there are safer alternatives to this
    /// function.
    /// @param inner the immutable Manager to extend
    /// @return the Manager that wraps `inner`
    static Manager Wrap(const Manager& inner) {
        Manager out;
        out.values_.Wrap(inner.values_);
        out.types = core::type::Manager::Wrap(inner.types);
        return out;
    }

    /// @param args the arguments used to construct the type, unique node or node.
    /// @return a pointer to an instance of `T` with the provided arguments.
    ///         If NODE derives from UniqueNode and an existing instance of `T` has been
    ///         constructed, then the same pointer is returned.
    template <typename NODE, typename... ARGS>
    NODE* Get(ARGS&&... args) {
        return values_.Get<NODE>(std::forward<ARGS>(args)...);
    }

    /// @returns an iterator to the beginning of the types
    TypeIterator begin() const { return values_.begin(); }
    /// @returns an iterator to the end of the types
    TypeIterator end() const { return values_.end(); }

    /// Constructs a constant of a vector, matrix or array type.
    ///
    /// Examines the element values and will return either a constant::Composite or a
    /// constant::Splat, depending on the element types and values.
    ///
    /// @param type the composite type
    /// @param elements the composite elements
    /// @returns the value pointer
    const constant::Value* Composite(const core::type::Type* type,
                                     VectorRef<const constant::Value*> elements);

    /// Constructs a splat constant.
    /// @param type the splat type
    /// @param element the splat element
    /// @returns the value pointer
    const constant::Splat* Splat(const core::type::Type* type, const constant::Value* element);

    /// @param value the constant value
    /// @return a Scalar holding the i32 value @p value
    const Scalar<i32>* Get(i32 value);

    /// @param value the constant value
    /// @return a Scalar holding the u32 value @p value
    const Scalar<u32>* Get(u32 value);

    /// @param value the constant value
    /// @return a Scalar holding the u64 value @p value
    const Scalar<u64>* Get(u64 value);

    /// @param value the constant value
    /// @return a Scalar holding the i8 value @p value
    const Scalar<i8>* Get(i8 value);

    /// @param value the constant value
    /// @return a Scalar holding the u8 value @p value
    const Scalar<u8>* Get(u8 value);

    /// @param value the constant value
    /// @return a Scalar holding the f32 value @p value
    const Scalar<f32>* Get(f32 value);

    /// @param value the constant value
    /// @return a Scalar holding the f16 value @p value
    const Scalar<f16>* Get(f16 value);

    /// @param value the constant value
    /// @return a Scalar holding the bool value @p value
    const Scalar<bool>* Get(bool value);

    /// @param value the constant value
    /// @return a Scalar holding the AFloat value @p value
    const Scalar<AFloat>* Get(AFloat value);

    /// @param value the constant value
    /// @return a Scalar holding the AInt value @p value
    const Scalar<AInt>* Get(AInt value);

    /// Constructs a constant zero-value of the type @p type.
    /// @param type the constant type
    /// @returns a constant zero-value for the type
    const Value* Zero(const core::type::Type* type);

    /// Constructs an invalid constant
    /// @returns an invalid constant
    const constant::Invalid* Invalid();

    /// The type manager
    core::type::Manager types;

  private:
    /// A specialization of Hasher for constant::Value
    struct Hasher {
        /// @param value the value to hash
        /// @returns a hash of the value
        HashCode operator()(const constant::Value& value) const { return value.Hash(); }
    };

    /// An equality helper for constant::Value
    struct Equal {
        /// @param a the LHS value
        /// @param b the RHS value
        /// @returns true if the two constants are equal
        bool operator()(const constant::Value& a, const constant::Value& b) const {
            return a.Equal(&b);
        }
    };

    /// Unique types owned by the manager
    UniqueAllocator<Value, Hasher, Equal> values_;
};

}  // namespace tint::core::constant

#endif  // SRC_TINT_LANG_CORE_CONSTANT_MANAGER_H_
