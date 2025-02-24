//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Color.inc : Inline definitions of some functions from Color.h

namespace angle
{

template <typename T>
Color<T>::Color() : Color(0, 0, 0, 0)
{
}

template <typename T>
constexpr Color<T>::Color(T r, T g, T b, T a) : red(r), green(g), blue(b), alpha(a)
{
}

template <typename T>
bool operator==(const Color<T> &a, const Color<T> &b)
{
    return a.red == b.red &&
           a.green == b.green &&
           a.blue == b.blue &&
           a.alpha == b.alpha;
}

template <typename T>
bool operator!=(const Color<T> &a, const Color<T> &b)
{
    return !(a == b);
}


ColorGeneric::ColorGeneric() : colorF(), type(Type::Float) {}

ColorGeneric::ColorGeneric(const ColorF &color) : colorF(color), type(Type::Float) {}

ColorGeneric::ColorGeneric(const ColorI &color) : colorI(color), type(Type::Int) {}

ColorGeneric::ColorGeneric(const ColorUI &color) : colorUI(color), type(Type::UInt) {}

bool operator==(const ColorGeneric &a, const ColorGeneric &b)
{
    // Switching on the type and comparing individual
    // channels yield much less efficient code.
    return memcmp(&a, &b, sizeof(ColorGeneric)) == 0;
}

bool operator!=(const ColorGeneric &a, const ColorGeneric &b)
{
    return !(a == b);
}

}  // namespace angle
