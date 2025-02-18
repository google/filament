// Copyright 2024 The langsvr Authors
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

#include "langsvr/json/builder.h"
#include "langsvr/json/value.h"
#include "langsvr/span.h"
#include "src/utils/block_allocator.h"

#include "json/reader.h"
#include "json/writer.h"

namespace langsvr::json {

namespace {

class BuilderImpl;

class ValueImpl : public Value {
  public:
    ValueImpl(Json::Value&& value, BuilderImpl& builder) : v(std::move(value)), b(builder) {}

    std::string Json() const override;
    json::Kind Kind() const override;
    Result<SuccessType> Null() const override;
    Result<json::Bool> Bool() const override;
    Result<json::I64> I64() const override;
    Result<json::U64> U64() const override;
    Result<json::F64> F64() const override;
    Result<json::String> String() const override;
    Result<const Value*> Get(size_t index) const override;
    Result<const Value*> Get(std::string_view name) const override;
    size_t Count() const override;
    Result<std::vector<std::string>> MemberNames() const override;
    bool Has(std::string_view name) const override;

    Failure ErrIncorrectType(std::string_view wanted) const;

    Json::Value v;
    BuilderImpl& b;
};

class BuilderImpl : public Builder {
  public:
    Result<const Value*> Parse(std::string_view json) override;
    const Value* Null() override;
    const Value* Bool(json::Bool value) override;
    const Value* I64(json::I64 value) override;
    const Value* U64(json::U64 value) override;
    const Value* F64(json::F64 value) override;
    const Value* String(json::String value) override;
    const Value* Array(Span<const Value*> elements) override;
    const Value* Object(Span<Member> members) override;

    BlockAllocator<ValueImpl> allocator;
};

////////////////////////////////////////////////////////////////////////////////
// ValueImpl
////////////////////////////////////////////////////////////////////////////////
std::string ValueImpl::Json() const {
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    builder["enableYAMLCompatibility"] = false;
    return Json::writeString(builder, v);
}

json::Kind ValueImpl::Kind() const {
    switch (v.type()) {
        case Json::ValueType::nullValue:
            return json::Kind::kNull;
        case Json::ValueType::intValue:
            return json::Kind::kI64;
        case Json::ValueType::uintValue:
            return json::Kind::kU64;
        case Json::ValueType::realValue:
            return json::Kind::kF64;
        case Json::ValueType::stringValue:
            return json::Kind::kString;
        case Json::ValueType::booleanValue:
            return json::Kind::kBool;
        case Json::ValueType::arrayValue:
            return json::Kind::kArray;
        case Json::ValueType::objectValue:
            return json::Kind::kObject;
    }

    return json::Kind::kNull;
}

// json::Reader compliance

Result<SuccessType> ValueImpl::Null() const {
    if (v.isNull()) {
        return Success;
    }
    return ErrIncorrectType("Bool");
}

Result<json::Bool> ValueImpl::Bool() const {
    if (v.isBool()) {
        return v.asBool();
    }
    return ErrIncorrectType("Bool");
}

Result<json::I64> ValueImpl::I64() const {
    if (v.isInt64()) {
        return v.asInt64();
    }
    return ErrIncorrectType("I64");
}

Result<json::U64> ValueImpl::U64() const {
    if (v.isUInt64()) {
        return v.asUInt64();
    }
    return ErrIncorrectType("U64");
}

Result<json::F64> ValueImpl::F64() const {
    if (v.isDouble()) {
        return v.asDouble();
    }
    return ErrIncorrectType("F64");
}

Result<json::String> ValueImpl::String() const {
    if (v.isString()) {
        return v.asString();
    }
    return ErrIncorrectType("String");
}

Result<const Value*> ValueImpl::Get(size_t index) const {
    if (v.isArray()) {
        if (index < v.size()) {
            return b.allocator.Create(
                v.get(static_cast<Json::ArrayIndex>(index), Json::Value::null), b);
        }
        std::stringstream err;
        err << "index >= array length of " << v.size();
        return Failure{err.str()};
    }
    return ErrIncorrectType("Array");
}

Result<const Value*> ValueImpl::Get(std::string_view name) const {
    if (v.isObject()) {
        if (v.isMember(name.data(), name.data() + name.length())) {
            return b.allocator.Create(
                v.get(name.data(), name.data() + name.length(), Json::Value::null), b);
        }
        std::stringstream err;
        err << "object has no field with name '" << name << "'";
        return Failure{err.str()};
    }
    return ErrIncorrectType("Object");
}

size_t ValueImpl::Count() const {
    return static_cast<size_t>(v.size());
}

Result<std::vector<std::string>> ValueImpl::MemberNames() const {
    if (v.isObject()) {
        return v.getMemberNames();
    }
    return ErrIncorrectType("Object");
}

bool ValueImpl::Has(std::string_view name) const {
    return v.isObject() && v.isMember(name.data(), name.data() + name.length());
}

Failure ValueImpl::ErrIncorrectType(std::string_view wanted) const {
    std::stringstream err;
    err << "value is " << v.type() << ", not " << wanted;
    return Failure{err.str()};
}

////////////////////////////////////////////////////////////////////////////////
// BuilderImpl
////////////////////////////////////////////////////////////////////////////////

Result<const Value*> BuilderImpl::Parse(std::string_view json) {
    Json::CharReaderBuilder builder;
    std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    Json::Value root;
    JSONCPP_STRING err;
    if (!reader->parse(json.data(), json.data() + json.length(), &root, &err)) {
        return Failure{err};
    }
    return allocator.Create(std::move(root), *this);
}

const Value* BuilderImpl::Null() {
    return allocator.Create(Json::nullValue, *this);
}

const Value* BuilderImpl::Bool(json::Bool value) {
    return allocator.Create(Json::Value(value), *this);
}

const Value* BuilderImpl::I64(json::I64 value) {
    return allocator.Create(Json::Value(value), *this);
}

const Value* BuilderImpl::U64(json::U64 value) {
    return allocator.Create(Json::Value(value), *this);
}

const Value* BuilderImpl::F64(json::F64 value) {
    return allocator.Create(Json::Value(value), *this);
}

const Value* BuilderImpl::String(json::String value) {
    return allocator.Create(Json::Value(value), *this);
}

const Value* BuilderImpl::Array(Span<const Value*> elements) {
    Json::Value array(Json::arrayValue);
    for (auto* el : elements) {
        array.append(static_cast<const ValueImpl*>(el)->v);
    }
    return allocator.Create(std::move(array), *this);
}

const Value* BuilderImpl::Object(Span<Member> members) {
    Json::Value object(Json::objectValue);
    for (auto& member : members) {
        object[member.name] = static_cast<const ValueImpl*>(member.value)->v;
    }
    return allocator.Create(std::move(object), *this);
}

}  // namespace

Value::~Value() = default;
Builder::~Builder() = default;

std::unique_ptr<Builder> Builder::Create() {
    return std::make_unique<BuilderImpl>();
}

}  // namespace langsvr::json
