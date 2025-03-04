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

#ifndef SRC_DAWN_NATIVE_CHAINUTILS_H_
#define SRC_DAWN_NATIVE_CHAINUTILS_H_

#include <bitset>
#include <string>
#include <tuple>
#include <type_traits>

#include "absl/strings/str_format.h"
#include "dawn/common/Math.h"
#include "dawn/native/ChainUtils_autogen.h"
#include "dawn/native/Error.h"

namespace dawn::native {
namespace detail {
// Gets the chain type for an extensible type.
template <typename T>
struct ChainTypeFor {
    using Type = typename std::conditional_t<ExtensibilityFor<T> == Extensibility::In,
                                             const wgpu::ChainedStruct*,
                                             wgpu::ChainedStructOut*>;
};
}  // namespace detail

template <typename T>
class UnpackedPtr;

namespace detail {
// Converts to the expected pointer types depending on the extensibility of the structure.
template <typename UnpackedPtrT, typename U>
struct PtrTypeFor;
template <typename T, typename U>
struct PtrTypeFor<UnpackedPtr<T>, U> {
    using Type =
        typename std::conditional_t<ExtensibilityFor<T> == Extensibility::In, const U*, U*>;
};
}  // namespace detail

// Unpacks chained structures in a best effort manner (skipping unknown chains) without applying
// validation. If the same structure is duplicated in the chain, it is unspecified which one the
// result of Get will be. These are the effective constructors for the wrapper types. Note that
// these are implemented in the generated ChainUtils_autogen.cpp file.
template <typename T,
          typename = std::enable_if_t<detail::ExtensibilityFor<T> == detail::Extensibility::In>>
UnpackedPtr<T> Unpack(const T* chain);
template <typename T,
          typename = std::enable_if_t<detail::ExtensibilityFor<T> == detail::Extensibility::Out>>
UnpackedPtr<T> Unpack(T* chain);

// Unpacks chained structures into UnpackedPtr<T> while applying validation.
template <typename T,
          typename = std::enable_if_t<detail::ExtensibilityFor<T> == detail::Extensibility::In>>
ResultOrError<UnpackedPtr<T>> ValidateAndUnpack(const T* chain);
template <typename T,
          typename = std::enable_if_t<detail::ExtensibilityFor<T> == detail::Extensibility::Out>>
ResultOrError<UnpackedPtr<T>> ValidateAndUnpack(T* chain);

//
// Wrapper class for unpacked pointers. The classes essentially acts like a const T* or T* with
// the additional capabilities to validate and retrieve chained structures.
//
template <typename T>
class UnpackedPtr {
  public:
    using ChainType =
        typename std::conditional_t<detail::ExtensibilityFor<T> == detail::Extensibility::In,
                                    const wgpu::ChainedStruct*,
                                    wgpu::ChainedStructOut*>;
    using PtrType = typename detail::PtrTypeFor<UnpackedPtr<T>, T>::Type;
    using TupleType = typename detail::UnpackedPtrTypeFor<T>::Type;
    using BitsetType = typename std::bitset<std::tuple_size_v<TupleType>>;

    UnpackedPtr() : mStruct(nullptr) {}

    explicit operator bool() const { return mStruct != nullptr; }
    PtrType operator->() const { return mStruct; }
    PtrType operator*() const { return mStruct; }

    // Returns true iff every allowed chain in this unpacked type is nullptr.
    bool Empty() const;
    // Returns a string of the non-nullptr STypes from an unpacked chain.
    std::string ToString() const;

    template <typename In>
    auto Get() const;

    // Validation functions. See implementations of these below for usage, details, and examples.
    template <typename... Branches>
    ResultOrError<wgpu::SType> ValidateBranches() const;
    template <typename... Allowed>
    MaybeError ValidateSubset() const;

  private:
    friend UnpackedPtr<T> Unpack<T>(PtrType chain);
    friend ResultOrError<UnpackedPtr<T>> ValidateAndUnpack<T>(PtrType chain);

    explicit UnpackedPtr(PtrType packed) : mStruct(packed) {}

    PtrType mStruct = nullptr;
    TupleType mUnpacked;
    BitsetType mBitset;
};

// Tuple type of a Branch and an optional list of corresponding Extensions.
template <typename B, typename... Exts>
struct Branch;

// ------------------------------------------------------------------------------------------------
// Implementation details start here so that the headers are terse.
// ------------------------------------------------------------------------------------------------

namespace detail {

// Helpers to get the index in an unpacked tuple type for a particular type.
template <typename UnpackedPtrT, typename Ext>
inline size_t UnpackedPtrTupleIndexOf;
template <typename Ext, typename... Exts>
constexpr inline size_t UnpackedPtrTupleIndexOf<std::tuple<Ext, Exts...>, Ext> = 0;
template <typename Ext, typename Other, typename... Exts>
constexpr inline size_t UnpackedPtrTupleIndexOf<std::tuple<Other, Exts...>, Ext> =
    1 + UnpackedPtrTupleIndexOf<std::tuple<Exts...>, Ext>;

template <typename UnpackedPtrT, typename Ext>
inline size_t UnpackedPtrIndexOf;
template <typename T, typename Ext>
constexpr inline size_t UnpackedPtrIndexOf<UnpackedPtr<T>, Ext> =
    UnpackedPtrTupleIndexOf<typename UnpackedPtr<T>::TupleType,
                            typename PtrTypeFor<UnpackedPtr<T>, std::remove_pointer_t<Ext>>::Type>;

// Currently using an internal 64-bit unsigned int for internal representation. This is necessary
// because std::bitset::operator| is not constexpr until C++23.
template <typename UnpackedPtrT, typename... Exts>
constexpr inline auto UnpackedPtrBitsetForExts = typename UnpackedPtrT::BitsetType(
    ((uint64_t(1) << UnpackedPtrIndexOf<UnpackedPtrT, Exts>) | ...));
template <typename UnpackedPtrT>
constexpr inline auto UnpackedPtrBitsetForExts<UnpackedPtrT> = typename UnpackedPtrT::BitsetType(0);

template <typename UnpackedPtrT, typename...>
struct OneBranchValidator;
template <typename UnpackedPtrT, typename R, typename... Exts>
struct OneBranchValidator<UnpackedPtrT, Branch<R, Exts...>> {
    using BitsetType = typename UnpackedPtrT::BitsetType;

    static bool Validate(const UnpackedPtrT& unpacked,
                         const BitsetType& actual,
                         wgpu::SType& match) {
        // Only check the full bitset when the main branch matches.
        if (unpacked.template Get<R>() != nullptr) {
            // Allowed set of extensions includes the branch root as well.
            constexpr auto allowed =
                UnpackedPtrBitsetForExts<UnpackedPtrT,
                                         typename detail::PtrTypeFor<UnpackedPtrT, R>::Type,
                                         typename detail::PtrTypeFor<UnpackedPtrT, Exts>::Type...>;

            // The configuration is allowed if the actual available chains are a subset.
            if (IsSubset(actual, allowed)) {
                match = STypeFor<R>;
                return true;
            }
        }
        return false;
    }

    static std::string ToString() {
        if constexpr (sizeof...(Exts) > 0) {
            return absl::StrFormat("[ %s -> (%s) ]", STypesToString<R>(),
                                   STypesToString<Exts...>());
        } else {
            return absl::StrFormat("[ %s ]", STypesToString<R>());
        }
    }
};

template <typename UnpackedPtrT, typename... Branches>
struct BranchesValidator {
    using BitsetType = typename UnpackedPtrT::BitsetType;

    static bool Validate(const UnpackedPtrT& unpacked,
                         const BitsetType& actual,
                         wgpu::SType& match) {
        return ((OneBranchValidator<UnpackedPtrT, Branches>::Validate(unpacked, actual, match)) ||
                ...);
    }

    static std::string ToString() {
        return (
            (absl::StrFormat("  - %s\n", OneBranchValidator<UnpackedPtrT, Branches>::ToString())) +
            ...);
    }
};

template <typename UnpackedPtrT, typename... Allowed>
struct SubsetValidator {
    using BitsetType = typename UnpackedPtrT::BitsetType;

    static MaybeError Validate(const UnpackedPtrT& unpacked, const BitsetType& bitset) {
        // Allowed set of extensions includes the branch root as well.
        constexpr auto allowed = detail::UnpackedPtrBitsetForExts<
            UnpackedPtrT, typename detail::PtrTypeFor<UnpackedPtrT, Allowed>::Type...>;
        if (!IsSubset(bitset, allowed)) {
            if constexpr (sizeof...(Allowed)) {
                return DAWN_VALIDATION_ERROR(
                    "Expected extension set to be a subset of: %s\nInstead found: %s",
                    absl::StrFormat("[ %s ]", STypesToString<Allowed...>()), unpacked.ToString());
            } else {
                return DAWN_VALIDATION_ERROR(
                    "Expected there to be no extensions. Instead found: %s", unpacked.ToString());
            }
        }
        return {};
    }
};

}  // namespace detail

template <typename T>
template <typename In>
auto UnpackedPtr<T>::Get() const {
    return std::get<typename detail::PtrTypeFor<UnpackedPtr<T>, In>::Type>(mUnpacked);
}

template <typename T>
bool UnpackedPtr<T>::Empty() const {
    return mBitset.none();
}

template <typename T>
std::string UnpackedPtr<T>::ToString() const {
    std::string result = "( ";
    std::apply(
        [&](auto*... args) {
            (([&](auto* arg) {
                 if (arg != nullptr) {
                     // reinterpret_cast because this chained struct might be forward-declared
                     // without a definition. The definition may only be available on a
                     // particular backend.
                     const auto* chainedStruct = reinterpret_cast<ChainType>(arg);
                     result += absl::StrFormat("%s, ", chainedStruct->sType);
                 }
             }(args)),
             ...);
        },
        mUnpacked);
    result += " )";
    return result;
}

// Validates that an unpacked chain retrieved via ValidateAndUnpack matches a valid "branch",
// where a "branch" is defined as a required "root" extension and optional follow-up
// extensions.
//
// Returns the wgpu::SType associated with the "root" extension of a "branch" if matched,
// otherwise returns an error.
//
// Example usage:
//     UnpackedPtr<T> u;
//     DAWN_TRY_ASSIGN(u, ValidateAndUnpack(desc));
//     wgpu::SType rootType;
//     DAWN_TRY_ASSIGN(rootType,
//         u.ValidateBranches<Branch<Root1>, Branch<Root2, R2Ext1>>());
//     switch (rootType) {
//         case Root1: {
//             <do something>
//         }
//         case Root2: {
//             R2Ext1 ext = u.Get<R2Ext1>(u);
//             if (ext) {
//                 <do something with optional extension(s)>
//             }
//         }
//         default:
//             DAWN_UNREACHABLE();
//     }
//
// The example above checks that the unpacked chain is either:
//     - only a Root1 extension
//     - or a Root2 extension with an optional R2Ext1 extension
// Any other configuration is deemed invalid.
template <typename T>
template <typename... Branches>
ResultOrError<wgpu::SType> UnpackedPtr<T>::ValidateBranches() const {
    using Validator = detail::BranchesValidator<UnpackedPtr<T>, Branches...>;

    wgpu::SType match = wgpu::SType(0u);
    if (Validator::Validate(*this, mBitset, match)) {
        return match;
    }
    return DAWN_VALIDATION_ERROR(
        "Expected chain root to match one of the following branch types with optional extensions:"
        "\n%sInstead found: %s",
        Validator::ToString(), ToString());
}

// Validates that an unpacked chain retrieved via ValidateAndUnpack contains a subset of the
// Allowed extensions. If there are any other extensions, returns an error.
//
// Example usage:
//     UnpackedPtr<T> u;
//     DAWN_TRY_ASSIGN(u, ValidateAndUnpack(desc));
//     DAWN_TRY(u.ValidateSubset<Ext1>());
//
// Even though "valid" extensions on descriptor may include both Ext1 and Ext2, ValidateSubset
// will further enforce that Ext2 is not on the chain in the example above.
template <typename T>
template <typename... Allowed>
MaybeError UnpackedPtr<T>::ValidateSubset() const {
    return detail::SubsetValidator<UnpackedPtr<T>, Allowed...>::Validate(*this, mBitset);
}

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_CHAINUTILS_H_
