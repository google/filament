# Copyright (C) Microsoft Corporation. All rights reserved.
# This file is distributed under the University of Illinois Open Source License. See LICENSE.TXT for details.
###############################################################################
# DXIL information.                                                           #
###############################################################################
import os

all_stages = (
    "vertex",
    "pixel",
    "geometry",
    "compute",
    "hull",
    "domain",
    "library",
    "raygeneration",
    "intersection",
    "anyhit",
    "closesthit",
    "miss",
    "callable",
    "mesh",
    "amplification",
)

# These counters aren't collected directly from instructions,
# so they need to be added manually so they can be accessed
# with custom code in DxilCounters.cpp.
extra_counters = [
    "insts",
    "branches",
    "array_tgsm_bytes",
    "array_static_bytes",
    "array_local_bytes",
    "array_tgsm_ldst",
    "array_static_ldst",
    "array_local_ldst",
]


class db_dxil_enum_value(object):
    "A representation for a value in an enumeration type"

    def __init__(self, name, value, doc):
        self.name = name  # Name (identifier)
        self.value = value  # Numeric value
        self.doc = doc  # Documentation string
        self.category = None


class db_dxil_enum(object):
    "A representation for an enumeration type"

    def __init__(self, name, doc, valNameDocTuples=()):
        self.name = name
        self.doc = doc
        self.values = [
            db_dxil_enum_value(n, v, d) for v, n, d in valNameDocTuples
        ]  # Note transmutation
        self.is_internal = False  # whether this is never serialized

    def value_names(self):
        return [i.name for i in self.values]


class db_dxil_inst(object):
    "A representation for a DXIL instruction"

    def __init__(self, name, **kwargs):
        self.name = name  # short, unique name
        self.llvm_id = 0  # ID of LLVM instruction
        self.llvm_name = ""  # name of LLVM instruction type

        self.is_dxil_op = False  # whether this is a call into a built-in DXIL function
        self.dxil_op = ""  # name of DXIL operation
        self.dxil_opid = 0  # ID of DXIL operation
        self.dxil_class = ""  # name of the opcode class
        self.category = ""  # classification for this instruction
        self.doc = ""  # the documentation description of this instruction
        self.remarks = ""  # long-form remarks on this instruction
        self.ops = []  # the operands that this instruction takes
        self.is_allowed = True  # whether this instruction is allowed in a DXIL program
        self.oload_types = ""  # overload types if applicable
        self.fn_attr = ""  # attribute shorthands: rn=does not access memory,ro=only reads from memory,
        self.is_deriv = False  # whether this is some kind of derivative
        self.is_gradient = False  # whether this requires a gradient calculation
        self.is_feedback = False  # whether this is a sampler feedback op
        self.is_wave = False  # whether this requires in-wave, cross-lane functionality
        self.requires_uniform_inputs = False  # whether this operation requires that all of its inputs are uniform across the wave
        self.is_barrier = False  # whether this is a barrier operation
        self.shader_stages = ()  # shader stages to which this applies, empty for all.
        self.shader_model = 6, 0  # minimum shader model required
        self.inst_helper_prefix = None
        self.fully_qualified_name_prefix = "hlsl::OP::OpCode"
        for k, v in list(kwargs.items()):
            setattr(self, k, v)
        self.is_dxil_op = self.dxil_op != ""  # whether this is a DXIL operation
        self.is_reserved = self.dxil_class == "Reserved"
        self.shader_model_translated = ()  # minimum shader model required with translation by linker
        self.props = {}  # extra properties

    def __str__(self):
        return self.name

    def fully_qualified_name(self):
        return "{}::{}".format(self.fully_qualified_name_prefix, self.name)


class db_dxil_metadata(object):
    "A representation for a metadata record"

    def __init__(self, name, doc, **kwargs):
        self.name = name  # named metadata, possibly empty
        self.doc = doc  # the documentation description of this record
        for k, v in list(kwargs.items()):
            setattr(self, k, v)


class db_dxil_param(object):
    "The parameter description for a DXIL instruction"

    def __init__(self, pos, llvm_type, name, doc, **kwargs):
        self.pos = pos  # position in parameter list
        self.llvm_type = llvm_type  # llvm type name, $o for overload, $r for resource type, $cb for legacy cbuffer, $u4 for u4 struct
        self.name = name  # short, unique name
        self.doc = doc  # the documentation description of this parameter
        self.is_const = (
            False  # whether this argument requires a constant value in the IR
        )
        self.enum_name = ""  # the name of the enum type if applicable
        self.max_value = None  # the maximum value for this parameter if applicable
        for k, v in kwargs.items():
            setattr(self, k, v)


class db_dxil_pass(object):
    "The description for a DXIL optimization pass"

    def __init__(self, name, **kwargs):
        self.name = name  # name for the option, typically the command-line switch name
        self.args = []  # modifiers for the option
        self.type_name = ""  # name of the class that implements the pass
        self.doc = ""  # documentation for the pass
        self.category_lib = ""  # lib which pass belongs to
        for k, v in kwargs.items():
            setattr(self, k, v)


class db_dxil_pass_arg(object):
    "An argument to a DXIL optimization pass"

    def __init__(self, name, **kwargs):
        self.name = name  # name for the option, typically the command-line switch name
        self.ident = ""  # identifier for a parameter or global switch
        self.is_ctor_param = False  # whether this is a constructor parameter
        for k, v in kwargs.items():
            setattr(self, k, v)
        if self.is_ctor_param:
            self.is_ctor_param = True


class db_dxil_valrule(object):
    "The description of a validation rule."

    def __init__(self, name, id, **kwargs):
        self.name = name.upper()  # short, unique name, eg META.KNOWN
        self.rule_id = id  # unique identifier
        self.enum_name = name.replace(".", "")  # remove period for enum name
        self.group_name = self.name[: self.name.index(".")]  # group name, eg META
        self.rule_name = self.name[self.name.index(".") + 1 :]  # rule name, eg KNOWN
        self.definition = (
            "Check" + self.group_name + self.rule_name
        )  # function name that defines this constraint
        self.is_disabled = False  # True if the validation rule does not apply
        self.err_msg = ""  # error message associated with rule
        self.category = ""  # classification for this rule
        self.doc = ""  # the documentation description of this rule
        self.shader_stages = ()  # shader stages to which this applies, empty for all.
        self.shader_model = 6, 0  # minimum shader model required
        for k, v in list(kwargs.items()):
            setattr(self, k, v)

    def __str__(self):
        return self.name


class db_dxil(object):
    "A database of DXIL instruction data"

    def __init__(self):
        self.instr = []  # DXIL instructions
        self.enums = []  # enumeration types
        self.val_rules = []  # validation rules
        self.metadata = []  # named metadata (db_dxil_metadata)
        self.passes = []  # inventory of available passes (db_dxil_pass)
        self.name_idx = {}  # DXIL instructions by name
        self.enum_idx = {}  # enumerations by name
        self.dxil_version_info = {}
        # list of counters for instructions and dxil ops,
        # starting with extra ones specified here
        self.counters = extra_counters

        self.populate_llvm_instructions()
        self.call_instr = self.get_instr_by_llvm_name("CallInst")
        self.populate_dxil_operations()
        self.build_indices()
        self.populate_extended_docs()
        self.populate_categories_and_models()
        self.build_opcode_enum()
        self.mark_disallowed_operations()
        self.populate_metadata()
        self.populate_passes()
        self.build_valrules()
        self.build_semantics()
        self.build_indices()
        self.populate_counters()

    def __str__(self):
        return "\n".join(str(i) for i in self.instr)

    def add_enum_type(self, name, doc, valNameDocTuples):
        "Adds a new enumeration type with name/value/doc tuples"
        self.enums.append(db_dxil_enum(name, doc, valNameDocTuples))

    def build_indices(self):
        "Build a name_idx dictionary with instructions and an enum_idx dictionary with enumeration types"
        self.name_idx = {}
        for i in self.instr:
            self.name_idx[i.name] = i
        self.enum_idx = {}
        for i in self.enums:
            self.enum_idx[i.name] = i

    def build_opcode_enum(self):
        # Build enumeration from instructions
        OpCodeEnum = db_dxil_enum(
            "OpCode", "Enumeration for operations specified by DXIL"
        )
        class_dict = {}
        class_dict["LlvmInst"] = "LLVM Instructions"
        for i in self.instr:
            if i.is_dxil_op:
                v = db_dxil_enum_value(i.dxil_op, i.dxil_opid, i.doc)
                v.category = i.category
                class_dict[i.dxil_class] = i.category
                OpCodeEnum.values.append(v)
        self.enums.append(OpCodeEnum)
        OpCodeClass = db_dxil_enum(
            "OpCodeClass",
            "Groups for DXIL operations with equivalent function templates",
        )
        OpCodeClass.is_internal = True
        for k, v in iter(class_dict.items()):
            ev = db_dxil_enum_value(k, 0, None)
            ev.category = v
            OpCodeClass.values.append(ev)
        self.enums.append(OpCodeClass)

    def mark_disallowed_operations(self):
        # Disallow indirect branching, unreachable instructions and support for exception unwinding.
        for i in "IndirectBr,Invoke,Resume,LandingPad,Unreachable".split(","):
            self.name_idx[i].is_allowed = False
        for i in "UserOp1,UserOp2,VAArg".split(","):
            self.name_idx[i].is_allowed = False
        # Disallow conversions used for pointer math; GEP is used exclusively in the current model.
        for i in "PtrToInt,IntToPtr".split(","):
            self.name_idx[i].is_allowed = False
        # Barrier supersedes Fence.
        self.name_idx["Fence"].is_allowed = False

    def verify_dense(self, it, pred, name_proj):
        val = None
        for i in it:
            i_val = pred(i)
            if not val is None:
                assert val + 1 == i_val, (
                    "values in predicate are not sequential and dense, %d follows %d for %s"
                    % (i_val, val, name_proj(i))
                )
            val = i_val

    def set_op_count_for_version(self, major, minor, op_count):
        info = self.dxil_version_info.setdefault((major, minor), dict())
        info["NumOpCodes"] = op_count
        info["NumOpClasses"] = len(set([op.dxil_class for op in self.instr]))

    def populate_categories_and_models(self):
        "Populate the category and shader_stages member of instructions."
        for (
            i
        ) in "TempRegLoad,TempRegStore,MinPrecXRegLoad,MinPrecXRegStore,LoadInput,StoreOutput".split(
            ","
        ):
            self.name_idx[i].category = "Temporary, indexable, input, output registers"
        for (
            i
        ) in "FAbs,Saturate,IsNaN,IsInf,IsFinite,IsNormal,Cos,Sin,Tan,Acos,Asin,Atan,Hcos,Hsin,Htan,Exp,Frc,Log,Sqrt,Rsqrt".split(
            ","
        ):
            self.name_idx[i].category = "Unary float"
        for i in "Round_ne,Round_ni,Round_pi,Round_z".split(","):
            self.name_idx[i].category = "Unary float - rounding"
        for i in "Bfrev,Countbits,FirstbitLo,FirstbitSHi".split(","):
            self.name_idx[i].category = "Unary int"
        for i in "FirstbitHi".split(","):
            self.name_idx[i].category = "Unary uint"
        for i in "FMax,FMin".split(","):
            self.name_idx[i].category = "Binary float"
        for i in "IMax,IMin,Add,Sub,Mul,SDiv,SRem,And,Or,Xor,AShr,LShr,Shl".split(","):
            self.name_idx[i].category = "Binary int"
        for i in "UMax,UMin,UMul,UDiv,URem".split(","):
            self.name_idx[i].category = "Binary uint"
        for i in "IMul".split(","):
            self.name_idx[i].category = "Binary int with two outputs"
        for i in "UMul,UDiv".split(","):  # Rename this UDiv OpCode to UDivMod
            self.name_idx[i].category = "Binary uint with two outputs"
        for i in "UAddc,USubb".split(","):
            self.name_idx[i].category = "Binary uint with carry or borrow"
        for i in "FMad,Fma".split(","):
            self.name_idx[i].category = "Tertiary float"
        for i in "IMad,Msad,Ibfe".split(","):
            self.name_idx[i].category = "Tertiary int"
        for i in "UMad,Ubfe".split(","):
            self.name_idx[i].category = "Tertiary uint"
        for i in "Bfi".split(","):
            self.name_idx[i].category = "Quaternary"
        for i in "Dot2,Dot3,Dot4".split(","):
            self.name_idx[i].category = "Dot"
        for (
            i
        ) in "CreateHandle,CBufferLoad,CBufferLoadLegacy,TextureLoad,TextureStore,TextureStoreSample,BufferLoad,BufferStore,BufferUpdateCounter,CheckAccessFullyMapped,GetDimensions,RawBufferLoad,RawBufferStore".split(
            ","
        ):
            self.name_idx[i].category = "Resources"
        for (
            i
        ) in "Sample,SampleBias,SampleLevel,SampleGrad,SampleCmp,SampleCmpLevelZero,SampleCmpLevel,SampleCmpBias,SampleCmpGrad,Texture2DMSGetSamplePosition,RenderTargetGetSamplePosition,RenderTargetGetSampleCount".split(
            ","
        ):
            self.name_idx[i].category = "Resources - sample"
        for i in "Sample,SampleBias,SampleCmp,SampleCmpBias".split(","):
            self.name_idx[i].shader_stages = (
                "library",
                "pixel",
                "compute",
                "amplification",
                "mesh",
                "node",
            )
        for i in "RenderTargetGetSamplePosition,RenderTargetGetSampleCount".split(","):
            self.name_idx[i].shader_stages = ("pixel",)
        for i in "TextureGather,TextureGatherCmp,TextureGatherRaw".split(","):
            self.name_idx[i].category = "Resources - gather"
        for i in "AtomicBinOp,AtomicCompareExchange".split(","):
            self.name_idx[i].category = "Synchronization"
        for i in "CalculateLOD,DerivCoarseX,DerivCoarseY,DerivFineX,DerivFineY".split(
            ","
        ):
            self.name_idx[i].category = "Derivatives"
            self.name_idx[i].shader_stages = (
                "library",
                "pixel",
                "compute",
                "amplification",
                "mesh",
                "node",
            )
        for (
            i
        ) in "Discard,EvalSnapped,EvalSampleIndex,EvalCentroid,SampleIndex,Coverage,InnerCoverage,AttributeAtVertex".split(
            ","
        ):
            self.name_idx[i].category = "Pixel shader"
            self.name_idx[i].shader_stages = ("pixel",)
        for i in "ThreadId,GroupId,ThreadIdInGroup,FlattenedThreadIdInGroup".split(","):
            self.name_idx[i].category = "Compute/Mesh/Amplification/Node shader"
            self.name_idx[i].shader_stages = (
                "compute",
                "mesh",
                "amplification",
                "node",
            )
        for i in "EmitStream,CutStream,EmitThenCutStream,GSInstanceID".split(","):
            self.name_idx[i].category = "Geometry shader"
            self.name_idx[i].shader_stages = ("geometry",)
        for i in "LoadOutputControlPoint,LoadPatchConstant".split(","):
            self.name_idx[i].category = "Domain and hull shader"
            self.name_idx[i].shader_stages = ("domain", "hull")
        for i in "DomainLocation".split(","):
            self.name_idx[i].category = "Domain shader"
            self.name_idx[i].shader_stages = ("domain",)
        for i in "StorePatchConstant,OutputControlPointID".split(","):
            self.name_idx[i].category = "Hull shader"
            self.name_idx[i].shader_stages = ("hull",)
        for i in "PrimitiveID".split(","):
            self.name_idx[i].category = "Hull, Domain and Geometry shaders"
            self.name_idx[i].shader_stages = ("geometry", "domain", "hull")
        for i in "ViewID".split(","):
            self.name_idx[i].category = "Graphics shader"
            self.name_idx[i].shader_stages = (
                "vertex",
                "hull",
                "domain",
                "geometry",
                "pixel",
                "mesh",
            )
        for (
            i
        ) in "MakeDouble,SplitDouble,LegacyDoubleToFloat,LegacyDoubleToSInt32,LegacyDoubleToUInt32".split(
            ","
        ):
            self.name_idx[i].category = "Double precision"
        for i in "CycleCounterLegacy".split(","):
            self.name_idx[i].category = "Other"
        for i in "LegacyF32ToF16,LegacyF16ToF32".split(","):
            self.name_idx[i].category = "Legacy floating-point"
        for i in self.instr:
            if i.name.startswith("Wave"):
                i.category = "Wave"
                i.is_wave = True
                i.shader_stages = (
                    "library",
                    "compute",
                    "amplification",
                    "mesh",
                    "pixel",
                    "vertex",
                    "hull",
                    "domain",
                    "geometry",
                    "raygeneration",
                    "intersection",
                    "anyhit",
                    "closesthit",
                    "miss",
                    "callable",
                    "node",
                )
            elif i.name.startswith("Quad"):
                i.category = "Quad Wave Ops"
                i.is_wave = True
                i.shader_stages = (
                    "library",
                    "compute",
                    "amplification",
                    "mesh",
                    "pixel",
                    "node",
                )
            elif i.name.startswith("Bitcast"):
                i.category = "Bitcasts with different sizes"
        for i in "ViewID,AttributeAtVertex".split(","):
            self.name_idx[i].shader_model = 6, 1
        for i in "RawBufferLoad,RawBufferStore".split(","):
            self.name_idx[i].shader_model = 6, 2
            self.name_idx[i].shader_model_translated = 6, 0
        for i in "DispatchRaysIndex,DispatchRaysDimensions".split(","):
            self.name_idx[i].category = "Ray Dispatch Arguments"
            self.name_idx[i].shader_model = 6, 3
            self.name_idx[i].shader_stages = (
                "library",
                "raygeneration",
                "intersection",
                "anyhit",
                "closesthit",
                "miss",
                "callable",
            )
        for i in "InstanceID,InstanceIndex,PrimitiveIndex".split(","):
            self.name_idx[i].category = "Raytracing object space uint System Values"
            self.name_idx[i].shader_model = 6, 3
            self.name_idx[i].shader_stages = (
                "library",
                "intersection",
                "anyhit",
                "closesthit",
            )
        for i in "GeometryIndex".split(","):
            self.name_idx[
                i
            ].category = (
                "Raytracing object space uint System Values, raytracing tier 1.1"
            )
            self.name_idx[i].shader_model = 6, 5
            self.name_idx[i].shader_stages = (
                "library",
                "intersection",
                "anyhit",
                "closesthit",
            )
        for i in "HitKind".split(","):
            self.name_idx[i].category = "Raytracing hit uint System Values"
            self.name_idx[i].shader_model = 6, 3
            self.name_idx[i].shader_stages = (
                "library",
                "intersection",
                "anyhit",
                "closesthit",
            )
        for i in "RayFlags".split(","):
            self.name_idx[i].category = "Raytracing uint System Values"
            self.name_idx[i].shader_model = 6, 3
            self.name_idx[i].shader_stages = (
                "library",
                "intersection",
                "anyhit",
                "closesthit",
                "miss",
            )
        for i in "WorldRayOrigin,WorldRayDirection".split(","):
            self.name_idx[i].category = "Ray Vectors"
            self.name_idx[i].shader_model = 6, 3
            self.name_idx[i].shader_stages = (
                "library",
                "intersection",
                "anyhit",
                "closesthit",
                "miss",
            )
        for i in "ObjectRayOrigin,ObjectRayDirection".split(","):
            self.name_idx[i].category = "Ray object space Vectors"
            self.name_idx[i].shader_model = 6, 3
            self.name_idx[i].shader_stages = (
                "library",
                "intersection",
                "anyhit",
                "closesthit",
            )
        for i in "ObjectToWorld,WorldToObject".split(","):
            self.name_idx[i].category = "Ray Transforms"
            self.name_idx[i].shader_model = 6, 3
            self.name_idx[i].shader_stages = (
                "library",
                "intersection",
                "anyhit",
                "closesthit",
            )
        for i in "RayTMin,RayTCurrent".split(","):
            self.name_idx[i].category = "RayT"
            self.name_idx[i].shader_model = 6, 3
            self.name_idx[i].shader_stages = (
                "library",
                "intersection",
                "anyhit",
                "closesthit",
                "miss",
            )
        for i in "IgnoreHit,AcceptHitAndEndSearch".split(","):
            self.name_idx[i].category = "AnyHit Terminals"
            self.name_idx[i].shader_model = 6, 3
            self.name_idx[i].shader_stages = ("anyhit",)
        for i in "CallShader".split(","):
            self.name_idx[i].category = "Indirect Shader Invocation"
            self.name_idx[i].shader_model = 6, 3
            self.name_idx[i].shader_stages = (
                "library",
                "closesthit",
                "raygeneration",
                "miss",
                "callable",
            )
        for i in "TraceRay".split(","):
            self.name_idx[i].category = "Indirect Shader Invocation"
            self.name_idx[i].shader_model = 6, 3
            self.name_idx[i].shader_stages = (
                "library",
                "raygeneration",
                "closesthit",
                "miss",
            )
        for i in "ReportHit".split(","):
            self.name_idx[i].category = "Indirect Shader Invocation"
            self.name_idx[i].shader_model = 6, 3
            self.name_idx[i].shader_stages = ("library", "intersection")
        for i in "CreateHandleForLib".split(","):
            self.name_idx[
                i
            ].category = (
                "Library create handle from resource struct (like HL intrinsic)"
            )
            self.name_idx[i].shader_model = 6, 3
            self.name_idx[i].shader_model_translated = 6, 0
        for i in "AnnotateHandle,CreateHandleFromBinding,CreateHandleFromHeap".split(
            ","
        ):
            self.name_idx[i].category = "Get handle from heap"
            self.name_idx[i].shader_model = 6, 6
        for i in "AnnotateHandle,CreateHandleFromBinding".split(","):
            self.name_idx[i].shader_model_translated = 6, 0
        for i in "Dot4AddU8Packed,Dot4AddI8Packed,Dot2AddHalf".split(","):
            self.name_idx[i].category = "Dot product with accumulate"
            self.name_idx[i].shader_model = 6, 4
        for i in "WaveMatch,WaveMultiPrefixOp,WaveMultiPrefixBitCount".split(","):
            self.name_idx[i].category = "Wave"
            self.name_idx[i].shader_model = 6, 5
        for (
            i
        ) in "SetMeshOutputCounts,EmitIndices,GetMeshPayload,StoreVertexOutput,StorePrimitiveOutput".split(
            ","
        ):
            self.name_idx[i].category = "Mesh shader instructions"
            self.name_idx[i].shader_stages = ("mesh",)
            self.name_idx[i].shader_model = 6, 5
        for i in "DispatchMesh".split(","):
            self.name_idx[i].category = "Amplification shader instructions"
            self.name_idx[i].shader_stages = ("amplification",)
            self.name_idx[i].shader_model = 6, 5
        for i in "WriteSamplerFeedback,WriteSamplerFeedbackBias".split(","):
            self.name_idx[i].category = "Sampler Feedback"
            self.name_idx[i].is_feedback = True
            self.name_idx[i].is_gradient = True
            self.name_idx[i].shader_model = 6, 5
            self.name_idx[i].shader_stages = (
                "library",
                "pixel",
            )
        for i in "WriteSamplerFeedbackLevel,WriteSamplerFeedbackGrad".split(","):
            self.name_idx[i].category = "Sampler Feedback"
            self.name_idx[i].is_feedback = True
            self.name_idx[i].shader_model = 6, 5
        for i in (
            "AllocateRayQuery,RayQuery_TraceRayInline,RayQuery_Proceed,RayQuery_Abort,RayQuery_CommitNonOpaqueTriangleHit,RayQuery_CommitProceduralPrimitiveHit,RayQuery_RayFlags,RayQuery_WorldRayOrigin,RayQuery_WorldRayDirection,RayQuery_RayTMin,"
            + "RayQuery_CandidateTriangleRayT,RayQuery_CommittedRayT,RayQuery_CandidateInstanceIndex,RayQuery_CandidateInstanceID,RayQuery_CandidateGeometryIndex,RayQuery_CandidatePrimitiveIndex,"
            + "RayQuery_CandidateObjectRayOrigin,RayQuery_CandidateObjectRayDirection,RayQuery_CommittedInstanceIndex,RayQuery_CommittedInstanceID,RayQuery_CommittedGeometryIndex,RayQuery_CommittedPrimitiveIndex,"
            + "RayQuery_CommittedObjectRayOrigin,RayQuery_CommittedObjectRayDirection,RayQuery_CandidateProceduralPrimitiveNonOpaque,RayQuery_CandidateTriangleFrontFace,RayQuery_CommittedTriangleFrontFace,"
            + "RayQuery_CandidateTriangleBarycentrics,RayQuery_CommittedTriangleBarycentrics,RayQuery_CommittedStatus,RayQuery_CandidateType,RayQuery_CandidateObjectToWorld3x4,"
            + "RayQuery_CandidateWorldToObject3x4,RayQuery_CommittedObjectToWorld3x4,RayQuery_CommittedWorldToObject3x4,RayQuery_CandidateInstanceContributionToHitGroupIndex,RayQuery_CommittedInstanceContributionToHitGroupIndex"
        ).split(","):
            self.name_idx[i].category = "Inline Ray Query"
            self.name_idx[i].shader_model = 6, 5
        for i in "Unpack4x8".split(","):
            self.name_idx[i].category = "Unpacking intrinsics"
            self.name_idx[i].shader_model = 6, 6
        for i in "Pack4x8".split(","):
            self.name_idx[i].category = "Packing intrinsics"
            self.name_idx[i].shader_model = 6, 6
        for i in "IsHelperLane".split(","):
            self.name_idx[i].category = "Helper Lanes"
            self.name_idx[i].shader_model = 6, 6
        for i in "QuadVote,TextureGatherRaw,SampleCmpLevel,TextureStoreSample".split(
            ","
        ):
            self.name_idx[i].shader_model = 6, 7
        for i in "QuadVote".split(","):
            self.name_idx[i].shader_model_translated = 6, 0
        for i in "CreateNodeOutputHandle".split(","):
            self.name_idx[i].category = "Create/Annotate Node Handles"
            self.name_idx[i].shader_model = 6, 8
            self.name_idx[i].shader_stages = ("node",)
        for i in "CreateNodeInputRecordHandle,AllocateNodeOutputRecords".split(","):
            self.name_idx[i].category = "Create/Annotate Node Handles"
            self.name_idx[i].shader_model = 6, 8
            self.name_idx[i].shader_stages = ("node",)
        for i in "IndexNodeHandle".split(","):
            self.name_idx[i].category = "Create/Annotate Node Handles"
            self.name_idx[i].shader_model = 6, 8
            self.name_idx[i].shader_stages = ("node",)  # TBD: add "library"
        for i in "AnnotateNodeHandle,AnnotateNodeRecordHandle".split(","):
            self.name_idx[i].category = "Create/Annotate Node Handles"
            self.name_idx[i].shader_model = 6, 8
            self.name_idx[i].shader_stages = ("node",)  # TBD: add "library"
        for i in "GetNodeRecordPtr".split(","):
            self.name_idx[i].category = "Get Pointer to Node Record in Address Space 6"
            self.name_idx[i].shader_model = 6, 8
            self.name_idx[i].shader_stages = ("node",)  # TBD: add "library"
        for i in (
            "IncrementOutputCount,OutputComplete,GetInputRecordCount,FinishedCrossGroupSharing,NodeOutputIsValid,GetRemainingRecursionLevels"
        ).split(","):
            self.name_idx[i].category = "Work Graph intrinsics"
            self.name_idx[i].shader_model = 6, 8
            self.name_idx[i].shader_stages = ("node",)
        # All barrier ops:
        for i in "Barrier".split(","):
            self.name_idx[i].category = "Synchronization"
            self.name_idx[i].is_barrier = True
        for i in "BarrierByMemoryType".split(","):
            self.name_idx[i].category = "Synchronization"
            self.name_idx[i].is_barrier = True
            self.name_idx[i].shader_model = 6, 8
            self.name_idx[i].shader_model_translated = 6, 0
        for i in "BarrierByMemoryHandle".split(","):
            self.name_idx[i].category = "Synchronization"
            self.name_idx[i].is_barrier = True
            self.name_idx[i].shader_model = 6, 8
        for i in "BarrierByNodeRecordHandle".split(","):
            self.name_idx[i].category = "Synchronization"
            self.name_idx[i].is_barrier = True
            self.name_idx[i].shader_model = 6, 8
            self.name_idx[i].shader_stages = ("node",)
        for i in "SampleCmpBias,SampleCmpGrad".split(","):
            self.name_idx[i].category = "Comparison Samples"
            self.name_idx[i].shader_model = 6, 8

        for i in "StartVertexLocation,StartInstanceLocation".split(","):
            self.name_idx[i].category = "Extended Command Information"
            self.name_idx[i].shader_stages = ("vertex",)
            self.name_idx[i].shader_model = 6, 8

    def populate_llvm_instructions(self):
        # Add instructions that map to LLVM instructions.
        # This is basically include\llvm\IR\Instruction.def
        #
        # Some instructions don't have their operands defined here because they are
        # very specific and expanding generality isn't worth it; for example,
        # branching refers to basic block arguments.
        retvoid_param = db_dxil_param(0, "v", "", "no return value")
        retoload_param = db_dxil_param(0, "$o", "", "no return value")
        oload_all_arith = "hfd1wil"  # note that 8 is missing
        oload_all_arith_v = "v" + oload_all_arith
        oload_int_arith = "wil"  # note that 8 is missing
        oload_int_arith_b = "1wil"  # note that 8 is missing
        oload_float_arith = "hfd"
        oload_cast_params = [
            retoload_param,
            db_dxil_param(1, "$o", "value", "Value to cast/convert"),
        ]
        oload_binary_params = [
            retoload_param,
            db_dxil_param(1, "$o", "a", "first value"),
            db_dxil_param(2, "$o", "b", "second value"),
        ]
        self.add_llvm_instr(
            "TERM",
            1,
            "Ret",
            "ReturnInst",
            "returns a value (possibly void), from a function.",
            oload_all_arith_v,
            [retoload_param],
        )
        self.add_llvm_instr(
            "TERM",
            2,
            "Br",
            "BranchInst",
            "branches (conditional or unconditional)",
            "",
            [],
        )
        self.add_llvm_instr(
            "TERM", 3, "Switch", "SwitchInst", "performs a multiway switch", "", []
        )
        self.add_llvm_instr(
            "TERM", 4, "IndirectBr", "IndirectBrInst", "branches indirectly", "", []
        )
        self.add_llvm_instr(
            "TERM",
            5,
            "Invoke",
            "InvokeInst",
            "invokes function with normal and exceptional returns",
            "",
            [],
        )
        self.add_llvm_instr(
            "TERM",
            6,
            "Resume",
            "ResumeInst",
            "resumes the propagation of an exception",
            "",
            [],
        )
        self.add_llvm_instr(
            "TERM", 7, "Unreachable", "UnreachableInst", "is unreachable", "", []
        )

        self.add_llvm_instr(
            "BINARY",
            8,
            "Add",
            "BinaryOperator",
            "returns the sum of its two operands",
            oload_int_arith,
            oload_binary_params,
            counters=("ints",),
        )
        self.add_llvm_instr(
            "BINARY",
            9,
            "FAdd",
            "BinaryOperator",
            "returns the sum of its two operands",
            oload_float_arith,
            oload_binary_params,
            counters=("floats",),
        )
        self.add_llvm_instr(
            "BINARY",
            10,
            "Sub",
            "BinaryOperator",
            "returns the difference of its two operands",
            oload_int_arith,
            oload_binary_params,
            counters=("ints",),
        )
        self.add_llvm_instr(
            "BINARY",
            11,
            "FSub",
            "BinaryOperator",
            "returns the difference of its two operands",
            oload_float_arith,
            oload_binary_params,
            counters=("floats",),
        )
        self.add_llvm_instr(
            "BINARY",
            12,
            "Mul",
            "BinaryOperator",
            "returns the product of its two operands",
            oload_int_arith,
            oload_binary_params,
            counters=("ints",),
        )
        self.add_llvm_instr(
            "BINARY",
            13,
            "FMul",
            "BinaryOperator",
            "returns the product of its two operands",
            oload_float_arith,
            oload_binary_params,
            counters=("floats",),
        )
        self.add_llvm_instr(
            "BINARY",
            14,
            "UDiv",
            "BinaryOperator",
            "returns the quotient of its two unsigned operands",
            oload_int_arith,
            oload_binary_params,
            counters=("uints",),
        )
        self.add_llvm_instr(
            "BINARY",
            15,
            "SDiv",
            "BinaryOperator",
            "returns the quotient of its two signed operands",
            oload_int_arith,
            oload_binary_params,
            counters=("ints",),
        )
        self.add_llvm_instr(
            "BINARY",
            16,
            "FDiv",
            "BinaryOperator",
            "returns the quotient of its two operands",
            oload_float_arith,
            oload_binary_params,
            counters=("floats",),
        )
        self.add_llvm_instr(
            "BINARY",
            17,
            "URem",
            "BinaryOperator",
            "returns the remainder from the unsigned division of its two operands",
            oload_int_arith,
            oload_binary_params,
            counters=("uints",),
        )
        self.add_llvm_instr(
            "BINARY",
            18,
            "SRem",
            "BinaryOperator",
            "returns the remainder from the signed division of its two operands",
            oload_int_arith,
            oload_binary_params,
            counters=("ints",),
        )
        self.add_llvm_instr(
            "BINARY",
            19,
            "FRem",
            "BinaryOperator",
            "returns the remainder from the division of its two operands",
            oload_float_arith,
            oload_binary_params,
            counters=("floats",),
        )

        self.add_llvm_instr(
            "BINARY",
            20,
            "Shl",
            "BinaryOperator",
            "shifts left (logical)",
            oload_int_arith,
            oload_binary_params,
            counters=("uints",),
        )
        self.add_llvm_instr(
            "BINARY",
            21,
            "LShr",
            "BinaryOperator",
            "shifts right (logical), with zero bit fill",
            oload_int_arith,
            oload_binary_params,
            counters=("uints",),
        )
        self.add_llvm_instr(
            "BINARY",
            22,
            "AShr",
            "BinaryOperator",
            "shifts right (arithmetic), with 'a' operand sign bit fill",
            oload_int_arith,
            oload_binary_params,
            counters=("ints",),
        )
        self.add_llvm_instr(
            "BINARY",
            23,
            "And",
            "BinaryOperator",
            "returns a  bitwise logical and of its two operands",
            oload_int_arith_b,
            oload_binary_params,
            counters=("uints",),
        )
        self.add_llvm_instr(
            "BINARY",
            24,
            "Or",
            "BinaryOperator",
            "returns a bitwise logical or of its two operands",
            oload_int_arith_b,
            oload_binary_params,
            counters=("uints",),
        )
        self.add_llvm_instr(
            "BINARY",
            25,
            "Xor",
            "BinaryOperator",
            "returns a bitwise logical xor of its two operands",
            oload_int_arith_b,
            oload_binary_params,
            counters=("uints",),
        )

        self.add_llvm_instr(
            "MEMORY",
            26,
            "Alloca",
            "AllocaInst",
            "allocates memory on the stack frame of the currently executing function",
            "",
            [],
        )
        self.add_llvm_instr(
            "MEMORY", 27, "Load", "LoadInst", "reads from memory", "", []
        )
        self.add_llvm_instr(
            "MEMORY", 28, "Store", "StoreInst", "writes to memory", "", []
        )
        self.add_llvm_instr(
            "MEMORY",
            29,
            "GetElementPtr",
            "GetElementPtrInst",
            "gets the address of a subelement of an aggregate value",
            "",
            [],
        )
        self.add_llvm_instr(
            "MEMORY",
            30,
            "Fence",
            "FenceInst",
            "introduces happens-before edges between operations",
            "",
            [],
            counters=("fence",),
        )
        self.add_llvm_instr(
            "MEMORY",
            31,
            "AtomicCmpXchg",
            "AtomicCmpXchgInst",
            "atomically modifies memory",
            "",
            [],
            counters=("atomic",),
        )
        self.add_llvm_instr(
            "MEMORY",
            32,
            "AtomicRMW",
            "AtomicRMWInst",
            "atomically modifies memory",
            "",
            [],
            counters=("atomic",),
        )

        self.add_llvm_instr(
            "CAST",
            33,
            "Trunc",
            "TruncInst",
            "truncates an integer",
            oload_int_arith_b,
            oload_cast_params,
            counters=("ints",),
        )
        self.add_llvm_instr(
            "CAST",
            34,
            "ZExt",
            "ZExtInst",
            "zero extends an integer",
            oload_int_arith_b,
            oload_cast_params,
            counters=("uints",),
        )
        self.add_llvm_instr(
            "CAST",
            35,
            "SExt",
            "SExtInst",
            "sign extends an integer",
            oload_int_arith_b,
            oload_cast_params,
            counters=("ints",),
        )
        self.add_llvm_instr(
            "CAST",
            36,
            "FPToUI",
            "FPToUIInst",
            "converts a floating point to UInt",
            oload_all_arith,
            oload_cast_params,
            counters=("floats",),
        )
        self.add_llvm_instr(
            "CAST",
            37,
            "FPToSI",
            "FPToSIInst",
            "converts a floating point to SInt",
            oload_all_arith,
            oload_cast_params,
            counters=("floats",),
        )
        self.add_llvm_instr(
            "CAST",
            38,
            "UIToFP",
            "UIToFPInst",
            "converts a UInt to floating point",
            oload_all_arith,
            oload_cast_params,
            counters=("floats",),
        )
        self.add_llvm_instr(
            "CAST",
            39,
            "SIToFP",
            "SIToFPInst",
            "converts a SInt to floating point",
            oload_all_arith,
            oload_cast_params,
            counters=("floats",),
        )
        self.add_llvm_instr(
            "CAST",
            40,
            "FPTrunc",
            "FPTruncInst",
            "truncates a floating point",
            oload_float_arith,
            oload_cast_params,
            counters=("floats",),
        )
        self.add_llvm_instr(
            "CAST",
            41,
            "FPExt",
            "FPExtInst",
            "extends a floating point",
            oload_float_arith,
            oload_cast_params,
            counters=("floats",),
        )
        self.add_llvm_instr(
            "CAST",
            42,
            "PtrToInt",
            "PtrToIntInst",
            "converts a pointer to integer",
            "i",
            oload_cast_params,
        )
        self.add_llvm_instr(
            "CAST",
            43,
            "IntToPtr",
            "IntToPtrInst",
            "converts an integer to Pointer",
            "i",
            oload_cast_params,
        )
        self.add_llvm_instr(
            "CAST",
            44,
            "BitCast",
            "BitCastInst",
            "performs a bit-preserving type cast",
            oload_all_arith,
            oload_cast_params,
        )
        self.add_llvm_instr(
            "CAST",
            45,
            "AddrSpaceCast",
            "AddrSpaceCastInst",
            "casts a value addrspace",
            "",
            oload_cast_params,
        )

        self.add_llvm_instr(
            "OTHER",
            46,
            "ICmp",
            "ICmpInst",
            "compares integers",
            oload_int_arith_b,
            oload_binary_params,
            counters=("ints",),
        )
        self.add_llvm_instr(
            "OTHER",
            47,
            "FCmp",
            "FCmpInst",
            "compares floating points",
            oload_float_arith,
            oload_binary_params,
            counters=("floats",),
        )
        self.add_llvm_instr(
            "OTHER", 48, "PHI", "PHINode", "is a PHI node instruction", "", []
        )
        self.add_llvm_instr("OTHER", 49, "Call", "CallInst", "calls a function", "", [])
        self.add_llvm_instr(
            "OTHER", 50, "Select", "SelectInst", "selects an instruction", "", []
        )
        self.add_llvm_instr(
            "OTHER",
            51,
            "UserOp1",
            "Instruction",
            "may be used internally in a pass",
            "",
            [],
        )
        self.add_llvm_instr(
            "OTHER", 52, "UserOp2", "Instruction", "internal to passes only", "", []
        )
        self.add_llvm_instr(
            "OTHER", 53, "VAArg", "VAArgInst", "vaarg instruction", "", []
        )
        self.add_llvm_instr(
            "OTHER",
            57,
            "ExtractValue",
            "ExtractValueInst",
            "extracts from aggregate",
            "",
            [],
        )
        self.add_llvm_instr(
            "OTHER",
            59,
            "LandingPad",
            "LandingPadInst",
            "represents a landing pad",
            "",
            [],
        )

    def populate_dxil_operations(self):
        # $o in a parameter type means the overload type
        # $r in a parameter type means the resource type
        # $cb in a parameter type means cbuffer legacy load return type
        # overload types are a string of (v)oid, (h)alf, (f)loat, (d)ouble, (1)-bit, (8)-bit, (w)ord, (i)nt, (l)ong
        self.opcode_param = db_dxil_param(1, "i32", "opcode", "DXIL opcode")
        retvoid_param = db_dxil_param(0, "v", "", "no return value")
        next_op_idx = 0
        self.add_dxil_op(
            "TempRegLoad",
            next_op_idx,
            "TempRegLoad",
            "helper load operation",
            "hfwi",
            "ro",
            [
                db_dxil_param(0, "$o", "", "register value"),
                db_dxil_param(2, "u32", "index", "linearized register index"),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "TempRegStore",
            next_op_idx,
            "TempRegStore",
            "helper store operation",
            "hfwi",
            "",
            [
                retvoid_param,
                db_dxil_param(2, "u32", "index", "linearized register index"),
                db_dxil_param(3, "$o", "value", "value to store"),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "MinPrecXRegLoad",
            next_op_idx,
            "MinPrecXRegLoad",
            "helper load operation for minprecision",
            "hw",
            "ro",
            [
                db_dxil_param(0, "$o", "", "register value"),
                db_dxil_param(2, "pf32", "regIndex", "pointer to indexable register"),
                db_dxil_param(3, "i32", "index", "index"),
                db_dxil_param(4, "u8", "component", "component"),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "MinPrecXRegStore",
            next_op_idx,
            "MinPrecXRegStore",
            "helper store operation for minprecision",
            "hw",
            "",
            [
                retvoid_param,
                db_dxil_param(2, "pf32", "regIndex", "pointer to indexable register"),
                db_dxil_param(3, "i32", "index", "index"),
                db_dxil_param(4, "u8", "component", "component"),
                db_dxil_param(5, "$o", "value", "value to store"),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "LoadInput",
            next_op_idx,
            "LoadInput",
            "loads the value from shader input",
            "hfwi",
            "rn",
            [
                db_dxil_param(0, "$o", "", "input value"),
                db_dxil_param(2, "u32", "inputSigId", "input signature element ID"),
                db_dxil_param(3, "u32", "rowIndex", "row index relative to element"),
                db_dxil_param(4, "u8", "colIndex", "column index relative to element"),
                db_dxil_param(5, "i32", "gsVertexAxis", "gsVertexAxis"),
            ],
            counters=("sig_ld",),
        )
        next_op_idx += 1
        self.add_dxil_op(
            "StoreOutput",
            next_op_idx,
            "StoreOutput",
            "stores the value to shader output",
            "hfwi",
            "",
            [  # note, cannot store bit even though load supports it
                retvoid_param,
                db_dxil_param(2, "u32", "outputSigId", "output signature element ID"),
                db_dxil_param(3, "u32", "rowIndex", "row index relative to element"),
                db_dxil_param(4, "u8", "colIndex", "column index relative to element"),
                db_dxil_param(5, "$o", "value", "value to store"),
            ],
            counters=("sig_st",),
        )
        next_op_idx += 1

        def UFI(name, **mappings):
            name = name.upper()
            for k, v in mappings.items():
                if name.startswith(k):
                    return v
            if name.upper().startswith("F"):
                return "floats"
            elif name.upper().startswith("U"):
                return "uints"
            else:
                return "ints"

        # Unary float operations are regular.
        for i in "FAbs,Saturate".split(","):
            self.add_dxil_op(
                i,
                next_op_idx,
                "Unary",
                "returns the " + i,
                "hfd",
                "rn",
                [
                    db_dxil_param(0, "$o", "", "operation result"),
                    db_dxil_param(2, "$o", "value", "input value"),
                ],
                counters=("floats",),
            )
            next_op_idx += 1
        for i in "IsNaN,IsInf,IsFinite,IsNormal".split(","):
            self.add_dxil_op(
                i,
                next_op_idx,
                "IsSpecialFloat",
                "returns the " + i,
                "hf",
                "rn",
                [
                    db_dxil_param(0, "i1", "", "operation result"),
                    db_dxil_param(2, "$o", "value", "input value"),
                ],
                counters=("floats",),
            )
            next_op_idx += 1
        for (
            i
        ) in "Cos,Sin,Tan,Acos,Asin,Atan,Hcos,Hsin,Htan,Exp,Frc,Log,Sqrt,Rsqrt,Round_ne,Round_ni,Round_pi,Round_z".split(
            ","
        ):
            self.add_dxil_op(
                i,
                next_op_idx,
                "Unary",
                "returns the " + i,
                "hf",
                "rn",
                [
                    db_dxil_param(0, "$o", "", "operation result"),
                    db_dxil_param(2, "$o", "value", "input value"),
                ],
                counters=("floats",),
            )
            next_op_idx += 1

        # Unary int operations are regular.
        for i in "Bfrev".split(","):
            self.add_dxil_op(
                i,
                next_op_idx,
                "Unary",
                "returns the reverse bit pattern of the input value",
                "wil",
                "rn",
                [
                    db_dxil_param(0, "$o", "", "operation result"),
                    db_dxil_param(2, "$o", "value", "input value"),
                ],
                counters=("uints",),
            )
            next_op_idx += 1
        for i in "Countbits,FirstbitLo".split(","):
            self.add_dxil_op(
                i,
                next_op_idx,
                "UnaryBits",
                "returns the " + i,
                "wil",
                "rn",
                [
                    db_dxil_param(0, "i32", "", "operation result"),
                    db_dxil_param(2, "$o", "value", "input value"),
                ],
                counters=("uints",),
            )
            next_op_idx += 1
        for i in "FirstbitHi,FirstbitSHi".split(","):
            self.add_dxil_op(
                i,
                next_op_idx,
                "UnaryBits",
                "returns src != 0? (BitWidth-1 - " + i + ") : -1",
                "wil",
                "rn",
                [
                    db_dxil_param(0, "i32", "", "operation result"),
                    db_dxil_param(2, "$o", "value", "input value"),
                ],
                counters=("uints",),
            )
            next_op_idx += 1

        # Binary float operations
        for i in "FMax,FMin".split(","):
            self.add_dxil_op(
                i,
                next_op_idx,
                "Binary",
                "returns the " + i + " of the input values",
                "hfd",
                "rn",
                [
                    db_dxil_param(0, "$o", "", "operation result"),
                    db_dxil_param(2, "$o", "a", "input value"),
                    db_dxil_param(3, "$o", "b", "input value"),
                ],
                counters=("floats",),
            )
            next_op_idx += 1

        # Binary int operations
        for i in "IMax,IMin,UMax,UMin".split(","):
            self.add_dxil_op(
                i,
                next_op_idx,
                "Binary",
                "returns the " + i + " of the input values",
                "wil",
                "rn",
                [
                    db_dxil_param(0, "$o", "", "operation result"),
                    db_dxil_param(2, "$o", "a", "input value"),
                    db_dxil_param(3, "$o", "b", "input value"),
                ],
                counters=(UFI(i),),
            )
            next_op_idx += 1

        # Binary int operations with two outputs
        for i in "IMul,UMul,UDiv".split(","):
            self.add_dxil_op(
                i,
                next_op_idx,
                "BinaryWithTwoOuts",
                "returns the " + i + " of the input values",
                "i",
                "rn",
                [
                    db_dxil_param(0, "twoi32", "", "operation result"),
                    db_dxil_param(2, "$o", "a", "input value"),
                    db_dxil_param(3, "$o", "b", "input value"),
                ],
                counters=(UFI(i),),
            )
            next_op_idx += 1

        # Binary int operations with carry
        for i in "UAddc,USubb".split(","):
            self.add_dxil_op(
                i,
                next_op_idx,
                "BinaryWithCarryOrBorrow",
                "returns the " + i + " of the input values",
                "i",
                "rn",
                [
                    db_dxil_param(
                        0, "i32c", "", "operation result with carry/borrow value"
                    ),
                    db_dxil_param(2, "$o", "a", "input value"),
                    db_dxil_param(3, "$o", "b", "input value"),
                ],
                counters=("uints",),
            )
            next_op_idx += 1

        # Tertiary float.
        self.add_dxil_op(
            "FMad",
            next_op_idx,
            "Tertiary",
            "performs a fused multiply add (FMA) of the form a * b + c",
            "hfd",
            "rn",
            [
                db_dxil_param(
                    0, "$o", "", "the fused multiply-addition of parameters a * b + c"
                ),
                db_dxil_param(2, "$o", "a", "first value for FMA, the first factor"),
                db_dxil_param(3, "$o", "b", "second value for FMA, the second factor"),
                db_dxil_param(4, "$o", "c", "third value for FMA, the addend"),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "Fma",
            next_op_idx,
            "Tertiary",
            "performs a fused multiply add (FMA) of the form a * b + c",
            "d",
            "rn",
            [
                db_dxil_param(
                    0,
                    "$o",
                    "",
                    "the double-precision fused multiply-addition of parameters a * b + c, accurate to 0.5 units of least precision (ULP)",
                ),
                db_dxil_param(2, "$o", "a", "first value for FMA, the first factor"),
                db_dxil_param(3, "$o", "b", "second value for FMA, the second factor"),
                db_dxil_param(4, "$o", "c", "third value for FMA, the addend"),
            ],
            counters=("floats",),
        )
        next_op_idx += 1

        # Tertiary int.
        for i in "IMad,UMad".split(","):
            self.add_dxil_op(
                i,
                next_op_idx,
                "Tertiary",
                "performs an integral " + i,
                "wil",
                "rn",
                [
                    db_dxil_param(0, "$o", "", "the operation result"),
                    db_dxil_param(
                        2, "$o", "a", "first value for FMA, the first factor"
                    ),
                    db_dxil_param(
                        3, "$o", "b", "second value for FMA, the second factor"
                    ),
                    db_dxil_param(4, "$o", "c", "third value for FMA, the addend"),
                ],
                counters=(UFI(i),),
            )
            next_op_idx += 1
        for i in "Msad,Ibfe,Ubfe".split(","):
            self.add_dxil_op(
                i,
                next_op_idx,
                "Tertiary",
                "performs an integral " + i,
                "il",
                "rn",
                [
                    db_dxil_param(0, "$o", "", "the operation result"),
                    db_dxil_param(
                        2, "$o", "a", "first value for FMA, the first factor"
                    ),
                    db_dxil_param(
                        3, "$o", "b", "second value for FMA, the second factor"
                    ),
                    db_dxil_param(4, "$o", "c", "third value for FMA, the addend"),
                ],
                counters=(UFI(i, M="uints"),),
            )
            next_op_idx += 1

        # Quaternary
        self.add_dxil_op(
            "Bfi",
            next_op_idx,
            "Quaternary",
            "given a bit range from the LSB of a number, places that number of bits in another number at any offset",
            "i",
            "rn",
            [
                db_dxil_param(0, "$o", "", "the operation result"),
                db_dxil_param(
                    2, "$o", "width", "the bitfield width to take from the value"
                ),
                db_dxil_param(
                    3, "$o", "offset", "the bitfield offset to replace in the value"
                ),
                db_dxil_param(4, "$o", "value", "the number the bits are taken from"),
                db_dxil_param(
                    5, "$o", "replacedValue", "the number with bits to be replaced"
                ),
            ],
            counters=("uints",),
        )
        next_op_idx += 1

        # Dot
        self.add_dxil_op(
            "Dot2",
            next_op_idx,
            "Dot2",
            "two-dimensional vector dot-product",
            "hf",
            "rn",
            [
                db_dxil_param(0, "$o", "", "the operation result"),
                db_dxil_param(2, "$o", "ax", "the first component of the first vector"),
                db_dxil_param(
                    3, "$o", "ay", "the second component of the first vector"
                ),
                db_dxil_param(
                    4, "$o", "bx", "the first component of the second vector"
                ),
                db_dxil_param(
                    5, "$o", "by", "the second component of the second vector"
                ),
            ],
            counters=("floats",),
        )
        next_op_idx += 1
        self.add_dxil_op(
            "Dot3",
            next_op_idx,
            "Dot3",
            "three-dimensional vector dot-product",
            "hf",
            "rn",
            [
                db_dxil_param(0, "$o", "", "the operation result"),
                db_dxil_param(2, "$o", "ax", "the first component of the first vector"),
                db_dxil_param(
                    3, "$o", "ay", "the second component of the first vector"
                ),
                db_dxil_param(4, "$o", "az", "the third component of the first vector"),
                db_dxil_param(
                    5, "$o", "bx", "the first component of the second vector"
                ),
                db_dxil_param(
                    6, "$o", "by", "the second component of the second vector"
                ),
                db_dxil_param(
                    7, "$o", "bz", "the third component of the second vector"
                ),
            ],
            counters=("floats",),
        )
        next_op_idx += 1
        self.add_dxil_op(
            "Dot4",
            next_op_idx,
            "Dot4",
            "four-dimensional vector dot-product",
            "hf",
            "rn",
            [
                db_dxil_param(0, "$o", "", "the operation result"),
                db_dxil_param(2, "$o", "ax", "the first component of the first vector"),
                db_dxil_param(
                    3, "$o", "ay", "the second component of the first vector"
                ),
                db_dxil_param(4, "$o", "az", "the third component of the first vector"),
                db_dxil_param(
                    5, "$o", "aw", "the fourth component of the first vector"
                ),
                db_dxil_param(
                    6, "$o", "bx", "the first component of the second vector"
                ),
                db_dxil_param(
                    7, "$o", "by", "the second component of the second vector"
                ),
                db_dxil_param(
                    8, "$o", "bz", "the third component of the second vector"
                ),
                db_dxil_param(
                    9, "$o", "bw", "the fourth component of the second vector"
                ),
            ],
            counters=("floats",),
        )
        next_op_idx += 1

        # Resources.
        self.add_dxil_op(
            "CreateHandle",
            next_op_idx,
            "CreateHandle",
            "creates the handle to a resource",
            "v",
            "ro",
            [
                db_dxil_param(0, "res", "", "the handle to the resource"),
                db_dxil_param(
                    2,
                    "i8",
                    "resourceClass",
                    "the class of resource to create (SRV, UAV, CBuffer, Sampler)",
                    is_const=True,
                ),  # maps to DxilResourceBase::Class
                db_dxil_param(
                    3, "i32", "rangeId", "range identifier for resource", is_const=True
                ),
                db_dxil_param(4, "i32", "index", "zero-based index into range"),
                db_dxil_param(
                    5,
                    "i1",
                    "nonUniformIndex",
                    "non-uniform resource index",
                    is_const=True,
                ),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "CBufferLoad",
            next_op_idx,
            "CBufferLoad",
            "loads a value from a constant buffer resource",
            "hfd8wil",
            "ro",
            [
                db_dxil_param(
                    0, "$o", "", "the value for the constant buffer variable"
                ),
                db_dxil_param(2, "res", "handle", "cbuffer handle"),
                db_dxil_param(3, "u32", "byteOffset", "linear byte offset of value"),
                db_dxil_param(
                    4, "u32", "alignment", "load access alignment", is_const=True
                ),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "CBufferLoadLegacy",
            next_op_idx,
            "CBufferLoadLegacy",
            "loads a value from a constant buffer resource",
            "hfdwil",
            "ro",
            [
                db_dxil_param(
                    0, "$cb", "", "the value for the constant buffer variable"
                ),
                db_dxil_param(2, "res", "handle", "cbuffer handle"),
                db_dxil_param(
                    3, "u32", "regIndex", "0-based index into cbuffer instance"
                ),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "Sample",
            next_op_idx,
            "Sample",
            "samples a texture",
            "hfwi",
            "ro",
            [
                db_dxil_param(0, "$r", "", "the sampled value"),
                db_dxil_param(2, "res", "srv", "handle of SRV to sample"),
                db_dxil_param(3, "res", "sampler", "handle of sampler to use"),
                db_dxil_param(4, "f", "coord0", "coordinate"),
                db_dxil_param(5, "f", "coord1", "coordinate, undef for Texture1D"),
                db_dxil_param(
                    6,
                    "f",
                    "coord2",
                    "coordinate, undef for Texture1D, Texture1DArray or Texture2D",
                ),
                db_dxil_param(
                    7, "f", "coord3", "coordinate, defined only for TextureCubeArray"
                ),
                db_dxil_param(
                    8,
                    "i32",
                    "offset0",
                    "optional offset, applicable to Texture1D, Texture1DArray, and as part of offset1",
                ),
                db_dxil_param(
                    9,
                    "i32",
                    "offset1",
                    "optional offset, applicable to Texture2D, Texture2DArray, and as part of offset2",
                ),
                db_dxil_param(
                    10, "i32", "offset2", "optional offset, applicable to Texture3D"
                ),
                db_dxil_param(11, "f", "clamp", "clamp value"),
            ],
            counters=("tex_norm",),
        )
        next_op_idx += 1
        self.add_dxil_op(
            "SampleBias",
            next_op_idx,
            "SampleBias",
            "samples a texture after applying the input bias to the mipmap level",
            "hfwi",
            "ro",
            [
                db_dxil_param(0, "$r", "", "the sampled value"),
                db_dxil_param(2, "res", "srv", "handle of SRV to sample"),
                db_dxil_param(3, "res", "sampler", "handle of sampler to use"),
                db_dxil_param(4, "f", "coord0", "coordinate"),
                db_dxil_param(5, "f", "coord1", "coordinate, undef for Texture1D"),
                db_dxil_param(
                    6,
                    "f",
                    "coord2",
                    "coordinate, undef for Texture1D, Texture1DArray or Texture2D",
                ),
                db_dxil_param(
                    7, "f", "coord3", "coordinate, defined only for TextureCubeArray"
                ),
                db_dxil_param(
                    8,
                    "i32",
                    "offset0",
                    "optional offset, applicable to Texture1D, Texture1DArray, and as part of offset1",
                ),
                db_dxil_param(
                    9,
                    "i32",
                    "offset1",
                    "optional offset, applicable to Texture2D, Texture2DArray, and as part of offset2",
                ),
                db_dxil_param(
                    10, "i32", "offset2", "optional offset, applicable to Texture3D"
                ),
                db_dxil_param(11, "f", "bias", "bias value"),
                db_dxil_param(12, "f", "clamp", "clamp value"),
            ],
            counters=("tex_bias",),
        )
        next_op_idx += 1
        self.add_dxil_op(
            "SampleLevel",
            next_op_idx,
            "SampleLevel",
            "samples a texture using a mipmap-level offset",
            "hfwi",
            "ro",
            [
                db_dxil_param(0, "$r", "", "the sampled value"),
                db_dxil_param(2, "res", "srv", "handle of SRV to sample"),
                db_dxil_param(3, "res", "sampler", "handle of sampler to use"),
                db_dxil_param(4, "f", "coord0", "coordinate"),
                db_dxil_param(5, "f", "coord1", "coordinate, undef for Texture1D"),
                db_dxil_param(
                    6,
                    "f",
                    "coord2",
                    "coordinate, undef for Texture1D, Texture1DArray or Texture2D",
                ),
                db_dxil_param(
                    7, "f", "coord3", "coordinate, defined only for TextureCubeArray"
                ),
                db_dxil_param(
                    8,
                    "i32",
                    "offset0",
                    "optional offset, applicable to Texture1D, Texture1DArray, and as part of offset1",
                ),
                db_dxil_param(
                    9,
                    "i32",
                    "offset1",
                    "optional offset, applicable to Texture2D, Texture2DArray, and as part of offset2",
                ),
                db_dxil_param(
                    10, "i32", "offset2", "optional offset, applicable to Texture3D"
                ),
                db_dxil_param(
                    11,
                    "f",
                    "LOD",
                    "level of detail, biggest map if less than or equal to zero; fraction used to interpolate across levels",
                ),
            ],
            counters=("tex_norm",),
        )
        next_op_idx += 1
        self.add_dxil_op(
            "SampleGrad",
            next_op_idx,
            "SampleGrad",
            "samples a texture using a gradient to influence the way the sample location is calculated",
            "hfwi",
            "ro",
            [
                db_dxil_param(0, "$r", "", "the sampled value"),
                db_dxil_param(2, "res", "srv", "handle of SRV to sample"),
                db_dxil_param(3, "res", "sampler", "handle of sampler to use"),
                db_dxil_param(4, "f", "coord0", "coordinate"),
                db_dxil_param(5, "f", "coord1", "coordinate, undef for Texture1D"),
                db_dxil_param(
                    6,
                    "f",
                    "coord2",
                    "coordinate, undef for Texture1D, Texture1DArray or Texture2D",
                ),
                db_dxil_param(
                    7, "f", "coord3", "coordinate, defined only for TextureCubeArray"
                ),
                db_dxil_param(
                    8,
                    "i32",
                    "offset0",
                    "optional offset, applicable to Texture1D, Texture1DArray, and as part of offset1",
                ),
                db_dxil_param(
                    9,
                    "i32",
                    "offset1",
                    "optional offset, applicable to Texture2D, Texture2DArray, and as part of offset2",
                ),
                db_dxil_param(
                    10, "i32", "offset2", "optional offset, applicable to Texture3D"
                ),
                db_dxil_param(
                    11,
                    "f",
                    "ddx0",
                    "rate of change of the texture coordinate in the x direction",
                ),
                db_dxil_param(
                    12,
                    "f",
                    "ddx1",
                    "rate of change of the texture coordinate in the x direction",
                ),
                db_dxil_param(
                    13,
                    "f",
                    "ddx2",
                    "rate of change of the texture coordinate in the x direction",
                ),
                db_dxil_param(
                    14,
                    "f",
                    "ddy0",
                    "rate of change of the texture coordinate in the y direction",
                ),
                db_dxil_param(
                    15,
                    "f",
                    "ddy1",
                    "rate of change of the texture coordinate in the y direction",
                ),
                db_dxil_param(
                    16,
                    "f",
                    "ddy2",
                    "rate of change of the texture coordinate in the y direction",
                ),
                db_dxil_param(17, "f", "clamp", "clamp value"),
            ],
            counters=("tex_grad",),
        )
        next_op_idx += 1
        self.add_dxil_op(
            "SampleCmp",
            next_op_idx,
            "SampleCmp",
            "samples a texture and compares a single component against the specified comparison value",
            "hf",
            "ro",
            [
                db_dxil_param(
                    0, "$r", "", "the value for the constant buffer variable"
                ),
                db_dxil_param(2, "res", "srv", "handle of SRV to sample"),
                db_dxil_param(3, "res", "sampler", "handle of sampler to use"),
                db_dxil_param(4, "f", "coord0", "coordinate"),
                db_dxil_param(5, "f", "coord1", "coordinate, undef for Texture1D"),
                db_dxil_param(
                    6,
                    "f",
                    "coord2",
                    "coordinate, undef for Texture1D, Texture1DArray or Texture2D",
                ),
                db_dxil_param(
                    7, "f", "coord3", "coordinate, defined only for TextureCubeArray"
                ),
                db_dxil_param(
                    8,
                    "i32",
                    "offset0",
                    "optional offset, applicable to Texture1D, Texture1DArray, and as part of offset1",
                ),
                db_dxil_param(
                    9,
                    "i32",
                    "offset1",
                    "optional offset, applicable to Texture2D, Texture2DArray, and as part of offset2",
                ),
                db_dxil_param(
                    10, "i32", "offset2", "optional offset, applicable to Texture3D"
                ),
                db_dxil_param(11, "f", "compareValue", "the value to compare with"),
                db_dxil_param(12, "f", "clamp", "clamp value"),
            ],
            counters=("tex_cmp",),
        )
        next_op_idx += 1
        self.add_dxil_op(
            "SampleCmpLevelZero",
            next_op_idx,
            "SampleCmpLevelZero",
            "samples a texture and compares a single component against the specified comparison value",
            "hf",
            "ro",
            [
                db_dxil_param(
                    0, "$r", "", "the value for the constant buffer variable"
                ),
                db_dxil_param(2, "res", "srv", "handle of SRV to sample"),
                db_dxil_param(3, "res", "sampler", "handle of sampler to use"),
                db_dxil_param(4, "f", "coord0", "coordinate"),
                db_dxil_param(5, "f", "coord1", "coordinate, undef for Texture1D"),
                db_dxil_param(
                    6,
                    "f",
                    "coord2",
                    "coordinate, undef for Texture1D, Texture1DArray or Texture2D",
                ),
                db_dxil_param(
                    7, "f", "coord3", "coordinate, defined only for TextureCubeArray"
                ),
                db_dxil_param(
                    8,
                    "i32",
                    "offset0",
                    "optional offset, applicable to Texture1D, Texture1DArray, and as part of offset1",
                ),
                db_dxil_param(
                    9,
                    "i32",
                    "offset1",
                    "optional offset, applicable to Texture2D, Texture2DArray, and as part of offset2",
                ),
                db_dxil_param(
                    10, "i32", "offset2", "optional offset, applicable to Texture3D"
                ),
                db_dxil_param(11, "f", "compareValue", "the value to compare with"),
            ],
            counters=("tex_cmp",),
        )
        next_op_idx += 1
        self.add_dxil_op(
            "TextureLoad",
            next_op_idx,
            "TextureLoad",
            "reads texel data without any filtering or sampling",
            "hfwi",
            "ro",
            [
                db_dxil_param(0, "$r", "", "the loaded value"),
                db_dxil_param(2, "res", "srv", "handle of SRV or UAV to sample"),
                db_dxil_param(
                    3,
                    "i32",
                    "mipLevelOrSampleCount",
                    "sample count for Texture2DMS, mip level otherwise",
                ),
                db_dxil_param(4, "i32", "coord0", "coordinate"),
                db_dxil_param(5, "i32", "coord1", "coordinate"),
                db_dxil_param(6, "i32", "coord2", "coordinate"),
                db_dxil_param(7, "i32", "offset0", "optional offset"),
                db_dxil_param(8, "i32", "offset1", "optional offset"),
                db_dxil_param(9, "i32", "offset2", "optional offset"),
            ],
            counters=("tex_load",),
        )
        next_op_idx += 1
        self.add_dxil_op(
            "TextureStore",
            next_op_idx,
            "TextureStore",
            "reads texel data without any filtering or sampling",
            "hfwi",
            "",
            [
                db_dxil_param(0, "v", "", ""),
                db_dxil_param(2, "res", "srv", "handle of UAV to store to"),
                db_dxil_param(3, "i32", "coord0", "coordinate"),
                db_dxil_param(4, "i32", "coord1", "coordinate"),
                db_dxil_param(5, "i32", "coord2", "coordinate"),
                db_dxil_param(6, "$o", "value0", "value"),
                db_dxil_param(7, "$o", "value1", "value"),
                db_dxil_param(8, "$o", "value2", "value"),
                db_dxil_param(9, "$o", "value3", "value"),
                db_dxil_param(10, "i8", "mask", "written value mask"),
            ],
            counters=("tex_store",),
        )
        next_op_idx += 1
        self.add_dxil_op(
            "BufferLoad",
            next_op_idx,
            "BufferLoad",
            "reads from a TypedBuffer",
            "hfwi",
            "ro",
            [
                db_dxil_param(0, "$r", "", "the loaded value"),
                db_dxil_param(2, "res", "srv", "handle of TypedBuffer SRV to sample"),
                db_dxil_param(3, "i32", "index", "element index"),
                db_dxil_param(4, "i32", "wot", "coordinate"),
            ],
            counters=("tex_load",),
        )
        next_op_idx += 1
        self.add_dxil_op(
            "BufferStore",
            next_op_idx,
            "BufferStore",
            "writes to a RWTypedBuffer",
            "hfwi",
            "",
            [
                db_dxil_param(0, "v", "", ""),
                db_dxil_param(2, "res", "uav", "handle of UAV to store to"),
                db_dxil_param(3, "i32", "coord0", "coordinate in elements"),
                db_dxil_param(4, "i32", "coord1", "coordinate (unused?)"),
                db_dxil_param(5, "$o", "value0", "value"),
                db_dxil_param(6, "$o", "value1", "value"),
                db_dxil_param(7, "$o", "value2", "value"),
                db_dxil_param(8, "$o", "value3", "value"),
                db_dxil_param(9, "i8", "mask", "written value mask"),
            ],
            counters=("tex_store",),
        )
        next_op_idx += 1
        self.add_dxil_op(
            "BufferUpdateCounter",
            next_op_idx,
            "BufferUpdateCounter",
            "atomically increments/decrements the hidden 32-bit counter stored with a Count or Append UAV",
            "v",
            "",
            [
                db_dxil_param(0, "i32", "", "the new value in the buffer"),
                db_dxil_param(
                    2,
                    "res",
                    "uav",
                    "handle to a structured buffer UAV with the count or append flag",
                ),
                db_dxil_param(3, "i8", "inc", "1 to increase, 0 to decrease"),
            ],
            counters=("atomic",),
        )
        next_op_idx += 1
        self.add_dxil_op(
            "CheckAccessFullyMapped",
            next_op_idx,
            "CheckAccessFullyMapped",
            "determines whether all values from a Sample, Gather, or Load operation accessed mapped tiles in a tiled resource",
            "i",
            "ro",
            [
                db_dxil_param(
                    0,
                    "i1",
                    "",
                    "nonzero if all values accessed mapped tiles in a tiled resource",
                ),
                db_dxil_param(
                    2,
                    "u32",
                    "status",
                    "status result from the Sample, Gather or Load operation",
                ),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "GetDimensions",
            next_op_idx,
            "GetDimensions",
            "gets texture size information",
            "v",
            "ro",
            [
                db_dxil_param(0, "dims", "", "dimension information for texture"),
                db_dxil_param(2, "res", "handle", "resource handle to query"),
                db_dxil_param(3, "i32", "mipLevel", "mip level to query"),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "TextureGather",
            next_op_idx,
            "TextureGather",
            "gathers the four texels that would be used in a bi-linear filtering operation",
            "hfwi",
            "ro",
            [
                db_dxil_param(0, "$r", "", "dimension information for texture"),
                db_dxil_param(2, "res", "srv", "handle of SRV to sample"),
                db_dxil_param(3, "res", "sampler", "handle of sampler to use"),
                db_dxil_param(4, "f", "coord0", "coordinate"),
                db_dxil_param(5, "f", "coord1", "coordinate, undef for Texture1D"),
                db_dxil_param(
                    6,
                    "f",
                    "coord2",
                    "coordinate, undef for Texture1D, Texture1DArray or Texture2D",
                ),
                db_dxil_param(
                    7, "f", "coord3", "coordinate, defined only for TextureCubeArray"
                ),
                db_dxil_param(
                    8,
                    "i32",
                    "offset0",
                    "optional offset, applicable to Texture1D, Texture1DArray, and as part of offset1",
                ),
                db_dxil_param(
                    9,
                    "i32",
                    "offset1",
                    "optional offset, applicable to Texture2D, Texture2DArray, and as part of offset2",
                ),
                db_dxil_param(10, "i32", "channel", "channel to sample"),
            ],
            counters=("tex_norm",),
        )
        next_op_idx += 1
        self.add_dxil_op(
            "TextureGatherCmp",
            next_op_idx,
            "TextureGatherCmp",
            "same as TextureGather, except this instrution performs comparison on texels, similar to SampleCmp",
            "hfwi",
            "ro",
            [
                db_dxil_param(0, "$r", "", "gathered texels"),
                db_dxil_param(2, "res", "srv", "handle of SRV to sample"),
                db_dxil_param(3, "res", "sampler", "handle of sampler to use"),
                db_dxil_param(4, "f", "coord0", "coordinate"),
                db_dxil_param(5, "f", "coord1", "coordinate, undef for Texture1D"),
                db_dxil_param(
                    6,
                    "f",
                    "coord2",
                    "coordinate, undef for Texture1D, Texture1DArray or Texture2D",
                ),
                db_dxil_param(
                    7, "f", "coord3", "coordinate, defined only for TextureCubeArray"
                ),
                db_dxil_param(
                    8,
                    "i32",
                    "offset0",
                    "optional offset, applicable to Texture1D, Texture1DArray, and as part of offset1",
                ),
                db_dxil_param(
                    9,
                    "i32",
                    "offset1",
                    "optional offset, applicable to Texture2D, Texture2DArray, and as part of offset2",
                ),
                db_dxil_param(10, "i32", "channel", "channel to sample"),
                db_dxil_param(11, "f", "compareValue", "value to compare with"),
            ],
            counters=("tex_cmp",),
        )
        next_op_idx += 1

        self.add_dxil_op(
            "Texture2DMSGetSamplePosition",
            next_op_idx,
            "Texture2DMSGetSamplePosition",
            "gets the position of the specified sample",
            "v",
            "ro",
            [
                db_dxil_param(0, "SamplePos", "", "sample position"),
                db_dxil_param(2, "res", "srv", "handle of SRV to sample"),
                db_dxil_param(3, "i32", "index", "zero-based sample index"),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "RenderTargetGetSamplePosition",
            next_op_idx,
            "RenderTargetGetSamplePosition",
            "gets the position of the specified sample",
            "v",
            "ro",
            [
                db_dxil_param(0, "SamplePos", "", "sample position"),
                db_dxil_param(2, "i32", "index", "zero-based sample index"),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "RenderTargetGetSampleCount",
            next_op_idx,
            "RenderTargetGetSampleCount",
            "gets the number of samples for a render target",
            "v",
            "ro",
            [
                db_dxil_param(
                    0, "u32", "", "number of sampling locations for a render target"
                )
            ],
        )
        next_op_idx += 1

        # Atomics. Note that on TGSM, atomics are performed with LLVM instructions.
        self.add_dxil_op(
            "AtomicBinOp",
            next_op_idx,
            "AtomicBinOp",
            "performs an atomic operation on two operands",
            "li",
            "",
            [
                db_dxil_param(
                    0, "$o", "", "the original value in the location updated"
                ),
                db_dxil_param(2, "res", "handle", "typed int or uint UAV handle"),
                db_dxil_param(
                    3,
                    "i32",
                    "atomicOp",
                    "atomic operation as per DXIL::AtomicBinOpCode",
                ),
                db_dxil_param(4, "i32", "offset0", "offset in elements"),
                db_dxil_param(5, "i32", "offset1", "offset"),
                db_dxil_param(6, "i32", "offset2", "offset"),
                db_dxil_param(7, "$o", "newValue", "new value"),
            ],
            counters=("atomic",),
        )
        next_op_idx += 1
        self.add_dxil_op(
            "AtomicCompareExchange",
            next_op_idx,
            "AtomicCompareExchange",
            "atomic compare and exchange to memory",
            "li",
            "",
            [
                db_dxil_param(
                    0, "$o", "", "the original value in the location updated"
                ),
                db_dxil_param(2, "res", "handle", "typed int or uint UAV handle"),
                db_dxil_param(3, "i32", "offset0", "offset in elements"),
                db_dxil_param(4, "i32", "offset1", "offset"),
                db_dxil_param(5, "i32", "offset2", "offset"),
                db_dxil_param(6, "$o", "compareValue", "value to compare for exchange"),
                db_dxil_param(7, "$o", "newValue", "new value"),
            ],
            counters=("atomic",),
        )
        next_op_idx += 1

        # Synchronization.
        self.add_dxil_op(
            "Barrier",
            next_op_idx,
            "Barrier",
            "inserts a memory barrier in the shader",
            "v",
            "nd",
            [
                retvoid_param,
                db_dxil_param(
                    2,
                    "i32",
                    "barrierMode",
                    "a mask of DXIL::BarrierMode values",
                    is_const=True,
                ),
            ],
            counters=("barrier",),
        )
        next_op_idx += 1

        # Pixel shader
        self.add_dxil_op(
            "CalculateLOD",
            next_op_idx,
            "CalculateLOD",
            "calculates the level of detail",
            "f",
            "ro",
            [
                db_dxil_param(0, "f", "", "level of detail"),
                db_dxil_param(2, "res", "handle", "resource handle"),
                db_dxil_param(3, "res", "sampler", "sampler handle"),
                db_dxil_param(4, "f", "coord0", "coordinate"),
                db_dxil_param(5, "f", "coord1", "coordinate"),
                db_dxil_param(6, "f", "coord2", "coordinate"),
                db_dxil_param(
                    7,
                    "i1",
                    "clamped",
                    "1 if clampled LOD should be calculated, 0 for unclamped",
                ),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "Discard",
            next_op_idx,
            "Discard",
            "discard the current pixel",
            "v",
            "",
            [
                retvoid_param,
                db_dxil_param(
                    2, "i1", "condition", "condition for conditional discard"
                ),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "DerivCoarseX",
            next_op_idx,
            "Unary",
            "computes the rate of change of components per stamp",
            "hf",
            "rn",
            [
                db_dxil_param(
                    0,
                    "$o",
                    "",
                    "rate of change in value with regards to RenderTarget x direction",
                ),
                db_dxil_param(2, "$o", "value", "input to rate of change"),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "DerivCoarseY",
            next_op_idx,
            "Unary",
            "computes the rate of change of components per stamp",
            "hf",
            "rn",
            [
                db_dxil_param(
                    0,
                    "$o",
                    "",
                    "rate of change in value with regards to RenderTarget y direction",
                ),
                db_dxil_param(2, "$o", "value", "input to rate of change"),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "DerivFineX",
            next_op_idx,
            "Unary",
            "computes the rate of change of components per pixel",
            "hf",
            "rn",
            [
                db_dxil_param(
                    0,
                    "$o",
                    "",
                    "rate of change in value with regards to RenderTarget x direction",
                ),
                db_dxil_param(2, "$o", "value", "input to rate of change"),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "DerivFineY",
            next_op_idx,
            "Unary",
            "computes the rate of change of components per pixel",
            "hf",
            "rn",
            [
                db_dxil_param(
                    0,
                    "$o",
                    "",
                    "rate of change in value with regards to RenderTarget y direction",
                ),
                db_dxil_param(2, "$o", "value", "input to rate of change"),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "EvalSnapped",
            next_op_idx,
            "EvalSnapped",
            "evaluates an input attribute at pixel center with an offset",
            "hf",
            "rn",
            [
                db_dxil_param(0, "$o", "", "result"),
                db_dxil_param(2, "i32", "inputSigId", "input signature element ID"),
                db_dxil_param(
                    3, "i32", "inputRowIndex", "row index of an input attribute"
                ),
                db_dxil_param(
                    4, "i8", "inputColIndex", "column index of an input attribute"
                ),
                db_dxil_param(
                    5,
                    "i32",
                    "offsetX",
                    "2D offset from the pixel center using a 16x16 grid",
                ),
                db_dxil_param(
                    6,
                    "i32",
                    "offsetY",
                    "2D offset from the pixel center using a 16x16 grid",
                ),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "EvalSampleIndex",
            next_op_idx,
            "EvalSampleIndex",
            "evaluates an input attribute at a sample location",
            "hf",
            "rn",
            [
                db_dxil_param(0, "$o", "", "result"),
                db_dxil_param(2, "i32", "inputSigId", "input signature element ID"),
                db_dxil_param(
                    3, "i32", "inputRowIndex", "row index of an input attribute"
                ),
                db_dxil_param(
                    4, "i8", "inputColIndex", "column index of an input attribute"
                ),
                db_dxil_param(5, "i32", "sampleIndex", "sample location"),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "EvalCentroid",
            next_op_idx,
            "EvalCentroid",
            "evaluates an input attribute at pixel center",
            "hf",
            "rn",
            [
                db_dxil_param(0, "$o", "", "result"),
                db_dxil_param(2, "i32", "inputSigId", "input signature element ID"),
                db_dxil_param(
                    3, "i32", "inputRowIndex", "row index of an input attribute"
                ),
                db_dxil_param(
                    4, "i8", "inputColIndex", "column index of an input attribute"
                ),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "SampleIndex",
            next_op_idx,
            "SampleIndex",
            "returns the sample index in a sample-frequency pixel shader",
            "i",
            "rn",
            [db_dxil_param(0, "i32", "", "result")],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "Coverage",
            next_op_idx,
            "Coverage",
            "returns the coverage mask input in a pixel shader",
            "i",
            "rn",
            [db_dxil_param(0, "i32", "", "result")],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "InnerCoverage",
            next_op_idx,
            "InnerCoverage",
            "returns underestimated coverage input from conservative rasterization in a pixel shader",
            "i",
            "rn",
            [db_dxil_param(0, "i32", "", "result")],
        )
        next_op_idx += 1

        # Compute shader.
        self.add_dxil_op(
            "ThreadId",
            next_op_idx,
            "ThreadId",
            "reads the thread ID",
            "i",
            "rn",
            [
                db_dxil_param(0, "i32", "", "thread ID component"),
                db_dxil_param(2, "i32", "component", "component to read (x,y,z)"),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "GroupId",
            next_op_idx,
            "GroupId",
            "reads the group ID (SV_GroupID)",
            "i",
            "rn",
            [
                db_dxil_param(0, "i32", "", "group ID component"),
                db_dxil_param(2, "i32", "component", "component to read"),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "ThreadIdInGroup",
            next_op_idx,
            "ThreadIdInGroup",
            "reads the thread ID within the group (SV_GroupThreadID)",
            "i",
            "rn",
            [
                db_dxil_param(0, "i32", "", "thread ID in group component"),
                db_dxil_param(2, "i32", "component", "component to read (x,y,z)"),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "FlattenedThreadIdInGroup",
            next_op_idx,
            "FlattenedThreadIdInGroup",
            "provides a flattened index for a given thread within a given group (SV_GroupIndex)",
            "i",
            "rn",
            [db_dxil_param(0, "i32", "", "result")],
        )
        next_op_idx += 1

        # Geometry shader
        self.add_dxil_op(
            "EmitStream",
            next_op_idx,
            "EmitStream",
            "emits a vertex to a given stream",
            "v",
            "",
            [
                retvoid_param,
                db_dxil_param(2, "i8", "streamId", "target stream ID for operation"),
            ],
            counters=("gs_emit",),
        )
        next_op_idx += 1
        self.add_dxil_op(
            "CutStream",
            next_op_idx,
            "CutStream",
            "completes the current primitive topology at the specified stream",
            "v",
            "",
            [
                retvoid_param,
                db_dxil_param(2, "i8", "streamId", "target stream ID for operation"),
            ],
            counters=("gs_cut",),
        )
        next_op_idx += 1
        self.add_dxil_op(
            "EmitThenCutStream",
            next_op_idx,
            "EmitThenCutStream",
            "equivalent to an EmitStream followed by a CutStream",
            "v",
            "",
            [
                retvoid_param,
                db_dxil_param(2, "i8", "streamId", "target stream ID for operation"),
            ],
            counters=("gs_emit", "gs_cut"),
        )
        next_op_idx += 1
        self.add_dxil_op(
            "GSInstanceID",
            next_op_idx,
            "GSInstanceID",
            "GSInstanceID",
            "i",
            "rn",
            [db_dxil_param(0, "i32", "", "result")],
        )
        next_op_idx += 1

        # Double precision
        self.add_dxil_op(
            "MakeDouble",
            next_op_idx,
            "MakeDouble",
            "creates a double value",
            "d",
            "rn",
            [
                db_dxil_param(0, "d", "", "result"),
                db_dxil_param(2, "i32", "lo", "low part of double"),
                db_dxil_param(3, "i32", "hi", "high part of double"),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "SplitDouble",
            next_op_idx,
            "SplitDouble",
            "splits a double into low and high parts",
            "d",
            "rn",
            [
                db_dxil_param(0, "splitdouble", "", "result"),
                db_dxil_param(2, "d", "value", "value to split"),
            ],
        )
        next_op_idx += 1

        # Domain & Hull shader.
        self.add_dxil_op(
            "LoadOutputControlPoint",
            next_op_idx,
            "LoadOutputControlPoint",
            "LoadOutputControlPoint",
            "hfwi",
            "rn",
            [
                db_dxil_param(0, "$o", "", "result"),
                db_dxil_param(2, "i32", "inputSigId", "input signature element ID"),
                db_dxil_param(3, "i32", "row", "row, relative to the element"),
                db_dxil_param(4, "i8", "col", "column, relative to the element"),
                db_dxil_param(5, "i32", "index", "vertex/point index"),
            ],
            counters=("sig_ld",),
        )
        next_op_idx += 1
        self.add_dxil_op(
            "LoadPatchConstant",
            next_op_idx,
            "LoadPatchConstant",
            "LoadPatchConstant",
            "hfwi",
            "rn",
            [
                db_dxil_param(0, "$o", "", "result"),
                db_dxil_param(2, "i32", "inputSigId", "input signature element ID"),
                db_dxil_param(3, "i32", "row", "row, relative to the element"),
                db_dxil_param(4, "i8", "col", "column, relative to the element"),
            ],
            counters=("sig_ld",),
        )
        next_op_idx += 1

        # Domain shader.
        self.add_dxil_op(
            "DomainLocation",
            next_op_idx,
            "DomainLocation",
            "DomainLocation",
            "f",
            "rn",
            [
                db_dxil_param(0, "f", "", "result"),
                db_dxil_param(2, "i8", "component", "input", is_const=True),
            ],
        )
        next_op_idx += 1

        # Hull shader.
        self.add_dxil_op(
            "StorePatchConstant",
            next_op_idx,
            "StorePatchConstant",
            "StorePatchConstant",
            "hfwi",
            "",
            [
                retvoid_param,
                db_dxil_param(2, "i32", "outputSigID", "output signature element ID"),
                db_dxil_param(3, "i32", "row", "row, relative to the element"),
                db_dxil_param(4, "i8", "col", "column, relative to the element"),
                db_dxil_param(5, "$o", "value", "value to store"),
            ],
            counters=("sig_st",),
        )
        next_op_idx += 1
        self.add_dxil_op(
            "OutputControlPointID",
            next_op_idx,
            "OutputControlPointID",
            "OutputControlPointID",
            "i",
            "rn",
            [db_dxil_param(0, "i32", "", "result")],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "PrimitiveID",
            next_op_idx,
            "PrimitiveID",
            "PrimitiveID",
            "i",
            "rn",
            [db_dxil_param(0, "i32", "", "result")],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "CycleCounterLegacy",
            next_op_idx,
            "CycleCounterLegacy",
            "CycleCounterLegacy",
            "v",
            "",
            [db_dxil_param(0, "twoi32", "", "result")],
        )
        next_op_idx += 1

        # Add wave intrinsics.
        self.add_dxil_op(
            "WaveIsFirstLane",
            next_op_idx,
            "WaveIsFirstLane",
            "returns 1 for the first lane in the wave",
            "v",
            "",
            [db_dxil_param(0, "i1", "", "operation result")],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "WaveGetLaneIndex",
            next_op_idx,
            "WaveGetLaneIndex",
            "returns the index of the current lane in the wave",
            "v",
            "ro",
            [db_dxil_param(0, "i32", "", "operation result")],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "WaveGetLaneCount",
            next_op_idx,
            "WaveGetLaneCount",
            "returns the number of lanes in the wave",
            "v",
            "rn",
            [db_dxil_param(0, "i32", "", "operation result")],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "WaveAnyTrue",
            next_op_idx,
            "WaveAnyTrue",
            "returns 1 if any of the lane evaluates the value to true",
            "v",
            "",
            [
                db_dxil_param(0, "i1", "", "operation result"),
                db_dxil_param(2, "i1", "cond", "condition to test"),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "WaveAllTrue",
            next_op_idx,
            "WaveAllTrue",
            "returns 1 if all the lanes evaluate the value to true",
            "v",
            "",
            [
                db_dxil_param(0, "i1", "", "operation result"),
                db_dxil_param(2, "i1", "cond", "condition to test"),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "WaveActiveAllEqual",
            next_op_idx,
            "WaveActiveAllEqual",
            "returns 1 if all the lanes have the same value",
            "hfd18wil",
            "",
            [
                db_dxil_param(0, "i1", "", "operation result"),
                db_dxil_param(2, "$o", "value", "value to compare"),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "WaveActiveBallot",
            next_op_idx,
            "WaveActiveBallot",
            "returns a struct with a bit set for each lane where the condition is true",
            "v",
            "",
            [
                db_dxil_param(0, "fouri32", "", "operation result"),
                db_dxil_param(2, "i1", "cond", "condition to ballot on"),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "WaveReadLaneAt",
            next_op_idx,
            "WaveReadLaneAt",
            "returns the value from the specified lane",
            "hfd18wil",
            "",
            [
                db_dxil_param(0, "$o", "", "operation result"),
                db_dxil_param(2, "$o", "value", "value to read"),
                db_dxil_param(3, "i32", "lane", "lane index"),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "WaveReadLaneFirst",
            next_op_idx,
            "WaveReadLaneFirst",
            "returns the value from the first lane",
            "hfd18wil",
            "",
            [
                db_dxil_param(0, "$o", "", "operation result"),
                db_dxil_param(2, "$o", "value", "value to read"),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "WaveActiveOp",
            next_op_idx,
            "WaveActiveOp",
            "returns the result the operation across waves",
            "hfd18wil",
            "",
            [
                db_dxil_param(0, "$o", "", "operation result"),
                db_dxil_param(2, "$o", "value", "input value"),
                db_dxil_param(
                    3,
                    "i8",
                    "op",
                    "kind of operation to perform",
                    enum_name="WaveOpKind",
                    is_const=True,
                ),
                db_dxil_param(
                    4,
                    "i8",
                    "sop",
                    "sign of operands",
                    enum_name="SignedOpKind",
                    is_const=True,
                ),
            ],
        )
        next_op_idx += 1
        self.add_enum_type(
            "SignedOpKind",
            "Sign vs. unsigned operands for operation",
            [
                (0, "Signed", "signed integer or floating-point operands"),
                (1, "Unsigned", "unsigned integer operands"),
            ],
        )
        self.add_enum_type(
            "WaveOpKind",
            "Kind of cross-lane operation",
            [
                (0, "Sum", "sum of values"),
                (1, "Product", "product of values"),
                (2, "Min", "minimum value"),
                (3, "Max", "maximum value"),
            ],
        )
        self.add_dxil_op(
            "WaveActiveBit",
            next_op_idx,
            "WaveActiveBit",
            "returns the result of the operation across all lanes",
            "8wil",
            "",
            [
                db_dxil_param(0, "$o", "", "operation result"),
                db_dxil_param(2, "$o", "value", "input value"),
                db_dxil_param(
                    3,
                    "i8",
                    "op",
                    "kind of operation to perform",
                    enum_name="WaveBitOpKind",
                    is_const=True,
                ),
            ],
        )
        next_op_idx += 1
        self.add_enum_type(
            "WaveBitOpKind",
            "Kind of bitwise cross-lane operation",
            [
                (0, "And", "bitwise and of values"),
                (1, "Or", "bitwise or of values"),
                (2, "Xor", "bitwise xor of values"),
            ],
        )
        self.add_dxil_op(
            "WavePrefixOp",
            next_op_idx,
            "WavePrefixOp",
            "returns the result of the operation on prior lanes",
            "hfd8wil",
            "",
            [
                db_dxil_param(0, "$o", "", "operation result"),
                db_dxil_param(2, "$o", "value", "input value"),
                db_dxil_param(
                    3,
                    "i8",
                    "op",
                    "0=sum,1=product",
                    enum_name="WaveOpKind",
                    is_const=True,
                ),
                db_dxil_param(
                    4,
                    "i8",
                    "sop",
                    "sign of operands",
                    enum_name="SignedOpKind",
                    is_const=True,
                ),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "QuadReadLaneAt",
            next_op_idx,
            "QuadReadLaneAt",
            "reads from a lane in the quad",
            "hfd18wil",
            "",
            [
                db_dxil_param(0, "$o", "", "operation result"),
                db_dxil_param(2, "$o", "value", "value to read"),
                db_dxil_param(
                    3, "u32", "quadLane", "lane to read from (0-4)", max_value=3
                ),
            ],
        )
        next_op_idx += 1
        self.add_enum_type(
            "QuadOpKind",
            "Kind of quad-level operation",
            [
                (
                    0,
                    "ReadAcrossX",
                    "returns the value from the other lane in the quad in the horizontal direction",
                ),
                (
                    1,
                    "ReadAcrossY",
                    "returns the value from the other lane in the quad in the vertical direction",
                ),
                (
                    2,
                    "ReadAcrossDiagonal",
                    "returns the value from the lane across the quad in horizontal and vertical direction",
                ),
            ],
        )
        self.add_dxil_op(
            "QuadOp",
            next_op_idx,
            "QuadOp",
            "returns the result of a quad-level operation",
            "hfd8wil",
            "",
            [
                db_dxil_param(0, "$o", "", "operation result"),
                db_dxil_param(2, "$o", "value", "value for operation"),
                db_dxil_param(
                    3, "i8", "op", "operation", enum_name="QuadOpKind", is_const=True
                ),
            ],
        )
        next_op_idx += 1

        # Add bitcasts
        self.add_dxil_op(
            "BitcastI16toF16",
            next_op_idx,
            "BitcastI16toF16",
            "bitcast between different sizes",
            "v",
            "rn",
            [
                db_dxil_param(0, "h", "", "operation result"),
                db_dxil_param(2, "i16", "value", "input value"),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "BitcastF16toI16",
            next_op_idx,
            "BitcastF16toI16",
            "bitcast between different sizes",
            "v",
            "rn",
            [
                db_dxil_param(0, "i16", "", "operation result"),
                db_dxil_param(2, "h", "value", "input value"),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "BitcastI32toF32",
            next_op_idx,
            "BitcastI32toF32",
            "bitcast between different sizes",
            "v",
            "rn",
            [
                db_dxil_param(0, "f", "", "operation result"),
                db_dxil_param(2, "i32", "value", "input value"),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "BitcastF32toI32",
            next_op_idx,
            "BitcastF32toI32",
            "bitcast between different sizes",
            "v",
            "rn",
            [
                db_dxil_param(0, "i32", "", "operation result"),
                db_dxil_param(2, "f", "value", "input value"),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "BitcastI64toF64",
            next_op_idx,
            "BitcastI64toF64",
            "bitcast between different sizes",
            "v",
            "rn",
            [
                db_dxil_param(0, "d", "", "operation result"),
                db_dxil_param(2, "i64", "value", "input value"),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "BitcastF64toI64",
            next_op_idx,
            "BitcastF64toI64",
            "bitcast between different sizes",
            "v",
            "rn",
            [
                db_dxil_param(0, "i64", "", "operation result"),
                db_dxil_param(2, "d", "value", "input value"),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "LegacyF32ToF16",
            next_op_idx,
            "LegacyF32ToF16",
            "legacy fuction to convert float (f32) to half (f16) (this is not related to min-precision)",
            "v",
            "rn",
            [
                db_dxil_param(
                    0, "i32", "", "low 16 bits - half value, high 16 bits - zeroes"
                ),
                db_dxil_param(2, "f", "value", "float value to convert"),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "LegacyF16ToF32",
            next_op_idx,
            "LegacyF16ToF32",
            "legacy fuction to convert half (f16) to float (f32) (this is not related to min-precision)",
            "v",
            "rn",
            [
                db_dxil_param(0, "f", "", "converted float value"),
                db_dxil_param(2, "i32", "value", "half value to convert"),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "LegacyDoubleToFloat",
            next_op_idx,
            "LegacyDoubleToFloat",
            "legacy fuction to convert double to float",
            "v",
            "rn",
            [
                db_dxil_param(0, "f", "", "float value"),
                db_dxil_param(2, "d", "value", "double value to convert"),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "LegacyDoubleToSInt32",
            next_op_idx,
            "LegacyDoubleToSInt32",
            "legacy fuction to convert double to int32",
            "v",
            "rn",
            [
                db_dxil_param(0, "i32", "", "i32 value"),
                db_dxil_param(2, "d", "value", "double value to convert"),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "LegacyDoubleToUInt32",
            next_op_idx,
            "LegacyDoubleToUInt32",
            "legacy fuction to convert double to uint32",
            "v",
            "rn",
            [
                db_dxil_param(0, "i32", "", "i32 value"),
                db_dxil_param(2, "d", "value", "double value to convert"),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "WaveAllBitCount",
            next_op_idx,
            "WaveAllOp",
            "returns the count of bits set to 1 across the wave",
            "v",
            "",
            [
                db_dxil_param(0, "i32", "", "operation result"),
                db_dxil_param(2, "i1", "value", "input value"),
            ],
        )
        next_op_idx += 1
        # WavePrefixBitCount has different signature compare to WavePrefixOp, set its opclass to WavePrefixOp is not correct.
        # It works now because WavePrefixOp and WavePrefixBitCount don't interfere on overload types.
        # Keep it unchanged for back-compat.
        self.add_dxil_op(
            "WavePrefixBitCount",
            next_op_idx,
            "WavePrefixOp",
            "returns the count of bits set to 1 on prior lanes",
            "v",
            "",
            [
                db_dxil_param(0, "i32", "", "operation result"),
                db_dxil_param(2, "i1", "value", "input value"),
            ],
        )
        next_op_idx += 1

        # End of DXIL 1.0 opcodes.
        self.set_op_count_for_version(1, 0, next_op_idx)

        self.add_dxil_op(
            "AttributeAtVertex",
            next_op_idx,
            "AttributeAtVertex",
            "returns the values of the attributes at the vertex.",
            "hfiw",
            "rn",
            [
                db_dxil_param(0, "$o", "", "result"),
                db_dxil_param(2, "i32", "inputSigId", "input signature element ID"),
                db_dxil_param(
                    3, "i32", "inputRowIndex", "row index of an input attribute"
                ),
                db_dxil_param(
                    4, "i8", "inputColIndex", "column index of an input attribute"
                ),
                db_dxil_param(5, "i8", "VertexID", "Vertex Index"),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "ViewID",
            next_op_idx,
            "ViewID",
            "returns the view index",
            "i",
            "rn",
            [db_dxil_param(0, "i32", "", "result")],
        )
        next_op_idx += 1

        # End of DXIL 1.1 opcodes.
        self.set_op_count_for_version(1, 1, next_op_idx)

        self.add_dxil_op(
            "RawBufferLoad",
            next_op_idx,
            "RawBufferLoad",
            "reads from a raw buffer and structured buffer",
            "hfwidl",
            "ro",
            [
                db_dxil_param(0, "$r", "", "the loaded value"),
                db_dxil_param(2, "res", "srv", "handle of TypedBuffer SRV to sample"),
                db_dxil_param(
                    3,
                    "i32",
                    "index",
                    "element index for StructuredBuffer, or byte offset for ByteAddressBuffer",
                ),
                db_dxil_param(
                    4,
                    "i32",
                    "elementOffset",
                    "offset into element for StructuredBuffer, or undef for ByteAddressBuffer",
                ),
                db_dxil_param(5, "i8", "mask", "loading value mask", is_const=True),
                db_dxil_param(
                    6,
                    "i32",
                    "alignment",
                    "relative load access alignment",
                    is_const=True,
                ),
            ],
            counters=("tex_load",),
        )
        next_op_idx += 1

        self.add_dxil_op(
            "RawBufferStore",
            next_op_idx,
            "RawBufferStore",
            "writes to a RWByteAddressBuffer or RWStructuredBuffer",
            "hfwidl",
            "",
            [
                db_dxil_param(0, "v", "", ""),
                db_dxil_param(2, "res", "uav", "handle of UAV to store to"),
                db_dxil_param(
                    3,
                    "i32",
                    "index",
                    "element index for StructuredBuffer, or byte offset for ByteAddressBuffer",
                ),
                db_dxil_param(
                    4,
                    "i32",
                    "elementOffset",
                    "offset into element for StructuredBuffer, or undef for ByteAddressBuffer",
                ),
                db_dxil_param(5, "$o", "value0", "value"),
                db_dxil_param(6, "$o", "value1", "value"),
                db_dxil_param(7, "$o", "value2", "value"),
                db_dxil_param(8, "$o", "value3", "value"),
                db_dxil_param(
                    9,
                    "i8",
                    "mask",
                    "mask of contiguous components stored starting at first component (valid: 1, 3, 7, 15)",
                    is_const=True,
                ),
                db_dxil_param(
                    10,
                    "i32",
                    "alignment",
                    "relative store access alignment",
                    is_const=True,
                ),
            ],
            counters=("tex_store",),
        )
        next_op_idx += 1

        # End of DXIL 1.2 opcodes.
        self.set_op_count_for_version(1, 2, next_op_idx)
        assert next_op_idx == 141, (
            "next operation index is %d rather than 141 and thus opcodes are broken"
            % next_op_idx
        )

        self.add_dxil_op(
            "InstanceID",
            next_op_idx,
            "InstanceID",
            "The user-provided InstanceID on the bottom-level acceleration structure instance within the top-level structure",
            "i",
            "rn",
            [db_dxil_param(0, "i32", "", "result")],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "InstanceIndex",
            next_op_idx,
            "InstanceIndex",
            "The autogenerated index of the current instance in the top-level structure",
            "i",
            "rn",
            [db_dxil_param(0, "i32", "", "result")],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "HitKind",
            next_op_idx,
            "HitKind",
            "Returns the value passed as HitKind in ReportIntersection().  If intersection was reported by fixed-function triangle intersection, HitKind will be one of HIT_KIND_TRIANGLE_FRONT_FACE or HIT_KIND_TRIANGLE_BACK_FACE.",
            "i",
            "rn",
            [db_dxil_param(0, "i32", "", "result")],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "RayFlags",
            next_op_idx,
            "RayFlags",
            "uint containing the current ray flags.",
            "i",
            "rn",
            [db_dxil_param(0, "i32", "", "result")],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "DispatchRaysIndex",
            next_op_idx,
            "DispatchRaysIndex",
            "The current x and y location within the Width and Height",
            "i",
            "rn",
            [
                db_dxil_param(0, "i32", "", "result"),
                db_dxil_param(2, "i8", "col", "column, relative to the element"),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "DispatchRaysDimensions",
            next_op_idx,
            "DispatchRaysDimensions",
            "The Width and Height values from the D3D12_DISPATCH_RAYS_DESC structure provided to the originating DispatchRays() call.",
            "i",
            "rn",
            [
                db_dxil_param(0, "i32", "", "result"),
                db_dxil_param(2, "i8", "col", "column, relative to the element"),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "WorldRayOrigin",
            next_op_idx,
            "WorldRayOrigin",
            "The world-space origin for the current ray.",
            "f",
            "rn",
            [
                db_dxil_param(0, "f", "", "result"),
                db_dxil_param(2, "i8", "col", "column, relative to the element"),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "WorldRayDirection",
            next_op_idx,
            "WorldRayDirection",
            "The world-space direction for the current ray.",
            "f",
            "rn",
            [
                db_dxil_param(0, "f", "", "result"),
                db_dxil_param(2, "i8", "col", "column, relative to the element"),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "ObjectRayOrigin",
            next_op_idx,
            "ObjectRayOrigin",
            "Object-space origin for the current ray.",
            "f",
            "rn",
            [
                db_dxil_param(0, "f", "", "result"),
                db_dxil_param(2, "i8", "col", "column, relative to the element"),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "ObjectRayDirection",
            next_op_idx,
            "ObjectRayDirection",
            "Object-space direction for the current ray.",
            "f",
            "rn",
            [
                db_dxil_param(0, "f", "", "result"),
                db_dxil_param(2, "i8", "col", "column, relative to the element"),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "ObjectToWorld",
            next_op_idx,
            "ObjectToWorld",
            "Matrix for transforming from object-space to world-space.",
            "f",
            "rn",
            [
                db_dxil_param(0, "f", "", "result"),
                db_dxil_param(2, "i32", "row", "row, relative to the element"),
                db_dxil_param(3, "i8", "col", "column, relative to the element"),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "WorldToObject",
            next_op_idx,
            "WorldToObject",
            "Matrix for transforming from world-space to object-space.",
            "f",
            "rn",
            [
                db_dxil_param(0, "f", "", "result"),
                db_dxil_param(2, "i32", "row", "row, relative to the element"),
                db_dxil_param(3, "i8", "col", "column, relative to the element"),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "RayTMin",
            next_op_idx,
            "RayTMin",
            "float representing the parametric starting point for the ray.",
            "f",
            "rn",
            [db_dxil_param(0, "f", "", "result")],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "RayTCurrent",
            next_op_idx,
            "RayTCurrent",
            "float representing the current parametric ending point for the ray",
            "f",
            "ro",
            [db_dxil_param(0, "f", "", "result")],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "IgnoreHit",
            next_op_idx,
            "IgnoreHit",
            "Used in an any hit shader to reject an intersection and terminate the shader",
            "v",
            "nr",
            [db_dxil_param(0, "v", "", "")],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "AcceptHitAndEndSearch",
            next_op_idx,
            "AcceptHitAndEndSearch",
            "Used in an any hit shader to abort the ray query and the intersection shader (if any). The current hit is committed and execution passes to the closest hit shader with the closest hit recorded so far",
            "v",
            "nr",
            [db_dxil_param(0, "v", "", "")],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "TraceRay",
            next_op_idx,
            "TraceRay",
            "initiates raytrace",
            "u",
            "",
            [
                db_dxil_param(0, "v", "", ""),
                db_dxil_param(
                    2,
                    "res",
                    "AccelerationStructure",
                    "Top-level acceleration structure to use",
                ),
                db_dxil_param(3, "i32", "RayFlags", "Valid combination of Ray_flags"),
                db_dxil_param(
                    4,
                    "i32",
                    "InstanceInclusionMask",
                    "Bottom 8 bits of InstanceInclusionMask are used to include/rejectgeometry instances based on the InstanceMask in each instance: if(!((InstanceInclusionMask & InstanceMask) & 0xff)) { ignore intersection }",
                ),
                db_dxil_param(
                    5,
                    "i32",
                    "RayContributionToHitGroupIndex",
                    "Offset to add into Addressing calculations within shader tables for hit group indexing.  Only the bottom 4 bits of this value are used",
                ),
                db_dxil_param(
                    6,
                    "i32",
                    "MultiplierForGeometryContributionToShaderIndex",
                    "Stride to multiply by per-geometry GeometryContributionToHitGroupIndex in Addressing calculations within shader tables for hit group indexing.  Only the bottom 4 bits of this value are used",
                ),
                db_dxil_param(
                    7,
                    "i32",
                    "MissShaderIndex",
                    "Miss shader index in Addressing calculations within shader tables.  Only the bottom 16 bits of this value are used",
                ),
                db_dxil_param(8, "f", "Origin_X", "Origin x of the ray"),
                db_dxil_param(9, "f", "Origin_Y", "Origin y of the ray"),
                db_dxil_param(10, "f", "Origin_Z", "Origin z of the ray"),
                db_dxil_param(11, "f", "TMin", "Tmin of the ray"),
                db_dxil_param(12, "f", "Direction_X", "Direction x of the ray"),
                db_dxil_param(13, "f", "Direction_Y", "Direction y of the ray"),
                db_dxil_param(14, "f", "Direction_Z", "Direction z of the ray"),
                db_dxil_param(15, "f", "TMax", "Tmax of the ray"),
                db_dxil_param(
                    16,
                    "udt",
                    "payload",
                    "User-defined intersection attribute structure",
                ),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "ReportHit",
            next_op_idx,
            "ReportHit",
            "returns true if hit was accepted",
            "u",
            "",
            [
                db_dxil_param(0, "i1", "", "result"),
                db_dxil_param(
                    2, "f", "THit", "parametric distance of the intersection"
                ),
                db_dxil_param(
                    3,
                    "i32",
                    "HitKind",
                    "User-specified value in range of 0-127 to identify the type of hit. Read by any_hit or closes_hit shaders with HitKind()",
                ),
                db_dxil_param(
                    4,
                    "udt",
                    "Attributes",
                    "User-defined intersection attribute structure",
                ),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "CallShader",
            next_op_idx,
            "CallShader",
            "Call a shader in the callable shader table supplied through the DispatchRays() API",
            "u",
            "",
            [
                db_dxil_param(0, "v", "", "result"),
                db_dxil_param(
                    2,
                    "i32",
                    "ShaderIndex",
                    "Provides index into the callable shader table supplied through the DispatchRays() API",
                ),
                db_dxil_param(
                    3,
                    "udt",
                    "Parameter",
                    "User-defined parameters to pass to the callable shader,This parameter structure must match the parameter structure used in the callable shader pointed to in the shader table",
                ),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "CreateHandleForLib",
            next_op_idx,
            "CreateHandleForLib",
            "create resource handle from resource struct for library",
            "o",
            "ro",
            [
                db_dxil_param(0, "res", "", "result"),
                db_dxil_param(2, "obj", "Resource", "resource to create the handle"),
            ],
        )
        next_op_idx += 1

        # Maps to PrimitiveIndex() intrinsics for raytracing (same meaning as PrimitiveID)
        self.add_dxil_op(
            "PrimitiveIndex",
            next_op_idx,
            "PrimitiveIndex",
            "PrimitiveIndex for raytracing shaders",
            "i",
            "rn",
            [db_dxil_param(0, "i32", "", "result")],
        )
        next_op_idx += 1

        # End of DXIL 1.3 opcodes.
        self.set_op_count_for_version(1, 3, next_op_idx)
        assert next_op_idx == 162, (
            "next operation index is %d rather than 162 and thus opcodes are broken"
            % next_op_idx
        )

        self.add_dxil_op(
            "Dot2AddHalf",
            next_op_idx,
            "Dot2AddHalf",
            "2D half dot product with accumulate to float",
            "f",
            "rn",
            [
                db_dxil_param(0, "$o", "", "accumulated result"),
                db_dxil_param(2, "$o", "acc", "input accumulator"),
                db_dxil_param(3, "h", "ax", "the first component of the first vector"),
                db_dxil_param(4, "h", "ay", "the second component of the first vector"),
                db_dxil_param(5, "h", "bx", "the first component of the second vector"),
                db_dxil_param(
                    6, "h", "by", "the second component of the second vector"
                ),
            ],
            counters=("floats",),
        )
        next_op_idx += 1

        self.add_dxil_op(
            "Dot4AddI8Packed",
            next_op_idx,
            "Dot4AddPacked",
            "signed dot product of 4 x i8 vectors packed into i32, with accumulate to i32",
            "i",
            "rn",
            [
                db_dxil_param(0, "i32", "", "accumulated result"),
                db_dxil_param(2, "i32", "acc", "input accumulator"),
                db_dxil_param(3, "i32", "a", "first packed 4 x i8 for dot product"),
                db_dxil_param(4, "i32", "b", "second packed 4 x i8 for dot product"),
            ],
            counters=("ints",),
        )
        next_op_idx += 1

        self.add_dxil_op(
            "Dot4AddU8Packed",
            next_op_idx,
            "Dot4AddPacked",
            "unsigned dot product of 4 x u8 vectors packed into i32, with accumulate to i32",
            "i",
            "rn",
            [
                db_dxil_param(0, "i32", "", "accumulated result"),
                db_dxil_param(2, "i32", "acc", "input accumulator"),
                db_dxil_param(3, "i32", "a", "first packed 4 x u8 for dot product"),
                db_dxil_param(4, "i32", "b", "second packed 4 x u8 for dot product"),
            ],
            counters=("uints",),
        )
        next_op_idx += 1

        # End of DXIL 1.4 opcodes.
        self.set_op_count_for_version(1, 4, next_op_idx)
        assert next_op_idx == 165, (
            "next operation index is %d rather than 165 and thus opcodes are broken"
            % next_op_idx
        )

        self.add_dxil_op(
            "WaveMatch",
            next_op_idx,
            "WaveMatch",
            "returns the bitmask of active lanes that have the same value",
            "hfd8wil",
            "",
            [
                db_dxil_param(0, "fouri32", "", "operation result"),
                db_dxil_param(2, "$o", "value", "input value"),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "WaveMultiPrefixOp",
            next_op_idx,
            "WaveMultiPrefixOp",
            "returns the result of the operation on groups of lanes identified by a bitmask",
            "hfd8wil",
            "",
            [
                db_dxil_param(0, "$o", "", "operation result"),
                db_dxil_param(2, "$o", "value", "input value"),
                db_dxil_param(3, "i32", "mask0", "mask 0"),
                db_dxil_param(4, "i32", "mask1", "mask 1"),
                db_dxil_param(5, "i32", "mask2", "mask 2"),
                db_dxil_param(6, "i32", "mask3", "mask 3"),
                db_dxil_param(
                    7,
                    "i8",
                    "op",
                    "operation",
                    enum_name="WaveMultiPrefixOpKind",
                    is_const=True,
                ),
                db_dxil_param(
                    8,
                    "i8",
                    "sop",
                    "sign of operands",
                    enum_name="SignedOpKind",
                    is_const=True,
                ),
            ],
        )
        next_op_idx += 1
        self.add_enum_type(
            "WaveMultiPrefixOpKind",
            "Kind of cross-lane for multi-prefix operation",
            [
                (0, "Sum", "sum of values"),
                (1, "And", "bitwise and of values"),
                (2, "Or", "bitwise or of values"),
                (3, "Xor", "bitwise xor of values"),
                (4, "Product", "product of values"),
            ],
        )

        self.add_dxil_op(
            "WaveMultiPrefixBitCount",
            next_op_idx,
            "WaveMultiPrefixBitCount",
            "returns the count of bits set to 1 on groups of lanes identified by a bitmask",
            "v",
            "",
            [
                db_dxil_param(0, "i32", "", "operation result"),
                db_dxil_param(2, "i1", "value", "input value"),
                db_dxil_param(3, "i32", "mask0", "mask 0"),
                db_dxil_param(4, "i32", "mask1", "mask 1"),
                db_dxil_param(5, "i32", "mask2", "mask 2"),
                db_dxil_param(6, "i32", "mask3", "mask 3"),
            ],
        )
        next_op_idx += 1

        # Mesh Shader
        self.add_dxil_op(
            "SetMeshOutputCounts",
            next_op_idx,
            "SetMeshOutputCounts",
            "Mesh shader intrinsic SetMeshOutputCounts",
            "v",
            "",
            [
                retvoid_param,
                db_dxil_param(2, "i32", "numVertices", "number of output vertices"),
                db_dxil_param(3, "i32", "numPrimitives", "number of output primitives"),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "EmitIndices",
            next_op_idx,
            "EmitIndices",
            "emit a primitive's vertex indices in a mesh shader",
            "v",
            "",
            [
                retvoid_param,
                db_dxil_param(2, "u32", "PrimitiveIndex", "a primitive's index"),
                db_dxil_param(
                    3, "u32", "VertexIndex0", "a primitive's first vertex index"
                ),
                db_dxil_param(
                    4, "u32", "VertexIndex1", "a primitive's second vertex index"
                ),
                db_dxil_param(
                    5, "u32", "VertexIndex2", "a primitive's third vertex index"
                ),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "GetMeshPayload",
            next_op_idx,
            "GetMeshPayload",
            "get the mesh payload which is from amplification shader",
            "u",
            "ro",
            [db_dxil_param(0, "$o", "", "mesh payload result")],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "StoreVertexOutput",
            next_op_idx,
            "StoreVertexOutput",
            "stores the value to mesh shader vertex output",
            "hfwi",
            "",
            [
                retvoid_param,
                db_dxil_param(
                    2, "u32", "outputSigId", "vertex output signature element ID"
                ),
                db_dxil_param(3, "u32", "rowIndex", "row index relative to element"),
                db_dxil_param(4, "u8", "colIndex", "column index relative to element"),
                db_dxil_param(5, "$o", "value", "value to store"),
                db_dxil_param(6, "u32", "vertexIndex", "vertex index"),
            ],
            counters=("sig_st",),
        )
        next_op_idx += 1
        self.add_dxil_op(
            "StorePrimitiveOutput",
            next_op_idx,
            "StorePrimitiveOutput",
            "stores the value to mesh shader primitive output",
            "hfwi",
            "",
            [
                retvoid_param,
                db_dxil_param(
                    2, "u32", "outputSigId", "primitive output signature element ID"
                ),
                db_dxil_param(3, "u32", "rowIndex", "row index relative to element"),
                db_dxil_param(4, "u8", "colIndex", "column index relative to element"),
                db_dxil_param(5, "$o", "value", "value to store"),
                db_dxil_param(6, "u32", "primitiveIndex", "primitive index"),
            ],
            counters=("sig_st",),
        )
        next_op_idx += 1

        # Amplification Shader
        self.add_dxil_op(
            "DispatchMesh",
            next_op_idx,
            "DispatchMesh",
            "Amplification shader intrinsic DispatchMesh",
            "u",
            "",
            [
                retvoid_param,
                db_dxil_param(2, "i32", "threadGroupCountX", "thread group count x"),
                db_dxil_param(3, "i32", "threadGroupCountY", "thread group count y"),
                db_dxil_param(4, "i32", "threadGroupCountZ", "thread group count z"),
                db_dxil_param(5, "$o", "payload", "payload"),
            ],
        )
        next_op_idx += 1

        # Sampler feedback
        self.add_dxil_op(
            "WriteSamplerFeedback",
            next_op_idx,
            "WriteSamplerFeedback",
            "updates a feedback texture for a sampling operation",
            "v",
            "",
            [
                db_dxil_param(0, "v", "", ""),
                db_dxil_param(
                    2, "res", "feedbackTex", "handle of feedback texture UAV"
                ),
                db_dxil_param(3, "res", "sampledTex", "handled of sampled texture SRV"),
                db_dxil_param(4, "res", "sampler", "handle of sampler"),
                db_dxil_param(5, "f", "c0", "coordinate c0"),
                db_dxil_param(6, "f", "c1", "coordinate c1"),
                db_dxil_param(7, "f", "c2", "coordinate c2"),
                db_dxil_param(8, "f", "c3", "coordinate c3"),
                db_dxil_param(9, "f", "clamp", "clamp"),
            ],
            counters=("tex_store",),
        )
        next_op_idx += 1
        self.add_dxil_op(
            "WriteSamplerFeedbackBias",
            next_op_idx,
            "WriteSamplerFeedbackBias",
            "updates a feedback texture for a sampling operation with a bias on the mipmap level",
            "v",
            "",
            [
                db_dxil_param(0, "v", "", ""),
                db_dxil_param(
                    2, "res", "feedbackTex", "handle of feedback texture UAV"
                ),
                db_dxil_param(3, "res", "sampledTex", "handled of sampled texture SRV"),
                db_dxil_param(4, "res", "sampler", "handle of sampler"),
                db_dxil_param(5, "f", "c0", "coordinate c0"),
                db_dxil_param(6, "f", "c1", "coordinate c1"),
                db_dxil_param(7, "f", "c2", "coordinate c2"),
                db_dxil_param(8, "f", "c3", "coordinate c3"),
                db_dxil_param(9, "f", "bias", "bias in [-16.f,15.99f]"),
                db_dxil_param(10, "f", "clamp", "clamp"),
            ],
            counters=("tex_store",),
        )
        next_op_idx += 1
        self.add_dxil_op(
            "WriteSamplerFeedbackLevel",
            next_op_idx,
            "WriteSamplerFeedbackLevel",
            "updates a feedback texture for a sampling operation with a mipmap-level offset",
            "v",
            "",
            [
                db_dxil_param(0, "v", "", ""),
                db_dxil_param(
                    2, "res", "feedbackTex", "handle of feedback texture UAV"
                ),
                db_dxil_param(3, "res", "sampledTex", "handled of sampled texture SRV"),
                db_dxil_param(4, "res", "sampler", "handle of sampler"),
                db_dxil_param(5, "f", "c0", "coordinate c0"),
                db_dxil_param(6, "f", "c1", "coordinate c1"),
                db_dxil_param(7, "f", "c2", "coordinate c2"),
                db_dxil_param(8, "f", "c3", "coordinate c3"),
                db_dxil_param(9, "f", "lod", "LOD"),
            ],
            counters=("tex_store",),
        )
        next_op_idx += 1
        self.add_dxil_op(
            "WriteSamplerFeedbackGrad",
            next_op_idx,
            "WriteSamplerFeedbackGrad",
            "updates a feedback texture for a sampling operation with explicit gradients",
            "v",
            "",
            [
                db_dxil_param(0, "v", "", ""),
                db_dxil_param(
                    2, "res", "feedbackTex", "handle of feedback texture UAV"
                ),
                db_dxil_param(3, "res", "sampledTex", "handled of sampled texture SRV"),
                db_dxil_param(4, "res", "sampler", "handle of sampler"),
                db_dxil_param(5, "f", "c0", "coordinate c0"),
                db_dxil_param(6, "f", "c1", "coordinate c1"),
                db_dxil_param(7, "f", "c2", "coordinate c2"),
                db_dxil_param(8, "f", "c3", "coordinate c3"),
                db_dxil_param(
                    9, "f", "ddx0", "rate of change of coordinate c0 in the x direction"
                ),
                db_dxil_param(
                    10,
                    "f",
                    "ddx1",
                    "rate of change of coordinate c1 in the x direction",
                ),
                db_dxil_param(
                    11,
                    "f",
                    "ddx2",
                    "rate of change of coordinate c2 in the x direction",
                ),
                db_dxil_param(
                    12,
                    "f",
                    "ddy0",
                    "rate of change of coordinate c0 in the y direction",
                ),
                db_dxil_param(
                    13,
                    "f",
                    "ddy1",
                    "rate of change of coordinate c1 in the y direction",
                ),
                db_dxil_param(
                    14,
                    "f",
                    "ddy2",
                    "rate of change of coordinate c2 in the y direction",
                ),
                db_dxil_param(15, "f", "clamp", "clamp"),
            ],
            counters=("tex_store",),
        )
        next_op_idx += 1

        # RayQuery
        self.add_dxil_op(
            "AllocateRayQuery",
            next_op_idx,
            "AllocateRayQuery",
            "allocates space for RayQuery and return handle",
            "v",
            "",
            [
                db_dxil_param(0, "i32", "", "handle to RayQuery state"),
                db_dxil_param(
                    2,
                    "u32",
                    "constRayFlags",
                    "Valid combination of RAY_FLAGS",
                    is_const=True,
                ),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "RayQuery_TraceRayInline",
            next_op_idx,
            "RayQuery_TraceRayInline",
            "initializes RayQuery for raytrace",
            "v",
            "",
            [
                db_dxil_param(0, "v", "", ""),
                db_dxil_param(2, "i32", "rayQueryHandle", "RayQuery handle"),
                db_dxil_param(
                    3,
                    "res",
                    "accelerationStructure",
                    "Top-level acceleration structure to use",
                ),
                db_dxil_param(
                    4,
                    "i32",
                    "rayFlags",
                    "Valid combination of RAY_FLAGS, combined with constRayFlags provided to AllocateRayQuery",
                ),
                db_dxil_param(
                    5,
                    "i32",
                    "instanceInclusionMask",
                    "Bottom 8 bits of InstanceInclusionMask are used to include/rejectgeometry instances based on the InstanceMask in each instance: if(!((InstanceInclusionMask & InstanceMask) & 0xff)) { ignore intersection }",
                ),
                db_dxil_param(6, "f", "origin_X", "Origin x of the ray"),
                db_dxil_param(7, "f", "origin_Y", "Origin y of the ray"),
                db_dxil_param(8, "f", "origin_Z", "Origin z of the ray"),
                db_dxil_param(9, "f", "tMin", "Tmin of the ray"),
                db_dxil_param(10, "f", "direction_X", "Direction x of the ray"),
                db_dxil_param(11, "f", "direction_Y", "Direction y of the ray"),
                db_dxil_param(12, "f", "direction_Z", "Direction z of the ray"),
                db_dxil_param(13, "f", "tMax", "Tmax of the ray"),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "RayQuery_Proceed",
            next_op_idx,
            "RayQuery_Proceed",
            "advances a ray query",
            "1",
            "",
            [
                db_dxil_param(0, "i1", "", "operation result"),
                db_dxil_param(2, "i32", "rayQueryHandle", "RayQuery handle"),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "RayQuery_Abort",
            next_op_idx,
            "RayQuery_Abort",
            "aborts a ray query",
            "v",
            "",
            [
                db_dxil_param(0, "v", "", ""),
                db_dxil_param(2, "i32", "rayQueryHandle", "RayQuery handle"),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "RayQuery_CommitNonOpaqueTriangleHit",
            next_op_idx,
            "RayQuery_CommitNonOpaqueTriangleHit",
            "commits a non opaque triangle hit",
            "v",
            "",
            [
                db_dxil_param(0, "v", "", ""),
                db_dxil_param(2, "i32", "rayQueryHandle", "RayQuery handle"),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "RayQuery_CommitProceduralPrimitiveHit",
            next_op_idx,
            "RayQuery_CommitProceduralPrimitiveHit",
            "commits a procedural primitive hit",
            "v",
            "",
            [
                db_dxil_param(0, "v", "", ""),
                db_dxil_param(2, "i32", "rayQueryHandle", "RayQuery handle"),
                db_dxil_param(
                    3, "f", "t", "Procedural primitive hit distance (t) to commit."
                ),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "RayQuery_CommittedStatus",
            next_op_idx,
            "RayQuery_StateScalar",
            "returns uint status (COMMITTED_STATUS) of the committed hit in a ray query",
            "i",
            "ro",
            [
                db_dxil_param(0, "i32", "", "operation result"),
                db_dxil_param(2, "i32", "rayQueryHandle", "RayQuery handle"),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "RayQuery_CandidateType",
            next_op_idx,
            "RayQuery_StateScalar",
            "returns uint candidate type (CANDIDATE_TYPE) of the current hit candidate in a ray query, after Proceed() has returned true",
            "i",
            "ro",
            [
                db_dxil_param(0, "i32", "", "operation result"),
                db_dxil_param(2, "i32", "rayQueryHandle", "RayQuery handle"),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "RayQuery_CandidateObjectToWorld3x4",
            next_op_idx,
            "RayQuery_StateMatrix",
            "returns matrix for transforming from object-space to world-space for a candidate hit.",
            "f",
            "ro",
            [
                db_dxil_param(0, "f", "", "operation result"),
                db_dxil_param(2, "i32", "rayQueryHandle", "RayQuery handle"),
                db_dxil_param(3, "i32", "row", "row [0..2], relative to the element"),
                db_dxil_param(4, "i8", "col", "column [0..3], relative to the element"),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "RayQuery_CandidateWorldToObject3x4",
            next_op_idx,
            "RayQuery_StateMatrix",
            "returns matrix for transforming from world-space to object-space for a candidate hit.",
            "f",
            "ro",
            [
                db_dxil_param(0, "f", "", "operation result"),
                db_dxil_param(2, "i32", "rayQueryHandle", "RayQuery handle"),
                db_dxil_param(3, "i32", "row", "row [0..2], relative to the element"),
                db_dxil_param(4, "i8", "col", "column [0..3], relative to the element"),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "RayQuery_CommittedObjectToWorld3x4",
            next_op_idx,
            "RayQuery_StateMatrix",
            "returns matrix for transforming from object-space to world-space for a Committed hit.",
            "f",
            "ro",
            [
                db_dxil_param(0, "f", "", "operation result"),
                db_dxil_param(2, "i32", "rayQueryHandle", "RayQuery handle"),
                db_dxil_param(3, "i32", "row", "row [0..2], relative to the element"),
                db_dxil_param(4, "i8", "col", "column [0..3], relative to the element"),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "RayQuery_CommittedWorldToObject3x4",
            next_op_idx,
            "RayQuery_StateMatrix",
            "returns matrix for transforming from world-space to object-space for a Committed hit.",
            "f",
            "ro",
            [
                db_dxil_param(0, "f", "", "operation result"),
                db_dxil_param(2, "i32", "rayQueryHandle", "RayQuery handle"),
                db_dxil_param(3, "i32", "row", "row [0..2], relative to the element"),
                db_dxil_param(4, "i8", "col", "column [0..3], relative to the element"),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "RayQuery_CandidateProceduralPrimitiveNonOpaque",
            next_op_idx,
            "RayQuery_StateScalar",
            "returns if current candidate procedural primitive is non opaque",
            "1",
            "ro",
            [
                db_dxil_param(0, "i1", "", "operation result"),
                db_dxil_param(2, "i32", "rayQueryHandle", "RayQuery handle"),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "RayQuery_CandidateTriangleFrontFace",
            next_op_idx,
            "RayQuery_StateScalar",
            "returns if current candidate triangle is front facing",
            "1",
            "ro",
            [
                db_dxil_param(0, "i1", "", "operation result"),
                db_dxil_param(2, "i32", "rayQueryHandle", "RayQuery handle"),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "RayQuery_CommittedTriangleFrontFace",
            next_op_idx,
            "RayQuery_StateScalar",
            "returns if current committed triangle is front facing",
            "1",
            "ro",
            [
                db_dxil_param(0, "i1", "", "operation result"),
                db_dxil_param(2, "i32", "rayQueryHandle", "RayQuery handle"),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "RayQuery_CandidateTriangleBarycentrics",
            next_op_idx,
            "RayQuery_StateVector",
            "returns candidate triangle hit barycentrics",
            "f",
            "ro",
            [
                db_dxil_param(0, "f", "", "operation result"),
                db_dxil_param(2, "i32", "rayQueryHandle", "RayQuery handle"),
                db_dxil_param(3, "i8", "component", "component [0..2]", is_const=True),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "RayQuery_CommittedTriangleBarycentrics",
            next_op_idx,
            "RayQuery_StateVector",
            "returns committed triangle hit barycentrics",
            "f",
            "ro",
            [
                db_dxil_param(0, "f", "", "operation result"),
                db_dxil_param(2, "i32", "rayQueryHandle", "RayQuery handle"),
                db_dxil_param(3, "i8", "component", "component [0..2]", is_const=True),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "RayQuery_RayFlags",
            next_op_idx,
            "RayQuery_StateScalar",
            "returns ray flags",
            "i",
            "ro",
            [
                db_dxil_param(0, "i32", "", "operation result"),
                db_dxil_param(2, "i32", "rayQueryHandle", "RayQuery handle"),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "RayQuery_WorldRayOrigin",
            next_op_idx,
            "RayQuery_StateVector",
            "returns world ray origin",
            "f",
            "ro",
            [
                db_dxil_param(0, "f", "", "operation result"),
                db_dxil_param(2, "i32", "rayQueryHandle", "RayQuery handle"),
                db_dxil_param(3, "i8", "component", "component [0..2]", is_const=True),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "RayQuery_WorldRayDirection",
            next_op_idx,
            "RayQuery_StateVector",
            "returns world ray direction",
            "f",
            "ro",
            [
                db_dxil_param(0, "f", "", "operation result"),
                db_dxil_param(2, "i32", "rayQueryHandle", "RayQuery handle"),
                db_dxil_param(3, "i8", "component", "component [0..2]", is_const=True),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "RayQuery_RayTMin",
            next_op_idx,
            "RayQuery_StateScalar",
            "returns float representing the parametric starting point for the ray.",
            "f",
            "ro",
            [
                db_dxil_param(0, "f", "", "operation result"),
                db_dxil_param(2, "i32", "rayQueryHandle", "RayQuery handle"),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "RayQuery_CandidateTriangleRayT",
            next_op_idx,
            "RayQuery_StateScalar",
            "returns float representing the parametric point on the ray for the current candidate triangle hit.",
            "f",
            "ro",
            [
                db_dxil_param(0, "f", "", "operation result"),
                db_dxil_param(2, "i32", "rayQueryHandle", "RayQuery handle"),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "RayQuery_CommittedRayT",
            next_op_idx,
            "RayQuery_StateScalar",
            "returns float representing the parametric point on the ray for the current committed hit.",
            "f",
            "ro",
            [
                db_dxil_param(0, "f", "", "operation result"),
                db_dxil_param(2, "i32", "rayQueryHandle", "RayQuery handle"),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "RayQuery_CandidateInstanceIndex",
            next_op_idx,
            "RayQuery_StateScalar",
            "returns candidate hit instance index",
            "i",
            "ro",
            [
                db_dxil_param(0, "i32", "", "operation result"),
                db_dxil_param(2, "i32", "rayQueryHandle", "RayQuery handle"),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "RayQuery_CandidateInstanceID",
            next_op_idx,
            "RayQuery_StateScalar",
            "returns candidate hit instance ID",
            "i",
            "ro",
            [
                db_dxil_param(0, "i32", "", "operation result"),
                db_dxil_param(2, "i32", "rayQueryHandle", "RayQuery handle"),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "RayQuery_CandidateGeometryIndex",
            next_op_idx,
            "RayQuery_StateScalar",
            "returns candidate hit geometry index",
            "i",
            "ro",
            [
                db_dxil_param(0, "i32", "", "operation result"),
                db_dxil_param(2, "i32", "rayQueryHandle", "RayQuery handle"),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "RayQuery_CandidatePrimitiveIndex",
            next_op_idx,
            "RayQuery_StateScalar",
            "returns candidate hit geometry index",
            "i",
            "ro",
            [
                db_dxil_param(0, "i32", "", "operation result"),
                db_dxil_param(2, "i32", "rayQueryHandle", "RayQuery handle"),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "RayQuery_CandidateObjectRayOrigin",
            next_op_idx,
            "RayQuery_StateVector",
            "returns candidate hit object ray origin",
            "f",
            "ro",
            [
                db_dxil_param(0, "f", "", "operation result"),
                db_dxil_param(2, "i32", "rayQueryHandle", "RayQuery handle"),
                db_dxil_param(3, "i8", "component", "component [0..2]", is_const=True),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "RayQuery_CandidateObjectRayDirection",
            next_op_idx,
            "RayQuery_StateVector",
            "returns candidate object ray direction",
            "f",
            "ro",
            [
                db_dxil_param(0, "f", "", "operation result"),
                db_dxil_param(2, "i32", "rayQueryHandle", "RayQuery handle"),
                db_dxil_param(3, "i8", "component", "component [0..2]", is_const=True),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "RayQuery_CommittedInstanceIndex",
            next_op_idx,
            "RayQuery_StateScalar",
            "returns committed hit instance index",
            "i",
            "ro",
            [
                db_dxil_param(0, "i32", "", "operation result"),
                db_dxil_param(2, "i32", "rayQueryHandle", "RayQuery handle"),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "RayQuery_CommittedInstanceID",
            next_op_idx,
            "RayQuery_StateScalar",
            "returns committed hit instance ID",
            "i",
            "ro",
            [
                db_dxil_param(0, "i32", "", "operation result"),
                db_dxil_param(2, "i32", "rayQueryHandle", "RayQuery handle"),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "RayQuery_CommittedGeometryIndex",
            next_op_idx,
            "RayQuery_StateScalar",
            "returns committed hit geometry index",
            "i",
            "ro",
            [
                db_dxil_param(0, "i32", "", "operation result"),
                db_dxil_param(2, "i32", "rayQueryHandle", "RayQuery handle"),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "RayQuery_CommittedPrimitiveIndex",
            next_op_idx,
            "RayQuery_StateScalar",
            "returns committed hit geometry index",
            "i",
            "ro",
            [
                db_dxil_param(0, "i32", "", "operation result"),
                db_dxil_param(2, "i32", "rayQueryHandle", "RayQuery handle"),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "RayQuery_CommittedObjectRayOrigin",
            next_op_idx,
            "RayQuery_StateVector",
            "returns committed hit object ray origin",
            "f",
            "ro",
            [
                db_dxil_param(0, "f", "", "operation result"),
                db_dxil_param(2, "i32", "rayQueryHandle", "RayQuery handle"),
                db_dxil_param(3, "i8", "component", "component [0..2]", is_const=True),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "RayQuery_CommittedObjectRayDirection",
            next_op_idx,
            "RayQuery_StateVector",
            "returns committed object ray direction",
            "f",
            "ro",
            [
                db_dxil_param(0, "f", "", "operation result"),
                db_dxil_param(2, "i32", "rayQueryHandle", "RayQuery handle"),
                db_dxil_param(3, "i8", "component", "component [0..2]", is_const=True),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "GeometryIndex",
            next_op_idx,
            "GeometryIndex",
            "The autogenerated index of the current geometry in the bottom-level structure",
            "i",
            "rn",
            [db_dxil_param(0, "i32", "", "result")],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "RayQuery_CandidateInstanceContributionToHitGroupIndex",
            next_op_idx,
            "RayQuery_StateScalar",
            "returns candidate hit InstanceContributionToHitGroupIndex",
            "i",
            "ro",
            [
                db_dxil_param(0, "i32", "", "operation result"),
                db_dxil_param(2, "i32", "rayQueryHandle", "RayQuery handle"),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "RayQuery_CommittedInstanceContributionToHitGroupIndex",
            next_op_idx,
            "RayQuery_StateScalar",
            "returns committed hit InstanceContributionToHitGroupIndex",
            "i",
            "ro",
            [
                db_dxil_param(0, "i32", "", "operation result"),
                db_dxil_param(2, "i32", "rayQueryHandle", "RayQuery handle"),
            ],
        )
        next_op_idx += 1

        # End of DXIL 1.5 opcodes.
        self.set_op_count_for_version(1, 5, next_op_idx)
        assert next_op_idx == 216, (
            "216 is expected next operation index but encountered %d and thus opcodes are broken"
            % next_op_idx
        )

        self.add_dxil_op(
            "AnnotateHandle",
            next_op_idx,
            "AnnotateHandle",
            "annotate handle with resource properties",
            "v",
            "rn",
            [
                db_dxil_param(0, "res", "", "annotated handle"),
                db_dxil_param(2, "res", "res", "input handle"),
                db_dxil_param(
                    3,
                    "resproperty",
                    "props",
                    "details like component type, structure stride...",
                    is_const=True,
                ),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "CreateHandleFromBinding",
            next_op_idx,
            "CreateHandleFromBinding",
            "create resource handle from binding",
            "v",
            "rn",
            [
                db_dxil_param(0, "res", "", "result"),
                db_dxil_param(
                    2, "resbind", "bind", "resource binding", is_const=True
                ),  # { rangeLowerBound, rangeUpperBound, spaceID, resourceClass }
                db_dxil_param(3, "i32", "index", "index"),
                db_dxil_param(
                    4,
                    "i1",
                    "nonUniformIndex",
                    "non-uniform resource index",
                    is_const=True,
                ),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "CreateHandleFromHeap",
            next_op_idx,
            "CreateHandleFromHeap",
            "create resource handle from heap",
            "v",
            "rn",
            [
                db_dxil_param(0, "res", "", "result"),
                db_dxil_param(2, "i32", "index", "heap index"),
                db_dxil_param(
                    3,
                    "i1",
                    "samplerHeap",
                    "If samplerHeap is 1, the heap indexed is the sampler descriptor heap, otherwise it is the CBV_SRV_UAV (resource) descriptor heap",
                    is_const=True,
                ),
                db_dxil_param(
                    4,
                    "i1",
                    "nonUniformIndex",
                    "non-uniform resource index",
                    is_const=True,
                ),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "Unpack4x8",
            next_op_idx,
            "Unpack4x8",
            "unpacks 4 8-bit signed or unsigned values into int32 or int16 vector",
            "iw",
            "rn",
            [
                db_dxil_param(0, "$vec4", "", "result"),
                db_dxil_param(2, "i8", "unpackMode", "signed/unsigned"),
                db_dxil_param(3, "i32", "pk", "packed 4 x i8"),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "Pack4x8",
            next_op_idx,
            "Pack4x8",
            "packs vector of 4 signed or unsigned values into a packed datatype, drops or clamps unused bits",
            "iw",
            "rn",
            [
                db_dxil_param(0, "i32", "", "result packed 4 x i8"),
                db_dxil_param(2, "i8", "packMode", "trunc/unsigned clamp/signed clamp"),
                db_dxil_param(3, "$o", "x", "the first component of the vector"),
                db_dxil_param(4, "$o", "y", "the second component of the vector"),
                db_dxil_param(5, "$o", "z", "the third component of the vector"),
                db_dxil_param(6, "$o", "w", "the fourth component of the vector"),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "IsHelperLane",
            next_op_idx,
            "IsHelperLane",
            "returns true on helper lanes in pixel shaders",
            "1",
            "ro",
            [db_dxil_param(0, "i1", "", "result")],
        )
        next_op_idx += 1

        # End of DXIL 1.6 opcodes.
        self.set_op_count_for_version(1, 6, next_op_idx)
        assert next_op_idx == 222, (
            "222 is expected next operation index but encountered %d and thus opcodes are broken"
            % next_op_idx
        )

        self.add_enum_type(
            "QuadVoteOpKind",
            "Kind of cross-quad vote operation",
            [
                (0, "Any", "true if any condition is true in this quad"),
                (1, "All", "true if all conditions are true in this quad"),
            ],
        )
        self.add_dxil_op(
            "QuadVote",
            next_op_idx,
            "QuadVote",
            "compares boolean accross a quad",
            "1",
            "",
            [
                db_dxil_param(0, "i1", "", "result - uniform across quad"),
                db_dxil_param(2, "i1", "cond", "condition"),
                db_dxil_param(
                    3,
                    "i8",
                    "op",
                    "QuadVoteOpKind: 0=Any, 1=All",
                    enum_name="QuadVoteOpKind",
                    is_const=True,
                ),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "TextureGatherRaw",
            next_op_idx,
            "TextureGatherRaw",
            "Gather raw elements from 4 texels with no type conversions (SRV type is constrained)",
            "wil",
            "ro",
            [
                db_dxil_param(0, "$r", "", "four raw texture elements gathered"),
                db_dxil_param(
                    2, "res", "srv", "handle of type-matched SRV to gather from"
                ),
                db_dxil_param(3, "res", "sampler", "handle of sampler to use"),
                db_dxil_param(4, "f", "coord0", "coordinate"),
                db_dxil_param(5, "f", "coord1", "coordinate, undef for Texture1D"),
                db_dxil_param(
                    6,
                    "f",
                    "coord2",
                    "coordinate, undef for Texture1D, Texture1DArray or Texture2D",
                ),
                db_dxil_param(
                    7, "f", "coord3", "coordinate, defined only for TextureCubeArray"
                ),
                db_dxil_param(
                    8,
                    "i32",
                    "offset0",
                    "optional offset, applicable to Texture1D, Texture1DArray, and as part of offset1",
                ),
                db_dxil_param(
                    9,
                    "i32",
                    "offset1",
                    "optional offset, applicable to Texture2D, Texture2DArray",
                ),
            ],
            counters=("tex_norm",),
        )
        next_op_idx += 1

        self.add_dxil_op(
            "SampleCmpLevel",
            next_op_idx,
            "SampleCmpLevel",
            "samples a texture and compares a single component against the specified comparison value",
            "hf",
            "ro",
            [
                db_dxil_param(0, "$r", "", "the result of the filtered comparisons"),
                db_dxil_param(2, "res", "srv", "handle of SRV to sample"),
                db_dxil_param(3, "res", "sampler", "handle of sampler to use"),
                db_dxil_param(4, "f", "coord0", "coordinate"),
                db_dxil_param(5, "f", "coord1", "coordinate, undef for Texture1D"),
                db_dxil_param(
                    6,
                    "f",
                    "coord2",
                    "coordinate, undef for Texture1D, Texture1DArray or Texture2D",
                ),
                db_dxil_param(
                    7, "f", "coord3", "coordinate, defined only for TextureCubeArray"
                ),
                db_dxil_param(
                    8,
                    "i32",
                    "offset0",
                    "optional offset, applicable to Texture1D, Texture1DArray, and as part of offset1",
                ),
                db_dxil_param(
                    9,
                    "i32",
                    "offset1",
                    "optional offset, applicable to Texture2D, Texture2DArray, and as part of offset2",
                ),
                db_dxil_param(
                    10, "i32", "offset2", "optional offset, applicable to Texture3D"
                ),
                db_dxil_param(11, "f", "compareValue", "the value to compare with"),
                db_dxil_param(
                    12,
                    "f",
                    "lod",
                    "level of detail, biggest map if less than or equal to zero; fraction used to interpolate across levels",
                ),
            ],
            counters=("tex_cmp",),
        )
        next_op_idx += 1

        self.add_dxil_op(
            "TextureStoreSample",
            next_op_idx,
            "TextureStoreSample",
            "stores texel data at specified sample index",
            "hfwi",
            "",
            [
                db_dxil_param(0, "v", "", ""),
                db_dxil_param(
                    2, "res", "srv", "handle of Texture2DMS[Array] UAV to store to"
                ),
                db_dxil_param(3, "i32", "coord0", "coordinate"),
                db_dxil_param(4, "i32", "coord1", "coordinate"),
                db_dxil_param(5, "i32", "coord2", "coordinate"),
                db_dxil_param(6, "$o", "value0", "value"),
                db_dxil_param(7, "$o", "value1", "value"),
                db_dxil_param(8, "$o", "value2", "value"),
                db_dxil_param(9, "$o", "value3", "value"),
                db_dxil_param(10, "i8", "mask", "written value mask", is_const=True),
                db_dxil_param(11, "i32", "sampleIdx", "sample index"),
            ],
            counters=("tex_store",),
        )
        next_op_idx += 1

        # End of DXIL 1.7 opcodes.
        self.set_op_count_for_version(1, 7, next_op_idx)
        assert next_op_idx == 226, (
            "226 is expected next operation index but encountered %d and thus opcodes are broken"
            % next_op_idx
        )

        # Reserved ops
        self.add_dxil_op(
            "Reserved0",
            next_op_idx,
            "Reserved",
            "Reserved",
            "v",
            "",
            [retvoid_param],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "Reserved1",
            next_op_idx,
            "Reserved",
            "Reserved",
            "v",
            "",
            [retvoid_param],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "Reserved2",
            next_op_idx,
            "Reserved",
            "Reserved",
            "v",
            "",
            [retvoid_param],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "Reserved3",
            next_op_idx,
            "Reserved",
            "Reserved",
            "v",
            "",
            [retvoid_param],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "Reserved4",
            next_op_idx,
            "Reserved",
            "Reserved",
            "v",
            "",
            [retvoid_param],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "Reserved5",
            next_op_idx,
            "Reserved",
            "Reserved",
            "v",
            "",
            [retvoid_param],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "Reserved6",
            next_op_idx,
            "Reserved",
            "Reserved",
            "v",
            "",
            [retvoid_param],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "Reserved7",
            next_op_idx,
            "Reserved",
            "Reserved",
            "v",
            "",
            [retvoid_param],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "Reserved8",
            next_op_idx,
            "Reserved",
            "Reserved",
            "v",
            "",
            [retvoid_param],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "Reserved9",
            next_op_idx,
            "Reserved",
            "Reserved",
            "v",
            "",
            [retvoid_param],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "Reserved10",
            next_op_idx,
            "Reserved",
            "Reserved",
            "v",
            "",
            [retvoid_param],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "Reserved11",
            next_op_idx,
            "Reserved",
            "Reserved",
            "v",
            "",
            [retvoid_param],
        )
        next_op_idx += 1

        # Work Graph
        self.add_dxil_op(
            "AllocateNodeOutputRecords",
            next_op_idx,
            "AllocateNodeOutputRecords",
            "returns a handle for the output records",
            "v",
            "",
            [
                db_dxil_param(0, "noderecordhandle", "", "handle of output record"),
                db_dxil_param(2, "nodehandle", "output", "handle of node output"),
                db_dxil_param(3, "i32", "numRecords", "number of records"),
                db_dxil_param(4, "i1", "perThread", "perThread flag", is_const=True),
            ],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "GetNodeRecordPtr",
            next_op_idx,
            "GetNodeRecordPtr",
            "retrieve node input/output record pointer in address space 6",
            "u",
            "rn",
            [
                db_dxil_param(0, "$o", "", "record pointer"),
                db_dxil_param(
                    2, "noderecordhandle", "recordhandle", "handle of record"
                ),
                db_dxil_param(3, "i32", "arrayIndex", "array index"),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "IncrementOutputCount",
            next_op_idx,
            "IncrementOutputCount",
            "Select the next logical output count for an EmptyNodeOutput for the whole group or per thread.",
            "v",
            "",
            [
                retvoid_param,
                db_dxil_param(2, "nodehandle", "output", "handle of node output"),
                db_dxil_param(
                    3, "i32", "count", "value by which to increment the count"
                ),
                db_dxil_param(4, "i1", "perThread", "perThread flag", is_const=True),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "OutputComplete",
            next_op_idx,
            "OutputComplete",
            "indicates all outputs for a given records are complete",
            "v",
            "",
            [
                retvoid_param,
                db_dxil_param(2, "noderecordhandle", "output", "handle of record"),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "GetInputRecordCount",
            next_op_idx,
            "GetInputRecordCount",
            "returns the number of records that have been coalesced into the current thread group",
            "v",
            "ro",
            [
                db_dxil_param(0, "i32", "", "number of records"),
                db_dxil_param(2, "noderecordhandle", "input", "handle of input record"),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "FinishedCrossGroupSharing",
            next_op_idx,
            "FinishedCrossGroupSharing",
            "returns true if the current thread group is the last to access the input",
            "v",
            "",
            [
                db_dxil_param(
                    0,
                    "i1",
                    "",
                    "true if current thread group is last to access the input ",
                ),
                db_dxil_param(2, "noderecordhandle", "input", "handle of input record"),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "BarrierByMemoryType",
            next_op_idx,
            "BarrierByMemoryType",
            "Request a barrier for a set of memory types and/or thread group execution sync",
            "v",
            "nd",
            [
                retvoid_param,
                db_dxil_param(
                    2, "i32", "MemoryTypeFlags", "memory type flags", is_const=True
                ),
                db_dxil_param(
                    3, "i32", "SemanticFlags", "semantic flags", is_const=True
                ),
            ],
            counters=("barrier",),
        )
        next_op_idx += 1
        self.add_dxil_op(
            "BarrierByMemoryHandle",
            next_op_idx,
            "BarrierByMemoryHandle",
            "Request a barrier for just the memory used by the specified object",
            "v",
            "nd",
            [
                retvoid_param,
                db_dxil_param(2, "res", "object", "handle of object"),
                db_dxil_param(
                    3, "i32", "SemanticFlags", "semantic flags", is_const=True
                ),
            ],
            counters=("barrier",),
        )
        next_op_idx += 1
        self.add_dxil_op(
            "BarrierByNodeRecordHandle",
            next_op_idx,
            "BarrierByNodeRecordHandle",
            "Request a barrier for just the memory used by the node record",
            "v",
            "nd",
            [
                retvoid_param,
                db_dxil_param(2, "noderecordhandle", "object", "handle of object"),
                db_dxil_param(
                    3, "i32", "SemanticFlags", "semantic flags", is_const=True
                ),
            ],
            counters=("barrier",),
        )
        next_op_idx += 1
        self.add_dxil_op(
            "CreateNodeOutputHandle",
            next_op_idx,
            "createNodeOutputHandle",
            "Creates a handle to a NodeOutput",
            "v",
            "rn",
            [
                db_dxil_param(0, "nodehandle", "output", "handle of object"),
                db_dxil_param(2, "i32", "MetadataIdx", "metadata index", is_const=True),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "IndexNodeHandle",
            next_op_idx,
            "IndexNodeHandle",
            "returns the handle for the location in the output node array at the indicated index",
            "v",
            "rn",
            [
                db_dxil_param(0, "nodehandle", "output", "handle of index"),
                db_dxil_param(
                    2,
                    "nodehandle",
                    "NodeOutputHandle",
                    "Handle from CreateNodeOutputHandle",
                ),
                db_dxil_param(3, "i32", "ArrayIndex", "array index"),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "AnnotateNodeHandle",
            next_op_idx,
            "AnnotateNodeHandle",
            "annotate handle with node properties",
            "v",
            "rn",
            [
                db_dxil_param(0, "nodehandle", "", "annotated node handle"),
                db_dxil_param(2, "nodehandle", "node", "input node handle"),
                db_dxil_param(
                    3,
                    "nodeproperty",
                    "props",
                    "details like NodeIOFlags, RecordSize ...",
                    is_const=True,
                ),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "CreateNodeInputRecordHandle",
            next_op_idx,
            "CreateNodeInputRecordHandle",
            "create a handle for an InputRecord",
            "v",
            "rn",
            [
                db_dxil_param(0, "noderecordhandle", "output", "output handle"),
                db_dxil_param(2, "i32", "MetadataIdx", "metadata index", is_const=True),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "AnnotateNodeRecordHandle",
            next_op_idx,
            "AnnotateNodeRecordHandle",
            "annotate handle with node record properties",
            "v",
            "rn",
            [
                db_dxil_param(
                    0, "noderecordhandle", "", "annotated node record handle"
                ),
                db_dxil_param(
                    2, "noderecordhandle", "noderecord", "input node record handle"
                ),
                db_dxil_param(
                    3,
                    "noderecordproperty",
                    "props",
                    "details like NodeIOFlags, MaxArraySize ...",
                    is_const=True,
                ),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "NodeOutputIsValid",
            next_op_idx,
            "NodeOutputIsValid",
            "returns true if the specified output node is present in the work graph",
            "v",
            "ro",
            [
                db_dxil_param(0, "i1", "", "true if output node present"),
                db_dxil_param(2, "nodehandle", "output", "handle of output node"),
            ],
        )
        next_op_idx += 1
        self.add_dxil_op(
            "GetRemainingRecursionLevels",
            next_op_idx,
            "GetRemainingRecursionLevels",
            "returns how many levels of recursion remain",
            "v",
            "ro",
            [db_dxil_param(0, "i32", "", "number of levels of recursion remaining")],
        )
        next_op_idx += 1

        # Comparison Sampling
        self.add_dxil_op(
            "SampleCmpGrad",
            next_op_idx,
            "SampleCmpGrad",
            "samples a texture using a gradient and compares a single component against the specified comparison value",
            "hf",
            "ro",
            [
                db_dxil_param(0, "$r", "", "the result of the filtered comparisons"),
                db_dxil_param(2, "res", "srv", "handle of SRV to sample"),
                db_dxil_param(3, "res", "sampler", "handle of sampler to use"),
                db_dxil_param(4, "f", "coord0", "coordinate"),
                db_dxil_param(5, "f", "coord1", "coordinate, undef for Texture1D"),
                db_dxil_param(
                    6,
                    "f",
                    "coord2",
                    "coordinate, undef for Texture1D, Texture1DArray or Texture2D",
                ),
                db_dxil_param(
                    7, "f", "coord3", "coordinate, defined only for TextureCubeArray"
                ),
                db_dxil_param(
                    8,
                    "i32",
                    "offset0",
                    "optional offset, applicable to Texture1D, Texture1DArray, and as part of offset1",
                ),
                db_dxil_param(
                    9,
                    "i32",
                    "offset1",
                    "optional offset, applicable to Texture2D, Texture2DArray, and as part of offset2",
                ),
                db_dxil_param(
                    10, "i32", "offset2", "optional offset, applicable to Texture3D"
                ),
                db_dxil_param(11, "f", "compareValue", "the value to compare with"),
                db_dxil_param(
                    12,
                    "f",
                    "ddx0",
                    "rate of change of coordinate c0 in the x direction",
                ),
                db_dxil_param(
                    13,
                    "f",
                    "ddx1",
                    "rate of change of coordinate c1 in the x direction",
                ),
                db_dxil_param(
                    14,
                    "f",
                    "ddx2",
                    "rate of change of coordinate c2 in the x direction",
                ),
                db_dxil_param(
                    15,
                    "f",
                    "ddy0",
                    "rate of change of coordinate c0 in the y direction",
                ),
                db_dxil_param(
                    16,
                    "f",
                    "ddy1",
                    "rate of change of coordinate c1 in the y direction",
                ),
                db_dxil_param(
                    17,
                    "f",
                    "ddy2",
                    "rate of change of coordinate c2 in the y direction",
                ),
                db_dxil_param(18, "f", "clamp", "clamp value"),
            ],
            counters=("tex_cmp",),
        )
        next_op_idx += 1

        self.add_dxil_op(
            "SampleCmpBias",
            next_op_idx,
            "SampleCmpBias",
            "samples a texture after applying the input bias to the mipmap level and compares a single component against the specified comparison value",
            "hf",
            "ro",
            [
                db_dxil_param(0, "$r", "", "the result of the filtered comparisons"),
                db_dxil_param(2, "res", "srv", "handle of SRV to sample"),
                db_dxil_param(3, "res", "sampler", "handle of sampler to use"),
                db_dxil_param(4, "f", "coord0", "coordinate"),
                db_dxil_param(5, "f", "coord1", "coordinate, undef for Texture1D"),
                db_dxil_param(
                    6,
                    "f",
                    "coord2",
                    "coordinate, undef for Texture1D, Texture1DArray or Texture2D",
                ),
                db_dxil_param(
                    7, "f", "coord3", "coordinate, defined only for TextureCubeArray"
                ),
                db_dxil_param(
                    8,
                    "i32",
                    "offset0",
                    "optional offset, applicable to Texture1D, Texture1DArray, and as part of offset1",
                ),
                db_dxil_param(
                    9,
                    "i32",
                    "offset1",
                    "optional offset, applicable to Texture2D, Texture2DArray, and as part of offset2",
                ),
                db_dxil_param(
                    10, "i32", "offset2", "optional offset, applicable to Texture3D"
                ),
                db_dxil_param(11, "f", "compareValue", "the value to compare with"),
                db_dxil_param(12, "f", "bias", "bias value"),
                db_dxil_param(13, "f", "clamp", "clamp value"),
            ],
            counters=("tex_cmp",),
        )
        next_op_idx += 1

        self.add_dxil_op(
            "StartVertexLocation",
            next_op_idx,
            "StartVertexLocation",
            "returns the BaseVertexLocation from DrawIndexedInstanced or StartVertexLocation from DrawInstanced",
            "i",
            "rn",
            [db_dxil_param(0, "i32", "", "result")],
        )
        next_op_idx += 1

        self.add_dxil_op(
            "StartInstanceLocation",
            next_op_idx,
            "StartInstanceLocation",
            "returns the StartInstanceLocation from Draw*Instanced",
            "i",
            "rn",
            [db_dxil_param(0, "i32", "", "result")],
        )
        next_op_idx += 1

        # End of DXIL 1.8 opcodes.
        self.set_op_count_for_version(1, 8, next_op_idx)
        assert next_op_idx == 258, (
            "258 is expected next operation index but encountered %d and thus opcodes are broken"
            % next_op_idx
        )

        # Set interesting properties.
        self.build_indices()
        for (
            i
        ) in "CalculateLOD,DerivCoarseX,DerivCoarseY,DerivFineX,DerivFineY,Sample,SampleBias,SampleCmp,SampleCmpBias".split(
            ","
        ):
            self.name_idx[i].is_gradient = True
        for i in "DerivCoarseX,DerivCoarseY,DerivFineX,DerivFineY".split(","):
            assert (
                self.name_idx[i].is_gradient == True
            ), "all derivatives are marked as requiring gradients"
            self.name_idx[i].is_deriv = True

        # TODO - some arguments are required to be immediate constants in DXIL, eg resource kinds; add this information
        # consider - report instructions that are overloaded on a single type, then turn them into non-overloaded version of that type
        self.verify_dense(
            self.get_dxil_insts(), lambda x: x.dxil_opid, lambda x: x.name
        )
        for i in self.instr:
            self.verify_dense(i.ops, lambda x: x.pos, lambda x: i.name)
        for i in self.instr:
            if i.is_dxil_op:
                assert i.oload_types != "", (
                    "overload for DXIL operation %s should not be empty - use void if n/a"
                    % (i.name)
                )
                assert i.oload_types == "v" or i.oload_types.find("v") < 0, (
                    "void overload should be exclusive to other types (%s)" % i.name
                )
            assert (
                type(i.oload_types) is str
            ), "overload for %s should be a string - use empty if n/a" % (i.name)

        # Verify that all operations in each class have the same signature.
        import itertools

        class_sort_func = lambda x, y: x < y
        class_key_func = lambda x: x.dxil_class
        instr_ordered_by_class = sorted(
            [i for i in self.instr if i.is_dxil_op], key=class_key_func
        )
        instr_grouped_by_class = itertools.groupby(
            instr_ordered_by_class, key=class_key_func
        )

        def calc_oload_sig(inst):
            result = ""
            for o in inst.ops:
                result += o.llvm_type
            return result

        for k, g in instr_grouped_by_class:
            group = list(g)
            if len(group) > 1:
                first = group[0]
                first_group = calc_oload_sig(first)
                for other in group[1:]:
                    other_group = calc_oload_sig(other)
                    # TODO: uncomment assert when opcodes are fixed
                    # assert first_group == other_group, "overload signature %s for instruction %s differs from %s in %s" % (first.name, first_group, other.name, other_group)

    def populate_extended_docs(self):
        "Update the documentation with text from external files."
        inst_starter = "* Inst: "
        block_starter = "* BLOCK-BEGIN"
        block_end = "* BLOCK-END"
        thisdir = os.path.dirname(os.path.realpath(__file__))
        with open(os.path.join(thisdir, "hctdb_inst_docs.txt")) as ops_file:
            inst_name = ""
            inst_doc = ""
            inst_remarks = ""
            is_block = False
            for idx, line in enumerate(ops_file):
                if line.startswith("#"):
                    continue
                if line.startswith(block_starter):
                    assert is_block == False, "unexpected block begin at line %i" % (
                        idx + 1
                    )
                    is_block = True
                    continue
                if line.startswith(block_end):
                    assert is_block == True, "unexpected block end at line %i" % (
                        idx + 1
                    )
                    is_block = False
                    continue
                if line.startswith(inst_starter):
                    if inst_name:
                        # print(inst_name + " - " + inst_remarks.strip())
                        self.name_idx[inst_name].doc = inst_doc
                        self.name_idx[inst_name].remarks = inst_remarks.strip()
                    inst_remarks = ""
                    line = line[len(inst_starter) :]
                    sep_idx = line.find("-")
                    inst_name = line[:sep_idx].strip()
                    inst_doc = line[sep_idx + 1 :].strip()
                else:
                    inst_remarks += line if is_block else "\n" + line.strip()
            if inst_name:
                self.name_idx[inst_name].remarks = inst_remarks.strip()

    def populate_metadata(self):
        # For now, simply describe the allowed named metadata.
        m = self.metadata
        m.append(
            db_dxil_metadata(
                "dx.controlflow.hints", "Provides control flow hints to an instruction."
            )
        )
        m.append(db_dxil_metadata("dx.entryPoints", "Entry point functions."))
        m.append(db_dxil_metadata("dx.precise", "Marks an instruction as precise."))
        m.append(
            db_dxil_metadata(
                "dx.resources", "Resources used by the entry point shaders."
            )
        )
        m.append(db_dxil_metadata("dx.shaderModel", "Shader model for the module."))
        m.append(
            db_dxil_metadata("dx.typeAnnotations", "Provides annotations for types.")
        )
        m.append(db_dxil_metadata("dx.typevar.*", "."))
        m.append(db_dxil_metadata("dx.valver", "Optional validator version."))
        m.append(
            db_dxil_metadata("dx.version", "Optional DXIL version for the module.")
        )
        # dx.typevar.* is not the name of metadata, but the prefix for global variables
        # that will be referenced by structure type annotations.

    def populate_passes(self):
        # Populate passes and their options.
        p = self.passes
        category_lib = "set this before add_pass"

        def add_pass(name, type_name, doc, opts):
            apass = db_dxil_pass(
                name, type_name=type_name, doc=doc, category_lib=category_lib
            )
            for o in opts:
                assert "n" in o, "option in %s has no 'n' member" % name
                apass.args.append(
                    db_dxil_pass_arg(
                        o["n"],
                        ident=o.get("i"),
                        type_name=o.get("t"),
                        is_ctor_param=o.get("c"),
                        doc=o.get("d"),
                    )
                )
            p.append(apass)

        category_lib = "llvm"
        # Add discriminators is a DWARF 4 thing, useful for the profiler.
        # Consider removing lib\Transforms\Utils\AddDiscriminators.cpp altogether
        add_pass(
            "add-discriminators",
            "AddDiscriminators",
            "Add DWARF path discriminators",
            [{"n": "no-discriminators", "i": "NoDiscriminators", "t": "bool"}],
        )
        # Sample profile is part of the sample profiling infrastructure.
        # Consider removing lib\Transforms\Scalar\SampleProfile.cpp
        add_pass(
            "sample-profile",
            "SampleProfileLoader",
            "Sample Profile loader",
            [
                {"n": "sample-profile-file", "i": "SampleProfileFile", "t": "string"},
                {
                    "n": "sample-profile-max-propagate-iterations",
                    "i": "SampleProfileMaxPropagateIterations",
                    "t": "unsigned",
                },
            ],
        )
        # inline and always-inline share a base class - those are the arguments we document for each of them.
        inliner_args = [
            {
                "n": "InsertLifetime",
                "t": "bool",
                "c": 1,
                "d": "Insert @llvm.lifetime intrinsics",
            },
            {
                "n": "InlineThreshold",
                "t": "unsigned",
                "c": 1,
                "d": "Insert @llvm.lifetime intrinsics",
            },
        ]
        add_pass(
            "inline", "SimpleInliner", "Function Integration/Inlining", inliner_args
        )
        #            {'n':"OptLevel", 't':"unsigned", 'c':1},
        #            {'n':"SizeOptLevel", 't':'unsigned', 'c':1}
        add_pass(
            "always-inline",
            "AlwaysInliner",
            "Inliner for always_inline functions",
            inliner_args,
        )
        #            {'n':'InsertLifetime', 't':'bool', 'c':1, 'd':'Insert @llvm.lifetime intrinsics'}
        # Consider a review of the target-specific wrapper.
        add_pass(
            "tti",
            "TargetTransformInfoWrapperPass",
            "Target Transform Information",
            [{"n": "TIRA", "t": "TargetIRAnalysis", "c": 1}],
        )
        add_pass(
            "verify",
            "VerifierLegacyPass",
            "Module Verifier",
            [
                {"n": "FatalErrors", "t": "bool", "c": 1},
                {"n": "verify-debug-info", "i": "VerifyDebugInfo", "t": "bool"},
            ],
        )
        add_pass(
            "targetlibinfo",
            "TargetLibraryInfoWrapperPass",
            "Target Library Information",
            [
                {"n": "TLIImpl", "t": "TargetLibraryInfoImpl", "c": 1},
                {
                    "n": "vector-library",
                    "i": "ClVectorLibrary",
                    "t": "TargetLibraryInfoImpl::VectorLibrary",
                },
            ],
        )
        add_pass("cfl-aa", "CFLAliasAnalysis", "CFL-Based AA implementation", [])
        add_pass(
            "tbaa",
            "TypeBasedAliasAnalysis",
            "Type-Based Alias Analysis",
            [
                {
                    "n": "enable-tbaa",
                    "i": "EnableTBAA",
                    "t": "bool",
                    "d": "Use to disable TBAA functionality",
                }
            ],
        )
        add_pass(
            "scoped-noalias",
            "ScopedNoAliasAA",
            "Scoped NoAlias Alias Analysis",
            [
                {
                    "n": "enable-scoped-noalias",
                    "i": "EnableScopedNoAlias",
                    "t": "bool",
                    "d": "Use to disable scoped no-alias",
                }
            ],
        )
        add_pass(
            "basicaa",
            "BasicAliasAnalysis",
            "Basic Alias Analysis (stateless AA impl)",
            [],
        )
        add_pass(
            "simplifycfg",
            "CFGSimplifyPass",
            "Simplify the CFG",
            [
                {"n": "Threshold", "t": "int", "c": 1},
                {"n": "Ftor", "t": "std::function<bool(const Function &)>", "c": 1},
                {
                    "n": "bonus-inst-threshold",
                    "i": "UserBonusInstThreshold",
                    "t": "unsigned",
                    "d": "Control the number of bonus instructions (default = 1)",
                },
            ],
        )
        # UseNewSROA is used by PassManagerBuilder::populateFunctionPassManager, not a pass per se.
        add_pass(
            "sroa",
            "SROA",
            "Scalar Replacement Of Aggregates",
            [
                {"n": "RequiresDomTree", "t": "bool", "c": 1},
                {"n": "SkipHLSLMat", "t": "bool", "c": 1},
                {
                    "n": "force-ssa-updater",
                    "i": "ForceSSAUpdater",
                    "t": "bool",
                    "d": "Force the pass to not use DomTree and mem2reg, insteadforming SSA values through the SSAUpdater infrastructure.",
                },
                {
                    "n": "sroa-random-shuffle-slices",
                    "i": "SROARandomShuffleSlices",
                    "t": "bool",
                    "d": "Enable randomly shuffling the slices to help uncover instability in their order.",
                },
                {
                    "n": "sroa-strict-inbounds",
                    "i": "SROAStrictInbounds",
                    "t": "bool",
                    "d": "Experiment with completely strict handling of inbounds GEPs.",
                },
            ],
        )
        add_pass(
            "dxil-cond-mem2reg",
            "DxilConditionalMem2Reg",
            "Dxil Conditional Mem2Reg",
            [
                {"n": "NoOpt", "t": "bool", "c": 1},
            ],
        )
        add_pass(
            "scalarrepl",
            "SROA_DT",
            "Scalar Replacement of Aggregates (DT)",
            [
                {"n": "Threshold", "t": "int", "c": 1},
                {"n": "StructMemberThreshold", "t": "int", "c": 1},
                {"n": "ArrayElementThreshold", "t": "int", "c": 1},
                {"n": "ScalarLoadThreshold", "t": "int", "c": 1},
            ],
        )
        add_pass(
            "scalarrepl-ssa",
            "SROA_SSAUp",
            "Scalar Replacement of Aggregates (SSAUp)",
            [
                {"n": "Threshold", "t": "int", "c": 1},
                {"n": "StructMemberThreshold", "t": "int", "c": 1},
                {"n": "ArrayElementThreshold", "t": "int", "c": 1},
                {"n": "ScalarLoadThreshold", "t": "int", "c": 1},
            ],
        )
        add_pass("early-cse", "EarlyCSELegacyPass", "Early CSE", [])
        # More branch weight support.
        add_pass(
            "lower-expect",
            "LowerExpectIntrinsic",
            "Lower 'expect' Intrinsics",
            [
                {
                    "n": "likely-branch-weight",
                    "i": "LikelyBranchWeight",
                    "t": "uint32_t",
                    "d": "Weight of the branch likely to be taken (default = 64)",
                },
                {
                    "n": "unlikely-branch-weight",
                    "i": "UnlikelyBranchWeight",
                    "t": "uint32_t",
                    "d": "Weight of the branch unlikely to be taken (default = 4)",
                },
            ],
        )
        # Consider removing lib\Transforms\Utils\SymbolRewriter.cpp
        add_pass(
            "rewrite-symbols",
            "RewriteSymbols",
            "Rewrite Symbols",
            [
                {"n": "DL", "t": "SymbolRewriter::RewriteDescriptorList", "c": 1},
                {"n": "rewrite-map-file", "i": "RewriteMapFiles", "t": "string"},
            ],
        )
        add_pass(
            "mergefunc",
            "MergeFunctions",
            "Merge Functions",
            [
                {
                    "n": "mergefunc-sanity",
                    "i": "NumFunctionsForSanityCheck",
                    "t": "unsigned",
                    "d": "How many functions in module could be used for MergeFunctions pass sanity check. '0' disables this check. Works only with '-debug' key.",
                }
            ],
        )
        # Consider removing GlobalExtensions globals altogether.
        add_pass("barrier", "BarrierNoop", "A No-Op Barrier Pass", [])
        add_pass("dce", "DCE", "Dead Code Elimination", [])
        add_pass("die", "DeadInstElimination", "Dead Instruction Elimination", [])
        add_pass("globaldce", "GlobalDCE", "Dead Global Elimination", [])
        add_pass("mem2reg", "PromotePass", "Promote Memory to Register", [])
        add_pass("scalarizer", "Scalarizer", "Scalarize vector operations", [])

        category_lib = "pix"
        add_pass(
            "hlsl-dxil-add-pixel-hit-instrmentation",
            "DxilAddPixelHitInstrumentation",
            "DXIL Count completed PS invocations and costs",
            [
                {"n": "force-early-z", "t": "int", "c": 1},
                {"n": "add-pixel-cost", "t": "int", "c": 1},
                {"n": "rt-width", "t": "int", "c": 1},
                {"n": "num-pixels", "t": "int", "c": 1},
                {"n": "upstream-sv-position-row", "t": "int", "c": 1},
            ],
        )
        add_pass(
            "hlsl-dxil-constantColor",
            "DxilOutputColorBecomesConstant",
            "DXIL Constant Color Mod",
            [
                {"n": "mod-mode", "t": "int", "c": 1},
                {"n": "constant-red", "t": "float", "c": 1},
                {"n": "constant-green", "t": "float", "c": 1},
                {"n": "constant-blue", "t": "float", "c": 1},
                {"n": "constant-alpha", "t": "float", "c": 1},
            ],
        )
        add_pass(
            "hlsl-dxil-remove-discards",
            "DxilRemoveDiscards",
            "HLSL DXIL Remove all discard instructions",
            [],
        )
        add_pass(
            "hlsl-dxil-force-early-z",
            "DxilForceEarlyZ",
            "HLSL DXIL Force the early Z global flag, if shader has no discard calls",
            [],
        )
        add_pass(
            "hlsl-dxil-pix-meshshader-output-instrumentation",
            "DxilPIXMeshShaderOutputInstrumentation",
            "DXIL mesh shader output instrumentation for PIX",
            [
                {"n": "expand-payload", "t": "int", "c": 1},
                {"n": "UAVSize", "t": "int", "c": 1},
                {"n": "dispatchArgY", "t": "int", "c": 1},
                {"n": "dispatchArgZ", "t": "int", "c": 1},
            ],
        )
        add_pass(
            "hlsl-dxil-pix-shader-access-instrumentation",
            "DxilShaderAccessTracking",
            "HLSL DXIL shader access tracking for PIX",
            [
                {"n": "config", "t": "int", "c": 1},
                {"n": "checkForDynamicIndexing", "t": "bool", "c": 1},
            ],
        )
        add_pass(
            "hlsl-dxil-debug-instrumentation",
            "DxilDebugInstrumentation",
            "HLSL DXIL debug instrumentation for PIX",
            [
                {"n": "UAVSize", "t": "int", "c": 1},
                {"n": "FirstInstruction", "t": "int", "c": 1},
                {"n": "LastInstruction", "t": "int", "c": 1},
                {"n": "parameter0", "t": "int", "c": 1},
                {"n": "parameter1", "t": "int", "c": 1},
                {"n": "parameter2", "t": "int", "c": 1},
                {"n": "upstreamSVPositionRow", "t": "int", "c": 1},
            ],
        )
        add_pass(
            "dxil-annotate-with-virtual-regs",
            "DxilAnnotateWithVirtualRegister",
            "Annotates each instruction in the DXIL module with a virtual register number",
            [{"n": "startInstruction", "t": "int", "c": 1}],
        )
        add_pass(
            "dxil-dbg-value-to-dbg-declare",
            "DxilDbgValueToDbgDeclare",
            "Converts llvm.dbg.value uses to llvm.dbg.declare.",
            [],
        )
        add_pass(
            "hlsl-dxil-reduce-msaa-to-single",
            "DxilReduceMSAAToSingleSample",
            "HLSL DXIL Reduce all MSAA reads to single-sample reads",
            [],
        )
        add_pass(
            "hlsl-dxil-PIX-add-tid-to-as-payload",
            "DxilPIXAddTidToAmplificationShaderPayload",
            "HLSL DXIL Add flat thread id to payload from AS to MS",
            [
                {"n": "dispatchArgY", "t": "int", "c": 1},
                {"n": "dispatchArgZ", "t": "int", "c": 1},
            ],
        )
        add_pass(
            "hlsl-dxil-pix-dxr-invocations-log",
            "DxilPIXDXRInvocationsLog",
            "HLSL DXIL Logs all non-RayGen DXR 1.0 invocations into a UAV",
            [{"n": "maxNumEntriesInLog", "t": "int", "c": 1}],
        )

        category_lib = "dxil_gen"

        add_pass("hlsl-hlemit", "HLEmitMetadata", "HLSL High-Level Metadata Emit.", [])
        add_pass(
            "hl-expand-store-intrinsics",
            "HLExpandStoreIntrinsics",
            "Expand HLSL store intrinsics",
            [],
        )
        add_pass(
            "hl-legalize-parameter", "HLLegalizeParameter", "Legalize parameter", []
        )
        add_pass(
            "scalarrepl-param-hlsl",
            "SROA_Parameter_HLSL",
            "Scalar Replacement of Aggregates HLSL (parameters)",
            [],
        )
        add_pass(
            "static-global-to-alloca",
            "LowerStaticGlobalIntoAlloca",
            "Lower static global into Alloca",
            [],
        )
        add_pass(
            "hlmatrixlower", "HLMatrixLowerPass", "HLSL High-Level Matrix Lower", []
        )
        add_pass(
            "matrixbitcastlower", "MatrixBitcastLowerPass", "Matrix Bitcast lower", []
        )
        add_pass(
            "reg2mem_hlsl",
            "RegToMemHlsl",
            "Demote values with phi-node usage to stack slots",
            [],
        )
        add_pass(
            "dynamic-vector-to-array",
            "DynamicIndexingVectorToArray",
            "Replace dynamic indexing vector with array",
            [{"n": "ReplaceAllVectors", "t": "bool", "c": 1}],
        )
        add_pass(
            "hlsl-dxil-promote-local-resources",
            "DxilPromoteLocalResources",
            "DXIL promote local resource use",
            [],
        )
        add_pass(
            "hlsl-dxil-promote-static-resources",
            "DxilPromoteStaticResources",
            "DXIL promote static resource use",
            [],
        )
        add_pass(
            "hlsl-dxil-legalize-resources",
            "DxilLegalizeResources",
            "DXIL legalize resource use",
            [],
        )
        add_pass(
            "hlsl-dxil-legalize-eval-operations",
            "DxilLegalizeEvalOperations",
            "DXIL legalize eval operations",
            [],
        )
        add_pass(
            "dxilgen",
            "DxilGenerationPass",
            "HLSL DXIL Generation",
            [{"n": "NotOptimized", "t": "bool", "c": 1}],
        )
        add_pass(
            "invalidate-undef-resource",
            "InvalidateUndefResources",
            "Invalidate undef resources",
            [],
        )
        add_pass("simplify-inst", "SimplifyInst", "Simplify Instructions", [])
        add_pass(
            "hlsl-dxil-precise",
            "DxilPrecisePropagatePass",
            "DXIL precise attribute propagate",
            [],
        )
        add_pass(
            "dxil-legalize-sample-offset",
            "DxilLegalizeSampleOffsetPass",
            "DXIL legalize sample offset",
            [],
        )
        add_pass("dxil-gvn-hoist", "DxilSimpleGVNHoist", "DXIL simple gvn hoist", [])
        add_pass(
            "dxil-gvn-eliminate-region",
            "DxilSimpleGVNEliminateRegion",
            "DXIL simple eliminate region",
            [],
        )
        add_pass(
            "hlsl-hlensure", "HLEnsureMetadata", "HLSL High-Level Metadata Ensure", []
        )
        add_pass(
            "multi-dim-one-dim",
            "MultiDimArrayToOneDimArray",
            "Flatten multi-dim array into one-dim array",
            [],
        )
        add_pass(
            "resource-handle", "ResourceToHandle", "Lower resource into handle", []
        )
        add_pass(
            "hlsl-passes-nopause",
            "NoPausePasses",
            "Clears metadata used for pause and resume",
            [],
        )
        add_pass("hlsl-passes-pause", "PausePasses", "Prepare to pause passes", [])
        add_pass("hlsl-passes-resume", "ResumePasses", "Prepare to resume passes", [])
        add_pass(
            "hlsl-dxil-lower-handle-for-lib",
            "DxilLowerCreateHandleForLib",
            "DXIL Lower createHandleForLib",
            [],
        )
        add_pass(
            "hlsl-dxil-cleanup-dynamic-resource-handle",
            "DxilCleanupDynamicResourceHandle",
            "DXIL Cleanup dynamic resource handle calls",
            [],
        )
        add_pass(
            "hlsl-dxil-allocate-resources-for-lib",
            "DxilAllocateResourcesForLib",
            "DXIL Allocate Resources For Library",
            [],
        )
        add_pass(
            "hlsl-dxil-convergent-mark", "DxilConvergentMark", "Mark convergent", []
        )
        add_pass(
            "hlsl-dxil-convergent-clear",
            "DxilConvergentClear",
            "Clear convergent before dxil emit",
            [],
        )
        add_pass(
            "hlsl-dxil-eliminate-output-dynamic",
            "DxilEliminateOutputDynamicIndexing",
            "DXIL eliminate ouptut dynamic indexing",
            [],
        )
        add_pass(
            "dxil-delete-redundant-debug-values",
            "DxilDeleteRedundantDebugValues",
            "Dxil Delete Redundant Debug Values",
            [],
        )
        add_pass(
            "hlsl-dxilfinalize", "DxilFinalizeModule", "HLSL DXIL Finalize Module", []
        )
        add_pass("hlsl-dxilemit", "DxilEmitMetadata", "HLSL DXIL Metadata Emit", [])
        add_pass("hlsl-dxilload", "DxilLoadMetadata", "HLSL DXIL Metadata Load", [])
        add_pass(
            "dxil-dfe",
            "DxilDeadFunctionElimination",
            "Remove all unused function except entry from DxilModule",
            [],
        )
        add_pass(
            "hl-dfe",
            "HLDeadFunctionElimination",
            "Remove all unused function except entry from HLModule",
            [],
        )
        add_pass(
            "hl-preprocess", "HLPreprocess", "Preprocess HLModule after inline", []
        )
        add_pass(
            "hlsl-dxil-expand-trig",
            "DxilExpandTrigIntrinsics",
            "DXIL expand trig intrinsics",
            [],
        )
        add_pass("hlsl-hca", "HoistConstantArray", "HLSL constant array hoisting", [])
        add_pass(
            "hlsl-dxil-preserve-all-outputs",
            "DxilPreserveAllOutputs",
            "DXIL write to all outputs in signature",
            [],
        )
        add_pass("red", "ReducibilityAnalysis", "Reducibility Analysis", [])
        add_pass(
            "viewid-state",
            "ComputeViewIdState",
            "Compute information related to ViewID",
            [],
        )
        add_pass(
            "hlsl-translate-dxil-opcode-version",
            "DxilTranslateRawBuffer",
            "Translates one version of dxil to another",
            [],
        )
        add_pass(
            "hlsl-dxil-cleanup-addrspacecast",
            "DxilCleanupAddrSpaceCast",
            "HLSL DXIL Cleanup Address Space Cast (part of hlsl-dxilfinalize)",
            [],
        )
        add_pass(
            "dxil-fix-array-init",
            "DxilFixConstArrayInitializer",
            "Dxil Fix Array Initializer",
            [],
        )
        add_pass(
            "hlsl-validate-wave-sensitivity",
            "DxilValidateWaveSensitivity",
            "HLSL DXIL wave sensitiveity validation",
            [],
        )
        add_pass(
            "dxil-elim-vector", "DxilEliminateVector", "Dxil Eliminate Vectors", []
        )
        add_pass(
            "dxil-rewrite-output-arg-debug-info",
            "DxilRewriteOutputArgDebugInfo",
            "Dxil Rewrite Output Arg Debug Info",
            [],
        )
        add_pass(
            "dxil-finalize-preserves",
            "DxilFinalizePreserves",
            "Dxil Finalize Preserves",
            [],
        )
        add_pass("dxil-reinsert-nops", "DxilReinsertNops", "Dxil Reinsert Nops", [])
        add_pass(
            "dxil-insert-preserves",
            "DxilInsertPreserves",
            "Dxil Insert Noops",
            [
                {"n": "AllowPreserves", "t": "bool", "c": 1},
            ],
        )
        add_pass(
            "dxil-preserves-to-select",
            "DxilPreserveToSelect",
            "Dxil Preserves To Select",
            [],
        )
        add_pass("dxil-delete-loop", "DxilLoopDeletion", "Dxil Loop Deletion", [])
        add_pass("dxil-value-cache", "DxilValueCache", "Dxil Value Cache", [])
        add_pass(
            "hlsl-cleanup-dxbreak",
            "CleanupDxBreak",
            "HLSL Remove unnecessary dx.break conditions",
            [],
        )
        add_pass(
            "dxil-rename-resources",
            "DxilRenameResources",
            "Rename resources to prevent merge by name during linking",
            [
                {
                    "n": "prefix",
                    "i": "Prefix",
                    "t": "string",
                    "d": "Prefix to add to resource names",
                },
                {
                    "n": "from-binding",
                    "i": "FromBinding",
                    "t": "bool",
                    "c": 1,
                    "d": "Append binding to name when bound",
                },
                {
                    "n": "keep-name",
                    "i": "KeepName",
                    "t": "bool",
                    "c": 1,
                    "d": "Keep name when appending binding",
                },
            ],
        )
        add_pass(
            "hlsl-dxil-resources-to-handle",
            "DxilMutateResourceToHandle",
            "Mutate resource to handle",
            [],
        )

        category_lib = "llvm"
        add_pass(
            "ipsccp",
            "IPSCCP",
            "Interprocedural Sparse Conditional Constant Propagation",
            [],
        )
        add_pass("globalopt", "GlobalOpt", "Global Variable Optimizer", [])
        add_pass("deadargelim", "DAE", "Dead Argument Elimination", [])
        # Should we get rid of this, or invest in bugpoint support?
        add_pass(
            "deadarghaX0r",
            "DAH",
            "Dead Argument Hacking (BUGPOINT USE ONLY; DO NOT USE)",
            [],
        )
        add_pass(
            "instcombine",
            "InstructionCombiningPass",
            "Combine redundant instructions",
            [
                {"n": "NoSink", "t": "bool", "c": 1},
            ],
        )
        add_pass("prune-eh", "PruneEH", "Remove unused exception handling info", [])
        add_pass("functionattrs", "FunctionAttrs", "Deduce function attributes", [])
        # add_pass('argpromotion', 'ArgPromotion', "Promote 'by reference' arguments to scalars", [
        #     {'n':'maxElements', 't':'unsigned', 'c':1}])
        add_pass(
            "jump-threading",
            "JumpThreading",
            "Jump Threading",
            [
                {"n": "Threshold", "t": "int", "c": 1},
                {
                    "n": "jump-threading-threshold",
                    "i": "BBDuplicateThreshold",
                    "t": "unsigned",
                    "d": "Max block size to duplicate for jump threading",
                },
            ],
        )
        add_pass(
            "correlated-propagation",
            "CorrelatedValuePropagation",
            "Value Propagation",
            [],
        )
        # createTailCallEliminationPass is removed - but is this checked before?
        add_pass(
            "reassociate",
            "Reassociate",
            "Reassociate expressions",
            [
                {"n": "EnableAggressiveReassociation", "t": "bool", "c": 1},
            ],
        )
        add_pass(
            "loop-rotate",
            "LoopRotate",
            "Rotate Loops",
            [
                {"n": "MaxHeaderSize", "t": "int", "c": 1},
                {
                    "n": "rotation-max-header-size",
                    "i": "DefaultRotationThreshold",
                    "t": "unsigned",
                    "d": "The default maximum header size for automatic loop rotation",
                },
            ],
        )
        add_pass(
            "licm",
            "LICM",
            "Loop Invariant Code Motion",
            [
                {
                    "n": "disable-licm-promotion",
                    "i": "DisablePromotion",
                    "t": "bool",
                    "d": "Disable memory promotion in LICM pass",
                }
            ],
        )
        add_pass(
            "loop-unswitch",
            "LoopUnswitch",
            "Unswitch loops",
            [
                {"n": "Os", "t": "bool", "c": 1, "d": "Optimize for size"},
                {
                    "n": "loop-unswitch-threshold",
                    "i": "Threshold",
                    "t": "unsigned",
                    "d": "Max loop size to unswitch",
                },
            ],
        )
        # C:\nobackup\work\HLSLonLLVM\lib\Transforms\IPO\PassManagerBuilder.cpp:353
        add_pass("indvars", "IndVarSimplify", "Induction Variable Simplification", [])
        add_pass("loop-idiom", "LoopIdiomRecognize", "Recognize loop idioms", [])
        add_pass(
            "dxil-loop-unroll",
            "DxilLoopUnroll",
            "DxilLoopUnroll",
            [
                {
                    "n": "MaxIterationAttempt",
                    "t": "unsigned",
                    "c": 1,
                    "d": "Maximum number of iterations to attempt when iteratively unrolling.",
                },
                {
                    "n": "OnlyWarnOnFail",
                    "t": "bool",
                    "c": 1,
                    "d": "Whether to just warn when unrolling fails.",
                },
                {
                    "n": "StructurizeLoopExits",
                    "t": "bool",
                    "c": 1,
                    "d": "Whether the unroller should try to structurize loop exits first.",
                },
            ],
        )
        add_pass(
            "dxil-erase-dead-region", "DxilEraseDeadRegion", "DxilEraseDeadRegion", []
        )
        add_pass(
            "dxil-remove-dead-blocks",
            "DxilRemoveDeadBlocks",
            "DxilRemoveDeadBlocks",
            [],
        )
        add_pass("dxil-o0-legalize", "DxilNoOptLegalize", "DXIL No-Opt Legalize", [])
        add_pass(
            "dxil-o0-simplify-inst",
            "DxilNoOptSimplifyInstructions",
            "DXIL No-Opt Simplify Inst",
            [],
        )
        add_pass(
            "dxil-loop-deletion",
            "DxilLoopDeletion",
            "Dxil Delete dead loops",
            [
                {"n": "NoSink", "t": "bool", "c": 1},
            ],
        )
        add_pass("loop-deletion", "LoopDeletion", "Delete dead loops", [])
        add_pass(
            "loop-interchange",
            "LoopInterchange",
            "Interchanges loops for cache reuse",
            [],
        )
        add_pass(
            "loop-unroll",
            "LoopUnroll",
            "Unroll loops",
            [
                {"n": "Threshold", "t": "int", "c": 1},
                {"n": "Count", "t": "int", "c": 1},
                {"n": "AllowPartial", "t": "int", "c": 1},
                {"n": "Runtime", "t": "int", "c": 1},
                {
                    "n": "unroll-threshold",
                    "i": "UnrollThreshold",
                    "t": "unsigned",
                    "d": "The baseline cost threshold for loop unrolling",
                },
                {
                    "n": "unroll-percent-dynamic-cost-saved-threshold",
                    "i": "UnrollPercentDynamicCostSavedThreshold",
                    "t": "unsigned",
                    "d": "The percentage of estimated dynamic cost which must be saved by unrolling to allow unrolling up to the max threshold.",
                },
                {
                    "n": "unroll-dynamic-cost-savings-discount",
                    "i": "UnrollDynamicCostSavingsDiscount",
                    "t": "unsigned",
                    "d": "This is the amount discounted from the total unroll cost when the unrolled form has a high dynamic cost savings (triggered by the '-unroll-perecent-dynamic-cost-saved-threshold' flag).",
                },
                {
                    "n": "unroll-max-iteration-count-to-analyze",
                    "i": "UnrollMaxIterationsCountToAnalyze",
                    "t": "unsigned",
                    "d": "Don't allow loop unrolling to simulate more than this number of iterations when checking full unroll profitability",
                },
                {
                    "n": "unroll-count",
                    "i": "UnrollCount",
                    "t": "unsigned",
                    "d": "Use this unroll count for all loops including those with unroll_count pragma values, for testing purposes",
                },
                {
                    "n": "unroll-allow-partial",
                    "i": "UnrollAllowPartial",
                    "t": "bool",
                    "d": "Allows loops to be partially unrolled until -unroll-threshold loop size is reached.",
                },
                {
                    "n": "unroll-runtime",
                    "i": "UnrollRuntime",
                    "t": "bool",
                    "d": "Unroll loops with run-time trip counts",
                },
                {
                    "n": "pragma-unroll-threshold",
                    "i": "PragmaUnrollThreshold",
                    "t": "unsigned",
                    "d": "Unrolled size limit for loops with an unroll(full) or unroll_count pragma.",
                },
                {
                    "n": "StructurizeLoopExits",
                    "t": "bool",
                    "c": 1,
                    "d": "Whether the unroller should try to structurize loop exits first.",
                },
            ],
        )
        add_pass("mldst-motion", "MergedLoadStoreMotion", "MergedLoadStoreMotion", [])
        add_pass(
            "gvn",
            "GVN",
            "Global Value Numbering",
            [
                {"n": "noloads", "t": "bool", "c": 1},
                {"n": "enable-pre", "i": "EnablePRE", "t": "bool"},
                {"n": "enable-load-pre", "i": "EnableLoadPRE", "t": "bool"},
                {
                    "n": "max-recurse-depth",
                    "i": "MaxRecurseDepth",
                    "t": "uint32_t",
                    "d": "Max recurse depth",
                },
            ],
        )
        add_pass("sccp", "SCCP", "Sparse Conditional Constant Propagation", [])
        add_pass("bdce", "BDCE", "Bit-Tracking Dead Code Elimination", [])
        add_pass("dse", "DSE", "Dead Store Elimination", [])
        add_pass(
            "loop-reroll",
            "LoopReroll",
            "Reroll loops",
            [
                {
                    "n": "max-reroll-increment",
                    "i": "MaxInc",
                    "t": "unsigned",
                    "d": "The maximum increment for loop rerolling",
                },
                {
                    "n": "reroll-num-tolerated-failed-matches",
                    "i": "NumToleratedFailedMatches",
                    "t": "unsigned",
                    "d": "The maximum number of failures to tolerate during fuzzy matching.",
                },
            ],
        )
        add_pass("load-combine", "LoadCombine", "Combine Adjacent Loads", [])
        add_pass("adce", "ADCE", "Aggressive Dead Code Elimination", [])
        add_pass(
            "float2int",
            "Float2Int",
            "Float to int",
            [
                {
                    "n": "float2int-max-integer-bw",
                    "i": "MaxIntegerBW",
                    "t": "unsigned",
                    "d": "Max integer bitwidth to consider in float2int",
                }
            ],
        )
        add_pass(
            "loop-distribute",
            "LoopDistribute",
            "Loop Distribition",
            [
                {
                    "n": "loop-distribute-verify",
                    "i": "LDistVerify",
                    "t": "bool",
                    "d": "Turn on DominatorTree and LoopInfo verification after Loop Distribution",
                },
                {
                    "n": "loop-distribute-non-if-convertible",
                    "i": "DistributeNonIfConvertible",
                    "t": "bool",
                    "d": "Whether to distribute into a loop that may not be if-convertible by the loop vectorizer",
                },
            ],
        )
        add_pass(
            "alignment-from-assumptions",
            "AlignmentFromAssumptions",
            "Alignment from assumptions",
            [],
        )
        add_pass(
            "strip-dead-prototypes",
            "StripDeadPrototypesPass",
            "Strip Unused Function Prototypes",
            [],
        )
        add_pass(
            "elim-avail-extern",
            "EliminateAvailableExternally",
            "Eliminate Available Externally Globals",
            [],
        )
        add_pass("constmerge", "ConstantMerge", "Merge Duplicate Global Constants", [])
        add_pass(
            "lowerbitsets",
            "LowerBitSets",
            "Lower bitset metadata",
            [
                {
                    "n": "lowerbitsets-avoid-reuse",
                    "i": "AvoidReuse",
                    "t": "bool",
                    "d": "Try to avoid reuse of byte array addresses using aliases",
                }
            ],
        )
        # TODO: turn STATISTICS macros into ETW events
        # assert no duplicate names
        self.pass_idx_args = set()
        p_names = set()
        p_ids = set()
        for ap in p:
            assert ap.name not in p_names
            p_names.add(ap.name)
            for anarg in ap.args:
                assert (
                    anarg.is_ctor_param or anarg.name not in p_ids
                ), "argument %s in %s is not ctor and is duplicate" % (
                    anarg.name,
                    ap.name,
                )
                if not anarg.is_ctor_param:
                    p_ids.add(anarg.name)
                self.pass_idx_args.add(anarg.name)

    def build_semantics(self):
        SemanticKind = db_dxil_enum(
            "SemanticKind",
            "Semantic kind; Arbitrary or specific system value.",
            [
                (0, "Arbitrary", ""),
                (1, "VertexID", ""),
                (2, "InstanceID", ""),
                (3, "Position", ""),
                (4, "RenderTargetArrayIndex", ""),
                (5, "ViewPortArrayIndex", ""),
                (6, "ClipDistance", ""),
                (7, "CullDistance", ""),
                (8, "OutputControlPointID", ""),
                (9, "DomainLocation", ""),
                (10, "PrimitiveID", ""),
                (11, "GSInstanceID", ""),
                (12, "SampleIndex", ""),
                (13, "IsFrontFace", ""),
                (14, "Coverage", ""),
                (15, "InnerCoverage", ""),
                (16, "Target", ""),
                (17, "Depth", ""),
                (18, "DepthLessEqual", ""),
                (19, "DepthGreaterEqual", ""),
                (20, "StencilRef", ""),
                (21, "DispatchThreadID", ""),
                (22, "GroupID", ""),
                (23, "GroupIndex", ""),
                (24, "GroupThreadID", ""),
                (25, "TessFactor", ""),
                (26, "InsideTessFactor", ""),
                (27, "ViewID", ""),
                (28, "Barycentrics", ""),
                (29, "ShadingRate", ""),
                (30, "CullPrimitive", ""),
                (31, "StartVertexLocation", ""),
                (32, "StartInstanceLocation", ""),
                (33, "Invalid", ""),
            ],
        )
        self.enums.append(SemanticKind)
        SigPointKind = db_dxil_enum(
            "SigPointKind",
            "Signature Point is more specific than shader stage or signature as it is unique in both stage and item dimensionality or frequency.",
            [
                (0, "VSIn", "Ordinary Vertex Shader input from Input Assembler"),
                (1, "VSOut", "Ordinary Vertex Shader output that may feed Rasterizer"),
                (2, "PCIn", "Patch Constant function non-patch inputs"),
                (3, "HSIn", "Hull Shader function non-patch inputs"),
                (4, "HSCPIn", "Hull Shader patch inputs - Control Points"),
                (5, "HSCPOut", "Hull Shader function output - Control Point"),
                (
                    6,
                    "PCOut",
                    "Patch Constant function output - Patch Constant data passed to Domain Shader",
                ),
                (
                    7,
                    "DSIn",
                    "Domain Shader regular input - Patch Constant data plus system values",
                ),
                (8, "DSCPIn", "Domain Shader patch input - Control Points"),
                (
                    9,
                    "DSOut",
                    "Domain Shader output - vertex data that may feed Rasterizer",
                ),
                (
                    10,
                    "GSVIn",
                    "Geometry Shader vertex input - qualified with primitive type",
                ),
                (11, "GSIn", "Geometry Shader non-vertex inputs (system values)"),
                (
                    12,
                    "GSOut",
                    "Geometry Shader output - vertex data that may feed Rasterizer",
                ),
                (13, "PSIn", "Pixel Shader input"),
                (14, "PSOut", "Pixel Shader output"),
                (15, "CSIn", "Compute Shader input"),
                (16, "MSIn", "Mesh Shader input"),
                (17, "MSOut", "Mesh Shader vertices output"),
                (18, "MSPOut", "Mesh Shader primitives output"),
                (19, "ASIn", "Amplification Shader input"),
                (21, "Invalid", ""),
            ],
        )
        self.enums.append(SigPointKind)
        PackingKind = db_dxil_enum(
            "PackingKind",
            "Kind of signature point",
            [
                (0, "None", "No packing should be performed"),
                (1, "InputAssembler", "Vertex Shader input from Input Assembler"),
                (2, "Vertex", "Vertex that may feed the Rasterizer"),
                (3, "PatchConstant", "Patch constant signature"),
                (4, "Target", "Render Target (Pixel Shader Output)"),
                (5, "Invalid", ""),
            ],
        )

        Float32DenormMode = db_dxil_enum(
            "Float32DenormMode",
            "float32 denorm behavior",
            [
                (0, "Any", "Undefined behavior for denormal numbers"),
                (1, "Preserve", "Preserve both input and output"),
                (2, "FTZ", "Preserve denormal inputs. Flush denorm outputs"),
                (3, "Reserve3", "Reserved Value. Not used for now"),
                (4, "Reserve4", "Reserved Value. Not used for now"),
                (5, "Reserve5", "Reserved Value. Not used for now"),
                (6, "Reserve6", "Reserved Value. Not used for now"),
                (7, "Reserve7", "Reserved Value. Not used for now"),
            ],
        )
        self.enums.append(Float32DenormMode)

        SigPointCSV = """
            SigPoint, Related, ShaderKind,    PackingKind,    SignatureKind
            VSIn,     Invalid, Vertex,        InputAssembler, Input
            VSOut,    Invalid, Vertex,        Vertex,         Output
            PCIn,     HSCPIn,  Hull,          None,           Invalid
            HSIn,     HSCPIn,  Hull,          None,           Invalid
            HSCPIn,   Invalid, Hull,          Vertex,         Input
            HSCPOut,  Invalid, Hull,          Vertex,         Output
            PCOut,    Invalid, Hull,          PatchConstant,  PatchConstOrPrim
            DSIn,     Invalid, Domain,        PatchConstant,  PatchConstOrPrim
            DSCPIn,   Invalid, Domain,        Vertex,         Input
            DSOut,    Invalid, Domain,        Vertex,         Output
            GSVIn,    Invalid, Geometry,      Vertex,         Input
            GSIn,     GSVIn,   Geometry,      None,           Invalid
            GSOut,    Invalid, Geometry,      Vertex,         Output
            PSIn,     Invalid, Pixel,         Vertex,         Input
            PSOut,    Invalid, Pixel,         Target,         Output
            CSIn,     Invalid, Compute,       None,           Invalid
            MSIn,     Invalid, Mesh,          None,           Invalid
            MSOut,    Invalid, Mesh,          Vertex,         Output
            MSPOut,   Invalid, Mesh,          Vertex,         PatchConstOrPrim
            ASIn,     Invalid, Amplification, None,           Invalid
            Invalid,  Invalid, Invalid,       Invalid,        Invalid
        """
        table = [
            list(map(str.strip, line.split(",")))
            for line in SigPointCSV.splitlines()
            if line.strip()
        ]
        for row in table[1:]:
            assert len(row) == len(table[0])  # Ensure table is rectangular
        # Make sure labels match enums, otherwise the table isn't aligned or in-sync
        if not ([row[0] for row in table[1:]] == SigPointKind.value_names()):
            assert False and "SigPointKind does not align with SigPointCSV row labels"
        self.sigpoint_table = table

        self.enums.append(PackingKind)
        SemanticInterpretationKind = db_dxil_enum(
            "SemanticInterpretationKind",
            "Defines how a semantic is interpreted at a particular SignaturePoint",
            [
                (0, "NA", "Not Available"),
                (1, "SV", "Normal System Value"),
                (2, "SGV", "System Generated Value (sorted last)"),
                (3, "Arb", "Treated as Arbitrary"),
                (4, "NotInSig", "Not included in signature (intrinsic access)"),
                (
                    5,
                    "NotPacked",
                    "Included in signature, but does not contribute to packing",
                ),
                (6, "Target", "Special handling for SV_Target"),
                (7, "TessFactor", "Special handling for tessellation factors"),
                (
                    8,
                    "Shadow",
                    "Shadow element must be added to a signature for compatibility",
                ),
                (
                    8,
                    "ClipCull",
                    "Special packing rules for SV_ClipDistance or SV_CullDistance",
                ),
                (9, "Invalid", ""),
            ],
        )
        self.enums.append(SemanticInterpretationKind)

        # The following has SampleIndex, Coverage, and InnerCoverage as loaded with instructions rather than from the signature
        SemanticInterpretationCSV = """
            Semantic,VSIn,VSOut,PCIn,HSIn,HSCPIn,HSCPOut,PCOut,DSIn,DSCPIn,DSOut,GSVIn,GSIn,GSOut,PSIn,PSOut,CSIn,MSIn,MSOut,MSPOut,ASIn
            Arbitrary,Arb,Arb,NA,NA,Arb,Arb,Arb,Arb,Arb,Arb,Arb,NA,Arb,Arb,NA,NA,NA,Arb,Arb,NA
            VertexID,SV,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA
            InstanceID,SV,Arb,NA,NA,Arb,Arb,NA,NA,Arb,Arb,Arb,NA,Arb,Arb,NA,NA,NA,NA,NA,NA
            Position,Arb,SV,NA,NA,SV,SV,Arb,Arb,SV,SV,SV,NA,SV,SV,NA,NA,NA,SV,NA,NA
            RenderTargetArrayIndex,Arb,SV,NA,NA,SV,SV,Arb,Arb,SV,SV,SV,NA,SV,SV,NA,NA,NA,NA,SV,NA
            ViewPortArrayIndex,Arb,SV,NA,NA,SV,SV,Arb,Arb,SV,SV,SV,NA,SV,SV,NA,NA,NA,NA,SV,NA
            ClipDistance,Arb,ClipCull,NA,NA,ClipCull,ClipCull,Arb,Arb,ClipCull,ClipCull,ClipCull,NA,ClipCull,ClipCull,NA,NA,NA,ClipCull,NA,NA
            CullDistance,Arb,ClipCull,NA,NA,ClipCull,ClipCull,Arb,Arb,ClipCull,ClipCull,ClipCull,NA,ClipCull,ClipCull,NA,NA,NA,ClipCull,NA,NA
            OutputControlPointID,NA,NA,NA,NotInSig,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA
            DomainLocation,NA,NA,NA,NA,NA,NA,NA,NotInSig,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA
            PrimitiveID,NA,NA,NotInSig,NotInSig,NA,NA,NA,NotInSig,NA,NA,NA,Shadow,SGV,SGV,NA,NA,NA,NA,SV,NA
            GSInstanceID,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NotInSig,NA,NA,NA,NA,NA,NA,NA,NA
            SampleIndex,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,Shadow _41,NA,NA,NA,NA,NA,NA
            IsFrontFace,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,SGV,SGV,NA,NA,NA,NA,NA,NA
            Coverage,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NotInSig _50,NotPacked _41,NA,NA,NA,NA,NA
            InnerCoverage,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NotInSig _50,NA,NA,NA,NA,NA,NA
            Target,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,Target,NA,NA,NA,NA,NA
            Depth,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NotPacked,NA,NA,NA,NA,NA
            DepthLessEqual,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NotPacked _50,NA,NA,NA,NA,NA
            DepthGreaterEqual,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NotPacked _50,NA,NA,NA,NA,NA
            StencilRef,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NotPacked _50,NA,NA,NA,NA,NA
            DispatchThreadID,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NotInSig,NotInSig,NA,NA,NotInSig
            GroupID,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NotInSig,NotInSig,NA,NA,NotInSig
            GroupIndex,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NotInSig,NotInSig,NA,NA,NotInSig
            GroupThreadID,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NotInSig,NotInSig,NA,NA,NotInSig
            TessFactor,NA,NA,NA,NA,NA,NA,TessFactor,TessFactor,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA
            InsideTessFactor,NA,NA,NA,NA,NA,NA,TessFactor,TessFactor,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA
            ViewID,NotInSig _61,NA,NotInSig _61,NotInSig _61,NA,NA,NA,NotInSig _61,NA,NA,NA,NotInSig _61,NA,NotInSig _61,NA,NA,NotInSig,NA,NA,NA
            Barycentrics,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NotPacked _61,NA,NA,NA,NA,NA,NA
            ShadingRate,NA,SV _64,NA,NA,SV _64,SV _64,NA,NA,SV _64,SV _64,SV _64,NA,SV _64,SV _64,NA,NA,NA,NA,SV,NA
            CullPrimitive,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NotInSig,NA,NA,NA,NA,NotPacked,NA
            StartVertexLocation,NotInSig _68,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA
            StartInstanceLocation,NotInSig _68,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA
        """
        table = [
            list(map(str.strip, line.split(",")))
            for line in SemanticInterpretationCSV.splitlines()
            if line.strip()
        ]
        for row in table[1:]:
            assert len(row) == len(table[0])  # Ensure table is rectangular
        # Make sure labels match enums, otherwise the table isn't aligned or in-sync
        assert table[0][1:] == SigPointKind.value_names()[:-1]  # exclude Invalid
        if not (
            [row[0] for row in table[1:]] == SemanticKind.value_names()[:-1]
        ):  # exclude Invalid
            assert (
                False
                and "SemanticKind does not align with SemanticInterpretationCSV row labels"
            )
        self.interpretation_table = table

    def build_valrules(self):
        self.add_valrule_msg(
            "Bitcode.Valid",
            "Module must be bitcode-valid",
            "Module bitcode is invalid.",
        )

        self.add_valrule_msg(
            "Container.PartMatches",
            "DXIL Container Parts must match Module",
            "Container part '%0' does not match expected for module.",
        )
        self.add_valrule_msg(
            "Container.PartRepeated",
            "DXIL Container must have only one of each part type",
            "More than one container part '%0'.",
        )
        self.add_valrule_msg(
            "Container.PartMissing",
            "DXIL Container requires certain parts, corresponding to module",
            "Missing part '%0' required by module.",
        )
        self.add_valrule_msg(
            "Container.PartInvalid",
            "DXIL Container must not contain unknown parts",
            "Unknown part '%0' found in DXIL container.",
        )
        self.add_valrule_msg(
            "Container.RootSignatureIncompatible",
            "Root Signature in DXIL Container must be compatible with shader",
            "Root Signature in DXIL container is not compatible with shader.",
        )
        self.add_valrule_msg(
            "Container.ContentMatches",
            "DXIL Container Content must match Module",
            "DXIL container mismatch for '%0' between '%1' part:('%2') and DXIL module:('%3')",
        )
        self.add_valrule_msg(
            "Container.ContentInvalid",
            "DXIL Container Content is well-formed",
            "In '%0', '%1' is not well-formed",
        )
        self.add_valrule_msg(
            "Container.UnusedItemInTable",
            "Items in Table must be used",
            "In '%0', '%1' is not used",
        )
        self.add_valrule("Meta.Required", "Required metadata missing.")
        self.add_valrule_msg(
            "Meta.ComputeWithNode",
            "Compute entry must not have node metadata",
            "Compute entry '%0' has unexpected node shader metadata",
        )
        self.add_valrule_msg(
            "Meta.Known",
            "Named metadata should be known",
            "Named metadata '%0' is unknown.",
        )
        self.add_valrule("Meta.Used", "All metadata must be used by dxil.")
        self.add_valrule_msg(
            "Meta.Target",
            "Target triple must be 'dxil-ms-dx'",
            "Unknown target triple '%0'.",
        )
        self.add_valrule(
            "Meta.WellFormed",
            "Metadata must be well-formed in operand count and types.",
        )  # TODO: add string arg for what metadata is malformed (this is emitted from a lot of places and provides no context whatsoever)
        self.add_valrule_msg(
            "Meta.VersionSupported",
            "Version in metadata must be supported.",
            "%0 version in metadata (%1.%2) is not supported; maximum: (%3.%4).",
        )
        self.add_valrule(
            "Meta.SemanticLen", "Semantic length must be at least 1 and at most 64."
        )
        self.add_valrule_msg(
            "Meta.InterpModeValid",
            "Interpolation mode must be valid",
            "Invalid interpolation mode for '%0'.",
        )
        self.add_valrule_msg(
            "Meta.SemaKindValid",
            "Semantic kind must be valid",
            "Semantic kind for '%0' is invalid.",
        )
        self.add_valrule_msg(
            "Meta.NoSemanticOverlap",
            "Semantics must not overlap",
            "Semantic '%0' overlap at %1.",
        )
        self.add_valrule_msg(
            "Meta.SemaKindMatchesName",
            "Semantic name must match system value, when defined.",
            "Semantic name %0 does not match System Value kind %1.",
        )
        self.add_valrule_msg(
            "Meta.DuplicateSysValue",
            "System value may only appear once in signature",
            "System value %0 appears more than once in the same signature.",
        )
        self.add_valrule_msg(
            "Meta.SemanticIndexMax",
            "System value semantics have a maximum valid semantic index",
            "%0 semantic index exceeds maximum (%1).",
        )
        self.add_valrule_msg(
            "Meta.SystemValueRows",
            "System value may only have 1 row",
            "rows for system value semantic %0 must be 1.",
        )
        self.add_valrule_msg(
            "Meta.SemanticShouldBeAllocated",
            "Semantic should have a valid packing location",
            "%0 Semantic '%1' should have a valid packing location.",
        )
        self.add_valrule_msg(
            "Meta.SemanticShouldNotBeAllocated",
            "Semantic should have a packing location of -1",
            "%0 Semantic '%1' should have a packing location of -1.",
        )
        self.add_valrule("Meta.ValueRange", "Metadata value must be within range.")
        self.add_valrule("Meta.FlagsUsage", "Flags must match usage.")
        self.add_valrule(
            "Meta.DenseResIDs", "Resource identifiers must be zero-based and dense."
        )
        self.add_valrule_msg(
            "Meta.SignatureOverlap",
            "Signature elements may not overlap in packing location.",
            "signature element %0 at location (%1,%2) size (%3,%4) overlaps another signature element.",
        )
        self.add_valrule_msg(
            "Meta.SignatureOutOfRange",
            "Signature elements must fit within maximum signature size",
            "signature element %0 at location (%1,%2) size (%3,%4) is out of range.",
        )
        self.add_valrule_msg(
            "Meta.SignatureIndexConflict",
            "Only elements with compatible indexing rules may be packed together",
            "signature element %0 at location (%1,%2) size (%3,%4) has an indexing conflict with another signature element packed into the same row.",
        )
        self.add_valrule_msg(
            "Meta.SignatureIllegalComponentOrder",
            "Component ordering for packed elements must be: arbitrary < system value < system generated value",
            "signature element %0 at location (%1,%2) size (%3,%4) violates component ordering rule (arb < sv < sgv).",
        )
        self.add_valrule_msg(
            "Meta.SignatureDataWidth",
            "Data width must be identical for all elements packed into the same row.",
            "signature element %0 at location (%1, %2) size (%3, %4) has data width that differs from another element packed into the same row.",
        )
        self.add_valrule_msg(
            "Meta.IntegerInterpMode",
            "Interpolation mode on integer must be Constant",
            "signature element %0 specifies invalid interpolation mode for integer component type.",
        )
        self.add_valrule_msg(
            "Meta.InterpModeInOneRow",
            "Interpolation mode must be identical for all elements packed into the same row.",
            "signature element %0 at location (%1,%2) size (%3,%4) has interpolation mode that differs from another element packed into the same row.",
        )
        self.add_valrule("Meta.SemanticCompType", "%0 must be %1.")
        self.add_valrule_msg(
            "Meta.ClipCullMaxRows",
            "Combined elements of SV_ClipDistance and SV_CullDistance must fit in two rows.",
            "ClipDistance and CullDistance occupy more than the maximum of 2 rows combined.",
        )
        self.add_valrule_msg(
            "Meta.ClipCullMaxComponents",
            "Combined elements of SV_ClipDistance and SV_CullDistance must fit in 8 components",
            "ClipDistance and CullDistance use more than the maximum of 8 components combined.",
        )
        self.add_valrule(
            "Meta.SignatureCompType",
            "signature %0 specifies unrecognized or invalid component type.",
        )
        self.add_valrule(
            "Meta.TessellatorPartition",
            "Invalid Tessellator Partitioning specified. Must be integer, pow2, fractional_odd or fractional_even.",
        )
        self.add_valrule(
            "Meta.TessellatorOutputPrimitive",
            "Invalid Tessellator Output Primitive specified. Must be point, line, triangleCW or triangleCCW.",
        )
        self.add_valrule(
            "Meta.MaxTessFactor",
            "Hull Shader MaxTessFactor must be [%0..%1].  %2 specified.",
        )
        self.add_valrule("Meta.ValidSamplerMode", "Invalid sampler mode on sampler .")
        self.add_valrule(
            "Meta.GlcNotOnAppendConsume",
            "globallycoherent cannot be used with append/consume buffers: '%0'.",
        )
        self.add_valrule_msg(
            "Meta.StructBufAlignment",
            "StructuredBuffer stride not aligned",
            "structured buffer element size must be a multiple of %0 bytes (actual size %1 bytes).",
        )
        self.add_valrule_msg(
            "Meta.StructBufAlignmentOutOfBound",
            "StructuredBuffer stride out of bounds",
            "structured buffer elements cannot be larger than %0 bytes (actual size %1 bytes).",
        )
        self.add_valrule("Meta.EntryFunction", "entrypoint not found.")
        self.add_valrule("Meta.InvalidControlFlowHint", "Invalid control flow hint.")
        self.add_valrule(
            "Meta.BranchFlatten", "Can't use branch and flatten attributes together."
        )
        self.add_valrule(
            "Meta.ForceCaseOnSwitch", "Attribute forcecase only works for switch."
        )
        self.add_valrule(
            "Meta.ControlFlowHintNotOnControlFlow",
            "Control flow hint only works on control flow inst.",
        )
        self.add_valrule(
            "Meta.TextureType",
            "elements of typed buffers and textures must fit in four 32-bit quantities.",
        )
        self.add_valrule(
            "Meta.BarycentricsInterpolation",
            "SV_Barycentrics cannot be used with 'nointerpolation' type.",
        )
        self.add_valrule(
            "Meta.BarycentricsFloat3",
            "only 'float3' type is allowed for SV_Barycentrics.",
        )
        self.add_valrule(
            "Meta.BarycentricsTwoPerspectives",
            "There can only be up to two input attributes of SV_Barycentrics with different perspective interpolation mode.",
        )
        self.add_valrule(
            "Meta.NoEntryPropsForEntry", "Entry point %0 must have entry properties."
        )

        self.add_valrule("Instr.Oload", "DXIL intrinsic overload must be valid.")
        self.add_valrule_msg(
            "Instr.CallOload",
            "Call to DXIL intrinsic must match overload signature",
            "Call to DXIL intrinsic '%0' does not match an allowed overload signature.",
        )
        self.add_valrule(
            "Instr.PtrBitCast", "Pointer type bitcast must be have same size."
        )
        self.add_valrule(
            "Instr.MinPrecisonBitCast", "Bitcast on minprecison types is not allowed."
        )
        self.add_valrule(
            "Instr.StructBitCast", "Bitcast on struct types is not allowed."
        )
        self.add_valrule(
            "Instr.Status",
            "Resource status should only be used by CheckAccessFullyMapped.",
        )
        self.add_valrule(
            "Instr.CheckAccessFullyMapped",
            "CheckAccessFullyMapped should only be used on resource status.",
        )
        self.add_valrule_msg(
            "Instr.OpConst",
            "DXIL intrinsic requires an immediate constant operand",
            "%0 of %1 must be an immediate constant.",
        )
        self.add_valrule("Instr.Allowed", "Instructions must be of an allowed type.")
        self.add_valrule(
            "Instr.OpCodeReserved", "Instructions must not reference reserved opcodes."
        )
        self.add_valrule_msg(
            "Instr.OperandRange",
            "DXIL intrinsic operand must be within defined range",
            "expect %0 between %1, got %2.",
        )
        self.add_valrule(
            "Instr.NoReadingUninitialized",
            "Instructions should not read uninitialized value.",
        )
        self.add_valrule(
            "Instr.NoGenericPtrAddrSpaceCast",
            "Address space cast between pointer types must have one part to be generic address space.",
        )
        self.add_valrule(
            "Instr.InBoundsAccess", "Access to out-of-bounds memory is disallowed."
        )
        self.add_valrule(
            "Instr.OpConstRange", "Constant values must be in-range for operation."
        )
        self.add_valrule(
            "Instr.ImmBiasForSampleB",
            "bias amount for sample_b must be in the range [%0,%1], but %2 was specified as an immediate.",
        )
        self.add_valrule(
            "Instr.IllegalDXILOpCode", "DXILOpCode must be [0..%0].  %1 specified."
        )
        self.add_valrule(
            "Instr.IllegalDXILOpFunction",
            "'%0' is not a DXILOpFuncition for DXILOpcode '%1'.",
        )
        # If streams have not been declared, you must use cut instead of cut_stream in GS - is there an equivalent rule here?

        # Need to clean up all error messages and actually implement.
        # Midlevel
        self.add_valrule("Instr.NoIndefiniteLog", "No indefinite logarithm.")
        self.add_valrule("Instr.NoIndefiniteAsin", "No indefinite arcsine.")
        self.add_valrule("Instr.NoIndefiniteAcos", "No indefinite arccosine.")
        self.add_valrule("Instr.NoIDivByZero", "No signed integer division by zero.")
        self.add_valrule("Instr.NoUDivByZero", "No unsigned integer division by zero.")
        self.add_valrule(
            "Instr.NoIndefiniteDsxy", "No indefinite derivative calculation."
        )
        self.add_valrule(
            "Instr.MinPrecisionNotPrecise",
            "Instructions marked precise may not refer to minprecision values.",
        )

        # Backend
        self.add_valrule(
            "Instr.OnlyOneAllocConsume",
            "RWStructuredBuffers may increment or decrement their counters, but not both.",
        )

        # CCompiler
        self.add_valrule(
            "Instr.TextureOffset",
            "offset texture instructions must take offset which can resolve to integer literal in the range -8 to 7.",
        )
        # D3D12
        self.add_valrule_msg(
            "Instr.CannotPullPosition",
            "pull-model evaluation of position disallowed",
            "%0 does not support pull-model evaluation of position.",
        )
        # self.add_valrule("Instr.ERR_GUARANTEED_RACE_CONDITION_UAV", "TODO - race condition writing to shared resource detected, consider making this write conditional.") warning on fxc.
        # self.add_valrule("Instr.ERR_GUARANTEED_RACE_CONDITION_GSM", "TODO - race condition writing to shared memory detected, consider making this write conditional.") warning on fxc.
        # self.add_valrule("Instr.ERR_INFINITE_LOOP", "TODO - ERR_INFINITE_LOOP") fxc will report error if it can prove the loop is infinite.
        self.add_valrule(
            "Instr.EvalInterpolationMode",
            "Interpolation mode on %0 used with eval_* instruction must be linear, linear_centroid, linear_noperspective, linear_noperspective_centroid, linear_sample or linear_noperspective_sample.",
        )
        self.add_valrule("Instr.ResourceCoordinateMiss", "coord uninitialized.")
        self.add_valrule(
            "Instr.ResourceCoordinateTooMany", "out of bound coord must be undef."
        )
        self.add_valrule("Instr.ResourceOffsetMiss", "offset uninitialized.")
        self.add_valrule(
            "Instr.ResourceOffsetTooMany", "out of bound offset must be undef."
        )
        self.add_valrule(
            "Instr.UndefResultForGetDimension",
            "GetDimensions used undef dimension %0 on %1.",
        )
        self.add_valrule(
            "Instr.SamplerModeForLOD",
            "lod instruction requires sampler declared in default mode.",
        )
        self.add_valrule(
            "Instr.SamplerModeForSample",
            "sample/_l/_d/_cl_s/gather instruction requires sampler declared in default mode.",
        )
        self.add_valrule(
            "Instr.SamplerModeForSampleC",
            "sample_c_*/gather_c instructions require sampler declared in comparison mode.",
        )
        self.add_valrule(
            "Instr.SampleCompType",
            "sample_* instructions require resource to be declared to return UNORM, SNORM or FLOAT.",
        )
        self.add_valrule(
            "Instr.BarrierModeUselessUGroup",
            "sync can't specify both _ugroup and _uglobal. If both are needed, just specify _uglobal.",
        )
        self.add_valrule(
            "Instr.BarrierModeNoMemory",
            "sync must include some form of memory barrier - _u (UAV) and/or _g (Thread Group Shared Memory).  Only _t (thread group sync) is optional.",
        )
        self.add_valrule(
            "Instr.BarrierModeForNonCS",
            "sync in a non-Compute/Amplification/Mesh/Node Shader must only sync UAV (sync_uglobal).",
        )
        self.add_valrule(
            "Instr.BarrierFlagInvalid", "Invalid %0 flags on DXIL operation '%1'"
        )
        self.add_valrule(
            "Instr.BarrierNonConstantFlagArgument",
            "Memory type, access, or sync flag is not constant",
        )
        self.add_valrule(
            "Instr.BarrierRequiresNode",
            "sync in a non-Node Shader must not sync node record memory.",
        )
        self.add_valrule(
            "Instr.WriteMaskForTypedUAVStore",
            "store on typed uav must write to all four components of the UAV.",
        )
        self.add_valrule(
            "Instr.WriteMaskGapForUAV",
            "UAV write mask must be contiguous, starting at x: .x, .xy, .xyz, or .xyzw.",
        )
        self.add_valrule(
            "Instr.ResourceKindForCalcLOD",
            "lod requires resource declared as texture1D/2D/3D/Cube/CubeArray/1DArray/2DArray.",
        )
        self.add_valrule(
            "Instr.ResourceKindForSample",
            "sample/_l/_d requires resource declared as texture1D/2D/3D/Cube/1DArray/2DArray/CubeArray.",
        )
        self.add_valrule(
            "Instr.ResourceKindForSampleC",
            "samplec requires resource declared as texture1D/2D/Cube/1DArray/2DArray/CubeArray.",
        )
        self.add_valrule(
            "Instr.ResourceKindForGather",
            "gather requires resource declared as texture/2D/Cube/2DArray/CubeArray.",
        )
        self.add_valrule(
            "Instr.WriteMaskMatchValueForUAVStore",
            "uav store write mask must match store value mask, write mask is %0 and store value mask is %1.",
        )
        self.add_valrule(
            "Instr.UndefinedValueForUAVStore", "Assignment of undefined values to UAV."
        )
        self.add_valrule(
            "Instr.ResourceKindForBufferLoadStore",
            "buffer load/store only works on Raw/Typed/StructuredBuffer.",
        )
        self.add_valrule(
            "Instr.ResourceKindForTextureStore",
            "texture store only works on Texture1D/1DArray/2D/2DArray/3D.",
        )
        self.add_valrule(
            "Instr.ResourceKindForGetDim", "Invalid resource kind on GetDimensions."
        )
        self.add_valrule(
            "Instr.ResourceKindForTextureLoad",
            "texture load only works on Texture1D/1DArray/2D/2DArray/3D/MS2D/MS2DArray.",
        )
        self.add_valrule(
            "Instr.ResourceClassForSamplerGather",
            "sample, lod and gather should be on srv resource.",
        )
        self.add_valrule(
            "Instr.ResourceClassForUAVStore", "store should be on uav resource."
        )
        self.add_valrule(
            "Instr.ResourceClassForLoad", "load can only run on UAV/SRV resource."
        )
        self.add_valrule(
            "Instr.ResourceMapToSingleEntry", "Fail to map resource to resource table."
        )
        self.add_valrule(
            "Instr.ResourceUser", "Resource should only be used by Load/GEP/Call."
        )
        self.add_valrule(
            "Instr.ResourceKindForTraceRay",
            "TraceRay should only use RTAccelerationStructure.",
        )
        self.add_valrule("Instr.OffsetOnUAVLoad", "uav load don't support offset.")
        self.add_valrule(
            "Instr.MipOnUAVLoad", "uav load don't support mipLevel/sampleIndex."
        )
        self.add_valrule(
            "Instr.SampleIndexForLoad2DMS",
            "load on Texture2DMS/2DMSArray require sampleIndex.",
        )
        self.add_valrule(
            "Instr.CoordinateCountForRawTypedBuf",
            "raw/typed buffer don't need 2 coordinates.",
        )
        self.add_valrule(
            "Instr.CoordinateCountForStructBuf",
            "structured buffer require 2 coordinates.",
        )
        self.add_valrule(
            "Instr.MipLevelForGetDimension",
            "Use mip level on buffer when GetDimensions.",
        )
        self.add_valrule(
            "Instr.DxilStructUser",
            "Dxil struct types should only be used by ExtractValue.",
        )
        self.add_valrule(
            "Instr.DxilStructUserOutOfBound",
            "Index out of bound when extract value from dxil struct types.",
        )
        self.add_valrule(
            "Instr.HandleNotFromCreateHandle",
            "Resource handle should returned by createHandle.",
        )
        self.add_valrule(
            "Instr.BufferUpdateCounterOnUAV", "BufferUpdateCounter valid only on UAV."
        )
        self.add_valrule(
            "Instr.BufferUpdateCounterOnResHasCounter",
            "BufferUpdateCounter valid only when HasCounter is true.",
        )
        self.add_valrule("Instr.CBufferOutOfBound", "Cbuffer access out of bound.")
        self.add_valrule(
            "Instr.CBufferClassForCBufferHandle",
            "Expect Cbuffer for CBufferLoad handle.",
        )
        self.add_valrule(
            "Instr.FailToResloveTGSMPointer",
            "TGSM pointers must originate from an unambiguous TGSM global variable.",
        )
        self.add_valrule(
            "Instr.ExtractValue",
            "ExtractValue should only be used on dxil struct types and cmpxchg.",
        )
        self.add_valrule(
            "Instr.TGSMRaceCond",
            "Race condition writing to shared memory detected, consider making this write conditional.",
        )
        self.add_valrule(
            "Instr.AttributeAtVertexNoInterpolation",
            "Attribute %0 must have nointerpolation mode in order to use GetAttributeAtVertex function.",
        )
        self.add_valrule(
            "Instr.CreateHandleImmRangeID",
            "Local resource must map to global resource.",
        )
        self.add_valrule(
            "Instr.SignatureOperationNotInEntry",
            "Dxil operation for input output signature must be in entryPoints.",
        )
        self.add_valrule(
            "Instr.MultipleSetMeshOutputCounts",
            "SetMeshOUtputCounts cannot be called multiple times.",
        )
        self.add_valrule(
            "Instr.MissingSetMeshOutputCounts", "Missing SetMeshOutputCounts call."
        )
        self.add_valrule(
            "Instr.NonDominatingSetMeshOutputCounts",
            "Non-Dominating SetMeshOutputCounts call.",
        )
        self.add_valrule(
            "Instr.MultipleGetMeshPayload",
            "GetMeshPayload cannot be called multiple times.",
        )
        self.add_valrule(
            "Instr.NotOnceDispatchMesh",
            "DispatchMesh must be called exactly once in an Amplification shader.",
        )
        self.add_valrule(
            "Instr.NonDominatingDispatchMesh", "Non-Dominating DispatchMesh call."
        )
        self.add_valrule(
            "Instr.AtomicOpNonGroupsharedOrRecord",
            "Non-groupshared or node record destination to atomic operation.",
        )
        self.add_valrule(
            "Instr.AtomicIntrinNonUAV", "Non-UAV destination to atomic intrinsic."
        )
        self.add_valrule_msg(
            "Instr.SVConflictingLaunchMode",
            "Input system values are compatible with node shader launch mode.",
            "Call to DXIL intrinsic %0 (%1) is not allowed in node shader launch type %2",
        )
        self.add_valrule("Instr.AtomicConst", "Constant destination to atomic.")

        # Work-Graphs
        self.add_valrule(
            "Instr.NodeRecordHandleUseAfterComplete",
            "Invalid use of completed record handle.",
        )

        # Some legacy rules:
        # - space is only supported for shader targets 5.1 and higher
        # - multiple rules regarding derivatives, which isn't a supported feature for DXIL
        # - multiple rules regarding library functions, which isn't a supported feature for DXIL (at this time)
        # - multiple rules regarding interfaces, which isn't a supported feature for DXIL
        # - rules for DX9-style intrinsics, which aren't supported for DXIL

        self.add_valrule_msg(
            "Types.NoVector",
            "Vector types must not be present",
            "Vector type '%0' is not allowed.",
        )
        self.add_valrule_msg(
            "Types.Defined",
            "Type must be defined based on DXIL primitives",
            "Type '%0' is not defined on DXIL primitives.",
        )
        self.add_valrule_msg(
            "Types.IntWidth",
            "Int type must be of valid width",
            "Int type '%0' has an invalid width.",
        )
        self.add_valrule(
            "Types.NoMultiDim", "Only one dimension allowed for array type."
        )
        self.add_valrule(
            "Types.NoPtrToPtr",
            "Pointers to pointers, or pointers in structures are not allowed.",
        )
        self.add_valrule(
            "Types.I8",
            "I8 can only be used as immediate value for intrinsic or as i8* via bitcast by lifetime intrinsics.",
        )

        self.add_valrule_msg(
            "Sm.Name",
            "Target shader model name must be known",
            "Unknown shader model '%0'.",
        )
        self.add_valrule_msg(
            "Sm.DxilVersion",
            "Target shader model requires specific Dxil Version",
            "Shader model requires Dxil Version %0.%1.",
        )
        self.add_valrule_msg(
            "Sm.ProgramVersion",
            "Program Version in Dxil Container does not match Dxil Module shader model version",
            "Program Version is %0.%1 but Dxil Module shader model version is %2.%3.",
        )
        self.add_valrule_msg(
            "Sm.Opcode",
            "Opcode must be defined in target shader model",
            "Opcode %0 not valid in shader model %1.",
        )
        self.add_valrule(
            "Sm.Operand", "Operand must be defined in target shader model."
        )
        self.add_valrule_msg(
            "Sm.Semantic",
            "Semantic must be defined in target shader model",
            "Semantic '%0' is invalid as %1 %2.",
        )
        self.add_valrule_msg(
            "Sm.NoInterpMode",
            "Interpolation mode must be undefined for VS input/PS output/patch constant.",
            "Interpolation mode for '%0' is set but should be undefined.",
        )
        self.add_valrule_msg(
            "Sm.ConstantInterpMode",
            "Interpolation mode must be constant for MS primitive output.",
            "Interpolation mode for '%0' should be constant.",
        )
        self.add_valrule(
            "Sm.NoPSOutputIdx", "Pixel shader output registers are not indexable."
        )  # TODO restrict to PS
        self.add_valrule(
            "Sm.PSConsistentInterp",
            "Interpolation mode for PS input position must be linear_noperspective_centroid or linear_noperspective_sample when outputting oDepthGE or oDepthLE and not running at sample frequency (which is forced by inputting SV_SampleIndex or declaring an input linear_sample or linear_noperspective_sample).",
        )
        self.add_valrule(
            "Sm.ThreadGroupChannelRange",
            "Declared Thread Group %0 size %1 outside valid range [%2..%3].",
        )
        self.add_valrule(
            "Sm.MaxTheadGroup",
            "Declared Thread Group Count %0 (X*Y*Z) is beyond the valid maximum of %1.",
        )
        self.add_valrule(
            "Sm.MaxTGSMSize",
            "Total Thread Group Shared Memory storage is %0, exceeded %1.",
        )
        self.add_valrule(
            "Sm.TGSMUnsupported", "Thread Group Shared Memory not supported %0."
        )
        self.add_valrule_msg(
            "Sm.WaveSizeValue",
            "WaveSize value must be a power of 2 in range [4..128]",
            "WaveSize %0 (%1) outside valid range [%2..%3], or not a power of 2.",
        )
        self.add_valrule_msg(
            "Sm.WaveSizeAllZeroWhenUndefined",
            "WaveSize Max and Preferred must be 0 when Min is 0",
            "WaveSize Max (%0) and Preferred (%1) must be 0 when Min is 0",
        )
        self.add_valrule_msg(
            "Sm.WaveSizeMaxAndPreferredZeroWhenNoRange",
            "WaveSize Max and Preferred must be 0 to encode min==max",
            "WaveSize Max (%0) and Preferred (%1) must be 0 to encode min==max",
        )
        self.add_valrule_msg(
            "Sm.WaveSizeMaxGreaterThanMin",
            "WaveSize Max must greater than Min",
            "WaveSize Max (%0) is less than Min (%1)",
        )
        self.add_valrule_msg(
            "Sm.WaveSizePreferredInRange",
            "WaveSize Preferred must be within Min..Max range",
            "WaveSize Preferred (%0) outside Min..Max range [%1..%2]",
        )
        self.add_valrule(
            "Sm.WaveSizeOnComputeOrNode",
            "WaveSize only allowed on compute or node shaders",
        )
        self.add_valrule(
            "Sm.WaveSizeNeedsSM66or67",
            "WaveSize is valid only for Shader Model 6.6 and 6.7.",
        )
        self.add_valrule(
            "Sm.WaveSizeRangeNeedsSM68Plus",
            "WaveSize Range is valid only for Shader Model 6.8 and higher.",
        )
        self.add_valrule(
            "Sm.WaveSizeRangeExpectsThreeParams",
            "WaveSize Range tag expects exactly 3 parameters.",
        )
        self.add_valrule(
            "Sm.WaveSizeExpectsOneParam",
            "WaveSize tag expects exactly 1 parameter.",
        )
        self.add_valrule(
            "Sm.WaveSizeTagDuplicate",
            "WaveSize or WaveSizeRange tag may only appear once per entry point.",
        )
        self.add_valrule(
            "Sm.WaveSizeNeedsConstantOperands",
            "WaveSize metadata operands must be constant values.",
        )
        self.add_valrule(
            "Sm.ROVOnlyInPS",
            "RasterizerOrdered objects are only allowed in 5.0+ pixel shaders.",
        )
        self.add_valrule(
            "Sm.TessFactorForDomain",
            "Required TessFactor for domain not found declared anywhere in Patch Constant data.",
        )
        self.add_valrule(
            "Sm.TessFactorSizeMatchDomain",
            "TessFactor rows, columns (%0, %1) invalid for domain %2.  Expected %3 rows and 1 column.",
        )
        self.add_valrule(
            "Sm.InsideTessFactorSizeMatchDomain",
            "InsideTessFactor rows, columns (%0, %1) invalid for domain %2.  Expected %3 rows and 1 column.",
        )
        self.add_valrule(
            "Sm.DomainLocationIdxOOB",
            "DomainLocation component index out of bounds for the domain.",
        )
        self.add_valrule(
            "Sm.HullPassThruControlPointCountMatch",
            "For pass thru hull shader, input control point count must match output control point count",
        )
        self.add_valrule(
            "Sm.OutputControlPointsTotalScalars",
            "Total number of scalars across all HS output control points must not exceed .",
        )
        self.add_valrule(
            "Sm.IsoLineOutputPrimitiveMismatch",
            "Hull Shader declared with IsoLine Domain must specify output primitive point or line. Triangle_cw or triangle_ccw output are not compatible with the IsoLine Domain.",
        )
        self.add_valrule(
            "Sm.TriOutputPrimitiveMismatch",
            "Hull Shader declared with Tri Domain must specify output primitive point, triangle_cw or triangle_ccw. Line output is not compatible with the Tri domain.",
        )
        self.add_valrule(
            "Sm.ValidDomain",
            "Invalid Tessellator Domain specified. Must be isoline, tri or quad.",
        )
        self.add_valrule(
            "Sm.PatchConstantOnlyForHSDS",
            "patch constant signature only valid in HS and DS.",
        )
        self.add_valrule(
            "Sm.StreamIndexRange", "Stream index (%0) must between 0 and %1."
        )
        self.add_valrule(
            "Sm.PSOutputSemantic",
            "Pixel Shader allows output semantics to be SV_Target, SV_Depth, SV_DepthGreaterEqual, SV_DepthLessEqual, SV_Coverage or SV_StencilRef, %0 found.",
        )
        self.add_valrule(
            "Sm.PSMultipleDepthSemantic",
            "Pixel Shader only allows one type of depth semantic to be declared.",
        )
        self.add_valrule(
            "Sm.PSTargetIndexMatchesRow",
            "SV_Target semantic index must match packed row location.",
        )
        self.add_valrule(
            "Sm.PSTargetCol0", "SV_Target packed location must start at column 0."
        )
        self.add_valrule(
            "Sm.PSCoverageAndInnerCoverage",
            "InnerCoverage and Coverage are mutually exclusive.",
        )
        self.add_valrule(
            "Sm.GSOutputVertexCountRange",
            "GS output vertex count must be [0..%0].  %1 specified.",
        )
        self.add_valrule(
            "Sm.GSInstanceCountRange",
            "GS instance count must be [1..%0].  %1 specified.",
        )
        self.add_valrule(
            "Sm.DSInputControlPointCountRange",
            "DS input control point count must be [0..%0].  %1 specified.",
        )
        self.add_valrule(
            "Sm.HSInputControlPointCountRange",
            "HS input control point count must be [0..%0].  %1 specified.",
        )
        self.add_valrule(
            "Sm.ZeroHSInputControlPointWithInput",
            "When HS input control point count is 0, no input signature should exist.",
        )
        self.add_valrule(
            "Sm.OutputControlPointCountRange",
            "output control point count must be [%0..%1].  %2 specified.",
        )
        self.add_valrule("Sm.GSValidInputPrimitive", "GS input primitive unrecognized.")
        self.add_valrule(
            "Sm.GSValidOutputPrimitiveTopology",
            "GS output primitive topology unrecognized.",
        )
        self.add_valrule(
            "Sm.AppendAndConsumeOnSameUAV",
            "BufferUpdateCounter inc and dec on a given UAV (%d) cannot both be in the same shader for shader model less than 5.1.",
        )
        self.add_valrule(
            "Sm.InvalidTextureKindOnUAV",
            "TextureCube[Array] resources are not supported with UAVs.",
        )
        self.add_valrule("Sm.InvalidResourceKind", "Invalid resources kind.")
        self.add_valrule("Sm.InvalidResourceCompType", "Invalid resource return type.")
        self.add_valrule(
            "Sm.InvalidSamplerFeedbackType", "Invalid sampler feedback type."
        )
        self.add_valrule(
            "Sm.SampleCountOnlyOn2DMS",
            "Only Texture2DMS/2DMSArray could has sample count.",
        )
        self.add_valrule(
            "Sm.CounterOnlyOnStructBuf",
            "BufferUpdateCounter valid only on structured buffers.",
        )
        self.add_valrule(
            "Sm.GSTotalOutputVertexDataRange",
            "Declared output vertex count (%0) multiplied by the total number of declared scalar components of output data (%1) equals %2.  This value cannot be greater than %3.",
        )
        self.add_valrule_msg(
            "Sm.MultiStreamMustBePoint",
            "When multiple GS output streams are used they must be pointlists",
            "Multiple GS output streams are used but '%0' is not pointlist.",
        )
        self.add_valrule(
            "Sm.CompletePosition", "Not all elements of SV_Position were written."
        )
        self.add_valrule(
            "Sm.UndefinedOutput", "Not all elements of output %0 were written."
        )
        self.add_valrule(
            "Sm.CSNoSignatures", "Compute shaders must not have shader signatures."
        )
        self.add_valrule(
            "Sm.CBufferTemplateTypeMustBeStruct",
            "D3D12 constant/texture buffer template element can only be a struct.",
        )
        self.add_valrule_msg(
            "Sm.ResourceRangeOverlap",
            "Resource ranges must not overlap",
            "Resource %0 with base %1 size %2 overlap with other resource with base %3 size %4 in space %5.",
        )
        self.add_valrule_msg(
            "Sm.CBufferSize",
            "CBuffer size must not exceed 65536 bytes",
            "CBuffer size is %0 bytes, exceeding maximum of 65536 bytes.",
        )
        self.add_valrule_msg(
            "Sm.CBufferOffsetOverlap",
            "CBuffer offsets must not overlap",
            "CBuffer %0 has offset overlaps at %1.",
        )
        self.add_valrule_msg(
            "Sm.CBufferElementOverflow",
            "CBuffer elements must not overflow",
            "CBuffer %0 size insufficient for element at offset %1.",
        )
        self.add_valrule_msg(
            "Sm.CBufferArrayOffsetAlignment",
            "CBuffer array offset must be aligned to 16-bytes",
            "CBuffer %0 has unaligned array offset at %1.",
        )
        self.add_valrule_msg(
            "Sm.OpcodeInInvalidFunction",
            "Invalid DXIL opcode usage like StorePatchConstant in patch constant function",
            "opcode '%0' should only be used in '%1'.",
        )
        self.add_valrule_msg(
            "Sm.ViewIDNeedsSlot",
            "ViewID requires compatible space in pixel shader input signature",
            "Pixel shader input signature lacks available space for ViewID.",
        )
        self.add_valrule(
            "Sm.64bitRawBufferLoadStore",
            "i64/f64 rawBufferLoad/Store overloads are allowed after SM 6.3.",
        )
        self.add_valrule(
            "Sm.RayShaderSignatures",
            "Ray tracing shader '%0' should not have any shader signatures.",
        )
        self.add_valrule(
            "Sm.RayShaderPayloadSize",
            "For shader '%0', %1 size is smaller than argument's allocation size.",
        )
        self.add_valrule(
            "Sm.MeshShaderMaxVertexCount",
            "MS max vertex output count must be [0..%0].  %1 specified.",
        )
        self.add_valrule(
            "Sm.MeshShaderMaxPrimitiveCount",
            "MS max primitive output count must be [0..%0].  %1 specified.",
        )
        self.add_valrule(
            "Sm.MeshShaderPayloadSize",
            "For mesh shader with entry '%0', payload size %1 is greater than maximum size of %2 bytes.",
        )
        self.add_valrule(
            "Sm.MeshShaderPayloadSizeDeclared",
            "For mesh shader with entry '%0', payload size %1 is greater than declared size of %2 bytes.",
        )
        self.add_valrule(
            "Sm.MeshShaderOutputSize",
            "For shader '%0', vertex plus primitive output size is greater than %1.",
        )
        self.add_valrule(
            "Sm.MeshShaderInOutSize",
            "For shader '%0', payload plus output size is greater than %1.",
        )
        self.add_valrule(
            "Sm.MeshVSigRowCount",
            "For shader '%0', vertex output signatures are taking up more than %1 rows.",
        )
        self.add_valrule(
            "Sm.MeshPSigRowCount",
            "For shader '%0', primitive output signatures are taking up more than %1 rows.",
        )
        self.add_valrule(
            "Sm.MeshTotalSigRowCount",
            "For shader '%0', vertex and primitive output signatures are taking up more than %1 rows.",
        )
        self.add_valrule(
            "Sm.MaxMSSMSize",
            "Total Thread Group Shared Memory storage is %0, exceeded %1.",
        )
        self.add_valrule(
            "Sm.AmplificationShaderPayloadSize",
            "For amplification shader with entry '%0', payload size %1 is greater than maximum size of %2 bytes.",
        )
        self.add_valrule(
            "Sm.AmplificationShaderPayloadSizeDeclared",
            "For amplification shader with entry '%0', payload size %1 is greater than declared size of %2 bytes.",
        )

        # fxc relaxed check of gradient check.
        # self.add_valrule("Uni.NoUniInDiv", "TODO - No instruction requiring uniform execution can be present in divergent block")
        # self.add_valrule("Uni.GradientFlow", "TODO - No divergent gradient operations inside flow control") # a bit more specific than the prior rule
        # self.add_valrule("Uni.ThreadSync", "TODO - Thread sync operation must be in non-varying flow control due to a potential race condition, adding a sync after reading any values controlling shader execution at this point")
        # self.add_valrule("Uni.NoWaveSensitiveGradient", "Gradient operations are not affected by wave-sensitive data or control flow.")

        self.add_valrule("Flow.Reducible", "Execution flow must be reducible.")
        self.add_valrule("Flow.NoRecursion", "Recursion is not permitted.")
        self.add_valrule("Flow.DeadLoop", "Loop must have break.")
        self.add_valrule_msg(
            "Flow.FunctionCall",
            "Function with parameter is not permitted",
            "Function %0 with parameter is not permitted, it should be inlined.",
        )

        self.add_valrule_msg(
            "Decl.DxilNsReserved",
            "The DXIL reserved prefixes must only be used by built-in functions and types",
            "Declaration '%0' uses a reserved prefix.",
        )
        self.add_valrule_msg(
            "Decl.DxilFnExtern",
            "External function must be a DXIL function",
            "External function '%0' is not a DXIL function.",
        )
        self.add_valrule_msg(
            "Decl.UsedInternal",
            "Internal declaration must be used",
            "Internal declaration '%0' is unused.",
        )
        self.add_valrule_msg(
            "Decl.NotUsedExternal",
            "External declaration should not be used",
            "External declaration '%0' is unused.",
        )
        self.add_valrule_msg(
            "Decl.UsedExternalFunction",
            "External function must be used",
            "External function '%0' is unused.",
        )
        self.add_valrule_msg(
            "Decl.FnIsCalled",
            "Functions can only be used by call instructions",
            "Function '%0' is used for something other than calling.",
        )
        self.add_valrule_msg(
            "Decl.FnFlattenParam",
            "Function parameters must not use struct types",
            "Type '%0' is a struct type but is used as a parameter in function '%1'.",
        )
        self.add_valrule_msg(
            "Decl.FnAttribute",
            "Functions should only contain known function attributes",
            "Function '%0' contains invalid attribute '%1' with value '%2'.",
        )
        self.add_valrule_msg(
            "Decl.ResourceInFnSig",
            "Resources not allowed in function signatures",
            "Function '%0' uses resource in function signature.",
        )
        self.add_valrule_msg(
            "Decl.RayQueryInFnSig",
            "Rayquery objects not allowed in function signatures",
            "Function '%0' uses rayquery object in function signature.",
        )
        self.add_valrule_msg(
            "Decl.PayloadStruct",
            "Payload parameter must be struct type",
            "Argument '%0' must be a struct type for payload in shader function '%1'.",
        )
        self.add_valrule_msg(
            "Decl.AttrStruct",
            "Attributes parameter must be struct type",
            "Argument '%0' must be a struct type for attributes in shader function '%1'.",
        )
        self.add_valrule_msg(
            "Decl.ParamStruct",
            "Callable function parameter must be struct type",
            "Argument '%0' must be a struct type for callable shader function '%1'.",
        )
        self.add_valrule_msg(
            "Decl.ExtraArgs",
            "Extra arguments not allowed for shader functions",
            "Extra argument '%0' not allowed for shader function '%1'.",
        )
        self.add_valrule_msg(
            "Decl.ShaderReturnVoid",
            "Shader functions must return void",
            "Shader function '%0' must have void return type.",
        )
        self.add_valrule_msg(
            "Decl.ShaderMissingArg",
            "payload/params/attributes parameter is required for certain shader types",
            "%0 shader '%1' missing required %2 parameter.",
        )
        self.add_valrule_msg(
            "Decl.MultipleNodeInputs",
            "A node shader may not have more than one input record",
            "node shader '%0' may not have more than one input record (%1 are declared)",
        )
        self.add_valrule_msg(
            "Decl.NodeLaunchInputType",
            "Invalid input record type for node launch type",
            "%0 node shader '%1' has incompatible input record type (should be %2)",
        )

        # These errors are emitted from ShaderCompatInfo validation.
        # If a called function is identifiable as a potential source of the
        # incompatibility, you get Sm.IncompatibleCallInEntry,
        # otherwise you get Sm.IncompatibleOperation.
        # You also get the specific incompatibilities found with one function
        # introducing each problem.
        # These may be emitted in addition to another specific operation
        # validation error that identifies the root cause, but is meant to
        # catch cases currently missed by other validation.
        self.add_valrule_msg(
            "Sm.IncompatibleCallInEntry",
            "Features used in internal function calls must be compatible with entry",
            "Entry function calls one or more functions using incompatible features.  See other errors for details.",
        )
        self.add_valrule_msg(
            "Sm.IncompatibleOperation",
            "Operations used in entry function must be compatible with shader stage and other properties",
            "Entry function performs some operation that is incompatible with the shader stage or other entry properties.  See other errors for details.",
        )
        self.add_valrule_msg(
            "Sm.IncompatibleStage",
            "Functions may only use features available in the entry function's stage",
            "Function uses features incompatible with the shader stage (%0) of the entry function.",
        )
        self.add_valrule_msg(
            "Sm.IncompatibleShaderModel",
            "Functions may only use features available in the current shader model",
            "Function uses features incompatible with the shader model.",
        )
        self.add_valrule_msg(
            "Sm.IncompatibleThreadGroupDim",
            "When derivatives are used in compute-model shaders, the thread group dimensions must be compatible",
            "Function uses derivatives in compute-model shader with NumThreads (%0, %1, %2); derivatives require NumThreads to be 1D and a multiple of 4, or 2D/3D with X and Y both being a multiple of 2.",
        )
        self.add_valrule_msg(
            "Sm.IncompatibleDerivInComputeShaderModel",
            "Derivatives in compute-model shaders require shader model 6.6 and above",
            "Function uses derivatives in compute-model shader, which is only supported in shader model 6.6 and above.",
        )
        self.add_valrule_msg(
            "Sm.IncompatibleRequiresGroup",
            "Functions requiring groupshared memory must be called from shaders with a visible group",
            "Function requires a visible group, but is called from a shader without one.",
        )
        self.add_valrule_msg(
            "Sm.IncompatibleDerivLaunch",
            "Node shaders only support derivatives in broadcasting launch mode",
            "Function called from %0 launch node shader uses derivatives; only broadcasting launch supports derivatives.",
        )

        # Assign sensible category names and build up an enumeration description
        cat_names = {
            "CONTAINER": "Container",
            "BITCODE": "Bitcode",
            "META": "Metadata",
            "INSTR": "Instruction",
            "FLOW": "Program flow",
            "TYPES": "Type system",
            "SM": "Shader model",
            "UNI": "Uniform analysis",
            "DECL": "Declaration",
        }
        valrule_enum = db_dxil_enum("ValidationRule", "Known validation rules")
        valrule_enum.is_internal = True
        for vr in self.val_rules:
            vr.category = cat_names[vr.group_name]
            vrval = db_dxil_enum_value(vr.enum_name, vr.rule_id, vr.doc)
            vrval.category = vr.category
            vrval.err_msg = vr.err_msg
            valrule_enum.values.append(vrval)
        self.enums.append(valrule_enum)

    def populate_counters(self):
        self.llvm_op_counters = set()
        self.dxil_op_counters = set()
        for i in self.instr:
            counters = getattr(i, "props", {}).get("counters", ())
            if i.dxil_opid:
                self.dxil_op_counters.update(counters)
            else:
                self.llvm_op_counters.update(counters)
        counter_set = set(self.counters)
        counter_set.update(self.llvm_op_counters)
        counter_set.update(self.dxil_op_counters)
        self.counters = list(sorted(counter_set))

    def add_valrule(self, name, desc):
        self.val_rules.append(
            db_dxil_valrule(name, len(self.val_rules), err_msg=desc, doc=desc)
        )

    def add_valrule_msg(self, name, desc, err_msg):
        self.val_rules.append(
            db_dxil_valrule(name, len(self.val_rules), err_msg=err_msg, doc=desc)
        )

    def add_llvm_instr(
        self, kind, llvm_id, name, llvm_name, doc, oload_types, op_params, **props
    ):
        i = db_dxil_inst(
            name,
            llvm_id=llvm_id,
            llvm_name=llvm_name,
            doc=doc,
            ops=op_params,
            oload_types=oload_types,
        )
        i.props = props
        self.instr.append(i)

    def add_dxil_op(
        self, name, code_id, code_class, doc, oload_types, fn_attr, op_params, **props
    ):
        # The return value is parameter 0, insert the opcode as 1.
        op_params.insert(1, self.opcode_param)
        i = db_dxil_inst(
            name,
            llvm_id=self.call_instr.llvm_id,
            llvm_name=self.call_instr.llvm_name,
            dxil_op=name,
            dxil_opid=code_id,
            doc=doc,
            ops=op_params,
            dxil_class=code_class,
            oload_types=oload_types,
            fn_attr=fn_attr,
        )
        i.props = props
        self.instr.append(i)

    def add_dxil_op_reserved(self, name, code_id):
        # The return value is parameter 0, insert the opcode as 1.
        op_params = [db_dxil_param(0, "v", "", "reserved"), self.opcode_param]
        i = db_dxil_inst(
            name,
            llvm_id=self.call_instr.llvm_id,
            llvm_name=self.call_instr.llvm_name,
            dxil_op=name,
            dxil_opid=code_id,
            doc="reserved",
            ops=op_params,
            dxil_class="Reserved",
            oload_types="v",
            fn_attr="",
        )
        self.instr.append(i)

    def get_instr_by_llvm_name(self, llvm_name):
        "Return the instruction with the given LLVM name"
        return next(i for i in self.instr if i.llvm_name == llvm_name)

    def get_dxil_insts(self):
        for i in self.instr:
            if i.dxil_op != "":
                yield i

    def print_stats(self):
        "Print some basic statistics on the instruction database."
        print("Instruction count:                  %d" % len(self.instr))
        print(
            "Max parameter count in instruction: %d"
            % max(len(i.ops) - 1 for i in self.instr)
        )
        print(
            "Parameter count:                    %d"
            % sum(len(i.ops) - 1 for i in self.instr)
        )


###############################################################################
# HLSL-specific information.                                                  #
###############################################################################


class db_hlsl_attribute(object):
    "An HLSL attribute declaration"

    def __init__(self, title_name, scope, args, doc):
        self.name = title_name.lower()  # lowercase attribute name
        self.title_name = title_name  # title-case attribute name
        self.scope = scope  # one of l (loop), c (condition), s (switch), f (function)
        self.args = args  # list of arguments
        self.doc = doc  # documentation


class db_hlsl_intrinsic(object):
    "An HLSL intrinsic declaration"

    def __init__(
        self,
        name,
        idx,
        opname,
        params,
        ns,
        ns_idx,
        doc,
        ro,
        rn,
        amo,
        wv,
        unsigned_op,
        overload_idx,
        hidden,
    ):
        self.name = name  # Function name
        self.idx = idx  # Unique number within namespace
        self.opname = opname  # D3D-style name
        self.params = params  # List of parameters
        self.ns = ns  # Function namespace
        self.ns_idx = ns_idx  # Namespace index
        self.doc = doc  # Documentation
        id_prefix = "IOP" if ns == "Intrinsics" else "MOP"
        # SPIR-V Change Starts
        if ns == "VkIntrinsics":
            name = "Vk" + name
            self.name = "Vk" + self.name
            id_prefix = "IOP"
        # SPIR-V Change Ends
        self.enum_name = "%s_%s" % (id_prefix, name)  # enum name
        self.readonly = ro  # Only read memory
        self.readnone = rn  # Not read memory
        self.argmemonly = amo  # Only accesses memory through argument pointers
        self.wave = wv  # Is wave-sensitive
        self.unsigned_op = unsigned_op  # Unsigned opcode if exist
        if unsigned_op != "":
            self.unsigned_op = "%s_%s" % (id_prefix, unsigned_op)
        self.overload_param_index = (
            overload_idx  # Parameter determines the overload type, -1 means ret type
        )
        self.hidden = hidden  # Internal high-level op, not exposed to HLSL
        self.key = (
            ("%3d" % ns_idx)
            + "!"
            + name
            + "!"
            + ("%2d" % len(params))
            + "!"
            + ("%3d" % idx)
        )  # Unique key
        self.vulkanSpecific = ns.startswith(
            "Vk"
        )  # Vulkan specific intrinsic - SPIRV change


class db_hlsl_namespace(object):
    "A grouping of HLSL intrinsics"

    def __init__(self, name):
        self.name = name
        self.intrinsics = []


class db_hlsl_intrisic_param(object):
    "An HLSL parameter declaration for an intrinsic"

    def __init__(
        self,
        name,
        param_qual,
        template_id,
        template_list,
        component_id,
        component_list,
        rows,
        cols,
        type_name,
        idx,
        template_id_idx,
        component_id_idx,
    ):
        self.name = name  # Parameter name
        self.param_qual = param_qual  # Parameter qualifier expressions
        self.template_id = template_id  # Template ID (possibly identifier)
        self.template_list = template_list  # Template list (possibly identifier)
        self.component_id = component_id  # Component ID (possibly identifier)
        self.component_list = component_list  # Component list (possibly identifier)
        self.rows = rows  # Row count for parameter, possibly identifier
        self.cols = cols  # Row count for parameter, possibly identifier
        self.type_name = type_name  # Type name
        self.idx = idx  # Argument index
        self.template_id_idx = template_id_idx  # Template ID numeric value
        self.component_id_idx = component_id_idx  # Component ID numeric value


class db_hlsl(object):
    "A database of HLSL language data"

    def __init__(self, intrinsic_defs):
        self.base_types = {
            "bool": "LICOMPTYPE_BOOL",
            "int": "LICOMPTYPE_INT",
            "int32_only": "LICOMPTYPE_INT32_ONLY",
            "int64_only": "LICOMPTYPE_INT64_ONLY",
            "int16_t": "LICOMPTYPE_INT16",
            "uint": "LICOMPTYPE_UINT",
            "uint16_t": "LICOMPTYPE_UINT16",
            "u64": "LICOMPTYPE_UINT64",
            "any_int": "LICOMPTYPE_ANY_INT",
            "any_int32": "LICOMPTYPE_ANY_INT32",
            "any_int64": "LICOMPTYPE_ANY_INT64",
            "uint_only": "LICOMPTYPE_UINT_ONLY",
            "int8_t4_packed": "LICOMPTYPE_INT8_4PACKED",
            "uint8_t4_packed": "LICOMPTYPE_UINT8_4PACKED",
            "float16_t": "LICOMPTYPE_FLOAT16",
            "float": "LICOMPTYPE_FLOAT",
            "float32_only": "LICOMPTYPE_FLOAT32_ONLY",
            "fldbl": "LICOMPTYPE_FLOAT_DOUBLE",
            "any_float": "LICOMPTYPE_ANY_FLOAT",
            "float_like": "LICOMPTYPE_FLOAT_LIKE",
            "double": "LICOMPTYPE_DOUBLE",
            "double_only": "LICOMPTYPE_DOUBLE_ONLY",
            "numeric": "LICOMPTYPE_NUMERIC",
            "numeric16_only": "LICOMPTYPE_NUMERIC16_ONLY",
            "numeric32": "LICOMPTYPE_NUMERIC32",
            "numeric32_only": "LICOMPTYPE_NUMERIC32_ONLY",
            "any": "LICOMPTYPE_ANY",
            "sampler1d": "LICOMPTYPE_SAMPLER1D",
            "sampler2d": "LICOMPTYPE_SAMPLER2D",
            "sampler3d": "LICOMPTYPE_SAMPLER3D",
            "sampler_cube": "LICOMPTYPE_SAMPLERCUBE",
            "sampler_cmp": "LICOMPTYPE_SAMPLERCMP",
            "sampler": "LICOMPTYPE_SAMPLER",
            "resource": "LICOMPTYPE_RESOURCE",
            "ray_desc": "LICOMPTYPE_RAYDESC",
            "acceleration_struct": "LICOMPTYPE_ACCELERATION_STRUCT",
            "udt": "LICOMPTYPE_USER_DEFINED_TYPE",
            "void": "LICOMPTYPE_VOID",
            "string": "LICOMPTYPE_STRING",
            "Texture2D": "LICOMPTYPE_TEXTURE2D",
            "Texture2DArray": "LICOMPTYPE_TEXTURE2DARRAY",
            "wave": "LICOMPTYPE_WAVE",
            "p32i8": "LICOMPTYPE_INT8_4PACKED",
            "p32u8": "LICOMPTYPE_UINT8_4PACKED",
            "any_int16or32": "LICOMPTYPE_ANY_INT16_OR_32",
            "sint16or32_only": "LICOMPTYPE_SINT16_OR_32_ONLY",
            "any_sampler": "LICOMPTYPE_ANY_SAMPLER",
            "ByteAddressBuffer": "LICOMPTYPE_BYTEADDRESSBUFFER",
            "RWByteAddressBuffer": "LICOMPTYPE_RWBYTEADDRESSBUFFER",
            "NodeRecordOrUAV": "LICOMPTYPE_NODE_RECORD_OR_UAV",
            "AnyNodeOutputRecord": "LICOMPTYPE_ANY_NODE_OUTPUT_RECORD",
            "GroupNodeOutputRecords": "LICOMPTYPE_GROUP_NODE_OUTPUT_RECORDS",
            "ThreadNodeOutputRecords": "LICOMPTYPE_THREAD_NODE_OUTPUT_RECORDS",
        }

        self.trans_rowcol = {"r": "IA_R", "c": "IA_C", "r2": "IA_R2", "c2": "IA_C2"}
        self.param_qual = {
            "in": "AR_QUAL_IN",
            "inout": "AR_QUAL_IN | AR_QUAL_OUT",
            "ref": "AR_QUAL_REF",
            "out": "AR_QUAL_OUT",
            "col_major": "AR_QUAL_COLMAJOR",
            "row_major": "AR_QUAL_ROWMAJOR",
            "groupshared": "AR_QUAL_GROUPSHARED",
        }
        self.intrinsics = []
        self.load_intrinsics(intrinsic_defs)
        self.create_namespaces()
        self.populate_attributes()
        self.opcode_namespace = "hlsl::IntrinsicOp"

    def create_namespaces(self):
        last_ns = None
        self.namespaces = {}
        for i in sorted(self.intrinsics, key=lambda x: x.key):
            if last_ns is None or last_ns.name != i.ns:
                last_ns = db_hlsl_namespace(i.ns)
                self.namespaces[i.ns] = last_ns
            last_ns.intrinsics.append(i)

    def load_intrinsics(self, intrinsic_defs):
        import re

        blank_re = re.compile(r"^\s*$")
        comment_re = re.compile(r"^\s*//")
        namespace_beg_re = re.compile(r"^namespace\s+(\w+)\s*{\s*$")
        namespace_end_re = re.compile(r"^}\s*namespace\s*$")
        intrinsic_re = re.compile(
            r"^\s*([^(]+)\s+\[\[(\S*)\]\]\s+(\w+)\s*\(\s*([^)]*)\s*\)\s*(:\s*\w+\s*)?;$"
        )
        operand_re = re.compile(r"^:\s*(\w+)\s*$")
        bracket_cleanup_re = re.compile(
            r"<\s*(\S+)\s*,\s*(\S+)\s*>"
        )  # change <a,b> to <a@> to help split params and parse
        params_split_re = re.compile(r"\s*,\s*")
        ws_split_re = re.compile(r"\s+")
        typeref_re = re.compile(r"\$type(\d+)$")
        type_matrix_re = re.compile(r"(\S+)<(\S+)@(\S+)>$")
        type_vector_re = re.compile(r"(\S+)<(\S+)>$")
        type_any_re = re.compile(r"(\S+)<>$")
        type_array_re = re.compile(r"(\S+)\[\]$")
        type_object_re = re.compile(
            r"""(
            sampler\w* | string |
            (?:RW)?(?:Texture\w*|ByteAddressBuffer) |
            acceleration_struct | ray_desc |
            Node\w* | RWNode\w* | EmptyNode\w* |
            AnyNodeOutput\w* | NodeOutputRecord\w* | GroupShared\w*
            $)""",
            flags=re.VERBOSE,
        )
        digits_re = re.compile(r"^\d+$")
        opt_param_match_re = re.compile(r"^\$match<(\S+)@(\S+)>$")
        ns_idx = 0
        num_entries = 0

        def add_flag(val, new_val):
            if val == "" or val == "0":
                return new_val
            return val + " | " + new_val

        def translate_rowcol(val):
            digits_match = digits_re.match(val)
            if digits_match:
                return val
            assert val in self.trans_rowcol, "unknown row/col %s" % val
            return self.trans_rowcol[val]

        def process_arg(desc, idx, done_args, intrinsic_name):
            "Process a single parameter description."
            opt_list = []
            desc = desc.strip()
            if desc == "...":
                param_name = "..."
                type_name = "..."
            else:
                opt_list = ws_split_re.split(desc)
                assert len(opt_list) > 0, "malformed parameter desc %s" % (desc)
                param_name = opt_list.pop()  # last token is name
                type_name = opt_list.pop()  # next-to-last is type specifier

            param_qual = "0"
            template_id = str(idx)
            template_list = "LITEMPLATE_ANY"
            component_id = str(idx)
            component_list = "LICOMPTYPE_ANY"
            rows = "1"
            cols = "1"
            if type_name == "$classT":
                assert idx == 0, "'$classT' can only be used as the return type"
                # template_id may be -1 in other places other than return type, for example in Stream.Append().
                # $unspec is a shorthand for return types only though.
                template_id = "-1"
                component_id = "0"
                type_name = "void"
            if type_name == "$funcT":
                template_id = "-3"
                component_id = "0"
                type_name = "void"
            elif type_name == "...":
                assert idx != 0, "'...' can only be used in the parameter list"
                template_id = "-2"
                component_id = "0"
                type_name = "void"
            else:
                typeref_match = typeref_re.match(type_name)
                if typeref_match:
                    template_id = typeref_match.group(1)
                    component_id = template_id
                    assert idx != 1, "Can't use $type on the first argument"
                    assert template_id != "0", "Can't match an input to the return type"
                    done_idx = int(template_id) - 1
                    assert (
                        done_idx <= len(args) + 1
                    ), "$type must refer to a processed arg"
                    done_arg = done_args[done_idx]
                    type_name = done_arg.type_name

            # Determine matrix/vector/any/scalar/array/object type names.
            base_type = type_name

            def do_matrix(m):
                base_type, rows, cols = m.groups()
                template_list = "LITEMPLATE_MATRIX"
                return base_type, rows, cols, template_list

            def do_vector(m):
                base_type, cols = m.groups()
                template_list = "LITEMPLATE_VECTOR"
                return base_type, rows, cols, template_list

            def do_any(m):
                base_type = m.group(1)
                rows = "r"
                cols = "c"
                template_list = "LITEMPLATE_ANY"
                return base_type, rows, cols, template_list

            def do_array(m):
                base_type = m.group(1)
                cols = "c"
                template_list = "LITEMPLATE_ARRAY"
                return base_type, rows, cols, template_list

            def do_object(m):
                template_list = "LITEMPLATE_OBJECT"
                return base_type, rows, cols, template_list

            templates = [
                (do_matrix, type_matrix_re),
                (do_vector, type_vector_re),
                (do_any, type_any_re),
                (do_array, type_array_re),
                (do_object, type_object_re),
            ]
            for do, type_re in templates:
                m = type_re.match(type_name)
                if m:
                    base_type, rows, cols, template_list = do(m)
                    break
            else:
                type_vector_match = type_vector_re.match(type_name)
                if type_vector_match:
                    base_type = type_vector_match.group(1)
                    cols = type_vector_match.group(2)
                    template_list = "LITEMPLATE_VECTOR"
                else:
                    type_any_match = type_any_re.match(type_name)
                    if type_any_match:
                        base_type = type_any_match.group(1)
                        rows = "r"
                        cols = "c"
                        template_list = "LITEMPLATE_ANY"
                    else:
                        base_type = type_name
                        if (
                            base_type.startswith("sampler")
                            or base_type.startswith("string")
                            or base_type.startswith("Texture")
                            or base_type.startswith("wave")
                            or base_type.startswith("acceleration_struct")
                            or base_type.startswith("ray_desc")
                            or base_type.startswith("any_sampler")
                        ):
                            template_list = "LITEMPLATE_OBJECT"
                        else:
                            template_list = "LITEMPLATE_SCALAR"
            assert base_type in self.base_types, "Unknown base type '%s' in '%s'" % (
                base_type,
                desc,
            )
            component_list = self.base_types[base_type]
            rows = translate_rowcol(rows)
            cols = translate_rowcol(cols)
            for opt in opt_list:
                if opt in self.param_qual:
                    param_qual = add_flag(param_qual, self.param_qual[opt])
                else:
                    opt_param_match_match = opt_param_match_re.match(opt)
                    assert opt_param_match_match, "Unknown parameter qualifier '%s'" % (
                        opt
                    )
                    template_id = opt_param_match_match.group(1)
                    component_id = opt_param_match_match.group(2)
            if component_list == "LICOMPTYPE_VOID":
                if type_name == "void":
                    template_list = "LITEMPLATE_VOID"
                    rows = "0"
                    cols = "0"
                    if template_id == "0":
                        param_qual = "0"
            # Keep these as numeric values.
            template_id_idx = int(template_id)
            component_id_idx = int(component_id)
            # Verify that references don't point to the right (except for the return value).
            assert idx == 0 or template_id_idx <= int(
                idx
            ), "Argument '%s' has a forward reference" % (param_name)
            assert idx == 0 or component_id_idx <= int(
                idx
            ), "Argument '%s' has a forward reference" % (param_name)
            if template_id == "-1":
                template_id = "INTRIN_TEMPLATE_FROM_TYPE"
            elif template_id == "-2":
                template_id = "INTRIN_TEMPLATE_VARARGS"
            elif template_id == "-3":
                template_id = "INTRIN_TEMPLATE_FROM_FUNCTION"
            if component_id == "-1":
                component_id = "INTRIN_COMPTYPE_FROM_TYPE_ELT0"
            if component_id == "-2":
                component_id = "INTRIN_COMPTYPE_FROM_NODEOUTPUT"
            return db_hlsl_intrisic_param(
                param_name,
                param_qual,
                template_id,
                template_list,
                component_id,
                component_list,
                rows,
                cols,
                type_name,
                idx,
                template_id_idx,
                component_id_idx,
            )

        def process_attr(attr):
            attrs = attr.split(",")
            readonly = False  # Only read memory
            readnone = False  # Not read memory
            argmemonly = False  # Only reads memory through pointer arguments
            is_wave = False
            # Is wave-sensitive
            unsigned_op = ""  # Unsigned opcode if exist
            overload_param_index = (
                -1
            )  # Parameter determines the overload type, -1 means ret type.
            hidden = False
            for a in attrs:
                if a == "":
                    continue
                if a == "ro":
                    readonly = True
                    continue
                if a == "rn":
                    readnone = True
                    continue
                if a == "amo":
                    argmemonly = True
                    continue
                if a == "wv":
                    is_wave = True
                    continue
                if a == "hidden":
                    hidden = True
                    continue

                assign = a.split("=")

                if len(assign) != 2:
                    assert False, "invalid attr %s" % (a)
                    continue
                d = assign[0]
                v = assign[1]
                if d == "unsigned_op":
                    unsigned_op = v
                    continue
                if d == "overload":
                    overload_param_index = int(v)
                    continue
                assert False, "invalid attr %s" % (a)

            return (
                readonly,
                readnone,
                argmemonly,
                is_wave,
                unsigned_op,
                overload_param_index,
                hidden,
            )

        current_namespace = None
        for line in intrinsic_defs:
            if blank_re.match(line):
                continue
            if comment_re.match(line):
                continue
            match_obj = namespace_beg_re.match(line)
            if match_obj:
                assert (
                    not current_namespace
                ), "cannot open namespace without closing prior one"
                current_namespace = match_obj.group(1)
                num_entries = 0
                ns_idx += 1
                continue
            if namespace_end_re.match(line):
                assert (
                    current_namespace
                ), "cannot close namespace without previously opening it"
                current_namespace = None
                continue
            match_obj = intrinsic_re.match(line)
            if match_obj:
                assert current_namespace, "instruction missing namespace %s" % (line)
                # Get a D3D-style operand name for the instruction.
                # Unused for DXIL.
                opts = match_obj.group(1)
                attr = match_obj.group(2)
                name = match_obj.group(3)
                params = match_obj.group(4)
                op = match_obj.group(5)
                if op:
                    operand_match = operand_re.match(op)
                    if operand_match:
                        op = operand_match.group(1)
                if not op:
                    op = name
                (
                    readonly,
                    readnone,
                    argmemonly,
                    is_wave,
                    unsigned_op,
                    overload_param_index,
                    hidden,
                ) = process_attr(attr)
                # Add an entry for this intrinsic.
                if bracket_cleanup_re.search(opts):
                    opts = bracket_cleanup_re.sub(r"<\1@\2>", opts)
                if bracket_cleanup_re.search(params):
                    params = bracket_cleanup_re.sub(r"<\g<1>@\2>", params)
                ret_desc = "out " + opts + " " + name
                if len(params) > 0:
                    in_args = params_split_re.split(params)
                else:
                    in_args = []
                arg_idx = 1
                args = []
                for in_arg in in_args:
                    args.append(process_arg(in_arg, arg_idx, args, name))
                    arg_idx += 1
                # We have to process the return type description last
                # to match the compiler's handling of it and allow
                # the return type to match an input type.
                # It needs to be the first entry, so prepend it.
                args.insert(0, process_arg(ret_desc, 0, args, name))
                # TODO: verify a single level of indirection
                self.intrinsics.append(
                    db_hlsl_intrinsic(
                        name,
                        num_entries,
                        op,
                        args,
                        current_namespace,
                        ns_idx,
                        "pending doc for " + name,
                        readonly,
                        readnone,
                        argmemonly,
                        is_wave,
                        unsigned_op,
                        overload_param_index,
                        hidden,
                    )
                )
                num_entries += 1
                continue
            assert False, "cannot parse line %s" % (line)

    def populate_attributes(self):
        "Populate basic definitions for attributes."
        attributes = []

        def add_attr(title_name, scope, doc):
            attributes.append(db_hlsl_attribute(title_name, scope, [], doc))

        def add_attr_arg(title_name, scope, args, doc):
            attributes.append(db_hlsl_attribute(title_name, scope, args, doc))

        add_attr(
            "Allow_UAV_Condition",
            "l",
            "Allows a compute shader loop termination condition to be based off of a UAV read. The loop must not contain synchronization intrinsics",
        )
        add_attr(
            "Branch",
            "c",
            "Evaluate only one side of the if statement depending on the given condition",
        )
        add_attr(
            "Call",
            "s",
            "The bodies of the individual cases in the switch will be moved into hardware subroutines and the switch will be a series of subroutine calls",
        )
        add_attr(
            "EarlyDepthStencil",
            "f",
            "Forces depth-stencil testing before a shader executes",
        )
        add_attr(
            "FastOpt",
            "l",
            "Reduces the compile time but produces less aggressive optimizations",
        )
        add_attr(
            "Flatten",
            "c",
            "Evaluate both sides of the if statement and choose between the two resulting values",
        )
        add_attr("ForceCase", "s", "Force a switch statement in the hardware")
        add_attr(
            "Loop",
            "l",
            "Generate code that uses flow control to execute each iteration of the loop",
        )
        add_attr_arg(
            "ClipPlanes",
            "f",
            "Optional list of clip planes",
            [{"name": "ClipPlane", "type": "int", "count": 6}],
        )
        add_attr_arg(
            "Domain",
            "f",
            "Defines the patch type used in the HS",
            [{"name": "DomainType", type: "string"}],
        )
        add_attr_arg(
            "Instance",
            "f",
            "Use this attribute to instance a geometry shader",
            [{"name": "Count", "type": "int"}],
        )
        add_attr_arg(
            "MaxTessFactor",
            "f",
            "Indicates the maximum value that the hull shader would return for any tessellation factor.",
            [{"name": "Count", "type": "int"}],
        )
        add_attr_arg(
            "MaxVertexCount",
            "f",
            "maxvertexcount doc",
            [{"name": "Count", "type": "int"}],
        )
        add_attr_arg(
            "NumThreads",
            "f",
            "Defines the number of threads to be executed in a single thread group.",
            [
                {"name": "x", "type": "int"},
                {"name": "z", "type": "int"},
                {"name": "y", "type": "int"},
            ],
        )
        add_attr_arg(
            "OutputControlPoints",
            "f",
            "Defines the number of output control points per thread that will be created in the hull shader",
            [{"name": "Count", "type": "int"}],
        )
        add_attr_arg(
            "OutputTopology",
            "f",
            "Defines the output primitive type for the tessellator",
            [{"name": "Topology", "type": "string"}],
        )
        add_attr_arg(
            "Partitioning",
            "f",
            "Defines the tesselation scheme to be used in the hull shader",
            [{"name": "Scheme", "type": "scheme"}],
        )
        add_attr_arg(
            "PatchConstantFunc",
            "f",
            "Defines the function for computing patch constant data",
            [{"name": "FunctionName", "type": "string"}],
        )
        add_attr_arg(
            "RootSignature",
            "f",
            "RootSignature doc",
            [{"name": "SignatureName", "type": "string"}],
        )
        add_attr_arg(
            "Unroll",
            "l",
            "Unroll the loop until it stops executing or a max count",
            [{"name": "Count", "type": "int"}],
        )
        self.attributes = attributes


if __name__ == "__main__":
    db = db_dxil()
    print(db)
    db.print_stats()
