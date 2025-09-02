// Copyright 2018 The Dawn & Tint Authors
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

#include <memory>
#include <utility>
#include <vector>

#include "dawn/common/Ref.h"
#include "dawn/common/RefCounted.h"
#include "dawn/common/Result.h"
#include "gtest/gtest.h"

namespace dawn {
namespace {

template <typename T, typename E>
void TestError(Result<T, E>* result, E expectedError) {
    EXPECT_TRUE(result->IsError());
    EXPECT_FALSE(result->IsSuccess());

    std::unique_ptr<E> storedError = result->AcquireError();
    EXPECT_EQ(*storedError, expectedError);
}

template <typename T, typename E>
void TestSuccess(Result<T, E>* result, T expectedSuccess) {
    EXPECT_FALSE(result->IsError());
    EXPECT_TRUE(result->IsSuccess());

    const T storedSuccess = result->AcquireSuccess();
    EXPECT_EQ(storedSuccess, expectedSuccess);

    // Once the success is acquired, result has an empty
    // payload and is neither in the success nor error state.
    EXPECT_FALSE(result->IsError());
    EXPECT_FALSE(result->IsSuccess());
}

static int placeholderError = 0xbeef;
static float placeholderSuccess = 42.0f;
static const float placeholderConstSuccess = 42.0f;

class AClass : public RefCounted {
  public:
    int a = 0;
};

// Tests using the following overload of TestSuccess make
// local Ref instances to placeholderSuccessObj. Tests should
// ensure any local Ref objects made along the way continue
// to point to placeholderSuccessObj.
template <typename T, typename E>
void TestSuccess(Result<Ref<T>, E>* result, T* expectedSuccess) {
    EXPECT_FALSE(result->IsError());
    EXPECT_TRUE(result->IsSuccess());

    // AClass starts with a reference count of 1 and stored
    // on the stack in the caller. The result parameter should
    // hold the only other reference to the object.
    EXPECT_EQ(expectedSuccess->GetRefCountForTesting(), 2u);

    const Ref<T> storedSuccess = result->AcquireSuccess();
    EXPECT_EQ(storedSuccess.Get(), expectedSuccess);

    // Once the success is acquired, result has an empty
    // payload and is neither in the success nor error state.
    EXPECT_FALSE(result->IsError());
    EXPECT_FALSE(result->IsSuccess());

    // Once we call AcquireSuccess, result no longer stores
    // the object. storedSuccess should contain the only other
    // reference to the object.
    EXPECT_EQ(storedSuccess->GetRefCountForTesting(), 2u);
}

// Result<void, E*>

// Test constructing an error Result<void, E>
TEST(ResultOnlyPointerError, ConstructingError) {
    Result<void, int> result(std::make_unique<int>(placeholderError));
    TestError(&result, placeholderError);
}

// Test moving an error Result<void, E>
TEST(ResultOnlyPointerError, MovingError) {
    Result<void, int> result(std::make_unique<int>(placeholderError));
    Result<void, int> movedResult(std::move(result));
    TestError(&movedResult, placeholderError);
}

// Test returning an error Result<void, E>
TEST(ResultOnlyPointerError, ReturningError) {
    auto CreateError = []() -> Result<void, int> {
        return {std::make_unique<int>(placeholderError)};
    };

    Result<void, int> result = CreateError();
    TestError(&result, placeholderError);
}

// Test constructing a success Result<void, E>
TEST(ResultOnlyPointerError, ConstructingSuccess) {
    Result<void, int> result;
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_FALSE(result.IsError());
}

// Test moving a success Result<void, E>
TEST(ResultOnlyPointerError, MovingSuccess) {
    Result<void, int> result;
    Result<void, int> movedResult(std::move(result));
    EXPECT_TRUE(movedResult.IsSuccess());
    EXPECT_FALSE(movedResult.IsError());
}

// Test returning a success Result<void, E>
TEST(ResultOnlyPointerError, ReturningSuccess) {
    auto CreateError = []() -> Result<void, int> { return {}; };

    Result<void, int> result = CreateError();
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_FALSE(result.IsError());
}

// Result<T*, E*>

// Test constructing an error Result<T*, E>
TEST(ResultBothPointer, ConstructingError) {
    Result<float*, int> result(std::make_unique<int>(placeholderError));
    TestError(&result, placeholderError);
}

// Test moving an error Result<T*, E>
TEST(ResultBothPointer, MovingError) {
    Result<float*, int> result(std::make_unique<int>(placeholderError));
    Result<float*, int> movedResult(std::move(result));
    TestError(&movedResult, placeholderError);
}

// Test returning an error Result<T*, E>
TEST(ResultBothPointer, ReturningError) {
    auto CreateError = []() -> Result<float*, int> {
        return {std::make_unique<int>(placeholderError)};
    };

    Result<float*, int> result = CreateError();
    TestError(&result, placeholderError);
}

// Test constructing a success Result<T*, E>
TEST(ResultBothPointer, ConstructingSuccess) {
    Result<float*, int> result(&placeholderSuccess);
    TestSuccess(&result, &placeholderSuccess);
}

// Test moving a success Result<T*, E>
TEST(ResultBothPointer, MovingSuccess) {
    Result<float*, int> result(&placeholderSuccess);
    Result<float*, int> movedResult(std::move(result));
    TestSuccess(&movedResult, &placeholderSuccess);
}

// Test returning a success Result<T*, E>
TEST(ResultBothPointer, ReturningSuccess) {
    auto CreateSuccess = []() -> Result<float*, int*> { return {&placeholderSuccess}; };

    Result<float*, int*> result = CreateSuccess();
    TestSuccess(&result, &placeholderSuccess);
}

// Tests converting from a Result<TChild*, E>
TEST(ResultBothPointer, ConversionFromChildClass) {
    struct T {
        int a;
    };
    struct TChild : T {};

    TChild child;
    T* childAsT = &child;
    {
        Result<T*, int> result(&child);
        TestSuccess(&result, childAsT);
    }
    {
        Result<TChild*, int> resultChild(&child);
        Result<T*, int> result(std::move(resultChild));
        TestSuccess(&result, childAsT);
    }
    {
        Result<TChild*, int> resultChild(&child);
        Result<T*, int> result = std::move(resultChild);
        TestSuccess(&result, childAsT);
    }
}

// Result<const T*, E>

// Test constructing an error Result<const T*, E>
TEST(ResultBothPointerWithConstResult, ConstructingError) {
    Result<const float*, int> result(std::make_unique<int>(placeholderError));
    TestError(&result, placeholderError);
}

// Test moving an error Result<const T*, E>
TEST(ResultBothPointerWithConstResult, MovingError) {
    Result<const float*, int> result(std::make_unique<int>(placeholderError));
    Result<const float*, int> movedResult(std::move(result));
    TestError(&movedResult, placeholderError);
}

// Test returning an error Result<const T*, E*>
TEST(ResultBothPointerWithConstResult, ReturningError) {
    auto CreateError = []() -> Result<const float*, int> {
        return {std::make_unique<int>(placeholderError)};
    };

    Result<const float*, int> result = CreateError();
    TestError(&result, placeholderError);
}

// Test constructing a success Result<const T*, E*>
TEST(ResultBothPointerWithConstResult, ConstructingSuccess) {
    Result<const float*, int> result(&placeholderConstSuccess);
    TestSuccess(&result, &placeholderConstSuccess);
}

// Test moving a success Result<const T*, E*>
TEST(ResultBothPointerWithConstResult, MovingSuccess) {
    Result<const float*, int> result(&placeholderConstSuccess);
    Result<const float*, int> movedResult(std::move(result));
    TestSuccess(&movedResult, &placeholderConstSuccess);
}

// Test returning a success Result<const T*, E*>
TEST(ResultBothPointerWithConstResult, ReturningSuccess) {
    auto CreateSuccess = []() -> Result<const float*, int> { return {&placeholderConstSuccess}; };

    Result<const float*, int> result = CreateSuccess();
    TestSuccess(&result, &placeholderConstSuccess);
}

// Result<Ref<T>, E>

// Test constructing an error Result<Ref<T>, E>
TEST(ResultRefT, ConstructingError) {
    Result<Ref<AClass>, int> result(std::make_unique<int>(placeholderError));
    TestError(&result, placeholderError);
}

// Test moving an error Result<Ref<T>, E>
TEST(ResultRefT, MovingError) {
    Result<Ref<AClass>, int> result(std::make_unique<int>(placeholderError));
    Result<Ref<AClass>, int> movedResult(std::move(result));
    TestError(&movedResult, placeholderError);
}

// Test returning an error Result<Ref<T>, E>
TEST(ResultRefT, ReturningError) {
    auto CreateError = []() -> Result<Ref<AClass>, int> {
        return {std::make_unique<int>(placeholderError)};
    };

    Result<Ref<AClass>, int> result = CreateError();
    TestError(&result, placeholderError);
}

// Test constructing a success Result<Ref<T>, E>
TEST(ResultRefT, ConstructingSuccess) {
    AClass success;

    Ref<AClass> refObj(&success);
    Result<Ref<AClass>, int> result(std::move(refObj));
    TestSuccess(&result, &success);
}

// Test moving a success Result<Ref<T>, E>
TEST(ResultRefT, MovingSuccess) {
    AClass success;

    Ref<AClass> refObj(&success);
    Result<Ref<AClass>, int> result(std::move(refObj));
    Result<Ref<AClass>, int> movedResult(std::move(result));
    TestSuccess(&movedResult, &success);
}

// Test returning a success Result<Ref<T>, E>
TEST(ResultRefT, ReturningSuccess) {
    AClass success;
    auto CreateSuccess = [&success]() -> Result<Ref<AClass>, int> { return Ref<AClass>(&success); };

    Result<Ref<AClass>, int> result = CreateSuccess();
    TestSuccess(&result, &success);
}

class OtherClass {
  public:
    int a = 0;
};
class Base : public RefCounted {};
class Child : public OtherClass, public Base {};

// Test constructing a Result<Ref<TChild>, E>
TEST(ResultRefT, ConversionFromChildConstructor) {
    Child child;
    Ref<Child> refChild(&child);

    Result<Ref<Base>, int> result(std::move(refChild));
    TestSuccess<Base>(&result, &child);
}

// Test copy constructing Result<Ref<TChild>, E>
TEST(ResultRefT, ConversionFromChildCopyConstructor) {
    Child child;
    Ref<Child> refChild(&child);

    Result<Ref<Child>, int> resultChild(std::move(refChild));
    Result<Ref<Base>, int> result(std::move(resultChild));
    TestSuccess<Base>(&result, &child);
}

// Test assignment operator for Result<Ref<TChild>, E>
TEST(ResultRefT, ConversionFromChildAssignmentOperator) {
    Child child;
    Ref<Child> refChild(&child);

    Result<Ref<Child>, int> resultChild(std::move(refChild));
    Result<Ref<Base>, int> result = std::move(resultChild);
    TestSuccess<Base>(&result, &child);
}

// Result<T, E>

// Test constructing an error Result<T, E>
TEST(ResultGeneric, ConstructingError) {
    Result<std::vector<float>, int> result(std::make_unique<int>(placeholderError));
    TestError(&result, placeholderError);
}

// Test moving an error Result<T, E>
TEST(ResultGeneric, MovingError) {
    Result<std::vector<float>, int> result(std::make_unique<int>(placeholderError));
    Result<std::vector<float>, int> movedResult(std::move(result));
    TestError(&movedResult, placeholderError);
}

// Test returning an error Result<T, E>
TEST(ResultGeneric, ReturningError) {
    auto CreateError = []() -> Result<std::vector<float>, int> {
        return {std::make_unique<int>(placeholderError)};
    };

    Result<std::vector<float>, int> result = CreateError();
    TestError(&result, placeholderError);
}

// Test constructing a success Result<T, E>
TEST(ResultGeneric, ConstructingSuccess) {
    Result<std::vector<float>, int> result({1.0f});
    TestSuccess(&result, {1.0f});
}

// Test moving a success Result<T, E>
TEST(ResultGeneric, MovingSuccess) {
    Result<std::vector<float>, int> result({1.0f});
    Result<std::vector<float>, int> movedResult(std::move(result));
    TestSuccess(&movedResult, {1.0f});
}

// Test returning a success Result<T, E>
TEST(ResultGeneric, ReturningSuccess) {
    auto CreateSuccess = []() -> Result<std::vector<float>, int> { return {{1.0f}}; };

    Result<std::vector<float>, int> result = CreateSuccess();
    TestSuccess(&result, {1.0f});
}

class NonDefaultConstructible {
  public:
    explicit NonDefaultConstructible(float v) : v(v) {}
    bool operator==(const NonDefaultConstructible& other) const = default;
    float v;
};

// Test constructing an error Result<T, E> for non-default-constructible T
TEST(ResultNonDefaultConstructible, ConstructingError) {
    Result<NonDefaultConstructible, int> result(std::make_unique<int>(placeholderError));
    TestError(&result, placeholderError);
}

// Test moving an error Result<T, E> for non-default-constructible T
TEST(ResultNonDefaultConstructible, MovingError) {
    Result<NonDefaultConstructible, int> result(std::make_unique<int>(placeholderError));
    Result<NonDefaultConstructible, int> movedResult(std::move(result));
    TestError(&movedResult, placeholderError);
}

// Test returning an error Result<T, E> for non-default-constructible T
TEST(ResultNonDefaultConstructible, ReturningError) {
    auto CreateError = []() -> Result<NonDefaultConstructible, int> {
        return {std::make_unique<int>(placeholderError)};
    };

    Result<NonDefaultConstructible, int> result = CreateError();
    TestError(&result, placeholderError);
}

// Test constructing a success Result<T, E> for non-default-constructible T
TEST(ResultNonDefaultConstructible, ConstructingSuccess) {
    Result<NonDefaultConstructible, int> result(NonDefaultConstructible(1.0f));
    TestSuccess(&result, NonDefaultConstructible(1.0f));
}

// Test moving a success Result<T, E> for non-default-constructible T
TEST(ResultNonDefaultConstructible, MovingSuccess) {
    Result<NonDefaultConstructible, int> result(NonDefaultConstructible(1.0f));
    Result<NonDefaultConstructible, int> movedResult(std::move(result));
    TestSuccess(&movedResult, NonDefaultConstructible(1.0f));
}

// Test returning a success Result<T, E> for non-default-constructible T
TEST(ResultNonDefaultConstructible, ReturningSuccess) {
    auto CreateSuccess = []() -> Result<NonDefaultConstructible, int> {
        return {NonDefaultConstructible(1.0f)};
    };

    Result<NonDefaultConstructible, int> result = CreateSuccess();
    TestSuccess(&result, NonDefaultConstructible(1.0f));
}

}  // anonymous namespace
}  // namespace dawn
