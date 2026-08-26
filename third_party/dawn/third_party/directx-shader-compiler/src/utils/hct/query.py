from hctdb_instrhelp import get_db_dxil
from hctdb_instrhelp import get_db_hlsl
import argparse


# user defined function
def inst_query_dxil(insts, inst):
    # example

    if inst.ret_type == "v" and inst.fn_attr != "":
        return True
    return False

    # example
    """
    if inst.fn_attr == "" and "Quad" in inst.name:
        found = false
        for other_inst in insts:
            if not (other_inst == inst) and "Quad" in other_inst.name and other_inst.fn_attr != ""
                found = true
                break
        return found
        
    return false
    """


def inst_query_hlsl(insts, inst):
    # example
    """
    if inst.ret_type == "v":
        return true
    return false
    """

    # example
    """
    if inst.fn_attr == "" and "Quad" in inst.name:
        found = false
        for other_inst in insts:
            if not (other_inst == inst) and "Quad" in other_inst.name and other_inst.fn_attr != ""
                found = true
                break
        return found
        
    return false
    """

    # example
    if inst.wave:
        return True
    return False


class DxilInstructionWrapper:
    def set_ret_type_and_args(self, dxil_inst):
        ops = []
        for op in dxil_inst.ops:
            ops.append(op.llvm_type)

        if len(ops) > 0:
            self.ret_type = ops[0]

        if len(ops) > 1:
            self.args = ops[1:]

    def __init__(self, dxil_inst):
        self.dxil_inst = dxil_inst  # db_dxil_inst type, defined in hctdb.py
        self.name = dxil_inst.name
        self.fn_attr = dxil_inst.fn_attr
        self.wave = dxil_inst.is_wave  # bool
        self.ret_type = ""
        self.args = []
        self.set_ret_type_and_args(dxil_inst)

    def __str__(self):
        str_args = ", ".join(self.args)
        return "[{}] {} {}({})".format(self.fn_attr, self.ret_type, self.name, str_args)


class HLOperationWrapper:
    def set_ret_type_and_args(self, hl_op):
        ops = hl_op.params
        if len(ops) > 0:
            self.ret_type = ops[0].type_name

        if len(ops) > 1:
            self.args = [x.name + " " + x.type_name for x in ops[1:]]

    def __init__(self, hl_op):
        self.hl_op = hl_op  # db_hlsl_intrinsic type, defined in hctdb.py
        self.name = hl_op.name
        self.fn_attr = "rn" if hl_op.readnone else ""
        self.fn_attr += "ro" if hl_op.readonly else ""
        self.wave = hl_op.wave  # bool
        self.ret_type = ""
        self.args = []
        self.set_ret_type_and_args(hl_op)

    def __str__(self):
        str_args = ", ".join(self.args)
        return "[{}] {} {}({})".format(self.fn_attr, self.ret_type, self.name, str_args)


def parse_query_hlsl(db, options):
    HLInstructions = []

    # The query function should be using the HLOperationWrapper interface
    # because that is what's being loaded into the instructions list.
    for hl_op in db.intrinsics:
        new_op = HLOperationWrapper(hl_op)
        HLInstructions.append(new_op)

    # apply the query filter
    print("\nQUERY RESULTS:\n")
    for instruction in HLInstructions:
        if inst_query_hlsl(HLInstructions, instruction):
            print(instruction)


def parse_query_dxil(db, options):
    DxilInstructions = []

    # The query function should use the DxilInstructionWrapper interface
    # because that's what's being loaded into the DxilInstructions list
    for dxil_inst in db.instr:
        i = DxilInstructionWrapper(dxil_inst)
        DxilInstructions.append(i)

    # apply the query filter
    print("\nQUERY RESULTS:\n")
    for instruction in DxilInstructions:
        if inst_query_dxil(DxilInstructions, instruction):
            print(instruction)


parser = argparse.ArgumentParser(description="Query instructions.")
parser.add_argument(
    "-query", choices=["dxil", "hlsl"], help="type of instructions to query."
)
args = parser.parse_args()

if args.query == "dxil":
    db_dxil = get_db_dxil()
    parse_query_dxil(db_dxil, args)


if args.query == "hlsl":
    db_hlsl = get_db_hlsl()
    parse_query_hlsl(db_hlsl, args)
