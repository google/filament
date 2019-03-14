/*
 * Copyright 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifndef MATH_TVECHELPERS_H_
#define MATH_TVECHELPERS_H_

#include <math.h>
#include <stdint.h>
#include <sys/types.h>

#include <cmath>
#include <functional>
#include <limits>
#include <iostream>

#include <math/compiler.h>

namespace filament {
namespace math {
namespace details {
// -------------------------------------------------------------------------------------

/*
 * No user serviceable parts here.
 *
 * Don't use this file directly, instead include math/vec{2|3|4}.h
 */

/*
 * TVec{Add|Product}Operators implements basic arithmetic and basic compound assignments
 * operators on a vector of type BASE<T>.
 *
 * BASE only needs to implement operator[] and size().
 * By simply inheriting from TVec{Add|Product}Operators<BASE, T> BASE will automatically
 * get all the functionality here.
 */

template <template<typename T> class VECTOR, typename T>
class TVecAddOperators {
public:
    /* compound assignment from a another vector of the same size but different
     * element type.
     */
    template<typename OTHER>
    constexpr VECTOR<T>& operator +=(const VECTOR<OTHER>& v) {
        VECTOR<T>& lhs = static_cast<VECTOR<T>&>(*this);
        for (size_t i = 0; i < lhs.size(); i++) {
            lhs[i] += v[i];
        }
        return lhs;
    }
    template<typename OTHER>
    constexpr VECTOR<T>& operator -=(const VECTOR<OTHER>& v) {
        VECTOR<T>& lhs = static_cast<VECTOR<T>&>(*this);
        for (size_t i = 0; i < lhs.size(); i++) {
            lhs[i] -= v[i];
        }
        return lhs;
    }

    /* compound assignment from a another vector of the same type.
     * These operators can be used for implicit conversion and  handle operations
     * like "vector *= scalar" by letting the compiler implicitly convert a scalar
     * to a vector (assuming the BASE<T> allows it).
     */
    constexpr VECTOR<T>& operator +=(const VECTOR<T>& v) {
        VECTOR<T>& lhs = static_cast<VECTOR<T>&>(*this);
        for (size_t i = 0; i < lhs.size(); i++) {
            lhs[i] += v[i];
        }
        return lhs;
    }
    constexpr VECTOR<T>& operator -=(const VECTOR<T>& v) {
        VECTOR<T>& lhs = static_cast<VECTOR<T>&>(*this);
        for (size_t i = 0; i < lhs.size(); i++) {
            lhs[i] -= v[i];
        }
        return lhs;
    }

    /*
     * NOTE: the functions below ARE NOT member methods. They are friend functions
     * with they definition inlined with their declaration. This makes these
     * template functions available to the compiler when (and only when) this class
     * is instantiated, at which point they're only templated on the 2nd parameter
     * (the first one, BASE<T> being known).
     */

    /* The operators below handle operation between vectors of the same size
     * but of a different element type.
     */
    template<typename RT>
    friend inline constexpr VECTOR<T> MATH_PURE operator +(VECTOR<T> lv, const VECTOR<RT>& rv) {
        // don't pass lv by reference because we need a copy anyways
        return lv += rv;
    }
    template<typename RT>
    friend inline constexpr VECTOR<T> MATH_PURE operator -(VECTOR<T> lv, const VECTOR<RT>& rv) {
        // don't pass lv by reference because we need a copy anyways
        return lv -= rv;
    }

    /* The operators below (which are not templates once this class is instanced,
     * i.e.: BASE<T> is known) can be used for implicit conversion on both sides.
     * These handle operations like "vector + scalar" and "scalar + vector" by
     * letting the compiler implicitly convert a scalar to a vector (assuming
     * the BASE<T> allows it).
     */
    friend inline constexpr VECTOR<T> MATH_PURE operator +(VECTOR<T> lv, const VECTOR<T>& rv) {
        // don't pass lv by reference because we need a copy anyways
        return lv += rv;
    }
    friend inline constexpr VECTOR<T> MATH_PURE operator -(VECTOR<T> lv, const VECTOR<T>& rv) {
        // don't pass lv by reference because we need a copy anyways
        return lv -= rv;
    }
};

template<template<typename T> class VECTOR, typename T>
class TVecProductOperators {
public:
    /* compound assignment from a another vector of the same size but different
     * element type.
     */
    template<typename OTHER>
    constexpr VECTOR<T>& operator *=(const VECTOR<OTHER>& v) {
        VECTOR<T>& lhs = static_cast<VECTOR<T>&>(*this);
        for (size_t i = 0; i < lhs.size(); i++) {
            lhs[i] *= v[i];
        }
        return lhs;
    }
    template<typename OTHER>
    constexpr VECTOR<T>& operator /=(const VECTOR<OTHER>& v) {
        VECTOR<T>& lhs = static_cast<VECTOR<T>&>(*this);
        for (size_t i = 0; i < lhs.size(); i++) {
            lhs[i] /= v[i];
        }
        return lhs;
    }

    /* compound assignment from a another vector of the same type.
     * These operators can be used for implicit conversion and  handle operations
     * like "vector *= scalar" by letting the compiler implicitly convert a scalar
     * to a vector (assuming the BASE<T> allows it).
     */
    constexpr VECTOR<T>& operator *=(const VECTOR<T>& v) {
        VECTOR<T>& lhs = static_cast<VECTOR<T>&>(*this);
        for (size_t i = 0; i < lhs.size(); i++) {
            lhs[i] *= v[i];
        }
        return lhs;
    }
    constexpr VECTOR<T>& operator /=(const VECTOR<T>& v) {
        VECTOR<T>& lhs = static_cast<VECTOR<T>&>(*this);
        for (size_t i = 0; i < lhs.size(); i++) {
            lhs[i] /= v[i];
        }
        return lhs;
    }

    /*
     * NOTE: the functions below ARE NOT member methods. They are friend functions
     * with they definition inlined with their declaration. This makes these
     * template functions available to the compiler when (and only when) this class
     * is instantiated, at which point they're only templated on the 2nd parameter
     * (the first one, BASE<T> being known).
     */

    /* The operators below handle operation between vectors of the same size
     * but of a different element type.
     */
    template<typename RT>
    friend inline constexpr VECTOR<T> MATH_PURE operator *(VECTOR<T> lv, const VECTOR<RT>& rv) {
        // don't pass lv by reference because we need a copy anyways
        return lv *= rv;
    }
    template<typename RT>
    friend inline constexpr VECTOR<T> MATH_PURE operator /(VECTOR<T> lv, const VECTOR<RT>& rv) {
        // don't pass lv by reference because we need a copy anyways
        return lv /= rv;
    }

    /* The operators below (which are not templates once this class is instanced,
     * i.e.: BASE<T> is known) can be used for implicit conversion on both sides.
     * These handle operations like "vector * scalar" and "scalar * vector" by
     * letting the compiler implicitly convert a scalar to a vector (assuming
     * the BASE<T> allows it).
     */
    friend inline constexpr VECTOR<T> MATH_PURE operator *(VECTOR<T> lv, const VECTOR<T>& rv) {
        // don't pass lv by reference because we need a copy anyways
        return lv *= rv;
    }
    friend inline constexpr VECTOR<T> MATH_PURE operator /(VECTOR<T> lv, const VECTOR<T>& rv) {
        // don't pass lv by reference because we need a copy anyways
        return lv /= rv;
    }
};

/*
 * TVecUnaryOperators implements unary operators on a vector of type BASE<T>.
 *
 * BASE only needs to implement operator[] and size().
 * By simply inheriting from TVecUnaryOperators<BASE, T> BASE will automatically
 * get all the functionality here.
 *
 * These operators are implemented as friend functions of TVecUnaryOperators<BASE, T>
 */
template<template<typename T> class VECTOR, typename T>
class TVecUnaryOperators {
public:
    VECTOR<T> operator -() const {
        VECTOR<T> r;
        VECTOR<T> const& rv(static_cast<VECTOR<T> const&>(*this));
        for (size_t i = 0; i < r.size(); i++) {
            r[i] = -rv[i];
        }
        return r;
    }
};

/*
 * TVecComparisonOperators implements relational/comparison operators
 * on a vector of type BASE<T>.
 *
 * BASE only needs to implement operator[] and size().
 * By simply inheriting from TVecComparisonOperators<BASE, T> BASE will automatically
 * get all the functionality here.
 */
template<template<typename T> class VECTOR, typename T>
class TVecComparisonOperators {
public:
    /*
     * NOTE: the functions below ARE NOT member methods. They are friend functions
     * with they definition inlined with their declaration. This makes these
     * template functions available to the compiler when (and only when) this class
     * is instantiated, at which point they're only templated on the 2nd parameter
     * (the first one, BASE<T> being known).
     */
    template<typename RT>
    friend inline
    bool MATH_PURE operator ==(const VECTOR<T>& lv, const VECTOR<RT>& rv) {
        // w/ inlining we end-up with many branches that will pollute the BPU cache
        MATH_NOUNROLL
        for (size_t i = 0; i < lv.size(); i++)
            if (lv[i] != rv[i])
                return false;
        return true;
    }

    template<typename RT>
    friend inline
    bool MATH_PURE operator !=(const VECTOR<T>& lv, const VECTOR<RT>& rv) {
        return !operator ==(lv, rv);
    }

    template<typename RT>
    friend inline
    VECTOR<bool> MATH_PURE equal(const VECTOR<T>& lv, const VECTOR<RT>& rv) {
        VECTOR<bool> r;
        for (size_t i = 0; i < lv.size(); i++) {
            r[i] = lv[i] == rv[i];
        }
        return r;
    }

    template<typename RT>
    friend inline
    VECTOR<bool> MATH_PURE notEqual(const VECTOR<T>& lv, const VECTOR<RT>& rv) {
        VECTOR<bool> r;
        for (size_t i = 0; i < lv.size(); i++) {
            r[i] = lv[i] != rv[i];
        }
        return r;
    }

    template<typename RT>
    friend inline
    VECTOR<bool> MATH_PURE lessThan(const VECTOR<T>& lv, const VECTOR<RT>& rv) {
        VECTOR<bool> r;
        for (size_t i = 0; i < lv.size(); i++) {
            r[i] = lv[i] < rv[i];
        }
        return r;
    }

    template<typename RT>
    friend inline
    VECTOR<bool> MATH_PURE lessThanEqual(const VECTOR<T>& lv, const VECTOR<RT>& rv) {
        VECTOR<bool> r;
        for (size_t i = 0; i < lv.size(); i++) {
            r[i] = lv[i] <= rv[i];
        }
        return r;
    }

    template<typename RT>
    friend inline
    VECTOR<bool> MATH_PURE greaterThan(const VECTOR<T>& lv, const VECTOR<RT>& rv) {
        VECTOR<bool> r;
        for (size_t i = 0; i < lv.size(); i++) {
            r[i] = lv[i] > rv[i];
        }
        return r;
    }

    template<typename RT>
    friend inline
    VECTOR<bool> MATH_PURE greaterThanEqual(const VECTOR<T>& lv, const VECTOR<RT>& rv) {
        VECTOR<bool> r;
        for (size_t i = 0; i < lv.size(); i++) {
            r[i] = lv[i] >= rv[i];
        }
        return r;
    }
};

/*
 * TVecFunctions implements functions on a vector of type BASE<T>.
 *
 * BASE only needs to implement operator[] and size().
 * By simply inheriting from TVecFunctions<BASE, T> BASE will automatically
 * get all the functionality here.
 */
template<template<typename T> class VECTOR, typename T>
class TVecFunctions {
public:
    /*
     * NOTE: the functions below ARE NOT member methods. They are friend functions
     * with they definition inlined with their declaration. This makes these
     * template functions available to the compiler when (and only when) this class
     * is instantiated, at which point they're only templated on the 2nd parameter
     * (the first one, BASE<T> being known).
     */
    template<typename RT>
    friend constexpr inline T MATH_PURE dot(const VECTOR<T>& lv, const VECTOR<RT>& rv) {
        T r(0);
        for (size_t i = 0; i < lv.size(); i++) {
            r += lv[i] * rv[i];
        }
        return r;
    }

    friend inline T MATH_PURE norm(const VECTOR<T>& lv) {
        return std::sqrt(dot(lv, lv));
    }

    friend inline T MATH_PURE length(const VECTOR<T>& lv) {
        return norm(lv);
    }

    friend inline constexpr T MATH_PURE norm2(const VECTOR<T>& lv) {
        return dot(lv, lv);
    }

    friend inline constexpr T MATH_PURE length2(const VECTOR<T>& lv) {
        return norm2(lv);
    }

    template<typename RT>
    friend inline constexpr T MATH_PURE distance(const VECTOR<T>& lv, const VECTOR<RT>& rv) {
        return length(rv - lv);
    }

    template<typename RT>
    friend inline constexpr T MATH_PURE distance2(const VECTOR<T>& lv, const VECTOR<RT>& rv) {
        return length2(rv - lv);
    }

    friend inline constexpr VECTOR<T> MATH_PURE normalize(const VECTOR<T>& lv) {
        return lv * (T(1) / length(lv));
    }

    friend inline VECTOR<T> MATH_PURE rcp(VECTOR<T> v) {
        return T(1) / v;
    }

    friend inline VECTOR<T> MATH_PURE abs(VECTOR<T> v) {
        for (size_t i=0 ; i<v.size() ; i++) {
            v[i] = std::abs(v[i]);
        }
        return v;
    }

    friend inline VECTOR<T> MATH_PURE floor(VECTOR<T> v) {
        for (size_t i=0 ; i<v.size() ; i++) {
            v[i] = std::floor(v[i]);
        }
        return v;
    }

    friend inline VECTOR<T> MATH_PURE ceil(VECTOR<T> v) {
        for (size_t i=0 ; i<v.size() ; i++) {
            v[i] = std::ceil(v[i]);
        }
        return v;
    }

    friend inline VECTOR<T> MATH_PURE round(VECTOR<T> v) {
        for (size_t i=0 ; i<v.size() ; i++) {
            v[i] = std::round(v[i]);
        }
        return v;
    }

    friend inline VECTOR<T> MATH_PURE inversesqrt(VECTOR<T> v) {
        for (size_t i=0 ; i<v.size() ; i++) {
            v[i] = T(1) / std::sqrt(v[i]);
        }
        return v;
    }

    friend inline VECTOR<T> MATH_PURE sqrt(VECTOR<T> v) {
        for (size_t i=0 ; i<v.size() ; i++) {
            v[i] = std::sqrt(v[i]);
        }
        return v;
    }

    friend inline VECTOR<T> MATH_PURE pow(VECTOR<T> v, T p) {
        for (size_t i=0 ; i<v.size() ; i++) {
            v[i] = std::pow(v[i], p);
        }
        return v;
    }

    friend inline VECTOR<T> MATH_PURE saturate(const VECTOR<T>& lv) {
        return clamp(lv, T(0), T(1));
    }

    friend inline VECTOR<T> MATH_PURE clamp(VECTOR<T> v, T min, T max) {
        for (size_t i=0 ; i< v.size() ; i++) {
            v[i] = std::min(max, std::max(min, v[i]));
        }
        return v;
    }

    friend inline VECTOR<T> MATH_PURE clamp(VECTOR<T> v, VECTOR<T> min, VECTOR<T> max) {
        for (size_t i=0 ; i< v.size() ; i++) {
            v[i] = std::min(max[i], std::max(min[i], v[i]));
        }
        return v;
    }

    friend inline VECTOR<T> MATH_PURE fma(const VECTOR<T>& lv, const VECTOR<T>& rv, VECTOR<T> a) {
        for (size_t i=0 ; i<lv.size() ; i++) {
            //a[i] = std::fma(lv[i], rv[i], a[i]);
            a[i] += (lv[i] * rv[i]);
        }
        return a;
    }

    friend inline VECTOR<T> MATH_PURE min(const VECTOR<T>& u, VECTOR<T> v) {
        for (size_t i=0 ; i<v.size() ; i++) {
            v[i] = std::min(u[i], v[i]);
        }
        return v;
    }

    friend inline VECTOR<T> MATH_PURE max(const VECTOR<T>& u, VECTOR<T> v) {
        for (size_t i=0 ; i<v.size() ; i++) {
            v[i] = std::max(u[i], v[i]);
        }
        return v;
    }

    friend inline T MATH_PURE max(const VECTOR<T>& v) {
        T r(std::numeric_limits<T>::lowest());
        for (size_t i=0 ; i<v.size() ; i++) {
            r = std::max(r, v[i]);
        }
        return r;
    }

    friend inline T MATH_PURE min(const VECTOR<T>& v) {
        T r(std::numeric_limits<T>::max());
        for (size_t i=0 ; i<v.size() ; i++) {
            r = std::min(r, v[i]);
        }
        return r;
    }

    friend inline VECTOR<T> MATH_PURE apply(VECTOR<T> v, const std::function<T(T)>& f) {
        for (size_t i = 0; i < v.size(); i++) {
            v[i] = f(v[i]);
        }
        return v;
    }

    friend inline bool MATH_PURE any(const VECTOR<T>& v) {
        for (size_t i = 0; i < v.size(); i++) {
            if (v[i] != T(0)) return true;
        }
        return false;
    }

    friend inline bool MATH_PURE all(const VECTOR<T>& v) {
        bool result = true;
        for (size_t i = 0; i < v.size(); i++) {
            result &= (v[i] != T(0));
        }
        return result;
    }

    template<typename R>
    friend inline VECTOR<R> MATH_PURE map(VECTOR<T> v, const std::function<R(T)>& f) {
        VECTOR<R> result;
        for (size_t i = 0; i < v.size(); i++) {
            result[i] = f(v[i]);
        }
        return result;
    }
};

/*
 * TVecDebug implements functions on a vector of type BASE<T>.
 *
 * BASE only needs to implement operator[] and size().
 * By simply inheriting from TVecDebug<BASE, T> BASE will automatically
 * get all the functionality here.
 */
template<template<typename T> class VECTOR, typename T>
class TVecDebug {
public:
    /*
     * NOTE: the functions below ARE NOT member methods. They are friend functions
     * with they definition inlined with their declaration. This makes these
     * template functions available to the compiler when (and only when) this class
     * is instantiated, at which point they're only templated on the 2nd parameter
     * (the first one, BASE<T> being known).
     */
    friend std::ostream& operator<<(std::ostream& stream, const VECTOR<T>& v) {
        stream << "< ";
        for (size_t i = 0; i < v.size() - 1; i++) {
            stream << T(v[i]) << ", ";
        }
        stream << T(v[v.size() - 1]) << " >";
        return stream;
    }
};

// -------------------------------------------------------------------------------------
}  // namespace details
}  // namespace math
}  // namespace filament

#endif  // MATH_TVECHELPERS_H_
