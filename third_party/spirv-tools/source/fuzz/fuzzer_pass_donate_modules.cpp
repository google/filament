// Copyright (c) 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "source/fuzz/fuzzer_pass_donate_modules.h"

#include <map>
#include <queue>
#include <set>

#include "source/fuzz/instruction_message.h"
#include "source/fuzz/transformation_add_constant_boolean.h"
#include "source/fuzz/transformation_add_constant_composite.h"
#include "source/fuzz/transformation_add_constant_scalar.h"
#include "source/fuzz/transformation_add_function.h"
#include "source/fuzz/transformation_add_global_undef.h"
#include "source/fuzz/transformation_add_global_variable.h"
#include "source/fuzz/transformation_add_type_array.h"
#include "source/fuzz/transformation_add_type_boolean.h"
#include "source/fuzz/transformation_add_type_float.h"
#include "source/fuzz/transformation_add_type_function.h"
#include "source/fuzz/transformation_add_type_int.h"
#include "source/fuzz/transformation_add_type_matrix.h"
#include "source/fuzz/transformation_add_type_pointer.h"
#include "source/fuzz/transformation_add_type_struct.h"
#include "source/fuzz/transformation_add_type_vector.h"

namespace spvtools {
namespace fuzz {

FuzzerPassDonateModules::FuzzerPassDonateModules(
    opt::IRContext* ir_context, FactManager* fact_manager,
    FuzzerContext* fuzzer_context,
    protobufs::TransformationSequence* transformations,
    const std::vector<fuzzerutil::ModuleSupplier>& donor_suppliers)
    : FuzzerPass(ir_context, fact_manager, fuzzer_context, transformations),
      donor_suppliers_(donor_suppliers) {}

FuzzerPassDonateModules::~FuzzerPassDonateModules() = default;

void FuzzerPassDonateModules::Apply() {
  // If there are no donor suppliers, this fuzzer pass is a no-op.
  if (donor_suppliers_.empty()) {
    return;
  }

  // Donate at least one module, and probabilistically decide when to stop
  // donating modules.
  do {
    // Choose a donor supplier at random, and get the module that it provides.
    std::unique_ptr<opt::IRContext> donor_ir_context = donor_suppliers_.at(
        GetFuzzerContext()->RandomIndex(donor_suppliers_))();
    assert(donor_ir_context != nullptr && "Supplying of donor failed");
    // Donate the supplied module.
    //
    // Randomly decide whether to make the module livesafe (see
    // FactFunctionIsLivesafe); doing so allows it to be used for live code
    // injection but restricts its behaviour to allow this, and means that its
    // functions cannot be transformed as if they were arbitrary dead code.
    bool make_livesafe = GetFuzzerContext()->ChoosePercentage(
        GetFuzzerContext()->ChanceOfMakingDonorLivesafe());
    DonateSingleModule(donor_ir_context.get(), make_livesafe);
  } while (GetFuzzerContext()->ChoosePercentage(
      GetFuzzerContext()->GetChanceOfDonatingAdditionalModule()));
}

void FuzzerPassDonateModules::DonateSingleModule(
    opt::IRContext* donor_ir_context, bool make_livesafe) {
  // The ids used by the donor module may very well clash with ids defined in
  // the recipient module.  Furthermore, some instructions defined in the donor
  // module will be equivalent to instructions defined in the recipient module,
  // and it is not always legal to re-declare equivalent instructions.  For
  // example, OpTypeVoid cannot be declared twice.
  //
  // To handle this, we maintain a mapping from an id used in the donor module
  // to the corresponding id that will be used by the donated code when it
  // appears in the recipient module.
  //
  // This mapping is populated in two ways:
  // (1) by mapping a donor instruction's result id to the id of some equivalent
  //     existing instruction in the recipient (e.g. this has to be done for
  //     OpTypeVoid)
  // (2) by mapping a donor instruction's result id to a freshly chosen id that
  //     is guaranteed to be different from any id already used by the recipient
  //     (or from any id already chosen to handle a previous donor id)
  std::map<uint32_t, uint32_t> original_id_to_donated_id;

  HandleExternalInstructionImports(donor_ir_context,
                                   &original_id_to_donated_id);
  HandleTypesAndValues(donor_ir_context, &original_id_to_donated_id);
  HandleFunctions(donor_ir_context, &original_id_to_donated_id, make_livesafe);

  // TODO(https://github.com/KhronosGroup/SPIRV-Tools/issues/3115) Handle some
  //  kinds of decoration.
}

SpvStorageClass FuzzerPassDonateModules::AdaptStorageClass(
    SpvStorageClass donor_storage_class) {
  switch (donor_storage_class) {
    case SpvStorageClassFunction:
    case SpvStorageClassPrivate:
      // We leave these alone
      return donor_storage_class;
    case SpvStorageClassInput:
    case SpvStorageClassOutput:
    case SpvStorageClassUniform:
    case SpvStorageClassUniformConstant:
    case SpvStorageClassPushConstant:
      // We change these to Private
      return SpvStorageClassPrivate;
    default:
      // Handle other cases on demand.
      assert(false && "Currently unsupported storage class.");
      return SpvStorageClassMax;
  }
}

void FuzzerPassDonateModules::HandleExternalInstructionImports(
    opt::IRContext* donor_ir_context,
    std::map<uint32_t, uint32_t>* original_id_to_donated_id) {
  // Consider every external instruction set import in the donor module.
  for (auto& donor_import : donor_ir_context->module()->ext_inst_imports()) {
    const auto& donor_import_name_words = donor_import.GetInOperand(0).words;
    // Look for an identical import in the recipient module.
    for (auto& existing_import : GetIRContext()->module()->ext_inst_imports()) {
      const auto& existing_import_name_words =
          existing_import.GetInOperand(0).words;
      if (donor_import_name_words == existing_import_name_words) {
        // A matching import has found.  Map the result id for the donor import
        // to the id of the existing import, so that when donor instructions
        // rely on the import they will be rewritten to use the existing import.
        original_id_to_donated_id->insert(
            {donor_import.result_id(), existing_import.result_id()});
        break;
      }
    }
    // TODO(https://github.com/KhronosGroup/SPIRV-Tools/issues/3116): At present
    //  we do not handle donation of instruction imports, i.e. we do not allow
    //  the donor to import instruction sets that the recipient did not already
    //  import.  It might be a good idea to allow this, but it requires some
    //  thought.
    assert(original_id_to_donated_id->count(donor_import.result_id()) &&
           "Donation of imports is not yet supported.");
  }
}

void FuzzerPassDonateModules::HandleTypesAndValues(
    opt::IRContext* donor_ir_context,
    std::map<uint32_t, uint32_t>* original_id_to_donated_id) {
  // Consider every type/global/constant/undef in the module.
  for (auto& type_or_value : donor_ir_context->module()->types_values()) {
    // Each such instruction generates a result id, and as part of donation we
    // need to associate the donor's result id with a new result id.  That new
    // result id will either be the id of some existing instruction, or a fresh
    // id.  This variable captures it.
    uint32_t new_result_id;

    // Decide how to handle each kind of instruction on a case-by-case basis.
    //
    // Because the donor module is required to be valid, when we encounter a
    // type comprised of component types (e.g. an aggregate or pointer), we know
    // that its component types will have been considered previously, and that
    // |original_id_to_donated_id| will already contain an entry for them.
    switch (type_or_value.opcode()) {
      case SpvOpTypeVoid: {
        // Void has to exist already in order for us to have an entry point.
        // Get the existing id of void.
        opt::analysis::Void void_type;
        new_result_id = GetIRContext()->get_type_mgr()->GetId(&void_type);
        assert(new_result_id &&
               "The module being transformed will always have 'void' type "
               "declared.");
      } break;
      case SpvOpTypeBool: {
        // Bool cannot be declared multiple times, so use its existing id if
        // present, or add a declaration of Bool with a fresh id if not.
        opt::analysis::Bool bool_type;
        auto bool_type_id = GetIRContext()->get_type_mgr()->GetId(&bool_type);
        if (bool_type_id) {
          new_result_id = bool_type_id;
        } else {
          new_result_id = GetFuzzerContext()->GetFreshId();
          ApplyTransformation(TransformationAddTypeBoolean(new_result_id));
        }
      } break;
      case SpvOpTypeInt: {
        // Int cannot be declared multiple times with the same width and
        // signedness, so check whether an existing identical Int type is
        // present and use its id if so.  Otherwise add a declaration of the
        // Int type used by the donor, with a fresh id.
        const uint32_t width = type_or_value.GetSingleWordInOperand(0);
        const bool is_signed =
            static_cast<bool>(type_or_value.GetSingleWordInOperand(1));
        opt::analysis::Integer int_type(width, is_signed);
        auto int_type_id = GetIRContext()->get_type_mgr()->GetId(&int_type);
        if (int_type_id) {
          new_result_id = int_type_id;
        } else {
          new_result_id = GetFuzzerContext()->GetFreshId();
          ApplyTransformation(
              TransformationAddTypeInt(new_result_id, width, is_signed));
        }
      } break;
      case SpvOpTypeFloat: {
        // Similar to SpvOpTypeInt.
        const uint32_t width = type_or_value.GetSingleWordInOperand(0);
        opt::analysis::Float float_type(width);
        auto float_type_id = GetIRContext()->get_type_mgr()->GetId(&float_type);
        if (float_type_id) {
          new_result_id = float_type_id;
        } else {
          new_result_id = GetFuzzerContext()->GetFreshId();
          ApplyTransformation(TransformationAddTypeFloat(new_result_id, width));
        }
      } break;
      case SpvOpTypeVector: {
        // It is not legal to have two Vector type declarations with identical
        // element types and element counts, so check whether an existing
        // identical Vector type is present and use its id if so.  Otherwise add
        // a declaration of the Vector type used by the donor, with a fresh id.

        // When considering the vector's component type id, we look up the id
        // use in the donor to find the id to which this has been remapped.
        uint32_t component_type_id = original_id_to_donated_id->at(
            type_or_value.GetSingleWordInOperand(0));
        auto component_type =
            GetIRContext()->get_type_mgr()->GetType(component_type_id);
        assert(component_type && "The base type should be registered.");
        auto component_count = type_or_value.GetSingleWordInOperand(1);
        opt::analysis::Vector vector_type(component_type, component_count);
        auto vector_type_id =
            GetIRContext()->get_type_mgr()->GetId(&vector_type);
        if (vector_type_id) {
          new_result_id = vector_type_id;
        } else {
          new_result_id = GetFuzzerContext()->GetFreshId();
          ApplyTransformation(TransformationAddTypeVector(
              new_result_id, component_type_id, component_count));
        }
      } break;
      case SpvOpTypeMatrix: {
        // Similar to SpvOpTypeVector.
        uint32_t column_type_id = original_id_to_donated_id->at(
            type_or_value.GetSingleWordInOperand(0));
        auto column_type =
            GetIRContext()->get_type_mgr()->GetType(column_type_id);
        assert(column_type && column_type->AsVector() &&
               "The column type should be a registered vector type.");
        auto column_count = type_or_value.GetSingleWordInOperand(1);
        opt::analysis::Matrix matrix_type(column_type, column_count);
        auto matrix_type_id =
            GetIRContext()->get_type_mgr()->GetId(&matrix_type);
        if (matrix_type_id) {
          new_result_id = matrix_type_id;
        } else {
          new_result_id = GetFuzzerContext()->GetFreshId();
          ApplyTransformation(TransformationAddTypeMatrix(
              new_result_id, column_type_id, column_count));
        }

      } break;
      case SpvOpTypeArray: {
        // It is OK to have multiple structurally identical array types, so
        // we go ahead and add a remapped version of the type declared by the
        // donor.
        new_result_id = GetFuzzerContext()->GetFreshId();
        ApplyTransformation(TransformationAddTypeArray(
            new_result_id,
            original_id_to_donated_id->at(
                type_or_value.GetSingleWordInOperand(0)),
            original_id_to_donated_id->at(
                type_or_value.GetSingleWordInOperand(1))));
      } break;
      case SpvOpTypeStruct: {
        // Similar to SpvOpTypeArray.
        new_result_id = GetFuzzerContext()->GetFreshId();
        std::vector<uint32_t> member_type_ids;
        type_or_value.ForEachInId(
            [&member_type_ids,
             &original_id_to_donated_id](const uint32_t* component_type_id) {
              member_type_ids.push_back(
                  original_id_to_donated_id->at(*component_type_id));
            });
        ApplyTransformation(
            TransformationAddTypeStruct(new_result_id, member_type_ids));
      } break;
      case SpvOpTypePointer: {
        // Similar to SpvOpTypeArray.
        new_result_id = GetFuzzerContext()->GetFreshId();
        ApplyTransformation(TransformationAddTypePointer(
            new_result_id,
            AdaptStorageClass(static_cast<SpvStorageClass>(
                type_or_value.GetSingleWordInOperand(0))),
            original_id_to_donated_id->at(
                type_or_value.GetSingleWordInOperand(1))));
      } break;
      case SpvOpTypeFunction: {
        // It is not OK to have multiple function types that use identical ids
        // for their return and parameter types.  We thus go through all
        // existing function types to look for a match.  We do not use the
        // type manager here because we want to regard two function types that
        // are structurally identical but that differ with respect to the
        // actual ids used for pointer types as different.
        //
        // Example:
        //
        // %1 = OpTypeVoid
        // %2 = OpTypeInt 32 0
        // %3 = OpTypePointer Function %2
        // %4 = OpTypePointer Function %2
        // %5 = OpTypeFunction %1 %3
        // %6 = OpTypeFunction %1 %4
        //
        // We regard %5 and %6 as distinct function types here, even though
        // they both have the form "uint32* -> void"

        std::vector<uint32_t> return_and_parameter_types;
        for (uint32_t i = 0; i < type_or_value.NumInOperands(); i++) {
          return_and_parameter_types.push_back(original_id_to_donated_id->at(
              type_or_value.GetSingleWordInOperand(i)));
        }
        uint32_t existing_function_id = fuzzerutil::FindFunctionType(
            GetIRContext(), return_and_parameter_types);
        if (existing_function_id) {
          new_result_id = existing_function_id;
        } else {
          // No match was found, so add a remapped version of the function type
          // to the module, with a fresh id.
          new_result_id = GetFuzzerContext()->GetFreshId();
          std::vector<uint32_t> argument_type_ids;
          for (uint32_t i = 1; i < type_or_value.NumInOperands(); i++) {
            argument_type_ids.push_back(original_id_to_donated_id->at(
                type_or_value.GetSingleWordInOperand(i)));
          }
          ApplyTransformation(TransformationAddTypeFunction(
              new_result_id,
              original_id_to_donated_id->at(
                  type_or_value.GetSingleWordInOperand(0)),
              argument_type_ids));
        }
      } break;
      case SpvOpConstantTrue:
      case SpvOpConstantFalse: {
        // It is OK to have duplicate definitions of True and False, so add
        // these to the module, using a remapped Bool type.
        new_result_id = GetFuzzerContext()->GetFreshId();
        ApplyTransformation(TransformationAddConstantBoolean(
            new_result_id, type_or_value.opcode() == SpvOpConstantTrue));
      } break;
      case SpvOpConstant: {
        // It is OK to have duplicate constant definitions, so add this to the
        // module using a remapped result type.
        new_result_id = GetFuzzerContext()->GetFreshId();
        std::vector<uint32_t> data_words;
        type_or_value.ForEachInOperand(
            [&data_words](const uint32_t* in_operand) {
              data_words.push_back(*in_operand);
            });
        ApplyTransformation(TransformationAddConstantScalar(
            new_result_id,
            original_id_to_donated_id->at(type_or_value.type_id()),
            data_words));
      } break;
      case SpvOpConstantComposite: {
        // It is OK to have duplicate constant composite definitions, so add
        // this to the module using remapped versions of all consituent ids and
        // the result type.
        new_result_id = GetFuzzerContext()->GetFreshId();
        std::vector<uint32_t> constituent_ids;
        type_or_value.ForEachInId(
            [&constituent_ids,
             &original_id_to_donated_id](const uint32_t* constituent_id) {
              constituent_ids.push_back(
                  original_id_to_donated_id->at(*constituent_id));
            });
        ApplyTransformation(TransformationAddConstantComposite(
            new_result_id,
            original_id_to_donated_id->at(type_or_value.type_id()),
            constituent_ids));
      } break;
      case SpvOpVariable: {
        // This is a global variable that could have one of various storage
        // classes.  However, we change all global variable pointer storage
        // classes (such as Uniform, Input and Output) to private when donating
        // pointer types.  Thus this variable's pointer type is guaranteed to
        // have storage class private.  As a result, we simply add a Private
        // storage class global variable, using remapped versions of the result
        // type and initializer ids for the global variable in the donor.
        //
        // We regard the added variable as having an arbitrary value.  This
        // means that future passes can add stores to the variable in any
        // way they wish, and pass them as pointer parameters to functions
        // without worrying about whether their data might get modified.
        new_result_id = GetFuzzerContext()->GetFreshId();
        ApplyTransformation(TransformationAddGlobalVariable(
            new_result_id,
            original_id_to_donated_id->at(type_or_value.type_id()),
            type_or_value.NumInOperands() == 1
                ? 0
                : original_id_to_donated_id->at(
                      type_or_value.GetSingleWordInOperand(1)),
            true));
      } break;
      case SpvOpUndef: {
        // It is fine to have multiple Undef instructions of the same type, so
        // we just add this to the recipient module.
        new_result_id = GetFuzzerContext()->GetFreshId();
        ApplyTransformation(TransformationAddGlobalUndef(
            new_result_id,
            original_id_to_donated_id->at(type_or_value.type_id())));
      } break;
      default: {
        assert(0 && "Unknown type/value.");
        new_result_id = 0;
      } break;
    }
    // Update the id mapping to associate the instruction's result id with its
    // corresponding id in the recipient.
    original_id_to_donated_id->insert(
        {type_or_value.result_id(), new_result_id});
  }
}

void FuzzerPassDonateModules::HandleFunctions(
    opt::IRContext* donor_ir_context,
    std::map<uint32_t, uint32_t>* original_id_to_donated_id,
    bool make_livesafe) {
  // Get the ids of functions in the donor module, topologically sorted
  // according to the donor's call graph.
  auto topological_order =
      GetFunctionsInCallGraphTopologicalOrder(donor_ir_context);

  // Donate the functions in reverse topological order.  This ensures that a
  // function gets donated before any function that depends on it.  This allows
  // donation of the functions to be separated into a number of transformations,
  // each adding one function, such that every prefix of transformations leaves
  // the module valid.
  for (auto function_id = topological_order.rbegin();
       function_id != topological_order.rend(); ++function_id) {
    // Find the function to be donated.
    opt::Function* function_to_donate = nullptr;
    for (auto& function : *donor_ir_context->module()) {
      if (function.result_id() == *function_id) {
        function_to_donate = &function;
        break;
      }
    }
    assert(function_to_donate && "Function to be donated was not found.");

    // We will collect up protobuf messages representing the donor function's
    // instructions here, and use them to create an AddFunction transformation.
    std::vector<protobufs::Instruction> donated_instructions;

    // Scan through the function, remapping each result id that it generates to
    // a fresh id.  This is necessary because functions include forward
    // references, e.g. to labels.
    function_to_donate->ForEachInst([this, &original_id_to_donated_id](
                                        const opt::Instruction* instruction) {
      if (instruction->result_id()) {
        original_id_to_donated_id->insert(
            {instruction->result_id(), GetFuzzerContext()->GetFreshId()});
      }
    });

    // Consider every instruction of the donor function.
    function_to_donate->ForEachInst(
        [&donated_instructions,
         &original_id_to_donated_id](const opt::Instruction* instruction) {
          // Get the instruction's input operands into donation-ready form,
          // remapping any id uses in the process.
          opt::Instruction::OperandList input_operands;

          // Consider each input operand in turn.
          for (uint32_t in_operand_index = 0;
               in_operand_index < instruction->NumInOperands();
               in_operand_index++) {
            std::vector<uint32_t> operand_data;
            const opt::Operand& in_operand =
                instruction->GetInOperand(in_operand_index);
            switch (in_operand.type) {
              case SPV_OPERAND_TYPE_ID:
              case SPV_OPERAND_TYPE_TYPE_ID:
              case SPV_OPERAND_TYPE_RESULT_ID:
              case SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID:
              case SPV_OPERAND_TYPE_SCOPE_ID:
                // This is an id operand - it consists of a single word of data,
                // which needs to be remapped so that it is replaced with the
                // donated form of the id.
                operand_data.push_back(
                    original_id_to_donated_id->at(in_operand.words[0]));
                break;
              default:
                // For non-id operands, we just add each of the data words.
                for (auto word : in_operand.words) {
                  operand_data.push_back(word);
                }
                break;
            }
            input_operands.push_back({in_operand.type, operand_data});
          }
          // Remap the result type and result id (if present) of the
          // instruction, and turn it into a protobuf message.
          donated_instructions.push_back(MakeInstructionMessage(
              instruction->opcode(),
              instruction->type_id()
                  ? original_id_to_donated_id->at(instruction->type_id())
                  : 0,
              instruction->result_id()
                  ? original_id_to_donated_id->at(instruction->result_id())
                  : 0,
              input_operands));
        });

    if (make_livesafe) {
      // Various types and constants must be in place for a function to be made
      // live-safe.  Add them if not already present.
      FindOrCreateBoolType();  // Needed for comparisons
      FindOrCreatePointerTo32BitIntegerType(
          false, SpvStorageClassFunction);  // Needed for adding loop limiters
      FindOrCreate32BitIntegerConstant(
          0, false);  // Needed for initializing loop limiters
      FindOrCreate32BitIntegerConstant(
          1, false);  // Needed for incrementing loop limiters

      // Get a fresh id for the variable that will be used as a loop limiter.
      const uint32_t loop_limiter_variable_id =
          GetFuzzerContext()->GetFreshId();
      // Choose a random loop limit, and add the required constant to the
      // module if not already there.
      const uint32_t loop_limit = FindOrCreate32BitIntegerConstant(
          GetFuzzerContext()->GetRandomLoopLimit(), false);

      // Consider every loop header in the function to donate, and create a
      // structure capturing the ids to be used for manipulating the loop
      // limiter each time the loop is iterated.
      std::vector<protobufs::LoopLimiterInfo> loop_limiters;
      for (auto& block : *function_to_donate) {
        if (block.IsLoopHeader()) {
          protobufs::LoopLimiterInfo loop_limiter;
          // Grab the loop header's id, mapped to its donated value.
          loop_limiter.set_loop_header_id(
              original_id_to_donated_id->at(block.id()));
          // Get fresh ids that will be used to load the loop limiter, increment
          // it, compare it with the loop limit, and an id for a new block that
          // will contain the loop's original terminator.
          loop_limiter.set_load_id(GetFuzzerContext()->GetFreshId());
          loop_limiter.set_increment_id(GetFuzzerContext()->GetFreshId());
          loop_limiter.set_compare_id(GetFuzzerContext()->GetFreshId());
          loop_limiter.set_logical_op_id(GetFuzzerContext()->GetFreshId());
          loop_limiters.emplace_back(loop_limiter);
        }
      }

      // Consider every access chain in the function to donate, and create a
      // structure containing the ids necessary to clamp the access chain
      // indices to be in-bounds.
      std::vector<protobufs::AccessChainClampingInfo>
          access_chain_clamping_info;
      for (auto& block : *function_to_donate) {
        for (auto& inst : block) {
          switch (inst.opcode()) {
            case SpvOpAccessChain:
            case SpvOpInBoundsAccessChain: {
              protobufs::AccessChainClampingInfo clamping_info;
              clamping_info.set_access_chain_id(
                  original_id_to_donated_id->at(inst.result_id()));

              auto base_object = donor_ir_context->get_def_use_mgr()->GetDef(
                  inst.GetSingleWordInOperand(0));
              assert(base_object && "The base object must exist.");
              auto pointer_type = donor_ir_context->get_def_use_mgr()->GetDef(
                  base_object->type_id());
              assert(pointer_type &&
                     pointer_type->opcode() == SpvOpTypePointer &&
                     "The base object must have pointer type.");

              auto should_be_composite_type =
                  donor_ir_context->get_def_use_mgr()->GetDef(
                      pointer_type->GetSingleWordInOperand(1));

              // Walk the access chain, creating fresh ids to facilitate
              // clamping each index.  For simplicity we do this for every
              // index, even though constant indices will not end up being
              // clamped.
              for (uint32_t index = 1; index < inst.NumInOperands(); index++) {
                auto compare_and_select_ids =
                    clamping_info.add_compare_and_select_ids();
                compare_and_select_ids->set_first(
                    GetFuzzerContext()->GetFreshId());
                compare_and_select_ids->set_second(
                    GetFuzzerContext()->GetFreshId());

                // Get the bound for the component being indexed into.
                uint32_t bound =
                    TransformationAddFunction::GetBoundForCompositeIndex(
                        donor_ir_context, *should_be_composite_type);
                const uint32_t index_id = inst.GetSingleWordInOperand(index);
                auto index_inst =
                    donor_ir_context->get_def_use_mgr()->GetDef(index_id);
                auto index_type_inst =
                    donor_ir_context->get_def_use_mgr()->GetDef(
                        index_inst->type_id());
                assert(index_type_inst->opcode() == SpvOpTypeInt);
                assert(index_type_inst->GetSingleWordInOperand(0) == 32);
                opt::analysis::Integer* index_int_type =
                    donor_ir_context->get_type_mgr()
                        ->GetType(index_type_inst->result_id())
                        ->AsInteger();
                if (index_inst->opcode() != SpvOpConstant) {
                  // We will have to clamp this index, so we need a constant
                  // whose value is one less than the bound, to compare
                  // against and to use as the clamped value.
                  FindOrCreate32BitIntegerConstant(bound - 1,
                                                   index_int_type->IsSigned());
                }
                should_be_composite_type =
                    TransformationAddFunction::FollowCompositeIndex(
                        donor_ir_context, *should_be_composite_type, index_id);
              }
              access_chain_clamping_info.push_back(clamping_info);
              break;
            }
            default:
              break;
          }
        }
      }

      // If the function contains OpKill or OpUnreachable instructions, and has
      // non-void return type, then we need a value %v to use in order to turn
      // these into instructions of the form OpReturn %v.
      uint32_t kill_unreachable_return_value_id;
      auto function_return_type_inst =
          donor_ir_context->get_def_use_mgr()->GetDef(
              function_to_donate->type_id());
      if (function_return_type_inst->opcode() == SpvOpTypeVoid) {
        // The return type is void, so we don't need a return value.
        kill_unreachable_return_value_id = 0;
      } else {
        // We do need a return value; we use OpUndef.
        kill_unreachable_return_value_id =
            FindOrCreateGlobalUndef(function_return_type_inst->type_id());
      }
      // Add the function in a livesafe manner.
      ApplyTransformation(TransformationAddFunction(
          donated_instructions, loop_limiter_variable_id, loop_limit,
          loop_limiters, kill_unreachable_return_value_id,
          access_chain_clamping_info));
    } else {
      // Add the function in a non-livesafe manner.
      ApplyTransformation(TransformationAddFunction(donated_instructions));
    }
  }
}

std::vector<uint32_t>
FuzzerPassDonateModules::GetFunctionsInCallGraphTopologicalOrder(
    opt::IRContext* context) {
  // This is an implementation of Kahn’s algorithm for topological sorting.

  // For each function id, stores the number of distinct functions that call
  // the function.
  std::map<uint32_t, uint32_t> function_in_degree;

  // We first build a call graph for the module, and compute the in-degree for
  // each function in the process.
  // TODO(afd): If there is functionality elsewhere in the SPIR-V tools
  //  framework to construct call graphs it could be nice to re-use it here.
  std::map<uint32_t, std::set<uint32_t>> call_graph_edges;

  // Initialize function in-degree and call graph edges to 0 and empty.
  for (auto& function : *context->module()) {
    function_in_degree[function.result_id()] = 0;
    call_graph_edges[function.result_id()] = std::set<uint32_t>();
  }

  // Consider every function.
  for (auto& function : *context->module()) {
    // Avoid considering the same callee of this function multiple times by
    // recording known callees.
    std::set<uint32_t> known_callees;
    // Consider every function call instruction in every block.
    for (auto& block : function) {
      for (auto& instruction : block) {
        if (instruction.opcode() != SpvOpFunctionCall) {
          continue;
        }
        // Get the id of the function being called.
        uint32_t callee = instruction.GetSingleWordInOperand(0);
        if (known_callees.count(callee)) {
          // We have already considered a call to this function - ignore it.
          continue;
        }
        // Increase the callee's in-degree and add an edge to the call graph.
        function_in_degree[callee]++;
        call_graph_edges[function.result_id()].insert(callee);
        // Mark the callee as 'known'.
        known_callees.insert(callee);
      }
    }
  }

  // This is the sorted order of function ids that we will eventually return.
  std::vector<uint32_t> result;

  // Populate a queue with all those function ids with in-degree zero.
  std::queue<uint32_t> queue;
  for (auto& entry : function_in_degree) {
    if (entry.second == 0) {
      queue.push(entry.first);
    }
  }

  // Pop ids from the queue, adding them to the sorted order and decreasing the
  // in-degrees of their successors.  A successor who's in-degree becomes zero
  // gets added to the queue.
  while (!queue.empty()) {
    auto next = queue.front();
    queue.pop();
    result.push_back(next);
    for (auto successor : call_graph_edges.at(next)) {
      assert(function_in_degree.at(successor) > 0 &&
             "The in-degree cannot be zero if the function is a successor.");
      function_in_degree[successor] = function_in_degree.at(successor) - 1;
      if (function_in_degree.at(successor) == 0) {
        queue.push(successor);
      }
    }
  }

  assert(result.size() == function_in_degree.size() &&
         "Every function should appear in the sort.");

  return result;
}

}  // namespace fuzz
}  // namespace spvtools
