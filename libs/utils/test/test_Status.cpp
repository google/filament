/*
* Copyright (C) 2025 The Android Open Source Project
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
#include <gtest/gtest.h>

#include <utils/Status.h>

using namespace utils;

TEST(StatusTest, DefaultConstructorOkStatus) {
    Status actual;
    Status expected(StatusCode::OK, "");
    EXPECT_EQ(actual, expected);
}

TEST(StatusTest, Constructor) {
    std::string_view errorMessage = "invalid";
    Status status(StatusCode::INVALID_ARGUMENT, errorMessage);
    EXPECT_EQ(status.getCode(), StatusCode::INVALID_ARGUMENT);
    EXPECT_EQ(status.getErrorMessage(), errorMessage);
}

TEST(StatusTest, CopyOperator) {
    Status status1 = Status::ok();
    EXPECT_EQ(status1.getCode(), StatusCode::OK);
    EXPECT_EQ(status1.getErrorMessage(), "");

    Status status2(StatusCode::INTERNAL, "internal error");
    status1 = status2;

    EXPECT_EQ(status1, status2);
}

TEST(StatusTest, CopyConstructor) {
    Status original = Status::internal("internal error");
    Status copy(original);
    EXPECT_EQ(original, copy);
}

TEST(StatusTest, MoveOperator) {
    Status status;
    EXPECT_EQ(status.getCode(), StatusCode::OK);
    EXPECT_EQ(status.getErrorMessage(), "");

    std::string_view errorMessage = "internal error";
    Status another(StatusCode::INTERNAL, errorMessage);
    status = std::move(another);

    EXPECT_EQ(status.getCode(), StatusCode::INTERNAL);
    EXPECT_EQ(status.getErrorMessage(), errorMessage);
}

TEST(StatusTest, MoveConstructor) {
    std::string_view errorMessage = "internal error";
    Status original = Status::internal(errorMessage);
    Status moved(std::move(original));

    EXPECT_EQ(moved.getCode(), StatusCode::INTERNAL);
    EXPECT_EQ(moved.getErrorMessage(), errorMessage);
}

TEST(StatusTest, Equality) {
    Status status1(StatusCode::INTERNAL, "internal error");
    Status status2(StatusCode::INTERNAL, "internal error");

    EXPECT_EQ(status1, status2);
}

TEST(StatusTest, InEqualityWithDifferentStatusCode) {
    Status status1(StatusCode::INTERNAL, "internal error");
    Status status2(StatusCode::INVALID_ARGUMENT, "invalid argument error");

    EXPECT_NE(status1, status2);
}

TEST(StatusTest, InEqualityWithDifferentMessage) {
    Status status1(StatusCode::INTERNAL, "internal error 1");
    Status status2(StatusCode::INTERNAL, "invalid error 2");

    EXPECT_NE(status1, status2);
}

TEST(StatusTest, StaticOk) {
    Status expected(StatusCode::OK, "");
    EXPECT_EQ(Status::ok(), expected);
}

TEST(StatusTest, StaticInvalidArgumentError) {
    std::string_view errorMessage = "invalid argument";
    Status expected(StatusCode::INVALID_ARGUMENT, errorMessage);
    EXPECT_EQ(Status::invalidArgument(errorMessage), expected);
}

TEST(StatusTest, StaticInternalError) {
    std::string_view errorMessage = "internal error";
    Status expected(StatusCode::INTERNAL, errorMessage);
    EXPECT_EQ(Status::internal(errorMessage), expected);
}

TEST(StatusTest, IsOk) {
    Status status = Status::ok();
    EXPECT_TRUE(status.isOk());

    status = Status::invalidArgument("error");
    EXPECT_FALSE(status.isOk());
}

TEST(StatusTest, SelfAssignment) {
    Status status = Status::internal("error");
    Status original_copy = status;

    status = status;

    EXPECT_EQ(status, original_copy);
}
