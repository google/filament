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

#ifndef SRC_TINT_LANG_WGSL_AST_CLONE_CONTEXT_H_
#define SRC_TINT_LANG_WGSL_AST_CLONE_CONTEXT_H_

#include <algorithm>
#include <functional>
#include <type_traits>
#include <utility>
#include <vector>

#include "src/tint/lang/wgsl/ast/builder.h"
#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/containers/hashset.h"
#include "src/tint/utils/containers/vector.h"
#include "src/tint/utils/diagnostic/diagnostic.h"
#include "src/tint/utils/diagnostic/source.h"
#include "src/tint/utils/generation_id.h"
#include "src/tint/utils/ice/ice.h"
#include "src/tint/utils/macros/compiler.h"
#include "src/tint/utils/rtti/castable.h"
#include "src/tint/utils/rtti/traits.h"
#include "src/tint/utils/symbol/symbol.h"

// Forward declarations
namespace tint::ast {
class FunctionList;
class Node;
struct Type;
}  // namespace tint::ast

namespace tint::ast {

/// CloneContext holds the state used while cloning AST nodes.
class CloneContext {
    /// ParamTypeIsPtrOf<F, T> is true iff the first parameter of
    /// F is a pointer of (or derives from) type T.
    template <typename F, typename T>
    static constexpr bool ParamTypeIsPtrOf = tint::traits::
        IsTypeOrDerived<typename std::remove_pointer<tint::traits::ParameterType<F, 0>>::type, T>;

  public:
    /// SymbolTransform is a function that takes a symbol and returns a new
    /// symbol.
    using SymbolTransform = std::function<Symbol(Symbol)>;

    /// Constructor for cloning objects from `from` into `to`.
    /// @param to the target Builder to clone into
    /// @param from the generation ID of the source program being cloned
    CloneContext(Builder* to, GenerationID from);

    /// Destructor
    ~CloneContext();

    /// Clones the Node or type::Type @p object into the Builder #dst if @p object is not null. If
    /// @p object is null, then Clone() returns null.
    ///
    /// Clone() may use a function registered with ReplaceAll() to create a transformed version of
    /// the object. See ReplaceAll() for more information.
    ///
    /// If the CloneContext is cloning from a Program to a Builder, then the Node or type::Type @p
    /// object must be owned by the source program.
    ///
    /// @param object the type deriving from Node to clone
    /// @return the cloned node
    template <typename T>
    const T* Clone(const T* object) {
        TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(src_id, object);
        if (auto* cloned = CloneNode(object)) {
            auto* out = CheckedCast<T>(cloned);
            TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(dst, out);
            return out;
        }
        return nullptr;
    }

    /// Clones the Node or type::Type @p object into the Builder #dst if @p object is not null. If
    /// @p object is null, then Clone() returns null.
    ///
    /// Unlike Clone(), this method does not invoke or use any transformations registered by
    /// ReplaceAll().
    ///
    /// If the CloneContext is cloning from a Program to a Builder, then the Node or type::Type @p
    /// object must be owned by the source program.
    ///
    /// @param a the type deriving from Node to clone
    /// @return the cloned node
    template <typename T>
    const T* CloneWithoutTransform(const T* a) {
        // If the input is nullptr, there's nothing to clone - just return nullptr.
        if (a == nullptr) {
            return nullptr;
        }
        TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(src_id, a);
        auto* c = a->Clone(*this);
        return CheckedCast<T>(c);
    }

    /// Clones the ast::Type `ty` into the Builder #dst
    /// @param ty the AST type.
    /// @return the cloned node
    ast::Type Clone(const ast::Type& ty);

    /// Clones the Source `s` into #dst
    /// @note this 'clone' is a shallow copy.
    /// @param s the `Source` to clone
    /// @return the cloned source
    Source Clone(const Source& s) const { return s; }

    /// Clones the Symbol `s` into #dst
    ///
    /// The Symbol `s` must be owned by the source program.
    ///
    /// @param s the Symbol to clone
    /// @return the cloned source
    Symbol Clone(Symbol s);

    /// Clones each of the elements of the vector `v` into the Builder
    /// #dst.
    ///
    /// All the elements of the vector `v` must be owned by the source program.
    ///
    /// @param v the vector to clone
    /// @return the cloned vector
    template <typename T, size_t N>
    tint::Vector<T, N> Clone(const tint::Vector<T, N>& v) {
        tint::Vector<T, N> out;
        out.reserve(v.size());
        for (auto& el : v) {
            out.Push(Clone(el));
        }
        return out;
    }

    /// Clones each of the elements of the vector `v` using the Builder
    /// #dst, inserting any additional elements into the list that were registered
    /// with calls to InsertBefore().
    ///
    /// All the elements of the vector `v` must be owned by the source program.
    ///
    /// @param v the vector to clone
    /// @return the cloned vector
    template <typename T, size_t N>
    tint::Vector<T*, N> Clone(const tint::Vector<T*, N>& v) {
        tint::Vector<T*, N> out;
        Clone(out, v);
        return out;
    }

    /// Clones each of the elements of the vector `from` into the vector `to`,
    /// inserting any additional elements into the list that were registered with
    /// calls to InsertBefore().
    ///
    /// All the elements of the vector `from` must be owned by the source program.
    ///
    /// @param from the vector to clone
    /// @param to the cloned result
    template <typename T, size_t N>
    void Clone(tint::Vector<T*, N>& to, const tint::Vector<T*, N>& from) {
        to.Reserve(from.Length());

        auto transforms = list_transforms_.Get(&from);

        if (transforms) {
            for (auto& builder : transforms->insert_front_) {
                to.Push(CheckedCast<T>(builder()));
            }
            for (auto& el : from) {
                if (auto insert_before = transforms->insert_before_.Get(el)) {
                    for (auto& builder : *insert_before) {
                        to.Push(CheckedCast<T>(builder()));
                    }
                }
                if (!transforms->remove_.Contains(el)) {
                    to.Push(Clone(el));
                }
                if (auto insert_after = transforms->insert_after_.Get(el)) {
                    for (auto& builder : *insert_after) {
                        to.Push(CheckedCast<T>(builder()));
                    }
                }
            }
            for (auto& builder : transforms->insert_back_) {
                to.Push(CheckedCast<T>(builder()));
            }
        } else {
            for (auto& el : from) {
                to.Push(Clone(el));

                if (!transforms) {
                    // Clone(el) may have create a transformation list
                    transforms = list_transforms_.Get(&from);
                }
                if (transforms) {
                    if (auto insert_after = transforms->insert_after_.Get(el)) {
                        for (auto& builder : *insert_after) {
                            to.Push(CheckedCast<T>(builder()));
                        }
                    }
                }
            }

            if (!transforms) {
                // Clone(el) may have create a transformation list
                transforms = list_transforms_.Get(&from);
            }
            if (transforms) {
                for (auto& builder : transforms->insert_back_) {
                    to.Push(CheckedCast<T>(builder()));
                }
            }
        }
    }

    /// Clones each of the elements of the vector `v` into the Builder
    /// #dst.
    ///
    /// All the elements of the vector `v` must be owned by the source program.
    ///
    /// @param v the vector to clone
    /// @return the cloned vector
    ast::FunctionList Clone(const ast::FunctionList& v);

    /// ReplaceAll() registers `replacer` to be called whenever the Clone() method
    /// is called with a Node type that matches (or derives from) the type of
    /// the single parameter of `replacer`.
    /// The returned Node of `replacer` will be used as the replacement for
    /// all references to the object that's being cloned. This returned Node
    /// must be owned by the Program #dst.
    ///
    /// `replacer` must be function-like with the signature: `T* (T*)`
    ///  where `T` is a type deriving from Node.
    ///
    /// If `replacer` returns a nullptr then Clone() will call `T::Clone()` to
    /// clone the object.
    ///
    /// Example:
    ///
    /// ```
    ///   // Replace all ast::UintLiteralExpressions with the number 42
    ///   CloneCtx ctx(&out, in);
    ///   ctx.ReplaceAll([&] (ast::UintLiteralExpression* l) {
    ///       return ctx.dst->create<ast::UintLiteralExpression>(
    ///           ctx.Clone(l->source),
    ///           ctx.Clone(l->type),
    ///           42);
    ///     });
    ///   ctx.Clone();
    /// ```
    ///
    /// @warning a single handler can only be registered for any given type.
    /// Attempting to register two handlers for the same type will result in an
    /// ICE.
    /// @warning The replacement object must be of the correct type for all
    /// references of the original object. A type mismatch will result in an
    /// assertion in debug builds, and undefined behavior in release builds.
    /// @param replacer a function or function-like object with the signature
    ///        `T* (T*)`, where `T` derives from Node
    /// @returns this CloneContext so calls can be chained
    template <typename F>
    std::enable_if_t<ParamTypeIsPtrOf<F, ast::Node>, CloneContext>& ReplaceAll(F&& replacer) {
        using TPtr = traits::ParameterType<F, 0>;
        using T = typename std::remove_pointer<TPtr>::type;
        for (auto& transform : transforms_) {
            bool already_registered = transform.typeinfo->Is(&tint::TypeInfo::Of<T>()) ||
                                      tint::TypeInfo::Of<T>().Is(transform.typeinfo);
            if (DAWN_UNLIKELY(already_registered)) {
                TINT_ICE() << "ReplaceAll() called with a handler for type "
                           << TypeInfo::Of<T>().name
                           << " that is already handled by a handler for type "
                           << transform.typeinfo->name;
                return *this;
            }
        }
        CloneableTransform transform;
        transform.typeinfo = &TypeInfo::Of<T>();
        transform.function = [=](const ast::Node* in) { return replacer(in->As<T>()); };
        transforms_.Push(std::move(transform));
        return *this;
    }

    /// ReplaceAll() registers `replacer` to be called whenever the Clone() method
    /// is called with a Symbol.
    /// The returned symbol of `replacer` will be used as the replacement for
    /// all references to the symbol that's being cloned. This returned Symbol
    /// must be owned by the Program #dst.
    /// @param replacer a function the signature `Symbol(Symbol)`.
    /// @warning a SymbolTransform can only be registered once. Attempting to
    /// register a SymbolTransform more than once will result in an ICE.
    /// @returns this CloneContext so calls can be chained
    CloneContext& ReplaceAll(const SymbolTransform& replacer) {
        if (DAWN_UNLIKELY(symbol_transform_)) {
            TINT_ICE() << "ReplaceAll(const SymbolTransform&) called multiple times on the same "
                          "CloneContext";
        }
        symbol_transform_ = replacer;
        return *this;
    }

    /// Replace replaces all occurrences of `what` in the source program with the pointer `with`
    /// in #dst when calling Clone().
    /// [DEPRECATED]: This function cannot handle nested replacements. Use the
    /// overload of Replace() that take a function for the `WITH` argument.
    /// @param what a pointer to the object in the source program that will be replaced with
    /// `with`
    /// @param with a pointer to the replacement object owned by #dst that will be
    /// used as a replacement for `what`
    /// @warning The replacement object must be of the correct type for all
    /// references of the original object. A type mismatch will result in an
    /// assertion in debug builds, and undefined behavior in release builds.
    /// @returns this CloneContext so calls can be chained
    template <typename WHAT, typename WITH>
        requires(traits::IsTypeOrDerived<WITH, ast::Node>)
    CloneContext& Replace(const WHAT* what, const WITH* with) {
        TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(src_id, what);
        TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(dst, with);
        replacements_.Replace(what, [with]() -> const ast::Node* { return with; });
        return *this;
    }

    /// Replace replaces all occurrences of `what` in the source program with the result of the
    /// function `with` in #dst when calling Clone(). `with` will be called each
    /// time `what` is cloned by this context. If `what` is not cloned, then
    /// `with` may never be called.
    /// @param what a pointer to the object in the source program that will be replaced with
    /// `with`
    /// @param with a function that takes no arguments and returns a pointer to
    /// the replacement object owned by #dst. The returned pointer will be used as
    /// a replacement for `what`.
    /// @warning The replacement object must be of the correct type for all
    /// references of the original object. A type mismatch will result in an
    /// assertion in debug builds, and undefined behavior in release builds.
    /// @returns this CloneContext so calls can be chained
    template <typename WHAT, typename WITH, typename = std::invoke_result_t<WITH>>
    CloneContext& Replace(const WHAT* what, WITH&& with) {
        TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(src_id, what);
        replacements_.Replace(what, with);
        return *this;
    }

    /// Removes @p object from the cloned copy of @p vector.
    /// @param vector the vector in the source program
    /// @param object a pointer to the object in the source program that will be omitted from
    /// the cloned vector.
    /// @returns this CloneContext so calls can be chained
    template <typename T, size_t N, typename OBJECT>
    CloneContext& Remove(const Vector<T, N>& vector, OBJECT* object) {
        TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(src_id, object);
        if (DAWN_UNLIKELY((std::find(vector.begin(), vector.end(), object) == vector.end()))) {
            TINT_ICE() << "CloneContext::Remove() vector does not contain object";
            return *this;
        }

        list_transforms_.GetOrAddZero(&vector).remove_.Add(object);
        return *this;
    }

    /// Inserts @p object before any other objects of @p vector, when the vector is cloned.
    /// @param vector the vector in the source program
    /// @param object a pointer to the object in #dst that will be inserted at the
    /// front of the vector
    /// @returns this CloneContext so calls can be chained
    template <typename T, size_t N, typename OBJECT>
    CloneContext& InsertFront(const Vector<T, N>& vector, OBJECT* object) {
        TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(dst, object);
        return InsertFront(vector, [object] { return object; });
    }

    /// Inserts a lazily built object before any other objects of @p vector, when the vector is
    /// cloned.
    /// @param vector the vector in the source program
    /// @param builder a builder of the object that will be inserted at the front of the vector.
    /// @returns this CloneContext so calls can be chained
    template <typename T, size_t N, typename BUILDER>
    CloneContext& InsertFront(const tint::Vector<T, N>& vector, BUILDER&& builder) {
        list_transforms_.GetOrAddZero(&vector).insert_front_.Push(std::forward<BUILDER>(builder));
        return *this;
    }

    /// Inserts @p object after any other objects of @p vector, when the vector is cloned.
    /// @param vector the vector in the source program
    /// @param object a pointer to the object in #dst that will be inserted at the
    /// end of the vector
    /// @returns this CloneContext so calls can be chained
    template <typename T, size_t N, typename OBJECT>
    CloneContext& InsertBack(const Vector<T, N>& vector, OBJECT* object) {
        TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(dst, object);
        return InsertBack(vector, [object] { return object; });
    }

    /// Inserts a lazily built object after any other objects of @p vector, when the vector is
    /// cloned.
    /// @param vector the vector in the source program
    /// @param builder the builder of the object in #dst that will be inserted at the end of the
    /// vector.
    /// @returns this CloneContext so calls can be chained
    template <typename T, size_t N, typename BUILDER>
    CloneContext& InsertBack(const tint::Vector<T, N>& vector, BUILDER&& builder) {
        list_transforms_.GetOrAddZero(&vector).insert_back_.Push(std::forward<BUILDER>(builder));
        return *this;
    }

    /// Inserts @p object before @p before whenever @p vector is cloned.
    /// @param vector the vector in the source program
    /// @param before a pointer to the object in the source program
    /// @param object a pointer to the object in #dst that will be inserted before
    /// any occurrence of the clone of @p before
    /// @returns this CloneContext so calls can be chained
    template <typename T, size_t N, typename BEFORE, typename OBJECT>
    CloneContext& InsertBefore(const tint::Vector<T, N>& vector,
                               const BEFORE* before,
                               const OBJECT* object) {
        TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(src_id, before);
        TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(dst, object);
        if (DAWN_UNLIKELY((std::find(vector.begin(), vector.end(), before) == vector.end()))) {
            TINT_ICE() << "CloneContext::InsertBefore() vector does not contain before";
            return *this;
        }

        list_transforms_.GetOrAddZero(&vector).insert_before_.GetOrAddZero(before).Push(
            [object] { return object; });
        return *this;
    }

    /// Inserts a lazily created object before @p before whenever @p vector is cloned.
    /// @param vector the vector in the source program
    /// @param before a pointer to the object in the source program
    /// @param builder the builder of the object in #dst that will be inserted before any occurrence
    /// of the clone of @p before
    /// @returns this CloneContext so calls can be chained
    template <typename T,
              size_t N,
              typename BEFORE,
              typename BUILDER,
              typename _ = std::enable_if_t<!std::is_pointer_v<std::decay_t<BUILDER>>>>
    CloneContext& InsertBefore(const tint::Vector<T, N>& vector,
                               const BEFORE* before,
                               BUILDER&& builder) {
        list_transforms_.GetOrAddZero(&vector).insert_before_.GetOrAddZero(before).Push(
            std::forward<BUILDER>(builder));
        return *this;
    }

    /// Inserts @p object after @p after whenever @p vector is cloned.
    /// @param vector the vector in the source program
    /// @param after a pointer to the object in the source program
    /// @param object a pointer to the object in #dst that will be inserted after
    /// any occurrence of the clone of @p after
    /// @returns this CloneContext so calls can be chained
    template <typename T, size_t N, typename AFTER, typename OBJECT>
    CloneContext& InsertAfter(const tint::Vector<T, N>& vector,
                              const AFTER* after,
                              const OBJECT* object) {
        TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(src_id, after);
        TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(dst, object);
        if (DAWN_UNLIKELY((std::find(vector.begin(), vector.end(), after) == vector.end()))) {
            TINT_ICE() << "CloneContext::InsertAfter() vector does not contain after";
            return *this;
        }

        list_transforms_.GetOrAddZero(&vector).insert_after_.GetOrAddZero(after).Push(
            [object] { return object; });
        return *this;
    }

    /// Inserts a lazily created object after @p after whenever @p vector is cloned.
    /// @param vector the vector in the source program
    /// @param after a pointer to the object in the source program
    /// @param builder the builder of the object in #dst that will be inserted after any occurrence
    /// of the clone of @p after
    /// @returns this CloneContext so calls can be chained
    template <typename T,
              size_t N,
              typename AFTER,
              typename BUILDER,
              typename _ = std::enable_if_t<!std::is_pointer_v<std::decay_t<BUILDER>>>>
    CloneContext& InsertAfter(const tint::Vector<T, N>& vector,
                              const AFTER* after,
                              BUILDER&& builder) {
        list_transforms_.GetOrAddZero(&vector).insert_after_.GetOrAddZero(after).Push(
            std::forward<BUILDER>(builder));
        return *this;
    }

    /// The target Builder to clone into.
    Builder* const dst;

    /// The source Program generation identifier.
    GenerationID const src_id;

  private:
    struct CloneableTransform {
        /// Constructor
        CloneableTransform();
        /// Copy constructor
        /// @param other the CloneableTransform to copy
        CloneableTransform(const CloneableTransform& other);
        /// Destructor
        ~CloneableTransform();

        // TypeInfo of the Node that the transform operates on
        const TypeInfo* typeinfo;
        std::function<const ast::Node*(const ast::Node*)> function;
    };

    /// A vector of const ast::Node*
    using NodeBuilderList = Vector<std::function<const ast::Node*()>, 4>;

    /// Transformations to be applied to a list (vector)
    struct ListTransforms {
        /// A map of object in the source program to omit when cloned into #dst.
        Hashset<const ast::Node*, 4> remove_;

        /// A list of objects in #dst to insert before any others when the vector is cloned.
        NodeBuilderList insert_front_;

        /// A list of objects in #dst to insert after all others when the vector is cloned.
        NodeBuilderList insert_back_;

        /// A map of object in the source program to the list of cloned objects in #dst.
        /// Clone(const Vector<T*>& v) will use this to insert the map-value
        /// list into the target vector before cloning and inserting the map-key.
        Hashmap<const ast::Node*, NodeBuilderList, 4> insert_before_;

        /// A map of object in the source program to the list of cloned objects in #dst.
        /// Clone(const Vector<T*>& v) will use this to insert the map-value
        /// list into the target vector after cloning and inserting the map-key.
        Hashmap<const ast::Node*, NodeBuilderList, 4> insert_after_;
    };

    CloneContext(const CloneContext&) = delete;
    CloneContext& operator=(const CloneContext&) = delete;

    /// Cast `obj` from type `FROM` to type `TO`, returning the cast object.
    /// Reports an internal compiler error if the cast failed.
    template <typename TO, typename FROM>
    const TO* CheckedCast(const FROM* obj) {
        if (obj == nullptr) {
            return nullptr;
        }
        const TO* cast = obj->template As<TO>();
        if (DAWN_LIKELY(cast)) {
            return cast;
        }
        CheckedCastFailure(obj, tint::TypeInfo::Of<TO>());
    }

    /// Clones a Node object, using any replacements or transforms that have
    /// been configured.
    const ast::Node* CloneNode(const ast::Node* object);

    /// Aborts with an ICE describing that the cloned object type was not as required.
    [[noreturn]] void CheckedCastFailure(const ast::Node* got, const TypeInfo& expected);

    /// @returns the diagnostic list of #dst
    diag::List& Diagnostics() const;

    /// A map of object in the source program to functions that create their replacement in #dst
    Hashmap<const ast::Node*, std::function<const ast::Node*()>, 8> replacements_;

    /// A map of symbol in the source program to their cloned equivalent in #dst
    Hashmap<Symbol, Symbol, 32> cloned_symbols_;

    /// Node transform functions registered with ReplaceAll()
    Vector<CloneableTransform, 8> transforms_;

    /// Transformations to apply to vectors
    Hashmap<const void*, ListTransforms, 4> list_transforms_;

    /// Symbol transform registered with ReplaceAll()
    SymbolTransform symbol_transform_;
};

}  // namespace tint::ast

#endif  // SRC_TINT_LANG_WGSL_AST_CLONE_CONTEXT_H_
