//===--- DeclResultIdMapper.h - AST Decl to SPIR-V <result-id> mapper ------==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_SPIRV_DECLRESULTIDMAPPER_H
#define LLVM_CLANG_LIB_SPIRV_DECLRESULTIDMAPPER_H

#include <tuple>
#include <vector>

#include "dxc/Support/SPIRVOptions.h"
#include "spirv/unified1/spirv.hpp11"
#include "clang/AST/Attr.h"
#include "clang/SPIRV/FeatureManager.h"
#include "clang/SPIRV/SpirvBuilder.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/SmallVector.h"

#include "GlPerVertex.h"
#include "StageVar.h"

namespace clang {
namespace spirv {

class SpirvEmitter;

class ResourceVar {
public:
  ResourceVar(SpirvVariable *var, const Decl *decl, SourceLocation loc,
              const hlsl::RegisterAssignment *r, const VKBindingAttr *b,
              const VKCounterBindingAttr *cb, bool counter = false,
              bool globalsBuffer = false)
      : variable(var), declaration(decl), srcLoc(loc), reg(r), binding(b),
        counterBinding(cb), isCounterVar(counter),
        isGlobalsCBuffer(globalsBuffer) {}

  SpirvVariable *getSpirvInstr() const { return variable; }
  const Decl *getDeclaration() const { return declaration; }
  SourceLocation getSourceLocation() const { return srcLoc; }
  const hlsl::RegisterAssignment *getRegister() const { return reg; }
  const VKBindingAttr *getBinding() const { return binding; }
  bool isCounter() const { return isCounterVar; }
  bool isGlobalsBuffer() const { return isGlobalsCBuffer; }
  const VKCounterBindingAttr *getCounterBinding() const {
    return counterBinding;
  }

private:
  SpirvVariable *variable;                    ///< The variable
  const Decl *declaration;                    ///< The declaration
  SourceLocation srcLoc;                      ///< Source location
  const hlsl::RegisterAssignment *reg;        ///< HLSL register assignment
  const VKBindingAttr *binding;               ///< Vulkan binding assignment
  const VKCounterBindingAttr *counterBinding; ///< Vulkan counter binding
  bool isCounterVar;                          ///< Couter variable or not
  bool isGlobalsCBuffer;                      ///< $Globals cbuffer or not
};

/// A (instruction-pointer, is-alias-or-not) pair for counter variables
class CounterIdAliasPair {
public:
  /// Default constructor to satisfy llvm::DenseMap
  CounterIdAliasPair() : counterVar(nullptr), isAlias(false) {}
  CounterIdAliasPair(SpirvVariable *var, bool alias)
      : counterVar(var), isAlias(alias) {}

  /// Returns the pointer to the counter variable alias. This returns a pointer
  /// that can be used as the address to a store instruction when storing to an
  /// alias counter.
  SpirvInstruction *getAliasAddress() const;

  /// Returns the pointer to the counter variable. Dereferences first if this is
  /// an alias to a counter variable.
  SpirvInstruction *getCounterVariable(SpirvBuilder &builder,
                                       SpirvContext &spvContext) const;

  /// Stores the counter variable pointed to by src to the curent counter
  /// variable. The current counter variable must be an alias.
  inline void assign(SpirvInstruction *src, SpirvBuilder &) const;

private:
  SpirvVariable *counterVar;
  /// Note: legalization specific code
  bool isAlias;
};

/// A class for holding all the counter variables associated with a struct's
/// fields
///
/// A alias local RW/Append/Consume structured buffer will need an associated
/// counter variable generated. There are four forms such an alias buffer can
/// be:
///
/// 1 (AssocCounter#1). A stand-alone variable,
/// 2 (AssocCounter#2). A struct field,
/// 3 (AssocCounter#3). A struct containing alias fields,
/// 4 (AssocCounter#4). A nested struct containing alias fields.
///
/// We consider the first two cases as *final* alias entities; The last two
/// cases are called as *intermediate* alias entities, since we can still
/// decompose them and get final alias entities.
///
/// We need to create an associated counter variable no matter which form the
/// alias buffer is in, which means we need to recursively visit all fields of a
/// struct to discover if it's not AssocCounter#1. That means a hierarchy.
///
/// The purpose of this class is to provide such hierarchy in a *flattened* way.
/// Each field's associated counter is represented with an index vector and the
/// counter's <result-id>. For example, for the following structs,
///
/// struct S {
///       RWStructuredBuffer s1;
///   AppendStructuredBuffer s2;
/// };
///
/// struct T {
///   S t1;
///   S t2;
/// };
///
/// An instance of T will have four associated counters for
///   field: indices, <result-id>
///   t1.s1: [0, 0], <id-1>
///   t1.s2: [0, 1], <id-2>
///   t2.s1: [1, 0], <id-3>
///   t2.s2: [1, 1], <id-4>
class CounterVarFields {
public:
  CounterVarFields() = default;

  /// Registers a field's associated counter.
  void append(const llvm::SmallVector<uint32_t, 4> &indices,
              SpirvVariable *counter) {
    fields.emplace_back(indices, counter);
  }

  /// Returns the counter associated with the field at the given indices if it
  /// has. Returns nullptr otherwise.
  const CounterIdAliasPair *
  get(const llvm::SmallVectorImpl<uint32_t> &indices) const;

  /// Assigns to all the fields' associated counter from the srcFields.
  /// Returns true if there are no errors during the assignment.
  ///
  /// This first overload is for assigning a struct as whole: we need to update
  /// all the associated counters in the target struct. This second overload is
  /// for assigning a potentially nested struct.
  bool assign(const CounterVarFields &srcFields, SpirvBuilder &,
              SpirvContext &) const;
  bool assign(const CounterVarFields &srcFields,
              const llvm::SmallVector<uint32_t, 4> &dstPrefix,
              const llvm::SmallVector<uint32_t, 4> &srcPrefix, SpirvBuilder &,
              SpirvContext &) const;

private:
  struct IndexCounterPair {
    IndexCounterPair(const llvm::SmallVector<uint32_t, 4> &idx,
                     SpirvVariable *counter)
        : indices(idx), counterVar(counter, true) {}

    llvm::SmallVector<uint32_t, 4> indices; ///< Index vector
    CounterIdAliasPair counterVar;          ///< Counter variable information
  };

  llvm::SmallVector<IndexCounterPair, 4> fields;
};

/// \brief The class containing mappings from Clang frontend Decls to their
/// corresponding SPIR-V <result-id>s.
///
/// All symbols defined in the AST should be "defined" or registered in this
/// class and have their <result-id>s queried from this class. In the process
/// of defining a Decl, the SPIR-V module builder passed into the constructor
/// will be used to generate all SPIR-V instructions required.
///
/// This class acts as a middle layer to handle the mapping between HLSL
/// semantics and Vulkan stage (builtin/input/output) variables. Such mapping
/// is required because of the semantic differences between DirectX and
/// Vulkan and the essence of HLSL as the front-end language for DirectX.
/// A normal variable attached with some semantic will be translated into a
/// single stage variable if it is of non-struct type. If it is of struct
/// type, the fields with attached semantics will need to be translated into
/// stage variables per Vulkan's requirements.
class DeclResultIdMapper {
  /// \brief An internal class to handle binding number allocation.
  class BindingSet;

public:
  inline DeclResultIdMapper(ASTContext &context, SpirvContext &spirvContext,
                            SpirvBuilder &spirvBuilder, SpirvEmitter &emitter,
                            FeatureManager &features,
                            const SpirvCodeGenOptions &spirvOptions);

  /// \brief Returns the SPIR-V builtin variable. Uses sc as default storage
  /// class.
  SpirvVariable *getBuiltinVar(spv::BuiltIn builtIn, QualType type,
                               spv::StorageClass sc, SourceLocation);

  /// \brief Returns the SPIR-V builtin variable. Tries to infer storage class
  /// from the builtin.
  SpirvVariable *getBuiltinVar(spv::BuiltIn builtIn, QualType type,
                               SourceLocation);

  /// \brief If var is a raytracing stage variable, returns its entry point,
  /// otherwise returns nullptr.
  SpirvFunction *getRayTracingStageVarEntryFunction(SpirvVariable *var);

  /// \brief Creates the stage output variables by parsing the semantics
  /// attached to the given function's parameter or return value and returns
  /// true on success. SPIR-V instructions will also be generated to update the
  /// contents of the output variables by extracting sub-values from the given
  /// storedValue. forPCF should be set to true for handling decls in patch
  /// constant function.
  ///
  /// Note that the control point stage output variable of HS should be created
  /// by the other overload.
  bool createStageOutputVar(const DeclaratorDecl *decl,
                            SpirvInstruction *storedValue, bool forPCF);
  /// \brief Overload for handling HS control point stage ouput variable.
  bool createStageOutputVar(const DeclaratorDecl *decl, uint32_t arraySize,
                            SpirvInstruction *invocationId,
                            SpirvInstruction *storedValue);

  /// \brief Creates the stage input variables by parsing the semantics attached
  /// to the given function's parameter and returns true on success. SPIR-V
  /// instructions will also be generated to load the contents from the input
  /// variables and composite them into one and write to *loadedValue. forPCF
  /// should be set to true for handling decls in patch constant function.
  bool createStageInputVar(const ParmVarDecl *paramDecl,
                           SpirvInstruction **loadedValue, bool forPCF);

  /// \brief Creates stage variables for raytracing.
  SpirvVariable *createRayTracingNVStageVar(spv::StorageClass sc,
                                            const VarDecl *decl);
  SpirvVariable *createRayTracingNVStageVar(spv::StorageClass sc, QualType type,
                                            std::string name, bool isPrecise,
                                            bool isNointerp);

  /// \brief Creates the taskNV stage variables for payload struct variable
  /// and returns true on success. SPIR-V instructions will also be generated
  /// to load/store the contents from/to *value. payloadMemOffset is incremented
  /// based on payload struct member size, alignment and offset, and SPIR-V
  /// decorations PerTaskNV and Offset are assigned to each member.
  bool createPayloadStageVars(const hlsl::SigPoint *sigPoint,
                              spv::StorageClass sc, const NamedDecl *decl,
                              bool asInput, QualType type,
                              const llvm::StringRef namePrefix,
                              SpirvInstruction **value,
                              uint32_t payloadMemOffset = 0);

  /// \brief Creates a function-scope paramter in the current function and
  /// returns its instruction. dbgArgNumber is used to specify the argument
  /// number of param among function parameters, which will be used for the
  /// debug information. Note that dbgArgNumber for the first function
  /// parameter must have "1", not "0", which is what Clang generates for
  /// LLVM debug metadata.
  SpirvFunctionParameter *createFnParam(const ParmVarDecl *param,
                                        uint32_t dbgArgNumber = 0);

  /// \brief Creates the counter variable associated with the given param.
  /// This is meant to be used for forward-declared functions and this objects
  /// of methods.
  ///
  /// Note: legalization specific code
  inline void createFnParamCounterVar(const VarDecl *param);

  /// \brief Creates a function-scope variable in the current function and
  /// returns its instruction.
  SpirvVariable *createFnVar(const VarDecl *var,
                             llvm::Optional<SpirvInstruction *> init);

  /// \brief Creates a file-scope variable and returns its instruction.
  SpirvVariable *createFileVar(const VarDecl *var,
                               llvm::Optional<SpirvInstruction *> init);

  /// Creates a global variable for resource heaps containing elements of type
  /// |type|.
  SpirvVariable *createResourceHeap(const VarDecl *var, QualType type);

  /// \brief Creates an external-visible variable and returns its instruction.
  SpirvVariable *createExternVar(const VarDecl *var);

  /// \brief Creates an external-visible variable of type |type| and returns its
  /// instruction.
  SpirvVariable *createExternVar(const VarDecl *var, QualType type);

  /// \brief Returns an OpString instruction that represents the given VarDecl.
  /// VarDecl must be a variable of string type.
  ///
  /// This function inspects the VarDecl for an initialization expression. If
  /// initialization expression is not found, it will emit an error because the
  /// variable cannot be deduced to an OpString literal, and string variables do
  /// not exist in SPIR-V.
  ///
  /// Note: HLSL has the 'string' type which can be used for rare purposes such
  /// as printf (SPIR-V's DebugPrintf). SPIR-V does not have a 'char' or
  /// 'string' type, and therefore any variable of such type is never created.
  /// The string literal is evaluated when needed and an OpString is generated
  /// for it.
  SpirvInstruction *createOrUpdateStringVar(const VarDecl *);

  /// \brief Returns an instruction that represents the given VarDecl.
  /// VarDecl must be a variable of vk::ext_result_id<Type> type.
  ///
  /// This function inspects the VarDecl for an initialization expression. If
  /// initialization expression is not found, it will emit an error because the
  /// variable with result id requires an initialization.
  SpirvInstruction *createResultId(const VarDecl *var);

  /// \brief Creates an Enum constant.
  void createEnumConstant(const EnumConstantDecl *decl);

  /// \brief Creates a cbuffer/tbuffer from the given decl.
  ///
  /// In the AST, cbuffer/tbuffer is represented as a HLSLBufferDecl, which is
  /// a DeclContext, and all fields in the buffer are represented as VarDecls.
  /// We cannot do the normal translation path, which will translate a field
  /// into a standalone variable. We need to create a single SPIR-V variable
  /// for the whole buffer. When we refer to the field VarDecl later, we need
  /// to do an extra OpAccessChain to get its pointer from the SPIR-V variable
  /// standing for the whole buffer.
  SpirvVariable *createCTBuffer(const HLSLBufferDecl *decl);

  /// \brief Creates a PushConstant block from the given decl.
  SpirvVariable *createPushConstant(const VarDecl *decl);

  /// \brief Creates the $Globals cbuffer.
  void createGlobalsCBuffer(const VarDecl *var);

  /// \brief Returns the suitable type for the given decl, considering the
  /// given decl could possibly be created as an alias variable. If true, a
  /// pointer-to-the-value type will be returned, otherwise, just return the
  /// normal value type. For an alias variable having a associated counter, the
  /// counter variable will also be emitted.
  ///
  /// If the type is for an alias variable, writes true to *shouldBeAlias and
  /// writes storage class, layout rule, and valTypeId to *info.
  ///
  /// Note: legalization specific code
  QualType
  getTypeAndCreateCounterForPotentialAliasVar(const DeclaratorDecl *var,
                                              bool *shouldBeAlias = nullptr);

  /// \brief Sets the entry function.
  void setEntryFunction(SpirvFunction *fn) { entryFunction = fn; }

  /// \brief If the given decl is an implicit VarDecl that evaluates to a
  /// constant, it evaluates the constant and registers the resulting SPIR-V
  /// instruction in the astDecls map. Otherwise returns without doing anything.
  ///
  /// Note: There are many cases where the front-end might create such implicit
  /// VarDecls (such as some ray tracing enums).
  void tryToCreateImplicitConstVar(const ValueDecl *);

  /// \brief Creates instructions to copy output stage variables defined by
  /// outputPatchDecl to hullMainOutputPatch that is a variable for the
  /// OutputPatch argument passing. outputControlPointType is the template
  /// parameter type of OutputPatch and numOutputControlPoints is the number of
  /// output control points.
  void copyHullOutStageVarsToOutputPatch(SpirvInstruction *hullMainOutputPatch,
                                         const ParmVarDecl *outputPatchDecl,
                                         QualType outputControlPointType,
                                         uint32_t numOutputControlPoints);

  /// \brief An enum class for representing what the DeclContext is used for
  enum class ContextUsageKind {
    CBuffer,
    TBuffer,
    PushConstant,
    Globals,
    ShaderRecordBufferNV,
    ShaderRecordBufferKHR
  };

  /// Raytracing specific functions
  /// \brief Creates a ShaderRecordBufferEXT or ShaderRecordBufferNV block from
  /// the given decl.
  SpirvVariable *createShaderRecordBuffer(const VarDecl *decl,
                                          ContextUsageKind kind);
  SpirvVariable *createShaderRecordBuffer(const HLSLBufferDecl *decl,
                                          ContextUsageKind kind);

  // Records the TypedefDecl or TypeAliasDecl of vk::SpirvType so that any
  // required capabilities and extensions can be added if the type is used.
  void recordsSpirvTypeAlias(const Decl *decl);

private:
  /// The struct containing SPIR-V information of a AST Decl.
  struct DeclSpirvInfo {
    /// Default constructor to satisfy DenseMap
    DeclSpirvInfo() : instr(nullptr), indexInCTBuffer(-1) {}

    DeclSpirvInfo(SpirvInstruction *instr_, int index = -1)
        : instr(instr_), indexInCTBuffer(index) {}

    /// Implicit conversion to SpirvInstruction*.
    operator SpirvInstruction *() const { return instr; }

    SpirvInstruction *instr;
    /// Value >= 0 means that this decl is a VarDecl inside a cbuffer/tbuffer
    /// and this is the index; value < 0 means this is just a standalone decl.
    int indexInCTBuffer;
  };

  /// The struct containing the data needed to create the input and output
  /// variables for the decl.
  struct StageVarDataBundle {
    // The declaration of the variable for which we need to create the stage
    // variables.
    const NamedDecl *decl;

    // The HLSL semantic to apply to the variable. Note that this could be
    // different than the semantic attached to decl because it could inherit
    // the semantic from the parent declaration if this declaration is a member.
    SemanticInfo *semantic;

    // True if the variable is not suppose to be interpolated. Note that we
    // cannot just look at decl to determine this because the attribute might
    // have been applied to a parent declaration.
    bool asNoInterp;

    // The sigPoint is the shader stage that this variable should be added to,
    // and whether it is an input or output.
    const hlsl::SigPoint *sigPoint;

    // The type to use for the new variable. There are cases where the type
    // might be different. See the call sites for createStageVars.
    QualType type;

    // If the shader stage for the variable is HS, DS, or GS, the SPIR-V
    // requires that the stage variable is an array of type. The arraySize gives
    // the size for that array.
    uint32_t arraySize;

    // A prefix to use for the name of the variable.
    llvm::StringRef namePrefix;

    // If arraySize is not zero, invocationId gives the index to used when
    // generating a write to the stage variable.
    llvm::Optional<SpirvInstruction *> invocationId;
  };

  /// \brief Returns the SPIR-V information for the given decl.
  /// Returns nullptr if no such decl was previously registered.
  const DeclSpirvInfo *getDeclSpirvInfo(const ValueDecl *decl) const;

  /// \brief Creates DeclSpirvInfo using the given instr and index. It creates a
  /// clone variable if it is CTBuffer including matrix 1xN with FXC memory
  /// layout.
  DeclSpirvInfo createDeclSpirvInfo(SpirvInstruction *instr,
                                    int index = -1) const {
    if (auto *clone = spvBuilder.initializeCloneVarForFxcCTBuffer(instr))
      instr = clone;
    return DeclSpirvInfo(instr, index);
  }

public:
  /// \brief Returns the information for the given decl.
  ///
  /// This method will panic if the given decl is not registered.
  SpirvInstruction *getDeclEvalInfo(const ValueDecl *decl, SourceLocation loc,
                                    SourceRange range = {});

  /// \brief Returns the instruction pointer for the given function if already
  /// registered; otherwise, treats the given function as a normal decl and
  /// returns a newly created instruction for it.
  SpirvFunction *getOrRegisterFn(const FunctionDecl *fn);

  /// Registers that the given decl should be translated into the given spec
  /// constant.
  void registerSpecConstant(const VarDecl *decl,
                            SpirvInstruction *specConstant);

  /// \brief Returns the associated counter's (instr-ptr, is-alias-or-not)
  /// pair for the given {RW|Append|Consume}StructuredBuffer variable.
  /// If indices is not nullptr, walks trhough the fields of the decl, expected
  /// to be of struct type, using the indices to find the field. Returns nullptr
  /// if the given decl has no associated counter variable created.
  const CounterIdAliasPair *getCounterIdAliasPair(
      const DeclaratorDecl *decl,
      const llvm::SmallVector<uint32_t, 4> *indices = nullptr);

  /// \brief Returns the associated counter's (instr-ptr, is-alias-or-not)
  /// pair for the given {RW|Append|Consume}StructuredBuffer variable. Creates
  /// counter for RW buffer if not already created.
  const CounterIdAliasPair *
  createOrGetCounterIdAliasPair(const DeclaratorDecl *decl);

  /// \brief Returns all the associated counters for the given decl. The decl is
  /// expected to be a struct containing alias RW/Append/Consume structured
  /// buffers. Returns nullptr if it does not.
  const CounterVarFields *getCounterVarFields(const DeclaratorDecl *decl);

  /// \brief Returns all defined stage (builtin/input/ouput) variables for the
  /// entry point function entryPoint in this mapper.
  std::vector<SpirvVariable *>
  collectStageVars(SpirvFunction *entryPoint) const;

  /// \brief Writes out the contents in the function parameter for the GS
  /// stream output to the corresponding stage output variables in a recursive
  /// manner. Returns true on success, false if errors occur.
  ///
  /// decl is the Decl with semantic string attached and will be used to find
  /// the stage output variable to write to, value is the  SPIR-V variable to
  /// read data from.
  ///
  /// This method is specially for writing back per-vertex data at the time of
  /// OpEmitVertex in GS.
  bool writeBackOutputStream(const NamedDecl *decl, QualType type,
                             SpirvInstruction *value, SourceRange range = {});

  /// \brief Reciprocates to get the multiplicative inverse of SV_Position.w
  /// if requested.
  SpirvInstruction *invertWIfRequested(SpirvInstruction *position,
                                       SourceLocation loc);

  /// \brief Decorates all stage input and output variables with proper
  /// location and returns true on success.
  ///
  /// This method will write the location assignment into the module under
  /// construction.
  inline bool decorateStageIOLocations();

  /// \brief Decorates all resource variables with proper set and binding
  /// numbers and returns true on success.
  ///
  /// This method will write the set and binding number assignment into the
  /// module under construction.
  bool decorateResourceBindings();

  /// \brief Decorates resource variables with Coherent decoration if they
  /// are declared as globallycoherent.
  bool decorateResourceCoherent();

  /// \brief Returns whether the SPIR-V module requires SPIR-V legalization
  /// passes run to make it legal.
  bool requiresLegalization() const { return needsLegalization; }

  /// \brief Returns whether the SPIR-V module requires an optimization pass to
  /// flatten array/structure of resources.
  bool requiresFlatteningCompositeResources() const {
    return needsFlatteningCompositeResources;
  }

  /// \brief Returns the given decl's HLSL semantic information.
  static SemanticInfo getStageVarSemantic(const NamedDecl *decl);

  /// \brief Returns SPIR-V instruction for given stage var decl.
  SpirvInstruction *getStageVarInstruction(const DeclaratorDecl *decl) {
    auto *value = stageVarInstructions.lookup(decl);
    assert(value);
    return value;
  }

  SpirvVariable *getMSOutIndicesBuiltin() {
    assert(msOutIndicesBuiltin && "Variable usage before decl parsing.");
    return msOutIndicesBuiltin;
  }

  /// Decorate with spirv intrinsic attributes with lamda function variable
  /// check
  void decorateWithIntrinsicAttrs(
      const NamedDecl *decl, SpirvVariable *varInst,
      llvm::function_ref<void(VKDecorateExtAttr *)> extraFunctionForDecoAttr =
          [](VKDecorateExtAttr *) {});

  /// \brief Creates instructions to load the value of output stage variable
  /// defined by outputPatchDecl and store it to ptr. Since the output stage
  /// variable for OutputPatch is an array whose number of elements is the
  /// number of output control points, we need ctrlPointID to indicate which
  /// output control point is the target for copy. outputControlPointType is the
  /// template parameter type of OutputPatch.
  void storeOutStageVarsToStorage(const DeclaratorDecl *outputPatchDecl,
                                  SpirvConstant *ctrlPointID,
                                  QualType outputControlPointType,
                                  SpirvInstruction *ptr);

  spv::ExecutionMode getInterlockExecutionMode();

  /// Records any Spir-V capabilities and extensions for the given type so
  /// they will be added to the SPIR-V module. The capabilities and extension
  /// required for the type will be sourced from the decls that were recorded
  /// using `recordSpirvTypeAlias`.
  void registerCapabilitiesAndExtensionsForType(const TypedefType *type);

private:
  /// \brief Wrapper method to create a fatal error message and report it
  /// in the diagnostic engine associated with this consumer.
  template <unsigned N>
  DiagnosticBuilder emitFatalError(const char (&message)[N],
                                   SourceLocation loc) {
    const auto diagId =
        diags.getCustomDiagID(clang::DiagnosticsEngine::Fatal, message);
    return diags.Report(loc, diagId);
  }

  /// \brief Wrapper method to create an error message and report it
  /// in the diagnostic engine associated with this consumer.
  template <unsigned N>
  DiagnosticBuilder emitError(const char (&message)[N], SourceLocation loc) {
    const auto diagId =
        diags.getCustomDiagID(clang::DiagnosticsEngine::Error, message);
    return diags.Report(loc, diagId);
  }

  /// \brief Wrapper method to create a warning message and report it
  /// in the diagnostic engine associated with this consumer.
  template <unsigned N>
  DiagnosticBuilder emitWarning(const char (&message)[N], SourceLocation loc) {
    const auto diagId =
        diags.getCustomDiagID(clang::DiagnosticsEngine::Warning, message);
    return diags.Report(loc, diagId);
  }

  /// \brief Wrapper method to create a note message and report it
  /// in the diagnostic engine associated with this consumer.
  template <unsigned N>
  DiagnosticBuilder emitNote(const char (&message)[N], SourceLocation loc) {
    const auto diagId =
        diags.getCustomDiagID(clang::DiagnosticsEngine::Note, message);
    return diags.Report(loc, diagId);
  }

  /// \brief Checks whether some semantic is used more than once and returns
  /// true if no such cases. Returns false otherwise.
  bool checkSemanticDuplication(bool forInput);

  /// \brief Checks whether some location/index is used more than once and
  /// returns true if no such cases. Returns false otherwise.
  bool isDuplicatedStageVarLocation(
      llvm::DenseSet<StageVariableLocationInfo, StageVariableLocationInfo>
          *stageVariableLocationInfo,
      const StageVar &var, uint32_t location, uint32_t index);

  /// \brief Decorates vars with locations assigned by nextLocs.
  /// stageVariableLocationInfo will be used to check the duplication of stage
  /// variable locations.
  bool assignLocations(
      const std::vector<const StageVar *> &vars,
      llvm::function_ref<uint32_t(uint32_t)> nextLocs,
      llvm::DenseSet<StageVariableLocationInfo, StageVariableLocationInfo>
          *stageVariableLocationInfo);

  /// \brief Get a valid BindingInfo. If no user provided binding info is given,
  /// allocates a new binding and returns it.
  static SpirvCodeGenOptions::BindingInfo getBindingInfo(
      BindingSet &bindingSet,
      const std::optional<SpirvCodeGenOptions::BindingInfo> &userProvidedInfo);

  /// \brief Decorates used Resource/Sampler descriptor heaps with the correct
  /// binding/set decorations.
  void decorateResourceHeapsBindings(BindingSet &bindingSet);

  /// \brief Returns a map that divides all of the shader stage variables into
  /// separate vectors for each entry point.
  llvm::DenseMap<const SpirvFunction *, SmallVector<StageVar, 8>>
  getStageVarsPerFunction();

  /// \brief Decorates all stage variables in `functionStageVars` with proper
  /// location and returns true on success.
  ///
  /// It is assumed that all variables in `functionStageVars` belong to the same
  /// entry point.
  ///
  /// This method will write the location assignment into the module under
  /// construction.
  bool finalizeStageIOLocationsForASingleEntryPoint(
      bool forInput, ArrayRef<StageVar> functionStageVars);

  /// \brief Decorates all stage input (if forInput is true) or output (if
  /// forInput is false) variables with proper location and returns true on
  /// success.
  ///
  /// This method will write the location assignment into the module under
  /// construction.
  bool finalizeStageIOLocations(bool forInput);

  /// Creates a variable of struct type with explicit layout decorations.
  /// The sub-Decls in the given DeclContext will be treated as the struct
  /// fields. The struct type will be named as typeName, and the variable
  /// will be named as varName.
  ///
  /// This method should only be used for cbuffers/ContantBuffers, tbuffers/
  /// TextureBuffers, and PushConstants. usageKind must be set properly
  /// depending on the usage kind.
  ///
  /// If arraySize is 0, the variable will be created as a struct ; if arraySize
  /// is > 0, the variable will be created as an array; if arraySize is -1, the
  /// variable will be created as a runtime array.
  ///
  /// Panics if the DeclContext is neither HLSLBufferDecl or RecordDecl.
  SpirvVariable *createStructOrStructArrayVarOfExplicitLayout(
      const DeclContext *decl, llvm::ArrayRef<int> arraySize,
      ContextUsageKind usageKind, llvm::StringRef typeName,
      llvm::StringRef varName);

  /// Creates a variable of struct type with explicit layout decorations.
  /// The sub-Decls in the given DeclContext will be treated as the struct
  /// fields. The struct type will be named as typeName, and the variable
  /// will be named as varName.
  ///
  /// This method should only be used for cbuffers/ContantBuffers, tbuffers/
  /// TextureBuffers, and PushConstants. usageKind must be set properly
  /// depending on the usage kind.
  ///
  /// If arraySize is 0, the variable will be created as a struct ; if arraySize
  /// is > 0, the variable will be created as an array; if arraySize is -1, the
  /// variable will be created as a runtime array.
  ///
  /// Panics if the DeclContext is neither HLSLBufferDecl or RecordDecl.
  SpirvVariable *createStructOrStructArrayVarOfExplicitLayout(
      const DeclContext *decl, int arraySize, ContextUsageKind usageKind,
      llvm::StringRef typeName, llvm::StringRef varName);

  /// Creates all of the stage variables that must be generated for the given
  /// stage variable data. Returns true on success.
  ///
  /// stageVarData: See the definition of StageVarDataBundle to see how that
  /// data is used.
  ///
  /// asInput: True if the stage variable is an input.
  ///
  /// TODO(s-perron): a variable that is an input or an output depending on
  /// value of a flag is very hard to read. This function should be split up
  /// and flag variables removed.
  ///
  /// [in/out] value: If `asInput` is true, this is an
  /// output, and will be an instruction that loads the stage variable. If
  /// `asInput` is false, then it is an input to createStageVars, and contains
  /// the value to be stored in the new stage variable.
  ///
  /// noWriteBack: If true, the newly created stage variable will not be written
  /// to.
  bool createStageVars(StageVarDataBundle &stageVarData, bool asInput,
                       SpirvInstruction **value, bool noWriteBack);

  // Creates a variable to represent the output variable, which must be a
  // structure. If `noWriteBack` is false, then `value` will be written to the
  // new variable. Returns true if successful.
  //
  // stageVarData: The data needed to create the stage variable.
  //
  // noWriteBack: A flag to indicate if the variable should be written or not.
  //
  // value: The value to be written to the newly create variable.
  bool createStructOutputVar(const StageVarDataBundle &stageVarData,
                             SpirvInstruction *value, bool noWriteBack);

  // Creates a variable to represent the input variable, which must be a
  // structure. The value is loaded and the instruction with the final value is
  // return.
  //
  // stageVarData: The data needed to create the stage variable.
  //
  // noWriteBack: A flag to indicate if the variable should be written or not.
  SpirvInstruction *createStructInputVar(const StageVarDataBundle &stageVarData,
                                         bool noWriteBack);

  // Store `value` to the shader output variable `varInstr`. Since the type
  // could be different, stageVarData is used to know how to convert `value`
  // into the correct type for `varInstr`.
  //
  // varInstr: the output variable that corresponds to `stageVarData`. It must
  // not be a struct.
  //
  // value: The value to be written to the create variable.
  //
  // stageVarData: The data that was used to create `varInstr`.
  void storeToShaderOutputVariable(SpirvVariable *varInstr,
                                   SpirvInstruction *value,
                                   const StageVarDataBundle &stageVarData);

  // Loads shader input variable `varInstr`, and modifies the value to match the
  // type in stageVarData. The struct stageVarData is used to know how to
  // convert the value loaded from `varInstr` into the correct type.
  //
  // varInstr: the input variable that corresponds to `stageVarData`. It must
  // not be a struct.
  //
  // stageVarData: The data that was used to create `varInstr`.
  SpirvInstruction *
  loadShaderInputVariable(SpirvVariable *varInstr,
                          const StageVarDataBundle &stageVarData);

  // Creates a function scope variable to represent the "SV_InstanceID"
  // semantic, which it not immediately available in SPIR-V. Its value will be
  // set by subtracting the values of the given InstanceIndex and base instance
  // variables.
  //
  // instanceIndexVar: The SPIR-V input variable that decorated with
  // InstanceIndex.
  //
  // baseInstanceVar: The SPIR-V input variable that is decorated with
  // BaseInstance.
  SpirvVariable *getInstanceIdFromIndexAndBase(SpirvVariable *instanceIndexVar,
                                               SpirvVariable *baseInstanceVar);

  // Creates a function scope variable to represent the "SV_VertexID"
  // semantic, which is not immediately available in SPIR-V. Its value will be
  // set by subtracting the values of the given InstanceIndex and base instance
  // variables.
  //
  // vertexIndexVar: The SPIR-V input variable decorated with
  // vertexIndex.
  //
  // baseVertexVar: The SPIR-V input variable decorated with
  // BaseVertex.
  SpirvVariable *getVertexIdFromIndexAndBase(SpirvVariable *vertexIndexVar,
                                             SpirvVariable *baseVertexVar);

  // Creates and returns a variable that is the BaseInstance builtin input. The
  // variable is also added to the list of stage variable `this->stageVars`. Its
  // type will be a 32-bit integer.
  //
  // sigPoint: the signature point identifying which shader stage the variable
  // will be used in.
  //
  // type: The type to use for the new variable. Must be int or unsigned int.
  SpirvVariable *getBaseInstanceVariable(const hlsl::SigPoint *sigPoint,
                                         QualType type);

  // Creates and returns a variable that is the BaseVertex builtin input. The
  // variable is also added to the list of stage variable `this->stageVars`. Its
  // type will be a 32-bit integer.
  //
  // sigPoint: the signature point identifying which shader stage the variable
  // will be used in.
  //
  // type: The type to use for the new variable. Must be int or unsigned int.
  SpirvVariable *getBaseVertexVariable(const hlsl::SigPoint *sigPoint,
                                       QualType type);

  // Creates and return a new interface variable from the information provided.
  // The new variable with be add to `this->StageVars`.
  //
  //
  // stageVarData: the data needed to create the interface variable. See the
  // declaration of StageVarDataBundle for the details.
  SpirvVariable *
  createSpirvInterfaceVariable(const StageVarDataBundle &stageVarData);

  // Returns the type that the SPIR-V input or output variable must have to
  // correspond to a variable with the given information.
  //
  // stageVarData: the data needed to create the interface variable. See the
  // declaration of StageVarDataBundle for the details.
  QualType getTypeForSpirvStageVariable(const StageVarDataBundle &stageVarData);

  // Returns true if all of the stage variable data is consistent with a valid
  // shader stage variable. Issues an error and returns false otherwise.
  bool validateShaderStageVar(const StageVarDataBundle &stageVarData);

  /// Returns true if all vk:: attributes usages are valid.
  bool validateVKAttributes(const NamedDecl *decl);

  /// Returns true if all vk::builtin usages are valid.
  bool validateVKBuiltins(const StageVarDataBundle &stageVarData);

  // Returns true if the type in stageVarData is compatible with the rest of the
  // data. Issues an error and returns false otherwise.
  bool validateShaderStageVarType(const StageVarDataBundle &stageVarData);

  // Returns true if the semantic is consistent wit the rest of the given data.
  bool isValidSemanticInShaderModel(const StageVarDataBundle &stageVarData);

  /// Creates the SPIR-V variable instruction for the given StageVar and returns
  /// the instruction. Also sets whether the StageVar is a SPIR-V builtin and
  /// its storage class accordingly. name will be used as the debug name when
  /// creating a stage input/output variable.
  SpirvVariable *createSpirvStageVar(StageVar *, const NamedDecl *decl,
                                     const llvm::StringRef name,
                                     SourceLocation);

  /// Methods for creating counter variables associated with the given decl.

  /// Creates assoicated counter variables for all AssocCounter cases (see the
  /// comment of CounterVarFields).
  void createCounterVarForDecl(const DeclaratorDecl *decl);

  /// Creates the associated counter variable for final RW/Append/Consume
  /// structured buffer. Handles AssocCounter#1 and AssocCounter#2 (see the
  /// comment of CounterVarFields).
  ///
  /// declId is the SPIR-V instruction for the given decl. It should be non-zero
  /// for non-alias buffers.
  ///
  /// The counter variable will be created as an alias variable (of
  /// pointer-to-pointer type in Private storage class) if isAlias is true.
  ///
  /// Note: isAlias - legalization specific code
  void
  createCounterVar(const DeclaratorDecl *decl, SpirvInstruction *declInstr,
                   bool isAlias,
                   const llvm::SmallVector<uint32_t, 4> *indices = nullptr);
  /// Creates all assoicated counter variables by recursively visiting decl's
  /// fields. Handles AssocCounter#3 and AssocCounter#4 (see the comment of
  /// CounterVarFields).
  inline void createFieldCounterVars(const DeclaratorDecl *decl);
  void createFieldCounterVars(const DeclaratorDecl *rootDecl,
                              const DeclaratorDecl *decl,
                              llvm::SmallVector<uint32_t, 4> *indices);

  /// Decorates varInstr of the given asType with proper interpolation modes
  /// considering the attributes on the given decl.
  void decorateInterpolationMode(const NamedDecl *decl, QualType asType,
                                 SpirvVariable *varInstr,
                                 const SemanticInfo semanticInfo);

  /// Returns the proper SPIR-V storage class (Input or Output) for the given
  /// SigPoint.
  spv::StorageClass getStorageClassForSigPoint(const hlsl::SigPoint *);

  /// Returns true if the given SPIR-V stage variable has Input storage class.
  inline bool isInputStorageClass(const StageVar &v);

  /// Creates DebugGlobalVariable and returns it if rich debug information
  /// generation is enabled. Otherwise, returns nullptr.
  SpirvDebugGlobalVariable *createDebugGlobalVariable(SpirvVariable *var,
                                                      const QualType &type,
                                                      const SourceLocation &loc,
                                                      const StringRef &name);

  /// Determines the register type for a resource that does not have an
  /// explicit register() declaration.  Returns true if it is able to
  /// determine the register type and will set |*registerTypeOut| to
  /// 'u', 's', 'b', or 't'. Assumes |registerTypeOut| to be non-nullptr.
  ///
  /// Uses the following mapping of HLSL types to register spaces:
  /// t - for shader resource views (SRV)
  ///    TEXTURE1D
  ///    TEXTURE1DARRAY
  ///    TEXTURE2D
  ///    TEXTURE2DARRAY
  ///    TEXTURE3D
  ///    TEXTURECUBE
  ///    TEXTURECUBEARRAY
  ///    TEXTURE2DMS
  ///    TEXTURE2DMSARRAY
  ///    STRUCTUREDBUFFER
  ///    BYTEADDRESSBUFFER
  ///    BUFFER
  ///    TBUFFER
  ///
  /// s - for samplers
  ///    SAMPLER
  ///    SAMPLER1D
  ///    SAMPLER2D
  ///    SAMPLER3D
  ///    SAMPLERCUBE
  ///    SAMPLERSTATE
  ///    SAMPLERCOMPARISONSTATE
  ///
  /// u - for unordered access views (UAV)
  ///    RWBYTEADDRESSBUFFER
  ///    RWSTRUCTUREDBUFFER
  ///    APPENDSTRUCTUREDBUFFER
  ///    CONSUMESTRUCTUREDBUFFER
  ///    RWBUFFER
  ///    RWTEXTURE1D
  ///    RWTEXTURE1DARRAY
  ///    RWTEXTURE2D
  ///    RWTEXTURE2DARRAY
  ///    RWTEXTURE3D
  ///
  /// b - for constant buffer views (CBV)
  ///    CBUFFER
  ///    CONSTANTBUFFER
  bool getImplicitRegisterType(const ResourceVar &var,
                               char *registerTypeOut) const;

  /// \brief Decorates stage variable with spirv intrinsic attributes. If
  /// it is BuiltIn or Location decoration, sets locOrBuiltinDecorateAttr
  /// of stageVar as true.
  void decorateStageVarWithIntrinsicAttrs(const NamedDecl *decl,
                                          StageVar *stageVar,
                                          SpirvVariable *varInst);

  /// \brief Records which execution mode should be used for rasterizer order
  /// views.
  void setInterlockExecutionMode(spv::ExecutionMode mode);

  /// \brief Add |varInstr| to |astDecls| for every Decl for the variable |var|.
  /// It is possible for a variable to have multiple declarations, and all of
  /// them should be associated with the same variable.
  void registerVariableForDecl(const VarDecl *var, SpirvInstruction *varInstr);

  /// \brief Add |spirvInfo| to |astDecls| for every Decl for the variable
  /// |var|. It is possible for a variable to have multiple declarations, and
  /// all of them should be associated with the same variable.
  void registerVariableForDecl(const VarDecl *var, DeclSpirvInfo spirvInfo);

private:
  SpirvBuilder &spvBuilder;
  SpirvEmitter &theEmitter;
  FeatureManager &featureManager;
  const SpirvCodeGenOptions &spirvOptions;
  ASTContext &astContext;
  SpirvContext &spvContext;
  DiagnosticsEngine &diags;
  SpirvFunction *entryFunction;

  /// Mapping of all Clang AST decls to their instruction pointers.
  llvm::DenseMap<const ValueDecl *, DeclSpirvInfo> astDecls;
  llvm::DenseMap<const ValueDecl *, SpirvFunction *> astFunctionDecls;

  /// Vector of all defined stage variables.
  llvm::SmallVector<StageVar, 8> stageVars;
  /// Mapping from Clang AST decls to the corresponding stage variables.
  /// This field is only used by GS for manually emitting vertices, when
  /// we need to query the output stage variables involved in writing back. For
  /// other cases, stage variable reading and writing is done at the time of
  /// creating that stage variable, so that we don't need to query them again
  /// for reading and writing.
  llvm::DenseMap<const ValueDecl *, SpirvVariable *> stageVarInstructions;

  /// Special case for the Indices builtin:
  /// - this builtin has a different layout in HLSL & SPIR-V, meaning it
  /// requires
  ///   the same kind of handling as classic stageVarInstructions:
  ///   -> load into a HLSL compatible tmp
  ///   -> write back into the SPIR-V compatible layout.
  /// - but the builtin is shared across invocations (not only lanes).
  ///   -> we must only write/read from the indices requested by the user.
  /// - the variable can be passed to other functions as a out param
  ///   -> we cannot copy-in/copy-out because shared across invocations.
  ///   -> we cannot pass a simple pointer: layout differences between
  ///   HLSL/SPIR-V.
  ///
  /// All this means we must keep track of the builtin, and each assignment to
  /// this will have to handle the layout differences. The easiest solution is
  /// to keep this builtin global to the module if present.
  SpirvVariable *msOutIndicesBuiltin = nullptr;

  /// Vector of all defined resource variables.
  llvm::SmallVector<ResourceVar, 8> resourceVars;
  /// Mapping from {RW|Append|Consume}StructuredBuffers to their
  /// counter variables' (instr-ptr, is-alias-or-not) pairs
  ///
  /// conterVars holds entities of AssocCounter#1, fieldCounterVars holds
  /// entities of the rest.
  llvm::DenseMap<const DeclaratorDecl *, CounterIdAliasPair> counterVars;
  llvm::DenseMap<const DeclaratorDecl *, CounterVarFields> fieldCounterVars;

  /// Mapping from clang declarator to SPIR-V declaration instruction.
  /// This is used to defer creation of counter for RWStructuredBuffer
  /// until a Increment/DecrementCounter method is called on it.
  llvm::DenseMap<const DeclaratorDecl *, SpirvInstruction *> declRWSBuffers;

  /// The execution mode to use for rasterizer ordered views. Should be set to
  /// PixelInterlockOrderedEXT (default), SampleInterlockOrderedEXT, or
  /// ShadingRateInterlockOrderedEXT. This will be set based on which semantics
  /// are present in input variables, and will be used to determine which
  /// execution mode to attach to the entry point if it uses rasterizer ordered
  /// views.
  llvm::Optional<spv::ExecutionMode> interlockExecutionMode;

  /// The SPIR-V builtin variables accessed by WaveGetLaneCount(),
  /// WaveGetLaneIndex() and ray tracing builtins.
  ///
  /// These are the only few cases where SPIR-V builtin variables are accessed
  /// using HLSL intrinsic function calls. All other builtin variables are
  /// accessed using stage IO variables.
  llvm::DenseMap<uint32_t, SpirvVariable *> builtinToVarMap;

  /// Maps from a raytracing stage variable to the entry point that variable is
  /// for.
  llvm::DenseMap<SpirvVariable *, SpirvFunction *>
      rayTracingStageVarToEntryPoints;

  /// Whether the translated SPIR-V binary needs legalization.
  ///
  /// The following cases will require legalization:
  ///
  /// 1. Opaque types (textures, samplers) within structs
  /// 2. Structured buffer aliasing
  /// 3. Using SPIR-V instructions not allowed in the currect shader stage
  ///
  /// This covers the second case:
  ///
  /// When we have a kind of structured or byte buffer, meaning one of the
  /// following
  ///
  /// * StructuredBuffer
  /// * RWStructuredBuffer
  /// * AppendStructuredBuffer
  /// * ConsumeStructuredBuffer
  /// * ByteAddressStructuredBuffer
  /// * RWByteAddressStructuredBuffer
  ///
  /// and assigning to them (using operator=, passing in as function parameter,
  /// returning as function return), we need legalization.
  ///
  /// All variable definitions (including static/non-static local/global
  /// variables, function parameters/returns) will gain another level of
  /// pointerness, unless they will generate externally visible SPIR-V
  /// variables. So variables and parameters will be of pointer-to-pointer type,
  /// while function returns will be of pointer type. We adopt this mechanism to
  /// convey to the legalization passes that they are *alias* variables, and
  /// all accesses should happen to the aliased-to-variables. Loading such an
  /// alias variable will give the pointer to the aliased-to-variable, while
  /// storing into such an alias variable should write the pointer to the
  /// aliased-to-variable.
  ///
  /// Based on the above, CodeGen should take care of the following AST nodes:
  ///
  /// * Definition of alias variables: should add another level of pointers
  /// * Assigning non-alias variables to alias variables: should avoid the load
  ///   over the non-alias variables
  /// * Accessing alias variables: should load the pointer first and then
  ///   further compose access chains.
  ///
  /// Note that the associated counters bring about their own complication.
  /// We also need to apply the alias mechanism for them.
  ///
  /// If this is true, SPIRV-Tools legalization passes will be executed after
  /// the translation to legalize the generated SPIR-V binary.
  ///
  /// Note: legalization specific code
  bool needsLegalization;

  /// Whether the translated SPIR-V binary needs flattening of composite
  /// resources.
  ///
  /// If the source HLSL contains global structure of resources, we need to run
  /// an additional SPIR-V optimization pass to flatten such structures.
  bool needsFlatteningCompositeResources;

  uint32_t perspBaryCentricsIndex, noPerspBaryCentricsIndex;

  llvm::SmallVector<const TypedefNameDecl *, 4> typeAliasesWithAttributes;

public:
  /// The gl_PerVertex structs for both input and output
  GlPerVertex glPerVertex;
};

hlsl::Semantic::Kind SemanticInfo::getKind() const {
  assert(semantic);
  return semantic->GetKind();
}
bool SemanticInfo::isTarget() const {
  return semantic && semantic->GetKind() == hlsl::Semantic::Kind::Target;
}

void CounterIdAliasPair::assign(SpirvInstruction *src,
                                SpirvBuilder &builder) const {
  assert(isAlias);
  builder.createStore(counterVar, src, /* SourceLocation */ {});
}

DeclResultIdMapper::DeclResultIdMapper(ASTContext &context,
                                       SpirvContext &spirvContext,
                                       SpirvBuilder &spirvBuilder,
                                       SpirvEmitter &emitter,
                                       FeatureManager &features,
                                       const SpirvCodeGenOptions &options)
    : spvBuilder(spirvBuilder), theEmitter(emitter), featureManager(features),
      spirvOptions(options), astContext(context), spvContext(spirvContext),
      diags(context.getDiagnostics()), entryFunction(nullptr),
      needsLegalization(false), needsFlatteningCompositeResources(false),
      perspBaryCentricsIndex(2), noPerspBaryCentricsIndex(2),
      glPerVertex(context, spirvContext, spirvBuilder) {}

bool DeclResultIdMapper::decorateStageIOLocations() {
  if (spvContext.isRay() || spvContext.isAS()) {
    // No location assignment for any raytracing stage variables or
    // amplification shader variables
    return true;
  }
  // Try both input and output even if input location assignment failed
  return (int)finalizeStageIOLocations(true) &
         (int)finalizeStageIOLocations(false);
}

bool DeclResultIdMapper::isInputStorageClass(const StageVar &v) {
  return v.getStorageClass() == spv::StorageClass::Input;
}

void DeclResultIdMapper::createFnParamCounterVar(const VarDecl *param) {
  createCounterVarForDecl(param);
}

void DeclResultIdMapper::createFieldCounterVars(const DeclaratorDecl *decl) {
  llvm::SmallVector<uint32_t, 4> indices;
  createFieldCounterVars(decl, decl, &indices);
}

} // end namespace spirv
} // end namespace clang

#endif
