# Copyright (C) Microsoft Corporation. All rights reserved.
# This file is distributed under the University of Illinois Open Source License. See LICENSE.TXT for details.
import argparse
import functools
import collections
from hctdb import *
import json
import os

# get db singletons
g_db_dxil = None


def get_db_dxil():
    global g_db_dxil
    if g_db_dxil is None:
        g_db_dxil = db_dxil()
    return g_db_dxil


# opcode data contains fixed opcode assignments for HLSL intrinsics.
g_hlsl_opcode_data = None


def get_hlsl_opcode_data():
    global g_hlsl_opcode_data
    if g_hlsl_opcode_data is None:
        # Load the intrinsic opcodes from the JSON file.
        json_filepath = os.path.join(
            os.path.dirname(__file__), "hlsl_intrinsic_opcodes.json"
        )
        try:
            with open(json_filepath, "r") as file:
                g_hlsl_opcode_data = json.load(file)
        except FileNotFoundError:
            print(f"File not found: {json_filepath}")
        except json.JSONDecodeError as e:
            print(f"Error decoding JSON from {json_filepath}: {e}")
        if not g_hlsl_opcode_data:
            g_hlsl_opcode_data = {}
    return g_hlsl_opcode_data


g_db_hlsl = None


def get_db_hlsl():
    global g_db_hlsl
    if g_db_hlsl is None:
        thisdir = os.path.dirname(os.path.realpath(__file__))
        with open(os.path.join(thisdir, "gen_intrin_main.txt"), "r") as f:
            g_db_hlsl = db_hlsl(f, get_hlsl_opcode_data())
    return g_db_hlsl


def get_max_oload_dims():
    return f"const unsigned kDxilMaxOloadDims = {dxil_max_overload_dims};"


def format_comment(prefix, val):
    "Formats a value with a line-comment prefix."
    result = ""
    line_width = 80
    content_width = line_width - len(prefix)
    l = len(val)
    while l:
        if l < content_width:
            result += prefix + val.strip()
            result += "\n"
            l = 0
        else:
            split_idx = val.rfind(" ", 0, content_width)
            result += prefix + val[:split_idx].strip()
            result += "\n"
            val = val[split_idx + 1 :]
            l = len(val)
    return result


def format_rst_table(list_of_tuples):
    "Produces a reStructuredText simple table from the specified list of tuples."
    # Calculate widths.
    widths = None
    for t in list_of_tuples:
        if widths is None:
            widths = [0] * len(t)
        for i, v in enumerate(t):
            widths[i] = max(widths[i], len(str(v)))
    # Build banner line.
    banner = ""
    for i, w in enumerate(widths):
        if i > 0:
            banner += " "
        banner += "=" * w
    banner += "\n"
    # Build the result.
    result = banner
    for i, t in enumerate(list_of_tuples):
        for j, v in enumerate(t):
            if j > 0:
                result += " "
            result += str(v)
            result += " " * (widths[j] - len(str(v)))
        result = result.rstrip()
        result += "\n"
        if i == 0:
            result += banner
    result += banner
    return result


def build_range_tuples(i):
    "Produces a list of tuples with contiguous ranges in the input list."
    i = sorted(i)
    low_bound = None
    high_bound = None
    for val in i:
        if low_bound is None:
            low_bound = val
            high_bound = val
        else:
            assert not high_bound is None
            if val == high_bound + 1:
                high_bound = val
            else:
                yield (low_bound, high_bound)
                low_bound = val
                high_bound = val
    if not low_bound is None:
        yield (low_bound, high_bound)


def build_range_code(var, i):
    "Produces a fragment of code that tests whether the variable name matches values in the given range."
    ranges = build_range_tuples(i)
    result = ""
    for r in ranges:
        if r[0] == r[1]:
            cond = var + " == " + str(r[0])
        else:
            cond = "(%d <= %s && %s <= %d)" % (r[0], var, var, r[1])
        if result == "":
            result = cond
        else:
            result = result + " || " + cond
    return result


class db_docsref_gen:
    "A generator of reference documentation."

    def __init__(self, db):
        self.db = db
        instrs = [i for i in self.db.instr if i.is_dxil_op]
        instrs = sorted(
            instrs,
            key=lambda v: ("" if v.category == None else v.category) + "." + v.name,
        )
        self.instrs = instrs
        val_rules = sorted(
            db.val_rules,
            key=lambda v: ("" if v.category == None else v.category) + "." + v.name,
        )
        self.val_rules = val_rules

    def print_content(self):
        self.print_header()
        self.print_body()
        self.print_footer()

    def print_header(self):
        print("<!DOCTYPE html>")
        print("<html><head><title>DXIL Reference</title>")
        print("<style>body { font-family: Verdana; font-size: small; }</style>")
        print("</head><body><h1>DXIL Reference</h1>")
        self.print_toc("Instructions", "i", self.instrs)
        self.print_toc("Rules", "r", self.val_rules)

    def print_body(self):
        self.print_instruction_details()
        self.print_valrule_details()

    def print_instruction_details(self):
        print("<h2>Instruction Details</h2>")
        for i in self.instrs:
            print("<h3><a name='i%s'>%s</a></h3>" % (i.name, i.name))
            print("<div>Opcode: %d. This instruction %s.</div>" % (i.dxil_opid, i.doc))
            if i.remarks:
                # This is likely a .rst fragment, but this will do for now.
                print("<div> " + i.remarks + "</div>")
            print("<div>Operands:</div>")
            print("<ul>")
            for o in i.ops:
                if o.pos == 0:
                    print("<li>result: %s - %s</li>" % (o.llvm_type, o.doc))
                else:
                    enum_desc = (
                        ""
                        if o.enum_name == ""
                        else " one of %s: %s"
                        % (
                            o.enum_name,
                            ",".join(db.enum_idx[o.enum_name].value_names()),
                        )
                    )
                    print(
                        "<li>%d - %s: %s%s%s</li>"
                        % (
                            o.pos - 1,
                            o.name,
                            o.llvm_type,
                            "" if o.doc == "" else " - " + o.doc,
                            enum_desc,
                        )
                    )
            print("</ul>")
            print("<div><a href='#Instructions'>(top)</a></div>")

    def print_valrule_details(self):
        print("<h2>Rule Details</h2>")
        for i in self.val_rules:
            print("<h3><a name='r%s'>%s</a></h3>" % (i.name, i.name))
            print("<div>" + i.doc + "</div>")
            print("<div><a href='#Rules'>(top)</a></div>")

    def print_toc(self, name, aprefix, values):
        print("<h2><a name='" + name + "'>" + name + "</a></h2>")
        last_category = ""
        for i in values:
            if i.category != last_category:
                if last_category != None:
                    print("</ul>")
                print("<div><b>%s</b></div><ul>" % i.category)
                last_category = i.category
            print("<li><a href='#" + aprefix + "%s'>%s</a></li>" % (i.name, i.name))
        print("</ul>")

    def print_footer(self):
        print("</body></html>")


class db_instrhelp_gen:
    "A generator of instruction helper classes."

    def __init__(self, db):
        self.db = db
        TypeInfo = collections.namedtuple("TypeInfo", "name bits")
        self.llvm_type_map = {
            "i1": TypeInfo("bool", 1),
            "i8": TypeInfo("int8_t", 8),
            "u8": TypeInfo("uint8_t", 8),
            "i32": TypeInfo("int32_t", 32),
            "u32": TypeInfo("uint32_t", 32),
        }
        self.IsDxilOpFuncCallInst = "hlsl::OP::IsDxilOpFuncCallInst"

    def print_content(self):
        self.print_header()
        self.print_body()
        self.print_footer()

    def print_header(self):
        print(
            "///////////////////////////////////////////////////////////////////////////////"
        )
        print(
            "//                                                                           //"
        )
        print(
            "// Copyright (C) Microsoft Corporation. All rights reserved.                 //"
        )
        print(
            "// DxilInstructions.h                                                        //"
        )
        print(
            "//                                                                           //"
        )
        print(
            "// This file provides a library of instruction helper classes.               //"
        )
        print(
            "//                                                                           //"
        )
        print(
            "// MUCH WORK YET TO BE DONE - EXPECT THIS WILL CHANGE - GENERATED FILE       //"
        )
        print(
            "//                                                                           //"
        )
        print(
            "///////////////////////////////////////////////////////////////////////////////"
        )
        print("")
        print("// TODO: add correct include directives")
        print("// TODO: add accessors with values")
        print("// TODO: add validation support code, including calling into right fn")
        print("// TODO: add type hierarchy")
        print("namespace hlsl {")

    def bool_lit(self, val):
        return "true" if val else "false"

    def op_type(self, o):
        if o.llvm_type in self.llvm_type_map:
            return self.llvm_type_map[o.llvm_type].name
        raise ValueError(
            "Don't know how to describe type %s for operand %s." % (o.llvm_type, o.name)
        )

    def op_size(self, o):
        if o.llvm_type in self.llvm_type_map:
            return self.llvm_type_map[o.llvm_type].bits
        raise ValueError(
            "Don't know how to describe type %s for operand %s." % (o.llvm_type, o.name)
        )

    def op_const_expr(self, o):
        return (
            "(%s)(llvm::dyn_cast<llvm::ConstantInt>(Instr->getOperand(%d))->getZExtValue())"
            % (self.op_type(o), o.pos - 1)
        )

    def op_set_const_expr(self, o):
        type_size = self.op_size(o)
        return (
            "llvm::Constant::getIntegerValue(llvm::IntegerType::get(Instr->getContext(), %d), llvm::APInt(%d, (uint64_t)val))"
            % (type_size, type_size)
        )

    def print_body(self):
        for i in self.db.instr:
            if i.is_reserved:
                continue
            if i.inst_helper_prefix:
                struct_name = "%s_%s" % (i.inst_helper_prefix, i.name)
            elif i.is_dxil_op:
                struct_name = "DxilInst_%s" % i.name
            else:
                struct_name = "LlvmInst_%s" % i.name
            if i.doc:
                print("/// This instruction %s" % i.doc)
            print("struct %s {" % struct_name)
            print("  llvm::Instruction *Instr;")
            print("  // Construction and identification")
            print("  %s(llvm::Instruction *pInstr) : Instr(pInstr) {}" % struct_name)
            print("  operator bool() const {")
            if i.is_dxil_op:
                op_name = i.fully_qualified_name()
                print(
                    "    return %s(Instr, %s);" % (self.IsDxilOpFuncCallInst, op_name)
                )
            else:
                print(
                    "    return Instr->getOpcode() == llvm::Instruction::%s;" % i.name
                )
            print("  }")
            print("  // Validation support")
            print(
                "  bool isAllowed() const { return %s; }" % self.bool_lit(i.is_allowed)
            )
            if i.is_dxil_op:
                print("  bool isArgumentListValid() const {")
                print(
                    "    if (%d != llvm::dyn_cast<llvm::CallInst>(Instr)->getNumArgOperands()) return false;"
                    % (len(i.ops) - 1)
                )
                print("    return true;")
                # TODO - check operand types
                print("  }")
                print("  // Metadata")
                print(
                    "  bool requiresUniformInputs() const { return %s; }"
                    % self.bool_lit(i.requires_uniform_inputs)
                )
                EnumWritten = False
                for o in i.ops:
                    if o.pos > 1:  # 0 is return type, 1 is DXIL OP id
                        if not EnumWritten:
                            print("  // Operand indexes")
                            print("  enum OperandIdx {")
                            EnumWritten = True
                        print("    arg_%s = %d," % (o.name, o.pos - 1))
                if EnumWritten:
                    print("  };")
                AccessorsWritten = False
                for o in i.ops:
                    if o.pos > 1:  # 0 is return type, 1 is DXIL OP id
                        if not AccessorsWritten:
                            print("  // Accessors")
                            AccessorsWritten = True
                        print(
                            "  llvm::Value *get_%s() const { return Instr->getOperand(%d); }"
                            % (o.name, o.pos - 1)
                        )
                        print(
                            "  void set_%s(llvm::Value *val) { Instr->setOperand(%d, val); }"
                            % (o.name, o.pos - 1)
                        )
                        if o.is_const:
                            if o.llvm_type in self.llvm_type_map:
                                print(
                                    "  %s get_%s_val() const { return %s; }"
                                    % (self.op_type(o), o.name, self.op_const_expr(o))
                                )
                                print(
                                    "  void set_%s_val(%s val) { Instr->setOperand(%d, %s); }"
                                    % (
                                        o.name,
                                        self.op_type(o),
                                        o.pos - 1,
                                        self.op_set_const_expr(o),
                                    )
                                )
            print("};")
            print("")

    def print_footer(self):
        print("} // namespace hlsl")


class db_enumhelp_gen:
    "A generator of enumeration declarations."

    def __init__(self, db):
        self.db = db
        # Some enums should get a last enum marker.
        self.lastEnumNames = {"OpCode": "NumOpCodes", "OpCodeClass": "NumOpClasses"}

    def print_enum(self, e, **kwargs):
        print("// %s" % e.doc)
        print("enum class %s : unsigned {" % e.name)
        hide_val = kwargs.get("hide_val", False)
        sorted_values = e.values
        if kwargs.get("sort_val", True):
            sorted_values = sorted(
                e.values,
                key=lambda v: ("" if v.category == None else v.category) + "." + v.name,
            )
        last_category = None
        for v in sorted_values:
            if v.category != last_category:
                if last_category != None:
                    print("")
                print("  // %s" % v.category)
                last_category = v.category

            line_format = "  {name}"
            if not e.is_internal and not hide_val:
                line_format += " = {value}"
            line_format += ","
            if v.doc:
                line_format += " // {doc}"
            print(line_format.format(name=v.name, value=v.value, doc=v.doc))
        if e.name in self.lastEnumNames:
            lastName = self.lastEnumNames[e.name]
            versioned = [
                "%s_Dxil_%d_%d = %d," % (lastName, major, minor, info[lastName])
                for (major, minor), info in sorted(self.db.dxil_version_info.items())
                if lastName in info
            ]
            if versioned:
                print("")
                for val in versioned:
                    print("  " + val)
            print("")
            print(
                "  "
                + lastName
                + " = "
                + str(len(sorted_values))
                + " // exclusive last value of enumeration"
            )
        print("};")

    def print_rdat_enum(self, e, **kwargs):
        nodef = kwargs.get("nodef", False)
        for v in e.values:
            line_format = (
                "RDAT_ENUM_VALUE_NODEF({name})"
                if nodef
                else "RDAT_ENUM_VALUE({value}, {name})"
            )
            if v.doc:
                line_format += " // {doc}"
            print(line_format.format(name=v.name, value=v.value, doc=v.doc))

    def print_content(self):
        for e in sorted(self.db.enums, key=lambda e: e.name):
            self.print_enum(e)


class db_oload_gen:
    "A generator of overload tables."

    def __init__(self, db):
        self.db = db
        instrs = [i for i in self.db.instr if i.is_dxil_op]
        self.instrs = sorted(instrs, key=lambda i: i.dxil_opid)

        # Allow these to be overridden by external scripts.
        self.OP = "OP"
        self.OC = "OC"
        self.OCC = "OCC"

    def print_content(self):
        self.print_opfunc_props()
        print("...")
        self.print_opfunc_table()

    def print_opfunc_props(self):
        print(
            "const {OP}::OpCodeProperty {OP}::m_OpCodeProps[(unsigned){OP}::OpCode::NumOpCodes] = {{".format(
                OP=self.OP
            )
        )

        last_category = None
        lower_exceptions = {
            "CBufferLoad": "cbufferLoad",
            "CBufferLoadLegacy": "cbufferLoadLegacy",
            "GSInstanceID": "gsInstanceID",
        }
        lower_fn = lambda t: (
            lower_exceptions[t] if t in lower_exceptions else t[:1].lower() + t[1:]
        )
        attr_dict = {
            "": "None",
            "ro": "ReadOnly",
            "rn": "ReadNone",
            "amo": "ArgMemOnly",
            "nd": "NoDuplicate",
            "nr": "NoReturn",
            "wv": "None",
        }
        attr_fn = lambda i: "Attribute::" + attr_dict[i.fn_attr]
        oload_to_mask = lambda oload: sum(
            [1 << dxil_all_user_oload_chars.find(c) for c in oload]
        )
        oloads_fn = lambda oloads: (
            "{" + ",".join(["{0x%x}" % m for m in oloads]) + "}"
        )
        for i in self.instrs:
            if last_category != i.category:
                if last_category != None:
                    print("")
                if not i.is_reserved:
                    print(f"  // {i.category}")
                last_category = i.category
            scalar_masks = []
            vector_masks = []
            if i.num_oloads > 0:
                for n, o in enumerate(i.oload_types.split(",")):
                    if "<" in o:
                        v = o.split("<")
                        scalar_masks.append(oload_to_mask(v[0] + "<"))
                        vector_masks.append(oload_to_mask(v[1]))
                    else:
                        scalar_masks.append(oload_to_mask(o))
                        vector_masks.append(0)
            print(
                (
                    "  {{  {OC}::{name:24} {quotName:27} {OCC}::{className:25} "
                    + "{classNameQuot:28} {attr:20}, {num_oloads}, "
                    + "{scalar_masks:16}, {vector_masks:16} }}, "
                    + "// Overloads: {oloads}"
                ).format(
                    name=i.name + ",",
                    quotName='"' + i.name + '",',
                    className=i.dxil_class + ",",
                    classNameQuot='"' + lower_fn(i.dxil_class) + '",',
                    attr=attr_fn(i),
                    num_oloads=i.num_oloads,
                    scalar_masks=oloads_fn(scalar_masks),
                    vector_masks=oloads_fn(vector_masks),
                    oloads=i.oload_types,
                    OC=self.OC,
                    OCC=self.OCC,
                )
            )
        print("};")

    def print_opfunc_table(self):
        # Print the table for OP::GetOpFunc
        op_type_texts = {
            "$cb": "CBRT(pETy);",
            "$o": "A(pETy);",
            "$o_i1": "A(pOlTplI1);",
            "$o_i32": "A(pOlTplI32);",
            "$r": "RRT(pETy);",
            "d": "A(pF64);",
            "dims": "A(pDim);",
            "f": "A(pF32);",
            "h": "A(pF16);",
            "i1": "A(pI1);",
            "i16": "A(pI16);",
            "i32": "A(pI32);",
            "i32c": "A(pI32C);",
            "i64": "A(pI64);",
            "i8": "A(pI8);",
            "$u4": "A(pI4S);",
            "pf32": "A(pPF32);",
            "res": "A(pRes);",
            "splitdouble": "A(pSDT);",
            "twoi32": "A(p2I32);",
            "twof32": "A(p2F32);",
            "twof16": "A(p2F16);",
            "twoi16": "A(p2I16);",
            "threei32": "A(p3I32);",
            "threef32": "A(p3F32);",
            "fouri32": "A(p4I32);",
            "fourf32": "A(p4F32);",
            "fouri16": "A(p4I16);",
            "fourf16": "A(p4F16);",
            "u32": "A(pI32);",
            "u64": "A(pI64);",
            "u8": "A(pI8);",
            "v": "A(pV);",
            "$vec4": "VEC4(pETy);",
            "w": "A(pWav);",
            "SamplePos": "A(pPos);",
            "udt": "A(udt);",
            "obj": "A(obj);",
            "resproperty": "A(resProperty);",
            "resbind": "A(resBind);",
            "waveMat": "A(pWaveMatPtr);",
            "waveMatProps": "A(pWaveMatProps);",
            "$gsptr": "A(pGSEltPtrTy);",
            "nodehandle": "A(pNodeHandle);",
            "noderecordhandle": "A(pNodeRecordHandle);",
            "nodeproperty": "A(nodeProperty);",
            "noderecordproperty": "A(nodeRecordProperty);",
            "hit_object": "A(pHit);",
            # Extended overload slots, extend as needed:
            "$x0": "EXT(0);",
            "$x1": "EXT(1);",
        }
        last_category = None
        for i in self.instrs:
            if last_category != i.category:
                if last_category != None:
                    print("")
                print("    // %s" % i.category)
                last_category = i.category
            line = "  case OpCode::{name:24}".format(name=i.name + ":")
            for index, o in enumerate(i.ops):
                assert (
                    o.llvm_type in op_type_texts
                ), "llvm type %s in instruction %s is unknown" % (o.llvm_type, i.name)
                op_type_text = op_type_texts[o.llvm_type]
                if index == 0:
                    line = line + "{val:13}".format(val=op_type_text)
                else:
                    line = line + "{val:9}".format(val=op_type_text)
            line = line + "break;"
            print(line)

    def print_opfunc_oload_type(self):
        # Print the function for OP::GetOverloadType
        elt_ty = "$o"
        res_ret_ty = "$r"
        cb_ret_ty = "$cb"
        udt_ty = "udt"
        obj_ty = "obj"
        vec_ty = "$vec"
        gsptr_ty = "$gsptr"
        extended_ty = "$x"
        last_category = None

        index_dict = collections.OrderedDict()
        ptr_index_dict = collections.OrderedDict()
        single_dict = collections.OrderedDict()
        # extended_dict collects overloads with multiple overload types
        # grouped by the set of overload parameter indices.
        extended_dict = collections.OrderedDict()
        struct_list = []
        extended_list = []

        for instr in self.instrs:
            if instr.num_oloads > 1:
                # Process extended overloads separately.
                extended_list.append(instr)
                continue

            ret_ty = instr.ops[0].llvm_type
            # Skip case return type is overload type
            if ret_ty == elt_ty:
                continue

            if ret_ty == res_ret_ty:
                struct_list.append(instr.name)
                continue

            if ret_ty == cb_ret_ty:
                struct_list.append(instr.name)
                continue

            if ret_ty.startswith(vec_ty):
                struct_list.append(instr.name)
                continue

            in_param_ty = False
            # Try to find elt_ty in parameter types.
            for index, op in enumerate(instr.ops):
                # Skip return type.
                if op.pos == 0:
                    continue
                # Skip dxil opcode.
                if op.pos == 1:
                    continue

                op_type = op.llvm_type
                if op_type == elt_ty:
                    # Skip return op
                    index = index - 1
                    if index not in index_dict:
                        index_dict[index] = [instr.name]
                    else:
                        index_dict[index].append(instr.name)
                    in_param_ty = True
                    break
                if op_type == gsptr_ty:
                    # Skip return op
                    index = index - 1
                    if index not in ptr_index_dict:
                        ptr_index_dict[index] = [instr.name]
                    else:
                        ptr_index_dict[index].append(instr.name)
                    in_param_ty = True
                    break
                if op_type == udt_ty or op_type == obj_ty:
                    # Skip return op
                    index = index - 1
                    if index not in index_dict:
                        index_dict[index] = [instr.name]
                    else:
                        index_dict[index].append(instr.name)
                    in_param_ty = True

            if in_param_ty:
                continue

            # No overload, just return the single oload_type.
            assert len(instr.oload_types) == 1, "overload no elt_ty %s" % (instr.name)
            ty = instr.oload_types[0]
            type_code_texts = {
                "d": "Type::getDoubleTy(Ctx)",
                "f": "Type::getFloatTy(Ctx)",
                "h": "Type::getHalfTy",
                "1": "IntegerType::get(Ctx, 1)",
                "8": "IntegerType::get(Ctx, 8)",
                "w": "IntegerType::get(Ctx, 16)",
                "i": "IntegerType::get(Ctx, 32)",
                "l": "IntegerType::get(Ctx, 64)",
                "v": "Type::getVoidTy(Ctx)",
                # No other types should be referenced here.
            }
            assert ty in type_code_texts, "llvm type %s is unknown" % (ty)
            ty_code = type_code_texts[ty]

            if ty_code not in single_dict:
                single_dict[ty_code] = [instr.name]
            else:
                single_dict[ty_code].append(instr.name)

        for index, opcodes in index_dict.items():
            line = ""
            for opcode in opcodes:
                line = line + "case OpCode::{name}".format(name=opcode + ":\n")

            line = (
                line
                + "  if (FT->getNumParams() <= "
                + str(index)
                + ") return nullptr;\n"
            )
            line = line + "  return FT->getParamType(" + str(index) + ");"
            print(line)

        # ptr_index_dict for overload based on pointer element type
        for index, opcodes in ptr_index_dict.items():
            line = ""
            for opcode in opcodes:
                line = line + "case OpCode::{name}".format(name=opcode + ":\n")

            line = (
                line
                + "  if (FT->getNumParams() <= "
                + str(index)
                + ") return nullptr;\n"
            )
            line = (
                line
                + "  return FT->getParamType("
                + str(index)
                + ")->getPointerElementType();"
            )
            print(line)

        for code, opcodes in single_dict.items():
            line = ""
            for opcode in opcodes:
                line = line + "case OpCode::{name}".format(name=opcode + ":\n")
            line = line + "  return " + code + ";"
            print(line)

        line = ""
        for opcode in struct_list:
            line = line + "case OpCode::{name}".format(name=opcode + ":\n")
        line = line + "{\n"
        line = line + "  StructType *ST = cast<StructType>(Ty);\n"
        line = line + "  return ST->getElementType(0);\n"
        line = line + "}"
        print(line)

        for instr in extended_list:
            # Collect indices for overloaded return and types, make a tuple of
            # indices the key, and add the opcode to a list of opcodes for that
            # key.  Indices start with 0 for return type, and 1 for the first
            # function parameter, which is the DXIL OpCode.
            indices = []
            for index, op in enumerate(instr.ops):
                # Skip dxil opcode.
                if op.pos == 1:
                    continue

                op_type = op.llvm_type
                if op_type.startswith(extended_ty):
                    try:
                        extended_index = int(op_type[2:])
                    except:
                        raise ValueError(
                            "Error parsing extended operand type "
                            + f"'{op_type}' for DXIL op '{instr.name}'"
                        )
                    if extended_index != len(indices):
                        raise ValueError(
                            f"'$x{extended_index}' is not in sequential "
                            + f"order for DXIL op '{instr.name}'"
                        )
                    indices.append(op.pos)

            if len(indices) != instr.num_oloads:
                raise ValueError(
                    f"DXIL op {instr.name}: extended overload count "
                    + "mismatches the number of overload types"
                )
            extended_dict.setdefault(tuple(indices), []).append(instr.name)

        def get_type_at_index(index):
            if index == 0:
                return "FT->getReturnType()"
            return f"FT->getParamType({index - 1})"

        for index_tuple, opcodes in extended_dict.items():
            line = ""
            for opcode in opcodes:
                line = line + f"case OpCode::{opcode}:\n"
            if index_tuple[-1] > 0:
                line += (
                    f"  if (FT->getNumParams() < {index_tuple[-1]})\n"
                    + "    return nullptr;\n"
                )
            line += (
                "  return llvm::StructType::get(Ctx, {"
                + ", ".join([get_type_at_index(index) for index in index_tuple])
                + "});\n"
            )
            print(line)


class db_valfns_gen:
    "A generator of validation functions."

    def __init__(self, db):
        self.db = db

    def print_content(self):
        self.print_header()
        self.print_body()

    def print_header(self):
        print(
            "///////////////////////////////////////////////////////////////////////////////"
        )
        print(
            "// Instruction validation functions.                                         //"
        )

    def bool_lit(self, val):
        return "true" if val else "false"

    def op_type(self, o):
        if o.llvm_type == "i8":
            return "int8_t"
        if o.llvm_type == "u8":
            return "uint8_t"
        raise ValueError(
            "Don't know how to describe type %s for operand %s." % (o.llvm_type, o.name)
        )

    def op_const_expr(self, o):
        if o.llvm_type == "i8" or o.llvm_type == "u8":
            return (
                "(%s)(llvm::dyn_cast<llvm::ConstantInt>(Instr->getOperand(%d))->getZExtValue())"
                % (self.op_type(o), o.pos - 1)
            )
        raise ValueError(
            "Don't know how to describe type %s for operand %s." % (o.llvm_type, o.name)
        )

    def print_body(self):
        llvm_instrs = [i for i in self.db.instr if i.is_allowed and not i.is_dxil_op]
        print("static bool IsLLVMInstructionAllowed(llvm::Instruction &I) {")
        self.print_comment(
            "  // ",
            "Allow: %s"
            % ", ".join([i.name + "=" + str(i.llvm_id) for i in llvm_instrs]),
        )
        print("  unsigned op = I.getOpcode();")
        print("  return %s;" % build_range_code("op", [i.llvm_id for i in llvm_instrs]))
        print("}")
        print("")

    def print_comment(self, prefix, val):
        print(format_comment(prefix, val))


class macro_table_gen:
    "A generator for macro tables."

    def format_row(self, row, widths, sep=", "):
        frow = [
            str(item) + sep + (" " * (width - len(item)))
            for item, width in list(zip(row, widths))[:-1]
        ] + [str(row[-1])]
        return "".join(frow)

    def format_table(self, table, *args, **kwargs):
        widths = [
            functools.reduce(max, [len(row[i]) for row in table], 1)
            for i in range(len(table[0]))
        ]
        formatted = []
        for row in table:
            formatted.append(self.format_row(row, widths, *args, **kwargs))
        return formatted

    def print_table(self, table, macro_name):
        formatted = self.format_table(table)
        print(
            "//   %s\n" % formatted[0]
            + "#define %s(ROW) \\\n" % macro_name
            + " \\\n".join(["  ROW(%s)" % frow for frow in formatted[1:]])
        )


class db_sigpoint_gen(macro_table_gen):
    "A generator for SigPoint tables."

    def __init__(self, db):
        self.db = db

    def print_sigpoint_table(self):
        self.print_table(self.db.sigpoint_table, "DO_SIGPOINTS")

    def print_interpretation_table(self):
        self.print_table(self.db.interpretation_table, "DO_INTERPRETATION_TABLE")

    def print_content(self):
        self.print_sigpoint_table()
        self.print_interpretation_table()


class string_output:
    def __init__(self):
        self.val = ""

    def write(self, text):
        self.val = self.val + str(text)

    def __str__(self):
        return self.val


def run_with_stdout(fn):
    import sys

    _stdout_saved = sys.stdout
    so = string_output()
    try:
        sys.stdout = so
        fn()
    finally:
        sys.stdout = _stdout_saved
    return str(so)


def get_hlsl_intrinsic_stats():
    db = get_db_hlsl()
    longest_fn = db.intrinsics[0]
    longest_param = None
    longest_arglist_fn = db.intrinsics[0]
    for i in sorted(db.intrinsics, key=lambda x: x.key):
        # Get some values for maximum lengths.
        if len(i.name) > len(longest_fn.name):
            longest_fn = i
        for p_idx, p in enumerate(i.params):
            if p_idx > 0 and (
                longest_param is None or len(p.name) > len(longest_param.name)
            ):
                longest_param = p
        if len(i.params) > len(longest_arglist_fn.params):
            longest_arglist_fn = i
    result = ""
    for k in sorted(db.namespaces.keys()):
        v = db.namespaces[k]
        result += "static const UINT g_u%sCount = %d;\n" % (k, len(v.intrinsics))
    result += "\n"
    # NOTE:The min limits are needed to support allowing intrinsics in the extension mechanism that use longer values than the builtin hlsl intrisics.
    # TODO: remove code which dependent on g_MaxIntrinsic*.
    MIN_FUNCTION_NAME_LENTH = 44
    MIN_PARAM_NAME_LENTH = 48
    MIN_PARAM_COUNT = 29

    max_fn_name = longest_fn.name
    max_fn_name_len = len(longest_fn.name)
    max_param_name = longest_param.name
    max_param_name_len = len(longest_param.name)
    max_param_count_name = longest_arglist_fn.name
    max_param_count = len(longest_arglist_fn.params) - 1

    if max_fn_name_len < MIN_FUNCTION_NAME_LENTH:
        max_fn_name_len = MIN_FUNCTION_NAME_LENTH
        max_fn_name = "MIN_FUNCTION_NAME_LENTH"

    if max_param_name_len < MIN_PARAM_NAME_LENTH:
        max_param_name_len = MIN_PARAM_NAME_LENTH
        max_param_name = "MIN_PARAM_NAME_LENTH"

    if max_param_count < MIN_PARAM_COUNT:
        max_param_count = MIN_PARAM_COUNT
        max_param_count_name = "MIN_PARAM_COUNT"

    result += (
        "static const int g_MaxIntrinsicName = %d; // Count of characters for longest intrinsic name - '%s'\n"
        % (max_fn_name_len, max_fn_name)
    )
    result += (
        "static const int g_MaxIntrinsicParamName = %d; // Count of characters for longest intrinsic parameter name - '%s'\n"
        % (max_param_name_len, max_param_name)
    )
    result += (
        "static const int g_MaxIntrinsicParamCount = %d; // Count of parameters (without return) for longest intrinsic argument list - '%s'\n"
        % (max_param_count, max_param_count_name)
    )
    return result


def get_hlsl_intrinsics():
    db = get_db_hlsl()
    result = ""
    last_ns = ""
    ns_table = ""
    is_vk_table = False  # SPIRV Change
    arg_idx = 0
    opcode_namespace = db.opcode_namespace
    for i in sorted(db.intrinsics, key=lambda x: x.key):
        if last_ns != i.ns:
            last_ns = i.ns
            if len(ns_table):
                result += ns_table + "};\n"
                # SPIRV Change Starts
                if is_vk_table:
                    result += "\n#endif // ENABLE_SPIRV_CODEGEN\n"
                    is_vk_table = False
                # SPIRV Change Ends
            result += "\n//\n// Start of %s\n//\n\n" % (last_ns)
            # This used to be qualified as __declspec(selectany), but that's no longer necessary.
            ns_table = "static const HLSL_INTRINSIC g_%s[] =\n{\n" % (last_ns)
            # SPIRV Change Starts
            if i.vulkanSpecific:
                is_vk_table = True
                result += "#ifdef ENABLE_SPIRV_CODEGEN\n\n"
            # SPIRV Change Ends
            arg_idx = 0
        flags = []
        if i.readonly:
            flags.append("INTRIN_FLAG_READ_ONLY")
        if i.readnone:
            flags.append("INTRIN_FLAG_READ_NONE")
        if i.wave:
            flags.append("INTRIN_FLAG_IS_WAVE")
        if i.static_member:
            flags.append("INTRIN_FLAG_STATIC_MEMBER")
        if flags:
            flags = " | ".join(flags)
        else:
            flags = "0"
        ns_table += "    {(UINT)%s::%s, %s, 0x%x, %d, %d, g_%s_Args%s},\n" % (
            opcode_namespace,
            i.enum_name,
            flags,
            i.min_shader_model,
            i.overload_param_index,
            len(i.params),
            last_ns,
            arg_idx,
        )
        result += "static const HLSL_INTRINSIC_ARGUMENT g_%s_Args%s[] =\n{\n" % (
            last_ns,
            arg_idx,
        )
        for p in i.params:
            name = p.name
            if name == i.name and i.hidden:
                # First parameter defines intrinsic name for parsing in HLSL.
                # Prepend '$hidden$' for hidden intrinsic so it can't be used in HLSL.
                name = "$hidden$" + name
            result += '    {"%s", %s, %s, %s, %s, %s, %s, %s},\n' % (
                name,
                p.param_qual,
                p.template_id,
                p.template_list,
                p.component_id,
                p.component_list,
                p.rows,
                p.cols,
            )
        result += "};\n\n"
        arg_idx += 1
    result += ns_table + "};\n"
    result += (
        "\n#endif // ENABLE_SPIRV_CODEGEN\n" if is_vk_table else ""
    )  # SPIRV Change
    return result


# SPIRV Change Starts
def wrap_with_ifdef_if_vulkan_specific(intrinsic, text):
    if intrinsic.vulkanSpecific:
        return (
            "#ifdef ENABLE_SPIRV_CODEGEN\n" + text + "#endif // ENABLE_SPIRV_CODEGEN\n"
        )
    return text


# SPIRV Change Ends


def enum_hlsl_intrinsics():
    db = get_db_hlsl()
    result = ""
    enumed = set()
    for i in sorted(db.intrinsics, key=lambda x: x.key):
        if i.enum_name not in enumed:
            result += "  %s = %d,\n" % (i.enum_name, i.opcode)
            enumed.add(i.enum_name)
    # unsigned
    result += "  // unsigned\n"

    for i in sorted(db.intrinsics, key=lambda x: x.key):
        if i.unsigned_op != "":
            if i.unsigned_op not in enumed:
                result += "  %s = %d,\n" % (i.unsigned_op, i.unsigned_opcode)
                enumed.add(i.unsigned_op)

    Num_Intrinsics = get_hlsl_opcode_data()["IntrinsicOpCodes"]["Num_Intrinsics"]
    result += "  Num_Intrinsics = %d,\n" % (Num_Intrinsics)
    return result


def has_unsigned_hlsl_intrinsics():
    db = get_db_hlsl()
    result = ""
    enumed = []
    # unsigned
    for i in sorted(db.intrinsics, key=lambda x: x.key):
        if i.unsigned_op != "":
            if i.enum_name not in enumed:
                result += "  case IntrinsicOp::%s:\n" % (i.enum_name)
                enumed.append(i.enum_name)
    return result


def get_unsigned_hlsl_intrinsics():
    db = get_db_hlsl()
    result = ""
    enumed = []
    # unsigned
    for i in sorted(db.intrinsics, key=lambda x: x.key):
        if i.unsigned_op != "":
            if i.enum_name not in enumed:
                enumed.append(i.enum_name)
                result += "  case IntrinsicOp::%s:\n" % (i.enum_name)
                result += "    return static_cast<unsigned>(IntrinsicOp::%s);\n" % (
                    i.unsigned_op
                )
    return result


def get_oloads_props():
    db = get_db_dxil()
    gen = db_oload_gen(db)
    return run_with_stdout(lambda: gen.print_opfunc_props())


def get_oloads_funcs():
    db = get_db_dxil()
    gen = db_oload_gen(db)
    return run_with_stdout(lambda: gen.print_opfunc_table())


def get_funcs_oload_type():
    db = get_db_dxil()
    gen = db_oload_gen(db)
    return run_with_stdout(lambda: gen.print_opfunc_oload_type())


def get_enum_decl(name, **kwargs):
    db = get_db_dxil()
    gen = db_enumhelp_gen(db)
    return run_with_stdout(lambda: gen.print_enum(db.enum_idx[name], **kwargs))


def get_rdat_enum_decl(name, **kwargs):
    db = get_db_dxil()
    gen = db_enumhelp_gen(db)
    return run_with_stdout(lambda: gen.print_rdat_enum(db.enum_idx[name], **kwargs))


def get_valrule_enum():
    return get_enum_decl("ValidationRule", hide_val=True)


def get_valrule_text():
    db = get_db_dxil()
    result = "switch(value) {\n"
    for v in db.enum_idx["ValidationRule"].values:
        result += (
            "  case hlsl::ValidationRule::" + v.name + ': return "' + v.err_msg + '";\n'
        )
    result += "}\n"
    return result


def get_instrhelper():
    db = get_db_dxil()
    gen = db_instrhelp_gen(db)
    return run_with_stdout(lambda: gen.print_body())


def get_instrs_pred(varname, pred, attr_name="dxil_opid"):
    db = get_db_dxil()
    if type(pred) == str:
        pred_fn = lambda i: getattr(i, pred)
    else:
        pred_fn = pred
    llvm_instrs = [i for i in db.instr if pred_fn(i)]
    result = format_comment(
        "// ",
        "Instructions: %s"
        % ", ".join([i.name + "=" + str(getattr(i, attr_name)) for i in llvm_instrs]),
    )
    result += "return %s;" % build_range_code(
        varname, [getattr(i, attr_name) for i in llvm_instrs]
    )
    result += "\n"
    return result


def counter_pred(name, dxil_op=True):
    def pred(i):
        return (
            (dxil_op == i.is_dxil_op)
            and getattr(i, "props")
            and "counters" in i.props
            and name in i.props["counters"]
        )

    return pred


def get_counters():
    db = get_db_dxil()
    return db.counters


def get_llvm_op_counters():
    db = get_db_dxil()
    return [c for c in db.counters if c in db.llvm_op_counters]


def get_dxil_op_counters():
    db = get_db_dxil()
    return [c for c in db.counters if c in db.dxil_op_counters]


def get_instrs_rst():
    "Create an rst table of allowed LLVM instructions."
    db = get_db_dxil()
    instrs = [i for i in db.instr if i.is_allowed and not i.is_dxil_op]
    instrs = sorted(instrs, key=lambda v: v.llvm_id)
    rows = []
    rows.append(["Instruction", "Action", "Operand overloads"])
    for i in instrs:
        rows.append([i.name, i.doc, i.oload_types])
    result = "\n\n" + format_rst_table(rows) + "\n\n"
    # Add detailed instruction information where available.
    for i in instrs:
        if i.remarks:
            result += i.name + "\n" + ("~" * len(i.name)) + "\n\n" + i.remarks + "\n\n"
    return result + "\n"


def get_init_passes(category_libs):
    "Create a series of statements to initialize passes in a registry."
    db = get_db_dxil()
    result = ""
    for p in sorted(db.passes, key=lambda p: p.type_name):
        # Skip if not in target category.
        if p.category_lib not in category_libs:
            continue

        result += "initialize%sPass(Registry);\n" % p.type_name
    return result


def get_pass_arg_names():
    "Return an ArrayRef of argument names based on passName"
    db = get_db_dxil()
    decl_result = ""
    check_result = ""
    for p in sorted(db.passes, key=lambda p: p.type_name):
        if len(p.args):
            decl_result += "static const LPCSTR %sArgs[] = { " % p.type_name
            check_result += (
                'if (strcmp(passName, "%s") == 0) return ArrayRef<LPCSTR>(%sArgs, _countof(%sArgs));\n'
                % (p.name, p.type_name, p.type_name)
            )
            sep = ""
            for a in p.args:
                decl_result += sep + '"%s"' % a.name
                sep = ", "
            decl_result += " };\n"
    return decl_result + check_result


def get_pass_arg_descs():
    "Return an ArrayRef of argument descriptions based on passName"
    db = get_db_dxil()
    decl_result = ""
    check_result = ""
    for p in sorted(db.passes, key=lambda p: p.type_name):
        if len(p.args):
            decl_result += "static const LPCSTR %sArgs[] = { " % p.type_name
            check_result += (
                'if (strcmp(passName, "%s") == 0) return ArrayRef<LPCSTR>(%sArgs, _countof(%sArgs));\n'
                % (p.name, p.type_name, p.type_name)
            )
            sep = ""
            for a in p.args:
                decl_result += sep + '"%s"' % a.doc
                sep = ", "
            decl_result += " };\n"
    return decl_result + check_result


def get_is_pass_option_name():
    "Create a return expression to check whether a value 'S' is a pass option name."
    db = get_db_dxil()
    prefix = ""
    result = "return "
    for k in sorted(db.pass_idx_args):
        result += prefix + 'S.equals("%s")' % k
        prefix = "\n  ||  "
    return result + ";"


def get_opcodes_rst():
    "Create an rst table of opcodes"
    db = get_db_dxil()
    instrs = [i for i in db.instr if i.is_allowed and i.is_dxil_op]
    instrs = sorted(instrs, key=lambda v: v.dxil_opid)
    rows = []
    rows.append(["ID", "Name", "Description"])
    for i in instrs:
        op_name = i.dxil_op
        if i.remarks:
            op_name = (
                op_name + "_"
            )  # append _ to enable internal hyperlink on rst files
        rows.append([i.dxil_opid, op_name, i.doc])
    result = "\n\n" + format_rst_table(rows) + "\n\n"
    # Add detailed instruction information where available.
    instrs = sorted(instrs, key=lambda v: v.name)
    for i in instrs:
        if i.remarks:
            result += i.name + "\n" + ("~" * len(i.name)) + "\n\n" + i.remarks + "\n\n"
    return result + "\n"


def get_valrules_rst():
    "Create an rst table of validation rules instructions."
    db = get_db_dxil()
    rules = [i for i in db.val_rules if not i.is_disabled]
    rules = sorted(rules, key=lambda v: v.name)
    rows = []
    rows.append(["Rule Code", "Description"])
    for i in rules:
        rows.append([i.name, i.doc])
    return "\n\n" + format_rst_table(rows) + "\n\n"


def get_opsigs():
    # Create a list of DXIL operation signatures, sorted by ID.
    db = get_db_dxil()
    instrs = [i for i in db.instr if i.is_dxil_op]
    instrs = sorted(instrs, key=lambda v: v.dxil_opid)
    # db_dxil already asserts that the numbering is dense.
    # Create the code to write out.
    code = "static const char *OpCodeSignatures[] = {\n"
    for inst_idx, i in enumerate(instrs):
        code += '  "('
        for operand in i.ops:
            if operand.pos > 1:  # skip 0 (the return value) and 1 (the opcode itself)
                code += operand.name
                if operand.pos < len(i.ops) - 1:
                    code += ","
        code += ')"'
        if inst_idx < len(instrs) - 1:
            code += ","
        code += "  // " + i.name
        code += "\n"
    code += "};\n"
    return code


shader_stage_to_ShaderKind = {
    "vertex": "Vertex",
    "pixel": "Pixel",
    "geometry": "Geometry",
    "compute": "Compute",
    "hull": "Hull",
    "domain": "Domain",
    "library": "Library",
    "raygeneration": "RayGeneration",
    "intersection": "Intersection",
    "anyhit": "AnyHit",
    "closesthit": "ClosestHit",
    "miss": "Miss",
    "callable": "Callable",
    "mesh": "Mesh",
    "amplification": "Amplification",
    "node": "Node",
}


def get_min_sm_and_mask_text():
    db = get_db_dxil()
    instrs = [i for i in db.instr if i.is_dxil_op]
    instrs = sorted(
        instrs,
        key=lambda v: (
            v.shader_model,
            v.shader_model_translated,
            v.shader_stages,
            v.dxil_opid,
        ),
    )
    last_model = None
    last_model_translated = None
    last_stage = None
    grouped_instrs = []
    code = ""

    def flush_instrs(grouped_instrs, last_model, last_model_translated, last_stage):
        if len(grouped_instrs) == 0:
            return ""
        result = format_comment(
            "// ",
            "Instructions: %s"
            % ", ".join([i.name + "=" + str(i.dxil_opid) for i in grouped_instrs]),
        )
        result += (
            "if ("
            + build_range_code("op", [i.dxil_opid for i in grouped_instrs])
            + ") {\n"
        )
        default = True
        if last_model != (6, 0):
            default = False
            if last_model_translated:
                result += "  if (bWithTranslation) {\n"
                result += (
                    "    major = %d;  minor = %d;\n  } else {\n  "
                    % last_model_translated
                )
            result += "  major = %d;  minor = %d;\n" % last_model
            if last_model_translated:
                result += "  }\n"
        if last_stage:
            default = False
            result += "  mask = %s;\n" % " | ".join(
                ["SFLAG(%s)" % shader_stage_to_ShaderKind[c] for c in last_stage]
            )
        if default:
            # don't write these out, instead fall through
            return ""
        return result + "  return;\n}\n"

    for i in instrs:
        if (i.shader_model, i.shader_model_translated, i.shader_stages) != (
            last_model,
            last_model_translated,
            last_stage,
        ):
            code += flush_instrs(
                grouped_instrs, last_model, last_model_translated, last_stage
            )
            grouped_instrs = []
            last_model = i.shader_model
            last_model_translated = i.shader_model_translated
            last_stage = i.shader_stages
        grouped_instrs.append(i)
    code += flush_instrs(grouped_instrs, last_model, last_model_translated, last_stage)
    return code


check_pSM_for_shader_stage = {
    "vertex": "SK == DXIL::ShaderKind::Vertex",
    "pixel": "SK == DXIL::ShaderKind::Pixel",
    "geometry": "SK == DXIL::ShaderKind::Geometry",
    "compute": "SK == DXIL::ShaderKind::Compute",
    "hull": "SK == DXIL::ShaderKind::Hull",
    "domain": "SK == DXIL::ShaderKind::Domain",
    "library": "SK == DXIL::ShaderKind::Library",
    "raygeneration": "SK == DXIL::ShaderKind::RayGeneration",
    "intersection": "SK == DXIL::ShaderKind::Intersection",
    "anyhit": "SK == DXIL::ShaderKind::AnyHit",
    "closesthit": "SK == DXIL::ShaderKind::ClosestHit",
    "miss": "SK == DXIL::ShaderKind::Miss",
    "callable": "SK == DXIL::ShaderKind::Callable",
    "mesh": "SK == DXIL::ShaderKind::Mesh",
    "amplification": "SK == DXIL::ShaderKind::Amplification",
    "node": "SK == DXIL::ShaderKind::Node",
}


def get_valopcode_sm_text():
    db = get_db_dxil()
    instrs = [i for i in db.instr if i.is_dxil_op]
    instrs = sorted(
        instrs, key=lambda v: (v.shader_model, v.shader_stages, v.dxil_opid)
    )
    last_model = None
    last_stage = None
    grouped_instrs = []
    code = ""

    def flush_instrs(grouped_instrs, last_model, last_stage):
        if len(grouped_instrs) == 0:
            return ""
        result = format_comment(
            "// ",
            "Instructions: %s"
            % ", ".join([i.name + "=" + str(i.dxil_opid) for i in grouped_instrs]),
        )
        result += (
            "if ("
            + build_range_code("op", [i.dxil_opid for i in grouped_instrs])
            + ")\n"
        )
        result += "  return "

        model_cond = stage_cond = None
        if last_model != (6, 0):
            model_cond = "major > %d || (major == %d && minor >= %d)" % (
                last_model[0],
                last_model[0],
                last_model[1],
            )
        if last_stage:
            stage_cond = " || ".join(
                [check_pSM_for_shader_stage[c] for c in last_stage]
            )
        if model_cond or stage_cond:
            result += "\n      && ".join(
                ["(%s)" % expr for expr in (model_cond, stage_cond) if expr]
            )
            return result + ";\n"
        else:
            # don't write these out, instead fall through
            return ""

    for i in instrs:
        if (i.shader_model, i.shader_stages) != (last_model, last_stage):
            code += flush_instrs(grouped_instrs, last_model, last_stage)
            grouped_instrs = []
            last_model = i.shader_model
            last_stage = i.shader_stages
        grouped_instrs.append(i)
    code += flush_instrs(grouped_instrs, last_model, last_stage)
    code += "return true;\n"
    return code


def get_sigpoint_table():
    db = get_db_dxil()
    gen = db_sigpoint_gen(db)
    return run_with_stdout(lambda: gen.print_sigpoint_table())


def get_sigpoint_rst():
    "Create an rst table for SigPointKind."
    db = get_db_dxil()
    rows = [row[:] for row in db.sigpoint_table[:-1]]  # Copy table
    e = dict([(v.name, v) for v in db.enum_idx["SigPointKind"].values])
    rows[0] = ["ID"] + rows[0] + ["Description"]
    for i in range(1, len(rows)):
        row = rows[i]
        v = e[row[0]]
        rows[i] = [v.value] + row + [v.doc]
    return "\n\n" + format_rst_table(rows) + "\n\n"


def get_sem_interpretation_enum_rst():
    db = get_db_dxil()
    rows = [["ID", "Name", "Description"]] + [
        [v.value, v.name, v.doc]
        for v in db.enum_idx["SemanticInterpretationKind"].values[:-1]
    ]
    return "\n\n" + format_rst_table(rows) + "\n\n"


def get_sem_interpretation_table_rst():
    db = get_db_dxil()
    return "\n\n" + format_rst_table(db.interpretation_table) + "\n\n"


def get_interpretation_table():
    db = get_db_dxil()
    gen = db_sigpoint_gen(db)
    return run_with_stdout(lambda: gen.print_interpretation_table())


# highest minor is different than highest released minor,
# since there can be pre-release versions that are higher
# than the last released version
highest_major = 6
highest_minor = 9
highest_shader_models = {4: 1, 5: 1, 6: highest_minor}

# fetch the last released version from latest-released.json
json_path = os.path.dirname(os.path.dirname(__file__)) + "/version/latest-release.json"
with open(json_path, "r") as file:
    json_data = json.load(file)

highest_released_minor = int(json_data["version"]["minor"])


def getShaderModels():
    shader_models = []
    for major, minor in highest_shader_models.items():
        for i in range(0, minor + 1):
            shader_models.append(str(major) + "_" + str(i))

    return shader_models


def get_highest_released_shader_model():
    result = """static const unsigned kHighestReleasedMajor = %d;
static const unsigned kHighestReleasedMinor = %d;""" % (
        highest_major,
        highest_released_minor,
    )
    return result


def get_highest_shader_model():
    result = """static const unsigned kHighestMajor = %d;
static const unsigned kHighestMinor = %d;""" % (
        highest_major,
        highest_minor,
    )
    return result


def get_dxil_version_minor():
    return "const unsigned kDxilMinor = %d;" % highest_minor


def get_dxil_version_minor_int():
    return highest_minor


def get_is_shader_model_plus():
    result = ""

    for i in range(0, highest_minor + 1):
        result += "bool IsSM%d%dPlus() const { return IsSMAtLeast(%d, %d); }\n" % (
            highest_major,
            i,
            highest_major,
            i,
        )
    return result


profile_to_kind = {
    "ps": "Kind::Pixel",
    "vs": "Kind::Vertex",
    "gs": "Kind::Geometry",
    "hs": "5_0",
    "ds": "5_0",
    "cs": "4_0",
    "lib": "6_1",
    "ms": "6_5",
    "as": "6_5",
}


class shader_profile(object):
    "The profile description for a DXIL instruction"

    def __init__(self, kind, kind_name, enum_name, start_sm, input_size, output_size):
        self.kind = kind  # position in parameter list
        self.kind_name = kind_name
        self.enum_name = enum_name
        self.start_sm = start_sm
        self.input_size = input_size
        self.output_size = output_size


# kind is from DXIL::ShaderKind.
shader_profiles = [
    shader_profile(0, "ps", "Kind::Pixel", "4_0", 32, 8),
    shader_profile(1, "vs", "Kind::Vertex", "4_0", 32, 32),
    shader_profile(2, "gs", "Kind::Geometry", "4_0", 32, 32),
    shader_profile(3, "hs", "Kind::Hull", "5_0", 32, 32),
    shader_profile(4, "ds", "Kind::Domain", "5_0", 32, 32),
    shader_profile(5, "cs", "Kind::Compute", "4_0", 0, 0),
    shader_profile(6, "lib", "Kind::Library", "6_1", 32, 32),
    shader_profile(13, "ms", "Kind::Mesh", "6_5", 0, 0),
    shader_profile(14, "as", "Kind::Amplification", "6_5", 0, 0),
]


def getShaderProfiles():
    # order match DXIL::ShaderKind.
    profiles = (
        ("ps", "4_0"),
        ("vs", "4_0"),
        ("gs", "4_0"),
        ("hs", "5_0"),
        ("ds", "5_0"),
        ("cs", "4_0"),
        ("lib", "6_1"),
        ("ms", "6_5"),
        ("as", "6_5"),
    )
    return profiles


def get_shader_models():
    result = ""
    for profile in shader_profiles:
        min_sm = profile.start_sm
        input_size = profile.input_size
        output_size = profile.output_size
        kind = profile.kind
        kind_name = profile.kind_name
        enum_name = profile.enum_name

        for major, minor in highest_shader_models.items():
            UAV_info = "true, true, UINT_MAX"
            if major > 5:
                pass
            elif major == 4:
                UAV_info = "false, false, 0"
                if kind == "cs":
                    UAV_info = "true, false, 1"

            elif major == 5:
                UAV_info = "true, true, 64"

            for i in range(0, minor + 1):
                sm = "%d_%d" % (major, i)
                if min_sm > sm:
                    continue

                input_size = profile.input_size
                output_size = profile.output_size

                if major == 4:
                    if i == 0:
                        if kind_name == "gs":
                            input_size = 16
                        elif kind_name == "vs":
                            input_size = 16
                            output_size = 16

                sm_name = "%s_%s" % (kind_name, sm)
                result += 'SM(%s, %d, %d, "%s", %d, %d, %s),\n' % (
                    enum_name,
                    major,
                    i,
                    sm_name,
                    input_size,
                    output_size,
                    UAV_info,
                )

        if kind_name == "lib":
            result += (
                "// lib_6_x is for offline linking only, and relaxes restrictions\n"
            )
            result += 'SM(Kind::Library,  6, kOfflineMinor, "lib_6_x",  32, 32,  true,  true,  UINT_MAX),\n'

    result += (
        "// Values before Invalid must remain sorted by Kind, then Major, then Minor.\n"
    )
    result += 'SM(Kind::Invalid,  0, 0, "invalid", 0,  0,   false, false, 0),\n'
    return result


def get_num_shader_models():
    count = 0
    for profile in shader_profiles:
        min_sm = profile.start_sm
        input_size = profile.input_size
        output_size = profile.output_size
        kind = profile.kind
        kind_name = profile.kind_name
        enum_name = profile.enum_name

        for major, minor in highest_shader_models.items():
            for i in range(0, minor + 1):
                sm = "%d_%d" % (major, i)
                if min_sm > sm:
                    continue
                count += 1

        if kind_name == "lib":
            # for lib_6_x
            count += 1
    # for invalid shader_model.
    count += 1
    return "static const unsigned kNumShaderModels = %d;" % count


def build_shader_model_hash_idx_map():
    # must match get_shader_models.
    result = "const static std::pair<unsigned, unsigned> hashToIdxMap[] = {\n"
    count = 0
    for profile in shader_profiles:
        min_sm = profile.start_sm
        kind = profile.kind
        kind_name = profile.kind_name

        for major, minor in highest_shader_models.items():
            for i in range(0, minor + 1):
                sm = "%d_%d" % (major, i)
                if min_sm > sm:
                    continue
                sm_name = "%s_%s" % (kind_name, sm)
                hash_v = kind << 16 | major << 8 | i
                result += "{%d,%d}, //%s\n" % (hash_v, count, sm_name)
                count += 1

        if kind_name == "lib":
            result += (
                "// lib_6_x is for offline linking only, and relaxes restrictions\n"
            )
            major = 6
            # static const unsigned kOfflineMinor = 0xF;
            i = 15
            hash_v = kind << 16 | major << 8 | i
            result += "{%d,%d},//%s\n" % (hash_v, count, "lib_6_x")
            count += 1

    result += "};\n"
    return result


def get_validation_version():
    result = (
        """// 1.0 is the first validator.
// 1.1 adds:
// - ILDN container part support
// 1.2 adds:
// - Metadata for floating point denorm mode
// 1.3 adds:
// - Library support
// - Raytracing support
// - i64/f64 overloads for rawBufferLoad/Store
// 1.4 adds:
// - packed u8x4/i8x4 dot with accumulate to i32
// - half dot2 with accumulate to float
// 1.5 adds:
// - WaveMatch, WaveMultiPrefixOp, WaveMultiPrefixBitCount
// - HASH container part support
// - Mesh and Amplification shaders
// - DXR 1.1 & RayQuery support
*pMajor = 1;
*pMinor = %d;
"""
        % highest_minor
    )
    return result


def get_target_profiles():
    result = 'HelpText<"Set target profile. \\n'
    result += "\\t<profile>: "

    profiles = getShaderProfiles()
    shader_models = getShaderModels()

    base_sm = "%d_0" % highest_major
    for profile, min_sm in profiles:
        for shader_model in shader_models:
            if base_sm > shader_model:
                continue
            if min_sm > shader_model:
                continue
            result += "%s_%s, " % (profile, shader_model)
        result += "\\n\\t\\t "

    result += '">;'
    return result


def get_min_validator_version():
    result = ""
    for i in range(0, highest_minor + 1):
        result += "case %d:\n" % i
        result += "  ValMinor = %d;\n" % i
        result += "  break;\n"
    return result


def get_dxil_version():
    result = ""
    for i in range(0, highest_minor + 1):
        result += "case %d:\n" % i
        result += "  DxilMinor = %d;\n" % i
        result += "  break;\n"
    result += "case kOfflineMinor: // Always update this to highest dxil version\n"
    result += "  DxilMinor = %d;\n" % highest_minor
    result += "  break;\n"
    return result


def get_shader_model_get():
    # const static std::pair<unsigned, unsigned> hashToIdxMap[] = {};
    result = build_shader_model_hash_idx_map()
    result += "unsigned hash = (unsigned)Kind << 16 | Major << 8 | Minor;\n"
    result += "auto pred = [](const std::pair<unsigned, unsigned>& elem, unsigned val){ return elem.first < val;};\n"
    result += "auto it = std::lower_bound(std::begin(hashToIdxMap), std::end(hashToIdxMap), hash, pred);\n"
    result += "if (it == std::end(hashToIdxMap) || it->first != hash)\n"
    result += "  return GetInvalid();\n"
    result += "return &ms_ShaderModels[it->second];"
    return result


def get_shader_model_by_name():
    result = ""
    for i in range(2, highest_minor + 1):
        result += "case '%d':\n" % i
        result += "  if (Major == %d) {\n" % highest_major
        result += "    Minor = %d;\n" % i
        result += "    break;\n"
        result += "  }\n"
        result += "else return GetInvalid();\n"

    return result


def get_is_valid_for_dxil():
    result = ""
    for i in range(0, highest_minor + 1):
        result += "case %d:\n" % i
    return result


def RunCodeTagUpdate(file_path):
    import os
    import CodeTags

    print(" ... updating " + file_path)
    args = [file_path, file_path + ".tmp"]
    result = CodeTags.main(args)
    if result != 0:
        print(" ... error: %d" % result)
    else:
        with open(file_path, "rt") as f:
            before = f.read()
        with open(file_path + ".tmp", "rt") as f:
            after = f.read()
        if before == after:
            print("  --- no changes found")
        else:
            print("  +++ changes found, updating file")
            with open(file_path, "wt") as f:
                f.write(after)
        os.remove(file_path + ".tmp")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Generate code to handle instructions."
    )
    parser.add_argument(
        "-gen",
        choices=["docs-ref", "docs-spec", "inst-header", "enums", "oloads", "valfns"],
        help="Output type to generate.",
    )
    parser.add_argument("-update-files", action="store_const", const=True)
    args = parser.parse_args()

    db = get_db_dxil()  # used by all generators, also handy to have it run validation

    if args.gen == "docs-ref":
        gen = db_docsref_gen(db)
        gen.print_content()

    if args.gen == "docs-spec":
        import os, docutils.core

        assert (
            "HLSL_SRC_DIR" in os.environ
        ), "Environment variable HLSL_SRC_DIR is not defined"
        hlsl_src_dir = os.environ["HLSL_SRC_DIR"]
        spec_file = os.path.abspath(os.path.join(hlsl_src_dir, "docs/DXIL.rst"))
        with open(spec_file) as f:
            s = docutils.core.publish_file(f, writer_name="html")

    if args.gen == "inst-header":
        gen = db_instrhelp_gen(db)
        gen.print_content()

    if args.gen == "enums":
        gen = db_enumhelp_gen(db)
        gen.print_content()

    if args.gen == "oloads":
        gen = db_oload_gen(db)
        gen.print_content()

    if args.gen == "valfns":
        gen = db_valfns_gen(db)
        gen.print_content()

    if args.update_files:
        print("Updating files ...")
        import CodeTags
        import os

        assert (
            "HLSL_SRC_DIR" in os.environ
        ), "Environment variable HLSL_SRC_DIR is not defined"
        hlsl_src_dir = os.environ["HLSL_SRC_DIR"]
        pj = lambda *parts: os.path.abspath(os.path.join(*parts))
        files = [
            "docs/DXIL.rst",
            "lib/DXIL/DxilOperations.cpp",
            "lib/DXIL/DxilShaderModel.cpp",
            "include/dxc/DXIL/DxilConstants.h",
            "include/dxc/DXIL/DxilShaderModel.h",
            "include/dxc/Support/HLSLOptions.td",
            "include/dxc/DXIL/DxilInstructions.h",
            "lib/HLSL/DxcOptimizer.cpp",
            "lib/DxilPIXPasses/DxilPIXPasses.cpp",
            "lib/HLSL/DxilValidation.cpp",
            "tools/clang/lib/Sema/gen_intrin_main_tables_15.h",
            "include/dxc/HlslIntrinsicOp.h",
            "tools/clang/tools/dxcompiler/dxcdisassembler.cpp",
            "include/dxc/DXIL/DxilSigPoint.inl",
            "include/dxc/DXIL/DxilCounters.h",
            "lib/DXIL/DxilCounters.cpp",
            "lib/DXIL/DxilMetadataHelper.cpp",
            "include/dxc/DxilContainer/RDAT_LibraryTypes.inl",
        ]
        for relative_file_path in files:
            RunCodeTagUpdate(pj(hlsl_src_dir, relative_file_path))
