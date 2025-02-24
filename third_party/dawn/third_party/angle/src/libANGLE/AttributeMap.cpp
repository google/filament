//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "libANGLE/AttributeMap.h"

#include "common/debug.h"

namespace egl
{

AttributeMap::AttributeMap() = default;

AttributeMap::AttributeMap(const AttributeMap &other) = default;

AttributeMap &AttributeMap::operator=(const AttributeMap &other) = default;

AttributeMap::~AttributeMap() = default;

void AttributeMap::insert(EGLAttrib key, EGLAttrib value)
{
    mValidatedAttributes[key] = value;
}

bool AttributeMap::contains(EGLAttrib key) const
{
    return (attribs().find(key) != attribs().end());
}

EGLAttrib AttributeMap::get(EGLAttrib key) const
{
    auto iter = attribs().find(key);
    ASSERT(iter != attribs().end());
    return iter->second;
}

EGLAttrib AttributeMap::get(EGLAttrib key, EGLAttrib defaultValue) const
{
    auto iter = attribs().find(key);
    return (iter != attribs().end()) ? iter->second : defaultValue;
}

EGLint AttributeMap::getAsInt(EGLAttrib key) const
{
    return static_cast<EGLint>(get(key));
}

EGLint AttributeMap::getAsInt(EGLAttrib key, EGLint defaultValue) const
{
    return static_cast<EGLint>(get(key, static_cast<EGLAttrib>(defaultValue)));
}

bool AttributeMap::isEmpty() const
{
    return attribs().empty();
}

std::vector<EGLint> AttributeMap::toIntVector() const
{
    std::vector<EGLint> ret;
    for (const auto &pair : attribs())
    {
        ret.push_back(static_cast<EGLint>(pair.first));
        ret.push_back(static_cast<EGLint>(pair.second));
    }
    ret.push_back(EGL_NONE);

    return ret;
}

AttributeMap::const_iterator AttributeMap::begin() const
{
    return attribs().begin();
}

AttributeMap::const_iterator AttributeMap::end() const
{
    return attribs().end();
}

bool AttributeMap::validate(const ValidationContext *val,
                            const egl::Display *display,
                            AttributeValidationFunc validationFunc) const
{
    if (mIntPointer)
    {
        for (const EGLint *curAttrib = mIntPointer; curAttrib[0] != EGL_NONE; curAttrib += 2)
        {
            if (!validationFunc(val, display, curAttrib[0]))
            {
                return false;
            }

            mValidatedAttributes[static_cast<EGLAttrib>(curAttrib[0])] =
                static_cast<EGLAttrib>(curAttrib[1]);
        }
        mIntPointer = nullptr;
    }

    if (mAttribPointer)
    {
        for (const EGLAttrib *curAttrib = mAttribPointer; curAttrib[0] != EGL_NONE; curAttrib += 2)
        {
            if (!validationFunc(val, display, curAttrib[0]))
            {
                return false;
            }

            mValidatedAttributes[curAttrib[0]] = curAttrib[1];
        }
        mAttribPointer = nullptr;
    }

    return true;
}

void AttributeMap::initializeWithoutValidation() const
{
    auto alwaysTrue = [](const ValidationContext *, const egl::Display *, EGLAttrib) {
        return true;
    };
    (void)validate(nullptr, nullptr, alwaysTrue);
}

// static
AttributeMap AttributeMap::CreateFromIntArray(const EGLint *attributes)
{
    AttributeMap map;
    map.mIntPointer = attributes;
    map.mMapType    = AttributeMapType::Int;
    return map;
}

// static
AttributeMap AttributeMap::CreateFromAttribArray(const EGLAttrib *attributes)
{
    AttributeMap map;
    map.mAttribPointer = attributes;
    map.mMapType       = AttributeMapType::Attrib;
    return map;
}

bool AttributeMap::isValidated() const
{
    return mIntPointer == nullptr && mAttribPointer == nullptr;
}
}  // namespace egl
