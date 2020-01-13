/*
Open Asset Import Library (assimp)
----------------------------------------------------------------------

Copyright (c) 2006-2019, assimp team


All rights reserved.

Redistribution and use of this software in source and binary forms,
with or without modification, are permitted provided that the
following conditions are met:

* Redistributions of source code must retain the above
  copyright notice, this list of conditions and the
  following disclaimer.

* Redistributions in binary form must reproduce the above
  copyright notice, this list of conditions and the
  following disclaimer in the documentation and/or other
  materials provided with the distribution.

* Neither the name of the assimp team, nor the names of its
  contributors may be used to endorse or promote products
  derived from this software without specific prior
  written permission of the assimp team.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

----------------------------------------------------------------------
*/

/** @file  FBXProperties.cpp
 *  @brief Implementation of the FBX dynamic properties system
 */

#ifndef ASSIMP_BUILD_NO_FBX_IMPORTER

#include "FBXTokenizer.h"
#include "FBXParser.h"
#include "FBXDocument.h"
#include "FBXDocumentUtil.h"
#include "FBXProperties.h"

namespace Assimp {
namespace FBX {

    using namespace Util;

// ------------------------------------------------------------------------------------------------
Property::Property()
{
}

// ------------------------------------------------------------------------------------------------
Property::~Property()
{
}

namespace {

// ------------------------------------------------------------------------------------------------
// read a typed property out of a FBX element. The return value is NULL if the property cannot be read.
Property* ReadTypedProperty(const Element& element)
{
    ai_assert(element.KeyToken().StringContents() == "P");

    const TokenList& tok = element.Tokens();
    ai_assert(tok.size() >= 5);

    const std::string& s = ParseTokenAsString(*tok[1]);
    const char* const cs = s.c_str();
    if (!strcmp(cs,"KString")) {
        return new TypedProperty<std::string>(ParseTokenAsString(*tok[4]));
    }
    else if (!strcmp(cs,"bool") || !strcmp(cs,"Bool")) {
        return new TypedProperty<bool>(ParseTokenAsInt(*tok[4]) != 0);
    }
    else if (!strcmp(cs, "int") || !strcmp(cs, "Int") || !strcmp(cs, "enum") || !strcmp(cs, "Enum")) {
        return new TypedProperty<int>(ParseTokenAsInt(*tok[4]));
    }
    else if (!strcmp(cs, "ULongLong")) {
        return new TypedProperty<uint64_t>(ParseTokenAsID(*tok[4]));
    }
    else if (!strcmp(cs, "KTime")) {
        return new TypedProperty<int64_t>(ParseTokenAsInt64(*tok[4]));
    }
    else if (!strcmp(cs,"Vector3D") ||
        !strcmp(cs,"ColorRGB") ||
        !strcmp(cs,"Vector") ||
        !strcmp(cs,"Color") ||
        !strcmp(cs,"Lcl Translation") ||
        !strcmp(cs,"Lcl Rotation") ||
        !strcmp(cs,"Lcl Scaling")
        ) {
        return new TypedProperty<aiVector3D>(aiVector3D(
            ParseTokenAsFloat(*tok[4]),
            ParseTokenAsFloat(*tok[5]),
            ParseTokenAsFloat(*tok[6]))
        );
    }
    else if (!strcmp(cs,"double") || !strcmp(cs,"Number") || !strcmp(cs,"Float") || !strcmp(cs,"FieldOfView") || !strcmp( cs, "UnitScaleFactor" ) ) {
        return new TypedProperty<float>(ParseTokenAsFloat(*tok[4]));
    }
    return NULL;
}


// ------------------------------------------------------------------------------------------------
// peek into an element and check if it contains a FBX property, if so return its name.
std::string PeekPropertyName(const Element& element)
{
    ai_assert(element.KeyToken().StringContents() == "P");
    const TokenList& tok = element.Tokens();
    if(tok.size() < 4) {
        return "";
    }

    return ParseTokenAsString(*tok[0]);
}

} //! anon


// ------------------------------------------------------------------------------------------------
PropertyTable::PropertyTable()
: templateProps()
, element()
{
}

// ------------------------------------------------------------------------------------------------
PropertyTable::PropertyTable(const Element& element, std::shared_ptr<const PropertyTable> templateProps)
: templateProps(templateProps)
, element(&element)
{
    const Scope& scope = GetRequiredScope(element);
    for(const ElementMap::value_type& v : scope.Elements()) {
        if(v.first != "P") {
            DOMWarning("expected only P elements in property table",v.second);
            continue;
        }

        const std::string& name = PeekPropertyName(*v.second);
        if(!name.length()) {
            DOMWarning("could not read property name",v.second);
            continue;
        }

        LazyPropertyMap::const_iterator it = lazyProps.find(name);
        if (it != lazyProps.end()) {
            DOMWarning("duplicate property name, will hide previous value: " + name,v.second);
            continue;
        }

        lazyProps[name] = v.second;
    }
}


// ------------------------------------------------------------------------------------------------
PropertyTable::~PropertyTable()
{
    for(PropertyMap::value_type& v : props) {
        delete v.second;
    }
}


// ------------------------------------------------------------------------------------------------
const Property* PropertyTable::Get(const std::string& name) const
{
    PropertyMap::const_iterator it = props.find(name);
    if (it == props.end()) {
        // hasn't been parsed yet?
        LazyPropertyMap::const_iterator lit = lazyProps.find(name);
        if(lit != lazyProps.end()) {
            props[name] = ReadTypedProperty(*(*lit).second);
            it = props.find(name);

            ai_assert(it != props.end());
        }

        if (it == props.end()) {
            // check property template
            if(templateProps) {
                return templateProps->Get(name);
            }

            return NULL;
        }
    }

    return (*it).second;
}

DirectPropertyMap PropertyTable::GetUnparsedProperties() const
{
    DirectPropertyMap result;

    // Loop through all the lazy properties (which is all the properties)
    for(const LazyPropertyMap::value_type& element : lazyProps) {

        // Skip parsed properties
        if (props.end() != props.find(element.first)) continue;

        // Read the element's value.
        // Wrap the naked pointer (since the call site is required to acquire ownership)
        // std::unique_ptr from C++11 would be preferred both as a wrapper and a return value.
        std::shared_ptr<Property> prop = std::shared_ptr<Property>(ReadTypedProperty(*element.second));

        // Element could not be read. Skip it.
        if (!prop) continue;

        // Add to result
        result[element.first] = prop;
    }

    return result;
}

} //! FBX
} //! Assimp

#endif
