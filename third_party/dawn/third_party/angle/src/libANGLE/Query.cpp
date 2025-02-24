//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Query.cpp: Implements the gl::Query class

#include "libANGLE/Query.h"

#include "libANGLE/renderer/GLImplFactory.h"
#include "libANGLE/renderer/QueryImpl.h"

namespace gl
{
Query::Query(rx::GLImplFactory *factory, QueryType type, QueryID id)
    : RefCountObject(factory->generateSerial(), id), mQuery(factory->createQuery(type)), mLabel()
{}

Query::~Query()
{
    SafeDelete(mQuery);
}

void Query::onDestroy(const Context *context)
{
    ASSERT(mQuery);
    mQuery->onDestroy(context);
}

angle::Result Query::setLabel(const Context *context, const std::string &label)
{
    mLabel = label;

    if (mQuery)
    {
        return mQuery->onLabelUpdate(context);
    }
    return angle::Result::Continue;
}

const std::string &Query::getLabel() const
{
    return mLabel;
}

angle::Result Query::begin(const Context *context)
{
    return mQuery->begin(context);
}

angle::Result Query::end(const Context *context)
{
    return mQuery->end(context);
}

angle::Result Query::queryCounter(const Context *context)
{
    return mQuery->queryCounter(context);
}

angle::Result Query::getResult(const Context *context, GLint *params)
{
    return mQuery->getResult(context, params);
}

angle::Result Query::getResult(const Context *context, GLuint *params)
{
    return mQuery->getResult(context, params);
}

angle::Result Query::getResult(const Context *context, GLint64 *params)
{
    return mQuery->getResult(context, params);
}

angle::Result Query::getResult(const Context *context, GLuint64 *params)
{
    return mQuery->getResult(context, params);
}

angle::Result Query::isResultAvailable(const Context *context, bool *available)
{
    return mQuery->isResultAvailable(context, available);
}

QueryType Query::getType() const
{
    return mQuery->getType();
}

rx::QueryImpl *Query::getImplementation() const
{
    return mQuery;
}
}  // namespace gl
