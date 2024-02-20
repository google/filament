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

const char* rValueOperatorToString(RValueOperator op) {
    switch(op) {
        case RValueOperator::Negative: return "Negative";
        case RValueOperator::LogicalNot: return "LogicalNot";
        case RValueOperator::BitwiseNot: return "BitwiseNot";
        case RValueOperator::PostIncrement: return "PostIncrement";
        case RValueOperator::PostDecrement: return "PostDecrement";
        case RValueOperator::PreIncrement: return "PreIncrement";
        case RValueOperator::PreDecrement: return "PreDecrement";
        case RValueOperator::Add: return "Add";
        case RValueOperator::Sub: return "Sub";
        case RValueOperator::Mul: return "Mul";
        case RValueOperator::Div: return "Div";
        case RValueOperator::Mod: return "Mod";
        case RValueOperator::RightShift: return "RightShift";
        case RValueOperator::LeftShift: return "LeftShift";
        case RValueOperator::And: return "And";
        case RValueOperator::InclusiveOr: return "InclusiveOr";
        case RValueOperator::ExclusiveOr: return "ExclusiveOr";
        case RValueOperator::Equal: return "Equal";
        case RValueOperator::NotEqual: return "NotEqual";
        case RValueOperator::VectorEqual: return "VectorEqual";
        case RValueOperator::VectorNotEqual: return "VectorNotEqual";
        case RValueOperator::LessThan: return "LessThan";
        case RValueOperator::GreaterThan: return "GreaterThan";
        case RValueOperator::LessThanEqual: return "LessThanEqual";
        case RValueOperator::GreaterThanEqual: return "GreaterThanEqual";
        case RValueOperator::Comma: return "Comma";
        case RValueOperator::LogicalOr: return "LogicalOr";
        case RValueOperator::LogicalXor: return "LogicalXor";
        case RValueOperator::LogicalAnd: return "LogicalAnd";
        case RValueOperator::IndexDirect: return "IndexDirect";
        case RValueOperator::IndexIndirect: return "IndexIndirect";
        case RValueOperator::IndexDirectStruct: return "IndexDirectStruct";
        case RValueOperator::VectorSwizzle: return "VectorSwizzle";
        case RValueOperator::Assign: return "Assign";
        case RValueOperator::AddAssign: return "AddAssign";
        case RValueOperator::SubAssign: return "SubAssign";
        case RValueOperator::MulAssign: return "MulAssign";
        case RValueOperator::DivAssign: return "DivAssign";
        case RValueOperator::ModAssign: return "ModAssign";
        case RValueOperator::AndAssign: return "AndAssign";
        case RValueOperator::InclusiveOrAssign: return "InclusiveOrAssign";
        case RValueOperator::ExclusiveOrAssign: return "ExclusiveOrAssign";
        case RValueOperator::LeftShiftAssign: return "LeftShiftAssign";
        case RValueOperator::RightShiftAssign: return "RightShiftAssign";
        case RValueOperator::ArrayLength: return "ArrayLength";
        case RValueOperator::Ternary: return "Ternary";
        case RValueOperator::ConstructStruct: return "ConstructStruct";
    }
}

} // namespace astrict
