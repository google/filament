//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// JsonSerializer.cpp: Implementation of a JSON based serializer
// Note that for binary blob data only a checksum is stored so that
// a lossless  deserialization is not supported.

#include "JsonSerializer.h"

#include "common/debug.h"

#include <anglebase/sha1.h>
#include <rapidjson/document.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>

namespace angle
{

namespace js = rapidjson;

JsonSerializer::JsonSerializer() : mDoc(js::kObjectType), mAllocator(mDoc.GetAllocator()) {}

JsonSerializer::~JsonSerializer() {}

void JsonSerializer::startGroup(const std::string &name)
{
    mGroupValueStack.push(SortedValueGroup());
    mGroupNameStack.push(name);
}

void JsonSerializer::endGroup()
{
    ASSERT(!mGroupValueStack.empty());
    ASSERT(!mGroupNameStack.empty());

    rapidjson::Value group = makeValueGroup(mGroupValueStack.top());
    std::string name       = mGroupNameStack.top();

    mGroupValueStack.pop();
    mGroupNameStack.pop();

    addValue(name, std::move(group));
}

void JsonSerializer::addBlob(const std::string &name, const uint8_t *blob, size_t length)
{
    addBlobWithMax(name, blob, length, 16);
}

void JsonSerializer::addBlobWithMax(const std::string &name,
                                    const uint8_t *blob,
                                    size_t length,
                                    size_t maxSerializedLength)
{
    unsigned char hash[angle::base::kSHA1Length];
    angle::base::SHA1HashBytes(blob, length, hash);
    std::ostringstream os;

    // Since we don't want to de-serialize the data we just store a checksum of the blob
    os << "SHA1:";
    static constexpr char kASCII[] = "0123456789ABCDEF";
    for (size_t i = 0; i < angle::base::kSHA1Length; ++i)
    {
        os << kASCII[hash[i] & 0xf] << kASCII[hash[i] >> 4];
    }

    std::ostringstream hashName;
    hashName << name << "-hash";
    addString(hashName.str(), os.str());

    std::vector<uint8_t> data(
        (length < maxSerializedLength) ? length : static_cast<size_t>(maxSerializedLength));
    std::copy(blob, blob + data.size(), data.begin());

    std::ostringstream rawName;
    rawName << name << "-raw[0-" << data.size() - 1 << ']';
    addVector(rawName.str(), data);
}

void JsonSerializer::addCString(const std::string &name, const char *value)
{
    rapidjson::Value tag(name.c_str(), mAllocator);
    rapidjson::Value val(value, mAllocator);
    addValue(name, std::move(val));
}

void JsonSerializer::addString(const std::string &name, const std::string &value)
{
    addCString(name, value.c_str());
}

void JsonSerializer::addVectorOfStrings(const std::string &name,
                                        const std::vector<std::string> &value)
{
    rapidjson::Value arrayValue(rapidjson::kArrayType);
    arrayValue.SetArray();

    for (const std::string &v : value)
    {
        rapidjson::Value str(v.c_str(), mAllocator);
        arrayValue.PushBack(str, mAllocator);
    }

    addValue(name, std::move(arrayValue));
}

void JsonSerializer::addBool(const std::string &name, bool value)
{
    rapidjson::Value boolValue(value);
    addValue(name, std::move(boolValue));
}

void JsonSerializer::addHexValue(const std::string &name, int value)
{
    // JSON doesn't support hex values, so write it as a string
    std::stringstream hexStream;
    hexStream << "0x" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << value;
    addCString(name, hexStream.str().c_str());
}

const char *JsonSerializer::data()
{
    ensureEndDocument();
    return mResult.c_str();
}

std::vector<uint8_t> JsonSerializer::getData()
{
    ensureEndDocument();
    return std::vector<uint8_t>(mResult.begin(), mResult.end());
}

void JsonSerializer::ensureEndDocument()
{
    if (!mResult.empty())
    {
        return;
    }

    std::stringstream os;
    js::OStreamWrapper osw(os);
    js::PrettyWriter<js::OStreamWrapper> prettyOs(osw);
    mDoc.Accept(prettyOs);
    mResult = os.str();
}

size_t JsonSerializer::length()
{
    ensureEndDocument();
    return mResult.length();
}

rapidjson::Value JsonSerializer::makeValueGroup(SortedValueGroup &group)
{
    rapidjson::Value valueGroup(js::kObjectType);
    for (auto &it : group)
    {
        rapidjson::Value tag(it.first.c_str(), mAllocator);
        valueGroup.AddMember(tag, it.second, mAllocator);
    }
    return valueGroup;
}

void JsonSerializer::addValue(const std::string &name, rapidjson::Value &&value)
{
    if (!mGroupValueStack.empty())
    {
        mGroupValueStack.top().insert(std::make_pair(name, std::move(value)));
    }
    else
    {
        rapidjson::Value nameValue(name, mAllocator);
        mDoc.AddMember(nameValue, std::move(value), mAllocator);
    }
}
}  // namespace angle
