// Copyright 2021 The Dawn & Tint Authors
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

#include "src/dawn/node/interop/Core.h"

namespace wgpu::interop {

Result Success;

Result Error(std::string msg) {
    return {msg};
}

Result Converter<bool>::FromJS(Napi::Env env, Napi::Value value, bool& out) {
    if (value.IsBoolean()) {
        out = value.ToBoolean();
        return Success;
    }
    return Error("value is not a boolean");
}
Napi::Value Converter<bool>::ToJS(Napi::Env env, bool value) {
    return Napi::Value::From(env, value);
}

Result Converter<std::string>::FromJS(Napi::Env env, Napi::Value value, std::string& out) {
    if (value.IsString()) {
        out = value.ToString();
        return Success;
    }
    return Error("value is not a string");
}
Napi::Value Converter<std::string>::ToJS(Napi::Env env, std::string value) {
    return Napi::Value::From(env, value);
}

Result Converter<int8_t>::FromJS(Napi::Env env, Napi::Value value, int8_t& out) {
    if (value.IsNumber()) {
        out = value.ToNumber().Int32Value();
        return Success;
    }
    return Error("value is not a number");
}
Napi::Value Converter<int8_t>::ToJS(Napi::Env env, int8_t value) {
    return Napi::Value::From(env, value);
}

Result Converter<uint8_t>::FromJS(Napi::Env env, Napi::Value value, uint8_t& out) {
    if (value.IsNumber()) {
        out = value.ToNumber().Uint32Value();
        return Success;
    }
    return Error("value is not a number");
}
Napi::Value Converter<uint8_t>::ToJS(Napi::Env env, uint8_t value) {
    return Napi::Value::From(env, value);
}

Result Converter<int16_t>::FromJS(Napi::Env env, Napi::Value value, int16_t& out) {
    if (value.IsNumber()) {
        out = value.ToNumber().Int32Value();
        return Success;
    }
    return Error("value is not a number");
}
Napi::Value Converter<int16_t>::ToJS(Napi::Env env, int16_t value) {
    return Napi::Value::From(env, value);
}

Result Converter<uint16_t>::FromJS(Napi::Env env, Napi::Value value, uint16_t& out) {
    if (value.IsNumber()) {
        out = value.ToNumber().Uint32Value();
        return Success;
    }
    return Error("value is not a number");
}
Napi::Value Converter<uint16_t>::ToJS(Napi::Env env, uint16_t value) {
    return Napi::Value::From(env, value);
}

Result Converter<int32_t>::FromJS(Napi::Env env, Napi::Value value, int32_t& out) {
    if (value.IsNumber()) {
        out = value.ToNumber().Int32Value();
        return Success;
    }
    return Error("value is not a number");
}
Napi::Value Converter<int32_t>::ToJS(Napi::Env env, int32_t value) {
    return Napi::Value::From(env, value);
}

Result Converter<uint32_t>::FromJS(Napi::Env env, Napi::Value value, uint32_t& out) {
    if (value.IsNumber()) {
        out = value.ToNumber().Uint32Value();
        return Success;
    }
    return Error("value is not a number");
}
Napi::Value Converter<uint32_t>::ToJS(Napi::Env env, uint32_t value) {
    return Napi::Value::From(env, value);
}

Result Converter<int64_t>::FromJS(Napi::Env env, Napi::Value value, int64_t& out) {
    if (value.IsNumber()) {
        out = value.ToNumber().Int64Value();
        return Success;
    }
    return Error("value is not a number");
}
Napi::Value Converter<int64_t>::ToJS(Napi::Env env, int64_t value) {
    return Napi::Value::From(env, value);
}

Result Converter<uint64_t>::FromJS(Napi::Env env, Napi::Value value, uint64_t& out) {
    if (value.IsNumber()) {
        // Note that the JS Number type only stores doubles, so the max integer
        // range of values without precision loss is -2^53 to 2^53 (52 bit mantissa
        // with 1 implicit bit). This is why there's no UInt64Value() function.
        out = static_cast<uint64_t>(value.ToNumber().Int64Value());
        return Success;
    }
    return Error("value is not a number");
}
Napi::Value Converter<uint64_t>::ToJS(Napi::Env env, uint64_t value) {
    return Napi::Value::From(env, value);
}

Result
Converter<std::conditional_t<detail::kSizetIsUniqueType, size_t, detail::InvalidType>>::FromJS(
    Napi::Env env,
    Napi::Value value,
    size_t& out) {
    if (value.IsNumber()) {
        // Note that the JS Number type only stores doubles, so the max integer
        // range of values without precision loss is -2^53 to 2^53 (52 bit mantissa
        // with 1 implicit bit). This is why there's no UInt64Value() function.
        out = static_cast<size_t>(value.ToNumber().Int64Value());
        return Success;
    }
    return Error("value is not a number");
}
Napi::Value
Converter<std::conditional_t<detail::kSizetIsUniqueType, size_t, detail::InvalidType>>::ToJS(
    Napi::Env env,
    size_t value) {
    return Napi::Value::From(env, value);
}

Result Converter<float>::FromJS(Napi::Env env, Napi::Value value, float& out) {
    if (value.IsNumber()) {
        out = value.ToNumber().FloatValue();
        return Success;
    }
    return Error("value is not a number");
}
Napi::Value Converter<float>::ToJS(Napi::Env env, float value) {
    return Napi::Value::From(env, value);
}

Result Converter<double>::FromJS(Napi::Env env, Napi::Value value, double& out) {
    if (value.IsNumber()) {
        out = value.ToNumber().DoubleValue();
        return Success;
    }
    return Error("value is not a number");
}
Napi::Value Converter<double>::ToJS(Napi::Env env, double value) {
    return Napi::Value::From(env, value);
}

Result Converter<UndefinedType>::FromJS(Napi::Env, Napi::Value value, UndefinedType&) {
    if (value.IsUndefined()) {
        return Success;
    }
    return Error("value is undefined");
}
Napi::Value Converter<UndefinedType>::ToJS(Napi::Env env, UndefinedType) {
    return env.Undefined();
}

std::ostream& operator<<(std::ostream& o, const UndefinedType&) {
    o << "<undefined>";
    return o;
}

}  // namespace wgpu::interop
