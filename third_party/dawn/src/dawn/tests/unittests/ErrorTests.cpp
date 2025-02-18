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

#include "dawn/native/Error.h"
#include "dawn/native/ErrorData.h"
#include "gtest/gtest.h"

namespace dawn::native {
namespace {

int placeholderSuccess = 0xbeef;
constexpr const char* placeholderErrorMessage = "I am an error message :3";

// Check returning a success MaybeError with {};
TEST(ErrorTests, Error_Success) {
    auto ReturnSuccess = []() -> MaybeError { return {}; };

    MaybeError result = ReturnSuccess();
    ASSERT_TRUE(result.IsSuccess());
}

// Check returning an error MaybeError with "return DAWN_VALIDATION_ERROR"
TEST(ErrorTests, Error_Error) {
    auto ReturnError = []() -> MaybeError {
        return DAWN_VALIDATION_ERROR(placeholderErrorMessage);
    };

    MaybeError result = ReturnError();
    ASSERT_TRUE(result.IsError());

    std::unique_ptr<ErrorData> errorData = result.AcquireError();
    ASSERT_EQ(errorData->GetMessage(), placeholderErrorMessage);
}

// Check returning a success ResultOrError with an implicit conversion
TEST(ErrorTests, ResultOrError_Success) {
    auto ReturnSuccess = []() -> ResultOrError<int*> { return &placeholderSuccess; };

    ResultOrError<int*> result = ReturnSuccess();
    ASSERT_TRUE(result.IsSuccess());
    ASSERT_EQ(result.AcquireSuccess(), &placeholderSuccess);
}

// Check returning an error ResultOrError with "return DAWN_VALIDATION_ERROR"
TEST(ErrorTests, ResultOrError_Error) {
    auto ReturnError = []() -> ResultOrError<int*> {
        return DAWN_VALIDATION_ERROR(placeholderErrorMessage);
    };

    ResultOrError<int*> result = ReturnError();
    ASSERT_TRUE(result.IsError());

    std::unique_ptr<ErrorData> errorData = result.AcquireError();
    ASSERT_EQ(errorData->GetMessage(), placeholderErrorMessage);
}

// Check DAWN_TRY handles successes correctly.
TEST(ErrorTests, TRY_Success) {
    auto ReturnSuccess = []() -> MaybeError { return {}; };

    // We need to check that DAWN_TRY doesn't return on successes
    bool tryReturned = true;

    auto Try = [ReturnSuccess, &tryReturned]() -> MaybeError {
        DAWN_TRY(ReturnSuccess());
        tryReturned = false;
        return {};
    };

    MaybeError result = Try();
    ASSERT_TRUE(result.IsSuccess());
    ASSERT_FALSE(tryReturned);
}

// Check DAWN_TRY handles errors correctly.
TEST(ErrorTests, TRY_Error) {
    auto ReturnError = []() -> MaybeError {
        return DAWN_VALIDATION_ERROR(placeholderErrorMessage);
    };

    auto Try = [ReturnError]() -> MaybeError {
        DAWN_TRY(ReturnError());
        // DAWN_TRY should return before this point
        EXPECT_FALSE(true);
        return {};
    };

    MaybeError result = Try();
    ASSERT_TRUE(result.IsError());

    std::unique_ptr<ErrorData> errorData = result.AcquireError();
    ASSERT_EQ(errorData->GetMessage(), placeholderErrorMessage);
}

// Check DAWN_TRY adds to the backtrace.
TEST(ErrorTests, TRY_AddsToBacktrace) {
    auto ReturnError = []() -> MaybeError {
        return DAWN_VALIDATION_ERROR(placeholderErrorMessage);
    };

    auto SingleTry = [ReturnError]() -> MaybeError {
        DAWN_TRY(ReturnError());
        return {};
    };

    auto DoubleTry = [SingleTry]() -> MaybeError {
        DAWN_TRY(SingleTry());
        return {};
    };

    MaybeError singleResult = SingleTry();
    ASSERT_TRUE(singleResult.IsError());

    MaybeError doubleResult = DoubleTry();
    ASSERT_TRUE(doubleResult.IsError());

    std::unique_ptr<ErrorData> singleData = singleResult.AcquireError();
    std::unique_ptr<ErrorData> doubleData = doubleResult.AcquireError();

    // Backtraces are only added in debug mode.
#if defined(DAWN_ENABLE_ASSERTS)
    ASSERT_EQ(singleData->GetBacktrace().size() + 1, doubleData->GetBacktrace().size());
#else
    ASSERT_EQ(singleData->GetBacktrace().size(), doubleData->GetBacktrace().size());
#endif
}

// Check DAWN_TRY_ASSIGN handles successes correctly.
TEST(ErrorTests, TRY_RESULT_Success) {
    auto ReturnSuccess = []() -> ResultOrError<int*> { return &placeholderSuccess; };

    // We need to check that DAWN_TRY doesn't return on successes
    bool tryReturned = true;

    auto Try = [ReturnSuccess, &tryReturned]() -> ResultOrError<int*> {
        int* result = nullptr;
        DAWN_TRY_ASSIGN(result, ReturnSuccess());
        tryReturned = false;

        EXPECT_EQ(result, &placeholderSuccess);
        return result;
    };

    ResultOrError<int*> result = Try();
    ASSERT_TRUE(result.IsSuccess());
    ASSERT_FALSE(tryReturned);
    ASSERT_EQ(result.AcquireSuccess(), &placeholderSuccess);
}

// Check DAWN_TRY_ASSIGN handles errors correctly.
TEST(ErrorTests, TRY_RESULT_Error) {
    auto ReturnError = []() -> ResultOrError<int*> {
        return DAWN_VALIDATION_ERROR(placeholderErrorMessage);
    };

    auto Try = [ReturnError]() -> ResultOrError<int*> {
        [[maybe_unused]] int* result = nullptr;
        DAWN_TRY_ASSIGN(result, ReturnError());

        // DAWN_TRY should return before this point
        EXPECT_FALSE(true);
        return &placeholderSuccess;
    };

    ResultOrError<int*> result = Try();
    ASSERT_TRUE(result.IsError());

    std::unique_ptr<ErrorData> errorData = result.AcquireError();
    ASSERT_EQ(errorData->GetMessage(), placeholderErrorMessage);
}

// Check DAWN_TRY_ASSIGN adds to the backtrace.
TEST(ErrorTests, TRY_RESULT_AddsToBacktrace) {
    auto ReturnError = []() -> ResultOrError<int*> {
        return DAWN_VALIDATION_ERROR(placeholderErrorMessage);
    };

    auto SingleTry = [ReturnError]() -> ResultOrError<int*> {
        DAWN_TRY(ReturnError());
        return &placeholderSuccess;
    };

    auto DoubleTry = [SingleTry]() -> ResultOrError<int*> {
        DAWN_TRY(SingleTry());
        return &placeholderSuccess;
    };

    ResultOrError<int*> singleResult = SingleTry();
    ASSERT_TRUE(singleResult.IsError());

    ResultOrError<int*> doubleResult = DoubleTry();
    ASSERT_TRUE(doubleResult.IsError());

    std::unique_ptr<ErrorData> singleData = singleResult.AcquireError();
    std::unique_ptr<ErrorData> doubleData = doubleResult.AcquireError();

    // Backtraces are only added in debug mode.
#if defined(DAWN_ENABLE_ASSERTS)
    ASSERT_EQ(singleData->GetBacktrace().size() + 1, doubleData->GetBacktrace().size());
#else
    ASSERT_EQ(singleData->GetBacktrace().size(), doubleData->GetBacktrace().size());
#endif
}

// Check a ResultOrError can be DAWN_TRY_ASSIGNED in a function that returns an Error
TEST(ErrorTests, TRY_RESULT_ConversionToError) {
    auto ReturnError = []() -> ResultOrError<int*> {
        return DAWN_VALIDATION_ERROR(placeholderErrorMessage);
    };

    auto Try = [ReturnError]() -> MaybeError {
        [[maybe_unused]] int* result = nullptr;
        DAWN_TRY_ASSIGN(result, ReturnError());

        return {};
    };

    MaybeError result = Try();
    ASSERT_TRUE(result.IsError());

    std::unique_ptr<ErrorData> errorData = result.AcquireError();
    ASSERT_EQ(errorData->GetMessage(), placeholderErrorMessage);
}

// Check a ResultOrError can be DAWN_TRY_ASSIGNED in a function that returns an Error
// Version without Result<E*, T*>
TEST(ErrorTests, TRY_RESULT_ConversionToErrorNonPointer) {
    auto ReturnError = []() -> ResultOrError<int> {
        return DAWN_VALIDATION_ERROR(placeholderErrorMessage);
    };

    auto Try = [ReturnError]() -> MaybeError {
        [[maybe_unused]] int result = 0;
        DAWN_TRY_ASSIGN(result, ReturnError());

        return {};
    };

    MaybeError result = Try();
    ASSERT_TRUE(result.IsError());

    std::unique_ptr<ErrorData> errorData = result.AcquireError();
    ASSERT_EQ(errorData->GetMessage(), placeholderErrorMessage);
}

// Check DAWN_TRY_ASSIGN handles successes correctly.
TEST(ErrorTests, TRY_RESULT_CLEANUP_Success) {
    auto ReturnSuccess = []() -> ResultOrError<int*> { return &placeholderSuccess; };

    // We need to check that DAWN_TRY_ASSIGN_WITH_CLEANUP doesn't return on successes and the
    // cleanup is not called.
    bool tryReturned = true;
    bool tryCleanup = false;

    auto Try = [ReturnSuccess, &tryReturned, &tryCleanup]() -> ResultOrError<int*> {
        int* result = nullptr;
        DAWN_TRY_ASSIGN_WITH_CLEANUP(result, ReturnSuccess(), { tryCleanup = true; });
        tryReturned = false;

        EXPECT_EQ(result, &placeholderSuccess);
        return result;
    };

    ResultOrError<int*> result = Try();
    ASSERT_TRUE(result.IsSuccess());
    ASSERT_FALSE(tryReturned);
    ASSERT_FALSE(tryCleanup);
    ASSERT_EQ(result.AcquireSuccess(), &placeholderSuccess);
}

// Check DAWN_TRY_ASSIGN handles cleanups.
TEST(ErrorTests, TRY_RESULT_CLEANUP_Cleanup) {
    auto ReturnError = []() -> ResultOrError<int*> {
        return DAWN_VALIDATION_ERROR(placeholderErrorMessage);
    };

    // We need to check that DAWN_TRY_ASSIGN_WITH_CLEANUP calls cleanup when error.
    bool tryCleanup = false;

    auto Try = [ReturnError, &tryCleanup]() -> ResultOrError<int*> {
        [[maybe_unused]] int* result = nullptr;
        DAWN_TRY_ASSIGN_WITH_CLEANUP(result, ReturnError(), { tryCleanup = true; });

        // DAWN_TRY_ASSIGN_WITH_CLEANUP should return before this point
        EXPECT_FALSE(true);
        return &placeholderSuccess;
    };

    ResultOrError<int*> result = Try();
    ASSERT_TRUE(result.IsError());

    std::unique_ptr<ErrorData> errorData = result.AcquireError();
    ASSERT_EQ(errorData->GetMessage(), placeholderErrorMessage);
    ASSERT_TRUE(tryCleanup);
}

// Check DAWN_TRY_ASSIGN can override return value when needed.
TEST(ErrorTests, TRY_RESULT_CLEANUP_OverrideReturn) {
    auto ReturnError = []() -> ResultOrError<int*> {
        return DAWN_VALIDATION_ERROR(placeholderErrorMessage);
    };

    auto Try = [ReturnError]() -> bool {
        [[maybe_unused]] int* result = nullptr;
        DAWN_TRY_ASSIGN_WITH_CLEANUP(result, ReturnError(), {}, true);

        // DAWN_TRY_ASSIGN_WITH_CLEANUP should return before this point
        EXPECT_FALSE(true);
        return false;
    };

    bool result = Try();
    ASSERT_TRUE(result);
}

// Check a MaybeError can be DAWN_TRIED in a function that returns an ResultOrError
// Check DAWN_TRY handles errors correctly.
TEST(ErrorTests, TRY_ConversionToErrorOrResult) {
    auto ReturnError = []() -> MaybeError {
        return DAWN_VALIDATION_ERROR(placeholderErrorMessage);
    };

    auto Try = [ReturnError]() -> ResultOrError<int*> {
        DAWN_TRY(ReturnError());
        return &placeholderSuccess;
    };

    ResultOrError<int*> result = Try();
    ASSERT_TRUE(result.IsError());

    std::unique_ptr<ErrorData> errorData = result.AcquireError();
    ASSERT_EQ(errorData->GetMessage(), placeholderErrorMessage);
}

// Check a MaybeError can be DAWN_TRIED in a function that returns an ResultOrError
// Check DAWN_TRY handles errors correctly. Version without Result<E*, T*>
TEST(ErrorTests, TRY_ConversionToErrorOrResultNonPointer) {
    auto ReturnError = []() -> MaybeError {
        return DAWN_VALIDATION_ERROR(placeholderErrorMessage);
    };

    auto Try = [ReturnError]() -> ResultOrError<int> {
        DAWN_TRY(ReturnError());
        return 42;
    };

    ResultOrError<int> result = Try();
    ASSERT_TRUE(result.IsError());

    std::unique_ptr<ErrorData> errorData = result.AcquireError();
    ASSERT_EQ(errorData->GetMessage(), placeholderErrorMessage);
}

}  // namespace
}  // namespace dawn::native
