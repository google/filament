/*
 * Copyright (C) 2024 The Android Open Source Project
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

#include <astrict/DebugCommon.h>

namespace astrict {

const char* rValueOperatorToString(ExpressionOperator op) {
    switch(op) {
        case ExpressionOperator::Negative: return "Negative";
        case ExpressionOperator::LogicalNot: return "LogicalNot";
        case ExpressionOperator::BitwiseNot: return "BitwiseNot";
        case ExpressionOperator::PostIncrement: return "PostIncrement";
        case ExpressionOperator::PostDecrement: return "PostDecrement";
        case ExpressionOperator::PreIncrement: return "PreIncrement";
        case ExpressionOperator::PreDecrement: return "PreDecrement";
        case ExpressionOperator::ArrayLength: return "ArrayLength";
        case ExpressionOperator::Add: return "Add";
        case ExpressionOperator::Sub: return "Sub";
        case ExpressionOperator::Mul: return "Mul";
        case ExpressionOperator::Div: return "Div";
        case ExpressionOperator::Mod: return "Mod";
        case ExpressionOperator::RightShift: return "RightShift";
        case ExpressionOperator::LeftShift: return "LeftShift";
        case ExpressionOperator::And: return "And";
        case ExpressionOperator::InclusiveOr: return "InclusiveOr";
        case ExpressionOperator::ExclusiveOr: return "ExclusiveOr";
        case ExpressionOperator::Equal: return "Equal";
        case ExpressionOperator::NotEqual: return "NotEqual";
        case ExpressionOperator::LessThan: return "LessThan";
        case ExpressionOperator::GreaterThan: return "GreaterThan";
        case ExpressionOperator::LessThanEqual: return "LessThanEqual";
        case ExpressionOperator::GreaterThanEqual: return "GreaterThanEqual";
        case ExpressionOperator::LogicalOr: return "LogicalOr";
        case ExpressionOperator::LogicalXor: return "LogicalXor";
        case ExpressionOperator::LogicalAnd: return "LogicalAnd";
        case ExpressionOperator::Index: return "Index";
        case ExpressionOperator::IndexStruct: return "IndexStruct";
        case ExpressionOperator::VectorSwizzle: return "VectorSwizzle";
        case ExpressionOperator::Assign: return "Assign";
        case ExpressionOperator::AddAssign: return "AddAssign";
        case ExpressionOperator::SubAssign: return "SubAssign";
        case ExpressionOperator::MulAssign: return "MulAssign";
        case ExpressionOperator::DivAssign: return "DivAssign";
        case ExpressionOperator::ModAssign: return "ModAssign";
        case ExpressionOperator::AndAssign: return "AndAssign";
        case ExpressionOperator::InclusiveOrAssign: return "InclusiveOrAssign";
        case ExpressionOperator::ExclusiveOrAssign: return "ExclusiveOrAssign";
        case ExpressionOperator::LeftShiftAssign: return "LeftShiftAssign";
        case ExpressionOperator::RightShiftAssign: return "RightShiftAssign";
        case ExpressionOperator::Ternary: return "Ternary";
        case ExpressionOperator::Comma: return "Comma";
        case ExpressionOperator::ConstructStruct: return "ConstructStruct";
    }
}

} // namespace astrict
