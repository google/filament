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

#ifndef TNT_MATH_TVECHELPERS_H
#define TNT_MATH_TVECHELPERS_H

#include <math/compiler.h>

#include <cmath>            // for std:: namespace

#include <stdint.h>
#include <sys/types.h>

namespace filament::math::details {

template<typename U>
inline constexpr U min(U a, U b) noexcept {
    return a < b ? a : b;
}

template<typename U>
inline constexpr U max(U a, U b) noexcept {
    return a > b ? a : b;
}

template<typename T, typename U>
struct arithmetic_result {
    using type = decltype(std::declval<T>() + std::declval<U>());
};

template<typename T, typename U>
using arithmetic_result_t = typename arithmetic_result<T, U>::type;

template<typename A, typename B = int, typename C = int, typename D = int>
using enable_if_arithmetic_t = std::enable_if_t<
        is_arithmetic<A>::value &&
        is_arithmetic<B>::value &&
        is_arithmetic<C>::value &&
        is_arithmetic<D>::value>;

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

template<template<typename T> class VECTOR, typename T>
class TVecAddOperators {
public:
    /* compound assignment from a another vector of the same size but different
     * element type.
     */
    template<typename U>
    constexpr VECTOR<T>& operator+=(const VECTOR<U>& v) {
        VECTOR<T>& lhs = static_cast<VECTOR<T>&>(*this);
        for (size_t i = 0; i < lhs.size(); i++) {
            lhs[i] += v[i];
        }
        return lhs;
    }

    template<typename U, typename = enable_if_arithmetic_t<U>>
    constexpr VECTOR<T>& operator+=(U v) {
        return operator+=(VECTOR<U>(v));
    }

    template<typename U>
    constexpr VECTOR<T>& operator-=(const VECTOR<U>& v) {
        VECTOR<T>& lhs = static_cast<VECTOR<T>&>(*this);
        for (size_t i = 0; i < lhs.size(); i++) {
            lhs[i] -= v[i];
        }
        return lhs;
    }

    template<typename U, typename = enable_if_arithmetic_t<U>>
    constexpr VECTOR<T>& operator-=(U v) {
        return operator-=(VECTOR<U>(v));
    }

private:
    /*
     * NOTE: the functions below ARE NOT member methods. They are friend functions
     * with they definition inlined with their declaration. This makes these
     * template functions available to the compiler when (and only when) this class
     * is instantiated, at which point they're only templated on the 2nd parameter
     * (the first one, BASE<T> being known).
     */

    template<typename U>
    friend inline constexpr
    VECTOR<arithmetic_result_t<T, U>> MATH_PURE operator+(const VECTOR<T>& lv, const VECTOR<U>& rv) {
        VECTOR<arithmetic_result_t<T, U>> res(lv);
        res += rv;
        return res;
    }

    template<typename U, typename = enable_if_arithmetic_t<U>>
    friend inline constexpr
    VECTOR<arithmetic_result_t<T, U>> MATH_PURE operator+(const VECTOR<T>& lv, U rv) {
        return lv + VECTOR<U>(rv);
    }

    template<typename U, typename = enable_if_arithmetic_t<U>>
    friend inline constexpr
    VECTOR<arithmetic_result_t<T, U>> MATH_PURE operator+(U lv, const VECTOR<T>& rv) {
        return VECTOR<U>(lv) + rv;
    }

    template<typename U>
    friend inline constexpr
    VECTOR<arithmetic_result_t<T, U>> MATH_PURE operator-(const VECTOR<T>& lv, const VECTOR<U>& rv) {
        VECTOR<arithmetic_result_t<T, U>> res(lv);
        res -= rv;
        return res;
    }

    template<typename U, typename = enable_if_arithmetic_t<U>>
    friend inline constexpr
    VECTOR<arithmetic_result_t<T, U>> MATH_PURE operator-(const VECTOR<T>& lv, U rv) {
        return lv - VECTOR<U>(rv);
    }

    template<typename U, typename = enable_if_arithmetic_t<U>>
    friend inline constexpr
    VECTOR<arithmetic_result_t<T, U>> MATH_PURE operator-(U lv, const VECTOR<T>& rv) {
        return VECTOR<U>(lv) - rv;
    }
};

template<template<typename T> class VECTOR, typename T>
class TVecProductOperators {
public:
    /* compound assignment from a another vector of the same size but different
     * element type.
     */
    template<typename U>
    constexpr VECTOR<T>& operator*=(const VECTOR<U>& v) {
        VECTOR<T>& lhs = static_cast<VECTOR<T>&>(*this);
        for (size_t i = 0; i < lhs.size(); i++) {
            lhs[i] *= v[i];
        }
        return lhs;
    }

    template<typename U, typename = enable_if_arithmetic_t<U>>
    constexpr VECTOR<T>& operator*=(U v) {
        return operator*=(VECTOR<U>(v));
    }

    template<typename U>
    constexpr VECTOR<T>& operator/=(const VECTOR<U>& v) {
        VECTOR<T>& lhs = static_cast<VECTOR<T>&>(*this);
        for (size_t i = 0; i < lhs.size(); i++) {
            lhs[i] /= v[i];
        }
        return lhs;
    }

    template<typename U, typename = enable_if_arithmetic_t<U>>
    constexpr VECTOR<T>& operator/=(U v) {
        return operator/=(VECTOR<U>(v));
    }

private:
    /*
     * NOTE: the functions below ARE NOT member methods. They are friend functions
     * with they definition inlined with their declaration. This makes these
     * template functions available to the compiler when (and only when) this class
     * is instantiated, at which point they're only templated on the 2nd parameter
     * (the first one, BASE<T> being known).
     */

    template<typename U>
    friend inline constexpr
    VECTOR<arithmetic_result_t<T, U>> MATH_PURE operator*(const VECTOR<T>& lv, const VECTOR<U>& rv) {
        VECTOR<arithmetic_result_t<T, U>> res(lv);
        res *= rv;
        return res;
    }

    template<typename U, typename = enable_if_arithmetic_t<U>>
    friend inline constexpr
    VECTOR<arithmetic_result_t<T, U>> MATH_PURE operator*(const VECTOR<T>& lv, U rv) {
        return lv * VECTOR<U>(rv);
    }

    template<typename U, typename = enable_if_arithmetic_t<U>>
    friend inline constexpr
    VECTOR<arithmetic_result_t<T, U>> MATH_PURE operator*(U lv, const VECTOR<T>& rv) {
        return VECTOR<U>(lv) * rv;
    }

    template<typename U>
    friend inline constexpr
    VECTOR<arithmetic_result_t<T, U>> MATH_PURE operator/(const VECTOR<T>& lv, const VECTOR<U>& rv) {
        VECTOR<arithmetic_result_t<T, U>> res(lv);
        res /= rv;
        return res;
    }

    template<typename U, typename = enable_if_arithmetic_t<U>>
    friend inline constexpr
    VECTOR<arithmetic_result_t<T, U>> MATH_PURE operator/(const VECTOR<T>& lv, U rv) {
        return lv / VECTOR<U>(rv);
    }

    template<typename U, typename = enable_if_arithmetic_t<U>>
    friend inline constexpr
    VECTOR<arithmetic_result_t<T, U>> MATH_PURE operator/(U lv, const VECTOR<T>& rv) {
        return VECTOR<U>(lv) / rv;
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
    constexpr VECTOR<T> operator-() const {
        VECTOR<T> r{};
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
private:
    /*
     * NOTE: the functions below ARE NOT member methods. They are friend functions
     * with they definition inlined with their declaration. This makes these
     * template functions available to the compiler when (and only when) this class
     * is instantiated, at which point they're only templated on the 2nd parameter
     * (the first one, BASE<T> being known).
     */
    template<typename U>
    friend inline constexpr
    bool MATH_PURE operator==(const VECTOR<T>& lv, const VECTOR<U>& rv) {
        for (size_t i = 0; i < lv.size(); i++) {
            if (lv[i] != rv[i]) {
                return false;
            }
        }
        return true;
    }

    template<typename U>
    friend inline constexpr
    bool MATH_PURE operator!=(const VECTOR<T>& lv, const VECTOR<U>& rv) {
        return !operator==(lv, rv);
    }

    template<typename U>
    friend inline constexpr
    VECTOR<bool> MATH_PURE equal(const VECTOR<T>& lv, const VECTOR<U>& rv) {
        VECTOR<bool> r{};
        for (size_t i = 0; i < lv.size(); i++) {
            r[i] = lv[i] == rv[i];
        }
        return r;
    }

    template<typename U>
    friend inline constexpr
    VECTOR<bool> MATH_PURE notEqual(const VECTOR<T>& lv, const VECTOR<U>& rv) {
        VECTOR<bool> r{};
        for (size_t i = 0; i < lv.size(); i++) {
            r[i] = lv[i] != rv[i];
        }
        return r;
    }

    template<typename U>
    friend inline constexpr
    VECTOR<bool> MATH_PURE lessThan(const VECTOR<T>& lv, const VECTOR<U>& rv) {
        VECTOR<bool> r{};
        for (size_t i = 0; i < lv.size(); i++) {
            r[i] = lv[i] < rv[i];
        }
        return r;
    }

    template<typename U>
    friend inline constexpr
    VECTOR<bool> MATH_PURE lessThanEqual(const VECTOR<T>& lv, const VECTOR<U>& rv) {
        VECTOR<bool> r{};
        for (size_t i = 0; i < lv.size(); i++) {
            r[i] = lv[i] <= rv[i];
        }
        return r;
    }

    template<typename U>
    friend inline constexpr
    VECTOR<bool> MATH_PURE greaterThan(const VECTOR<T>& lv, const VECTOR<U>& rv) {
        VECTOR<bool> r;
        for (size_t i = 0; i < lv.size(); i++) {
            r[i] = lv[i] > rv[i];
        }
        return r;
    }

    template<typename U>
    friend inline
    VECTOR<bool> MATH_PURE greaterThanEqual(const VECTOR<T>& lv, const VECTOR<U>& rv) {
        VECTOR<bool> r{};
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
private:
    /*
     * NOTE: the functions below ARE NOT member methods. They are friend functions
     * with they definition inlined with their declaration. This makes these
     * template functions available to the compiler when (and only when) this class
     * is instantiated, at which point they're only templated on the 2nd parameter
     * (the first one, BASE<T> being known).
     */
    template<typename U>
    friend constexpr inline
    arithmetic_result_t<T, U> MATH_PURE dot(const VECTOR<T>& lv, const VECTOR<U>& rv) {
        arithmetic_result_t<T, U> r{};
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

    template<typename U>
    friend inline constexpr
    arithmetic_result_t<T, U> MATH_PURE distance(const VECTOR<T>& lv, const VECTOR<U>& rv) {
        return length(rv - lv);
    }

    template<typename U>
    friend inline constexpr
    arithmetic_result_t<T, U> MATH_PURE distance2(const VECTOR<T>& lv, const VECTOR<U>& rv) {
        return length2(rv - lv);
    }

    friend inline VECTOR<T> MATH_PURE normalize(const VECTOR<T>& lv) {
        return lv * (T(1) / length(lv));
    }

    friend inline VECTOR<T> MATH_PURE rcp(VECTOR<T> v) {
        return T(1) / v;
    }

    friend inline constexpr VECTOR<T> MATH_PURE abs(VECTOR<T> v) {
        for (size_t i = 0; i < v.size(); i++) {
            v[i] = v[i] < 0 ? -v[i] : v[i];
        }
        return v;
    }

    friend inline VECTOR<T> MATH_PURE floor(VECTOR<T> v) {
        for (size_t i = 0; i < v.size(); i++) {
            v[i] = std::floor(v[i]);
        }
        return v;
    }

    friend inline VECTOR<T> MATH_PURE ceil(VECTOR<T> v) {
        for (size_t i = 0; i < v.size(); i++) {
            v[i] = std::ceil(v[i]);
        }
        return v;
    }

    friend inline VECTOR<T> MATH_PURE round(VECTOR<T> v) {
        for (size_t i = 0; i < v.size(); i++) {
            v[i] = std::round(v[i]);
        }
        return v;
    }

    template<typename U>
    friend inline
    VECTOR<T> MATH_PURE fmod(VECTOR<T> const& x, VECTOR<U> const& y) {
        VECTOR<T> r;
        for (size_t i = 0; i < r.size(); i++) {
            r[i] = std::fmod(x[i], y[i]);
        }
        return r;
    }

    template<typename U>
    friend inline
    VECTOR<T> MATH_PURE remainder(VECTOR<T> const& x, VECTOR<U> const& y) {
        VECTOR<T> r;
        for (size_t i = 0; i < r.size(); i++) {
            r[i] = std::remainder(x[i], y[i]);
        }
        return r;
    }

    template<typename U>
    friend inline
    VECTOR<T> MATH_PURE remquo(VECTOR<T> const& x, VECTOR<U> const& y,
            VECTOR<int>* q) {
        VECTOR<T> r;
        for (size_t i = 0; i < r.size(); i++) {
            r[i] = std::remquo(x[i], y[i], &((*q)[i]));
        }
        return r;
    }

    friend inline VECTOR<T> MATH_PURE inversesqrt(VECTOR<T> v) {
        for (size_t i = 0; i < v.size(); i++) {
            v[i] = T(1) / std::sqrt(v[i]);
        }
        return v;
    }

    friend inline VECTOR<T> MATH_PURE sqrt(VECTOR<T> v) {
        for (size_t i = 0; i < v.size(); i++) {
            v[i] = std::sqrt(v[i]);
        }
        return v;
    }

    friend inline VECTOR<T> MATH_PURE cbrt(VECTOR<T> v) {
        for (size_t i = 0; i < v.size(); i++) {
            v[i] = std::cbrt(v[i]);
        }
        return v;
    }

    friend inline VECTOR<T> MATH_PURE exp(VECTOR<T> v) {
        for (size_t i = 0; i < v.size(); i++) {
            v[i] = std::exp(v[i]);
        }
        return v;
    }

    friend inline VECTOR<T> MATH_PURE sign(VECTOR<T> v) {
        for (size_t i = 0; i < v.size(); i++) {
            v[i] = std::copysign(T(1), v[i]);
        }
        return v;
    }

    friend inline VECTOR<T> MATH_PURE pow(VECTOR<T> v, T p) {
        for (size_t i = 0; i < v.size(); i++) {
            v[i] = std::pow(v[i], p);
        }
        return v;
    }

    friend inline VECTOR<T> MATH_PURE pow(T v, VECTOR<T> p) {
        for (size_t i = 0; i < p.size(); i++) {
            p[i] = std::pow(v, p[i]);
        }
        return p;
    }

    friend inline VECTOR<T> MATH_PURE pow(VECTOR<T> v, VECTOR<T> p) {
        for (size_t i = 0; i < v.size(); i++) {
            v[i] = std::pow(v[i], p[i]);
        }
        return v;
    }

    friend inline VECTOR<T> MATH_PURE log(VECTOR<T> v) {
        for (size_t i = 0; i < v.size(); i++) {
            v[i] = std::log(v[i]);
        }
        return v;
    }

    friend inline VECTOR<T> MATH_PURE log10(VECTOR<T> v) {
        for (size_t i = 0; i < v.size(); i++) {
            v[i] = std::log10(v[i]);
        }
        return v;
    }

    friend inline VECTOR<T> MATH_PURE log2(VECTOR<T> v) {
        for (size_t i = 0; i < v.size(); i++) {
            v[i] = std::log2(v[i]);
        }
        return v;
    }

    friend inline constexpr VECTOR<T> MATH_PURE saturate(const VECTOR<T>& lv) {
        return clamp(lv, T(0), T(1));
    }

    friend inline constexpr VECTOR<T> MATH_PURE clamp(VECTOR<T> v, T min, T max) {
        for (size_t i = 0; i < v.size(); i++) {
            v[i] = details::min(max, details::max(min, v[i]));
        }
        return v;
    }

    friend inline constexpr VECTOR<T> MATH_PURE clamp(VECTOR<T> v, VECTOR<T> min, VECTOR<T> max) {
        for (size_t i = 0; i < v.size(); i++) {
            v[i] = details::min(max[i], details::max(min[i], v[i]));
        }
        return v;
    }

    friend inline constexpr VECTOR<T> MATH_PURE fma(const VECTOR<T>& lv, const VECTOR<T>& rv,
            VECTOR<T> a) {
        for (size_t i = 0; i < lv.size(); i++) {
            a[i] += (lv[i] * rv[i]);
        }
        return a;
    }

    friend inline constexpr VECTOR<T> MATH_PURE min(const VECTOR<T>& u, VECTOR<T> v) {
        for (size_t i = 0; i < v.size(); i++) {
            v[i] = details::min(u[i], v[i]);
        }
        return v;
    }

    friend inline constexpr VECTOR<T> MATH_PURE max(const VECTOR<T>& u, VECTOR<T> v) {
        for (size_t i = 0; i < v.size(); i++) {
            v[i] = details::max(u[i], v[i]);
        }
        return v;
    }

    friend inline constexpr T MATH_PURE max(const VECTOR<T>& v) {
        T r(v[0]);
        for (size_t i = 1; i < v.size(); i++) {
            r = max(r, v[i]);
        }
        return r;
    }

    friend inline constexpr T MATH_PURE min(const VECTOR<T>& v) {
        T r(v[0]);
        for (size_t i = 1; i < v.size(); i++) {
            r = min(r, v[i]);
        }
        return r;
    }

    friend inline constexpr VECTOR<T> MATH_PURE mix(const VECTOR<T>& u, VECTOR<T> v, T a) {
        for (size_t i = 0; i < v.size(); i++) {
            v[i] = u[i] * (T(1) - a) + v[i] * a;
        }
        return v;
    }

    friend inline constexpr VECTOR<T> MATH_PURE smoothstep(T edge0, T edge1, VECTOR<T> v) {
        VECTOR<T> t = saturate((v - edge0) / (edge1 - edge0));
        return t * t * (T(3) - T(2) * t);
    }

    friend inline constexpr VECTOR<T> MATH_PURE step(T edge, VECTOR<T> v) {
        for (size_t i = 0; i < v.size(); i++) {
            v[i] = v[i] < edge ? T(0) : T(1);
        }
        return v;
    }

    friend inline constexpr VECTOR<T> MATH_PURE step(VECTOR<T> edge, VECTOR<T> v) {
        for (size_t i = 0; i < v.size(); i++) {
            v[i] = v[i] < edge[i] ? T(0) : T(1);
        }
        return v;
    }

    friend inline constexpr bool MATH_PURE any(const VECTOR<T>& v) {
        for (size_t i = 0; i < v.size(); i++) {
            if (v[i] != T(0)) return true;
        }
        return false;
    }

    friend inline constexpr bool MATH_PURE all(const VECTOR<T>& v) {
        bool result = true;
        for (size_t i = 0; i < v.size(); i++) {
            result &= (v[i] != T(0));
        }
        return result;
    }
};

}  // namespace filament::math::details

#endif  // TNT_MATH_TVECHELPERS_H
