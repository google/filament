# This file is distributed under the University of Illinois Open Source License. See LICENSE.TXT for details
###############################################################################
# This file contains driver test information for DXIL operations              #
###############################################################################

from hctdb import *
import xml.etree.ElementTree as ET
import argparse

parser = argparse.ArgumentParser(
    description="contains information about dxil op test cases."
)
parser.add_argument("mode", help="'gen-xml' or 'info'")

g_db_dxil = None


def get_db_dxil():
    global g_db_dxil
    if g_db_dxil is None:
        g_db_dxil = db_dxil()
    return g_db_dxil


"""
This class represents a test case for instructions for driver testings

DXIL instructions and test cases are two disjoint sets where each instruction can have multiple test cases,
and each test case can cover different DXIL instructions. So these two sets form a bipartite graph.

test_name: Test case identifier. Must be unique for each test case.
insts: dxil instructions
validation_type: validation type for test
    epsilon: absolute difference check
    ulp: units in last place check
    relative: relative error check
validation_tolerance: tolerance value for a given test
inputs: testing inputs
outputs: expected outputs for each input
shader_target: target for testing
shader_text: hlsl file that is used for testing dxil op
"""


class test_case(object):
    def __init__(
        self,
        test_name,
        insts,
        validation_type,
        validation_tolerance,
        input_lists,
        output_lists,
        shader_target,
        shader_text,
        **kwargs
    ):
        self.test_name = test_name
        self.validation_type = validation_type
        self.validation_tolerance = validation_tolerance
        self.input_lists = input_lists
        self.output_lists = output_lists
        self.shader_target = shader_target
        self.shader_text = shader_text
        self.insts = insts  # list of instructions each test case cover
        self.warp_version = -1  # known warp version that works
        self.shader_arguments = ""
        for k, v in kwargs.items():
            setattr(self, k, v)


# Wrapper for each DXIL instruction
class inst_node(object):
    def __init__(self, inst):
        self.inst = inst
        self.test_cases = []  # list of test_case


def add_test_case(
    test_name,
    inst_names,
    validation_type,
    validation_tolerance,
    input_lists,
    output_lists,
    shader_target,
    shader_text,
    **kwargs
):
    insts = []
    for inst_name in inst_names:
        assert inst_name in g_instruction_nodes
        insts += [g_instruction_nodes[inst_name].inst]
    case = test_case(
        test_name,
        insts,
        validation_type,
        validation_tolerance,
        input_lists,
        output_lists,
        shader_target,
        shader_text,
        **kwargs
    )
    g_test_cases[test_name] = case
    # update instruction nodes
    for inst_name in inst_names:
        g_instruction_nodes[inst_name].test_cases += [case]


def add_test_case_int(
    test_name,
    inst_names,
    validation_type,
    validation_tolerance,
    input_lists,
    output_lists,
    shader_key,
    shader_op_name,
    **kwargs
):
    add_test_case(
        test_name,
        inst_names,
        validation_type,
        validation_tolerance,
        input_lists,
        output_lists,
        "cs_6_0",
        get_shader_text(shader_key, shader_op_name),
        **kwargs
    )
    input_lists_16, output_lists_16 = input_lists, output_lists
    if "input_16" in kwargs:
        input_lists_16 = kwargs["input_16"]
    if "output_16" in kwargs:
        output_lists_16 = kwargs["output_16"]
    add_test_case(
        test_name + "Bit16",
        inst_names,
        validation_type,
        validation_tolerance,
        input_lists_16,
        output_lists_16,
        "cs_6_2",
        get_shader_text(shader_key.replace("int", "int16_t"), shader_op_name),
        shader_arguments="-enable-16bit-types",
        **kwargs
    )


def add_test_case_float_half(
    test_name,
    inst_names,
    validation_type,
    validation_tolerance,
    float_input_lists,
    float_output_lists,
    shader_key,
    shader_op_name,
    **kwargs
):
    add_test_case(
        test_name,
        inst_names,
        validation_type,
        validation_tolerance,
        float_input_lists,
        float_output_lists,
        "cs_6_0",
        get_shader_text(shader_key, shader_op_name),
        **kwargs
    )
    # if half test cases are different from float input lists, use those lists instead for half testings
    (
        half_input_lists,
        half_output_lists,
        half_validation_type,
        half_validation_tolerance,
    ) = (float_input_lists, float_output_lists, validation_type, validation_tolerance)
    if "half_inputs" in kwargs:
        half_input_lists = kwargs["half_inputs"]
    if "half_outputs" in kwargs:
        half_output_lists = kwargs["half_outputs"]
    if "half_validation_type" in kwargs:
        half_validation_type = kwargs["half_validation_type"]
    if "half_validation_tolerance" in kwargs:
        half_validation_tolerance = kwargs["half_validation_tolerance"]
    # skip relative error test check for half for now
    if validation_type != "Relative":
        add_test_case(
            test_name + "Half",
            inst_names,
            half_validation_type,
            half_validation_tolerance,
            half_input_lists,
            half_output_lists,
            "cs_6_2",
            get_shader_text(shader_key.replace("float", "half"), shader_op_name),
            shader_arguments="-enable-16bit-types",
            **kwargs
        )


def add_test_case_denorm(
    test_name,
    inst_names,
    validation_type,
    validation_tolerance,
    input_lists,
    output_lists_ftz,
    output_lists_preserve,
    shader_target,
    shader_text,
    **kwargs
):
    add_test_case(
        test_name + "FTZ",
        inst_names,
        validation_type,
        validation_tolerance,
        input_lists,
        output_lists_ftz,
        shader_target,
        shader_text,
        shader_arguments="-denorm ftz",
    )
    add_test_case(
        test_name + "Preserve",
        inst_names,
        validation_type,
        validation_tolerance,
        input_lists,
        output_lists_preserve,
        shader_target,
        shader_text,
        shader_arguments="-denorm preserve",
    )
    # we can expect the same output for "any" and "preserve" mode. We should make sure that for validation zero are accepted outputs for denormal outputs.
    add_test_case(
        test_name + "Any",
        inst_names,
        validation_type,
        validation_tolerance,
        input_lists,
        output_lists_preserve + output_lists_ftz,
        shader_target,
        shader_text,
        shader_arguments="-denorm any",
    )


g_shader_texts = {
    "unary int": """ struct SUnaryIntOp {
                int input;
                int output;
            };
            RWStructuredBuffer<SUnaryIntOp> g_buf : register(u0);
            [numthreads(8,8,1)]
            void main(uint GI : SV_GroupIndex) {
                SUnaryIntOp l = g_buf[GI];
                l.output = %s(l.input);
                g_buf[GI] = l;
            };""",
    "unary int16_t": """ struct SUnaryInt16Op {
                int16_t input;
                int16_t output;
            };
            RWStructuredBuffer<SUnaryInt16Op> g_buf : register(u0);
            [numthreads(8,8,1)]
            void main(uint GI : SV_GroupIndex) {
                SUnaryInt16Op l = g_buf[GI];
                l.output = %s(l.input);
                g_buf[GI] = l;
            };""",
    "unary uint": """ struct SUnaryUintOp {
                uint input;
                uint output;
            };
            RWStructuredBuffer<SUnaryUintOp> g_buf : register(u0);
            [numthreads(8,8,1)]
            void main(uint GI : SV_GroupIndex) {
                SUnaryUintOp l = g_buf[GI];
                l.output = %s(l.input);
                g_buf[GI] = l;
            };""",
    "unary uint16_t": """ struct SUnaryUint16Op {
                uint16_t input;
                uint16_t output;
            };
            RWStructuredBuffer<SUnaryUint16Op> g_buf : register(u0);
            [numthreads(8,8,1)]
            void main(uint GI : SV_GroupIndex) {
                SUnaryUint16Op l = g_buf[GI];
                l.output = %s(l.input);
                g_buf[GI] = l;
            };""",
    "unary float": """ struct SUnaryFPOp {
                float input;
                float output;
            };
            RWStructuredBuffer<SUnaryFPOp> g_buf : register(u0);
            [numthreads(8,8,1)]
            void main(uint GI : SV_GroupIndex) {
                SUnaryFPOp l = g_buf[GI];
                l.output = %s(l.input);
                g_buf[GI] = l;
            };""",
    "unary float bool": """ struct SUnaryFPOp {
                float input;
                float output;
            };
            RWStructuredBuffer<SUnaryFPOp> g_buf : register(u0);
            [numthreads(8,8,1)]
            void main(uint GI : SV_GroupIndex) {
                SUnaryFPOp l = g_buf[GI];
                if (%s(l.input))
                    l.output = 1;
                else
                    l.output = 0;
                g_buf[GI] = l;
            };""",
    "unary half": """ struct SUnaryFPOp {
                float16_t input;
                float16_t output;
            };
            RWStructuredBuffer<SUnaryFPOp> g_buf : register(u0);
            [numthreads(8,8,1)]
            void main(uint GI : SV_GroupIndex) {
                SUnaryFPOp l = g_buf[GI];
                l.output = %s(l.input);
                g_buf[GI] = l;
            };""",
    "unary half bool": """ struct SUnaryFPOp {
                float16_t input;
                float16_t output;
            };
            RWStructuredBuffer<SUnaryFPOp> g_buf : register(u0);
            [numthreads(8,8,1)]
            void main(uint GI : SV_GroupIndex) {
                SUnaryFPOp l = g_buf[GI];
                if (%s(l.input))
                    l.output = 1;
                else
                    l.output = 0;
                g_buf[GI] = l;
            };""",
    "binary int": """ struct SBinaryIntOp {
                int input1;
                int input2;
                int output1;
                int output2;
            };
            RWStructuredBuffer<SBinaryIntOp> g_buf : register(u0);
            [numthreads(8,8,1)]
            void main(uint GI : SV_GroupIndex) {
                SBinaryIntOp l = g_buf[GI];
                l.output1 = l.input1 %s l.input2;
                g_buf[GI] = l;
            };""",
    "binary int16_t": """ struct SBinaryInt16Op {
                int16_t input1;
                int16_t input2;
                int16_t output1;
                int16_t output2;
            };
            RWStructuredBuffer<SBinaryInt16Op> g_buf : register(u0);
            [numthreads(8,8,1)]
            void main(uint GI : SV_GroupIndex) {
                SBinaryInt16Op l = g_buf[GI];
                l.output1 = l.input1 %s l.input2;
                g_buf[GI] = l;
            };""",
    "binary int call": """ struct SBinaryIntOp {
                int input1;
                int input2;
                int output1;
                int output2;
            };
            RWStructuredBuffer<SBinaryIntOp> g_buf : register(u0);
            [numthreads(8,8,1)]
            void main(uint GI : SV_GroupIndex) {
                SBinaryIntOp l = g_buf[GI];
                l.output1 = %s(l.input1,l.input2);
                g_buf[GI] = l;
            };""",
    "binary int16_t call": """ struct SBinaryInt16Op {
                int16_t input1;
                int16_t input2;
                int16_t output1;
                int16_t output2;
            };
            RWStructuredBuffer<SBinaryInt16Op> g_buf : register(u0);
            [numthreads(8,8,1)]
            void main(uint GI : SV_GroupIndex) {
                SBinaryInt16Op l = g_buf[GI];
                l.output1 = %s(l.input1,l.input2);
                g_buf[GI] = l;
            };""",
    "binary uint": """ struct SBinaryUintOp {
                uint input1;
                uint input2;
                uint output1;
                uint output2;
            };
            RWStructuredBuffer<SBinaryUintOp> g_buf : register(u0);
            [numthreads(8,8,1)]
            void main(uint GI : SV_GroupIndex) {
                SBinaryUintOp l = g_buf[GI];
                l.output1 = l.input1 %s l.input2;
                g_buf[GI] = l;
            };""",
    "binary uint16_t": """ struct SBinaryUint16Op {
                uint16_t input1;
                uint16_t input2;
                uint16_t output1;
                uint16_t output2;
            };
            RWStructuredBuffer<SBinaryUint16Op> g_buf : register(u0);
            [numthreads(8,8,1)]
            void main(uint GI : SV_GroupIndex) {
                SBinaryUint16Op l = g_buf[GI];
                l.output1 = l.input1 %s l.input2;
                g_buf[GI] = l;
            };""",
    "binary uint call": """ struct SBinaryUintOp {
                uint input1;
                uint input2;
                uint output1;
                uint output2;
            };
            RWStructuredBuffer<SBinaryUintOp> g_buf : register(u0);
            [numthreads(8,8,1)]
            void main(uint GI : SV_GroupIndex) {
                SBinaryUintOp l = g_buf[GI];
                l.output1 = %s(l.input1,l.input2);
                g_buf[GI] = l;
            };""",
    "binary uint16_t call": """ struct SBinaryUint16Op {
                uint16_t input1;
                uint16_t input2;
                uint16_t output1;
                uint16_t output2;
            };
            RWStructuredBuffer<SBinaryUint16Op> g_buf : register(u0);
            [numthreads(8,8,1)]
            void main(uint GI : SV_GroupIndex) {
                SBinaryUint16Op l = g_buf[GI];
                l.output1 = %s(l.input1,l.input2);
                g_buf[GI] = l;
            };""",
    "binary float": """ struct SBinaryFPOp {
                float input1;
                float input2;
                float output1;
                float output2;
            };
            RWStructuredBuffer<SBinaryFPOp> g_buf : register(u0);
            [numthreads(8,8,1)]
            void main(uint GI : SV_GroupIndex) {
                SBinaryFPOp l = g_buf[GI];
                l.output1 = l.input1 %s l.input2;
                g_buf[GI] = l;
            };""",
    "binary float call": """ struct SBinaryFPOp {
                float input1;
                float input2;
                float output1;
                float output2;
            };
            RWStructuredBuffer<SBinaryFPOp> g_buf : register(u0);
            [numthreads(8,8,1)]
            void main(uint GI : SV_GroupIndex) {
                SBinaryFPOp l = g_buf[GI];
                l.output1 = %s(l.input1,l.input2);
                g_buf[GI] = l;
            };""",
    "binary half": """ struct SBinaryFPOp {
                half input1;
                half input2;
                half output1;
                half output2;
            };
            RWStructuredBuffer<SBinaryFPOp> g_buf : register(u0);
            [numthreads(8,8,1)]
            void main(uint GI : SV_GroupIndex) {
                SBinaryFPOp l = g_buf[GI];
                l.output1 = l.input1 %s l.input2;
                g_buf[GI] = l;
            };""",
    "binary half call": """ struct SBinaryFPOp {
                half input1;
                half input2;
                half output1;
                half output2;
            };
            RWStructuredBuffer<SBinaryFPOp> g_buf : register(u0);
            [numthreads(8,8,1)]
            void main(uint GI : SV_GroupIndex) {
                SBinaryFPOp l = g_buf[GI];
                l.output1 = %s(l.input1,l.input2);
                g_buf[GI] = l;
            };""",
    "tertiary int": """ struct STertiaryIntOp {
                int input1;
                int input2;
                int input3;
                int output;
            };
            RWStructuredBuffer<STertiaryIntOp> g_buf : register(u0);
            [numthreads(8,8,1)]
            void main(uint GI : SV_GroupIndex) {
                STertiaryIntOp l = g_buf[GI];
                l.output = %s(l.input1, l.input2, l.input3);
                g_buf[GI] = l;
            };""",
    "tertiary int16_t": """ struct STertiaryInt16Op {
                int16_t input1;
                int16_t input2;
                int16_t input3;
                int16_t output;
            };
            RWStructuredBuffer<STertiaryInt16Op> g_buf : register(u0);
            [numthreads(8,8,1)]
            void main(uint GI : SV_GroupIndex) {
                STertiaryInt16Op l = g_buf[GI];
                l.output = %s(l.input1, l.input2, l.input3);
                g_buf[GI] = l;
            };""",
    "tertiary uint": """ struct STertiaryUintOp {
                uint input1;
                uint input2;
                uint input3;
                uint output;
            };
            RWStructuredBuffer<STertiaryUintOp> g_buf : register(u0);
            [numthreads(8,8,1)]
            void main(uint GI : SV_GroupIndex) {
                STertiaryUintOp l = g_buf[GI];
                l.output = %s(l.input1, l.input2, l.input3);
                g_buf[GI] = l;
            };""",
    "tertiary uint16_t": """ struct STertiaryUint16Op {
                uint16_t input1;
                uint16_t input2;
                uint16_t input3;
                uint16_t output;
            };
            RWStructuredBuffer<STertiaryUint16Op> g_buf : register(u0);
            [numthreads(8,8,1)]
            void main(uint GI : SV_GroupIndex) {
                STertiaryUint16Op l = g_buf[GI];
                l.output = %s(l.input1, l.input2, l.input3);
                g_buf[GI] = l;
            };""",
    "tertiary float": """ struct STertiaryFloatOp {
                float input1;
                float input2;
                float input3;
                float output;
            };
            RWStructuredBuffer<STertiaryFloatOp> g_buf : register(u0);
            [numthreads(8,8,1)]
            void main(uint GI : SV_GroupIndex) {
                STertiaryFloatOp l = g_buf[GI];
                l.output = %s(l.input1, l.input2, l.input3);
                g_buf[GI] = l;
            };""",
    "tertiary half": """ struct STertiaryHalfOp {
                half input1;
                half input2;
                half input3;
                half output;
            };
            RWStructuredBuffer<STertiaryHalfOp> g_buf : register(u0);
            [numthreads(8,8,1)]
            void main(uint GI : SV_GroupIndex) {
                STertiaryHalfOp l = g_buf[GI];
                l.output = %s(l.input1, l.input2, l.input3);
                g_buf[GI] = l;
            };""",
    "wave op int": """ struct PerThreadData {
                        uint firstLaneId;
                        uint laneIndex;
                        int mask;
                        int input;
                        int output;
                    };
                    RWStructuredBuffer<PerThreadData> g_sb : register(u0);
                    [numthreads(8,12,1)]
                    void main(uint GI : SV_GroupIndex) {
                        PerThreadData pts = g_sb[GI];
                        pts.firstLaneId = WaveReadLaneFirst(GI);
                        pts.laneIndex = WaveGetLaneIndex();
                        if (pts.mask != 0) {
                            pts.output = %s(pts.input);
                        }
                        else {
                            pts.output = %s(pts.input);
                        }
                        g_sb[GI] = pts;
                    };""",
    "wave op uint": """ struct PerThreadData {
                        uint firstLaneId;
                        uint laneIndex;
                        int mask;
                        uint input;
                        uint output;
                    };
                    RWStructuredBuffer<PerThreadData> g_sb : register(u0);
                    [numthreads(8,12,1)]
                    void main(uint GI : SV_GroupIndex) {
                        PerThreadData pts = g_sb[GI];
                        pts.firstLaneId = WaveReadLaneFirst(GI);
                        pts.laneIndex = WaveGetLaneIndex();
                        if (pts.mask != 0) {
                            pts.output = %s(pts.input);
                        }
                        else {
                            pts.output = %s(pts.input);
                        }
                        g_sb[GI] = pts;
                    };""",
    "wave op int count": """ struct PerThreadData {
                        uint firstLaneId;
                        uint laneIndex;
                        int mask;
                        int input;
                        int output;
                    };
                    RWStructuredBuffer<PerThreadData> g_sb : register(u0);
                    [numthreads(8,12,1)]
                    void main(uint GI : SV_GroupIndex) {
                        PerThreadData pts = g_sb[GI];
                        pts.firstLaneId = WaveReadLaneFirst(GI);
                        pts.laneIndex = WaveGetLaneIndex();
                        if (pts.mask != 0) {
                            pts.output = %s(pts.input > 3);
                        }
                        else {
                            pts.output = %s(pts.input > 3);
                        }
                        g_sb[GI] = pts;
                    };""",
    "wave op multi prefix int": """ struct ThreadData {
                    uint key;
                    uint firstLaneId;
                    uint laneId;
                    uint mask;
                    int value;
                    int result;
                };
                RWStructuredBuffer<ThreadData> g_buffer : register(u0);
                [numthreads(8, 12, 1)]
                void main(uint id : SV_GroupIndex) {
                    ThreadData data = g_buffer[id];
                    data.firstLaneId = WaveReadLaneFirst(id);
                    data.laneId = WaveGetLaneIndex();
                    if (data.mask != 0) {
                        uint4 mask = WaveMatch(data.key);
                        data.result = %s(data.value, mask);
                    } else {
                        uint4 mask = WaveMatch(data.key);
                        data.result = %s(data.value, mask);
                    }
                    g_buffer[id] = data;
                }""",
    "wave op multi prefix uint": """ struct ThreadData {
                    uint key;
                    uint firstLaneId;
                    uint laneId;
                    uint mask;
                    uint value;
                    uint result;
                };
                RWStructuredBuffer<ThreadData> g_buffer : register(u0);
                [numthreads(8, 12, 1)]
                void main(uint id : SV_GroupIndex) {
                    ThreadData data = g_buffer[id];
                    data.firstLaneId = WaveReadLaneFirst(id);
                    data.laneId = WaveGetLaneIndex();
                    if (data.mask != 0) {
                        uint4 mask = WaveMatch(data.key);
                        data.result = %s(data.value, mask);
                    } else {
                        uint4 mask = WaveMatch(data.key);
                        data.result = %s(data.value, mask);
                    }
                    g_buffer[id] = data;
                }""",
}


def get_shader_text(op_type, op_call):
    assert op_type in g_shader_texts
    if op_type.startswith("wave op"):
        return g_shader_texts[op_type] % (op_call, op_call)
    return g_shader_texts[op_type] % (op_call)


g_denorm_tests = [
    "FAddDenormAny",
    "FAddDenormFTZ",
    "FAddDenormPreserve",
    "FSubDenormAny",
    "FSubDenormFTZ",
    "FSubDenormPreserve",
    "FMulDenormAny",
    "FMulDenormFTZ",
    "FMulDenormPreserve",
    "FDivDenormAny",
    "FDivDenormFTZ",
    "FDivDenormPreserve",
    "FMadDenormAny",
    "FMadDenormFTZ",
    "FMadDenormPreserve",
    "FAbsDenormAny",
    "FAbsDenormFTZ",
    "FAbsDenormPreserve",
    "FMinDenormAny",
    "FMinDenormFTZ",
    "FMinDenormPreserve",
    "FMaxDenormAny",
    "FMaxDenormFTZ",
    "FMaxDenormPreserve",
]


# This is a collection of test case for driver tests per instruction
# Warning: For test cases, when you want to pass in signed 32-bit integer,
# make sure to pass in negative numbers with decimal values instead of hexadecimal representation.
# For some reason, TAEF is not handling them properly.
# For half values, hex is preferable since the test framework will read string as float values
# and convert them to float16, possibly losing precision. The test will read hex values as it is.
def add_test_cases():
    nan = float("nan")
    p_inf = float("inf")
    n_inf = float("-inf")
    p_denorm = float("1e-38")
    n_denorm = float("-1e-38")
    # Unary Float
    add_test_case_float_half(
        "Sin",
        ["Sin"],
        "Epsilon",
        0.0008,
        [["NaN", "-Inf", "-denorm", "-0", "0", "denorm", "Inf", "-314.16", "314.16"]],
        [["NaN", "NaN", "-0", "-0", "0", "0", "NaN", "-0.0007346401", "0.0007346401"]],
        "unary float",
        "sin",
        half_validation_tolerance=0.003,
        half_inputs=[
            [
                "NaN",
                "-Inf",
                "-denorm",
                "-0",
                "0",
                "denorm",
                "Inf",
                "0.6279297",
                "1.255859",
                "1.884766",
                "2.511719",
                "3.140625",
                "3.769531",
                "4.398438",
                "5.023438",
                "5.652344",
                "6.281250",
            ]
        ],
        half_outputs=[
            [
                "NaN",
                "NaN",
                "-0",
                "-0",
                "0",
                "0",
                "NaN",
                "0.58747065",
                "0.95081574",
                "0.95111507",
                "0.58904284",
                "0.00096773",
                "-0.58747751",
                "-0.95112079",
                "-0.95201313",
                "-0.58982444",
                "-0.00193545",
            ]
        ],
    )
    add_test_case_float_half(
        "Cos",
        ["Cos"],
        "Epsilon",
        0.0008,
        [["NaN", "-Inf", "-denorm", "-0", "0", "denorm", "Inf", "-314.16", "314.16"]],
        [
            [
                "NaN",
                "NaN",
                "1.0",
                "1.0",
                "1.0",
                "1.0",
                "NaN",
                "0.99999973015",
                "0.99999973015",
            ]
        ],
        "unary float",
        "cos",
        half_validation_tolerance=0.003,
        half_inputs=[
            [
                "NaN",
                "-Inf",
                "-denorm",
                "-0",
                "0",
                "denorm",
                "Inf",
                "0.6279297",
                "1.255859",
                "1.884766",
                "2.511719",
                "3.140625",
                "3.769531",
                "4.398438",
                "5.023438",
                "5.652344",
                "6.281250",
            ]
        ],
        half_outputs=[
            [
                "NaN",
                "NaN",
                "1.0",
                "1.0",
                "1.0",
                "1.0",
                "NaN",
                "0.80924553",
                "0.30975693",
                "-0.30883664",
                "-0.80810183",
                "-0.99999952",
                "-0.80924052",
                "-0.30881903",
                "0.30605716",
                "0.80753154",
                "0.99999809",
            ]
        ],
    )
    add_test_case_float_half(
        "Tan",
        ["Tan"],
        "Epsilon",
        0.0008,
        [["NaN", "-Inf", "-denorm", "-0", "0", "denorm", "Inf", "-314.16", "314.16"]],
        [["NaN", "NaN", "-0.0", "-0.0", "0.0", "0.0", "NaN", "-0.000735", "0.000735"]],
        "unary float",
        "tan",
        half_validation_tolerance=0.016,
        half_inputs=[
            [
                "NaN",
                "-Inf",
                "-denorm",
                "-0",
                "0",
                "denorm",
                "Inf",
                "0.6279297",
                "1.255859",
                "1.884766",
                "2.511719",
                "3.140625",
                "3.769531",
                "4.398438",
                "5.652344",
                "6.281250",
            ]
        ],
        half_outputs=[
            [
                "NaN",
                "NaN",
                "-0",
                "-0",
                "0",
                "0",
                "NaN",
                "0.72594857",
                "3.06955433",
                "-3.07967043",
                "-0.72892153",
                "-0.00096773",
                "0.72596157",
                "3.07986474",
                "-0.7304042",
                "-0.00193546",
            ]
        ],
    )
    add_test_case_float_half(
        "Hcos",
        ["Hcos"],
        "Epsilon",
        0.0008,
        [["NaN", "-Inf", "-denorm", "-0", "0", "denorm", "Inf", "1", "-1"]],
        [["NaN", "Inf", "1.0", "1.0", "1.0", "1.0", "Inf", "1.543081", "1.543081"]],
        "unary float",
        "cosh",
        half_validation_type="ulp",
        half_validation_tolerance=2,
    )
    add_test_case_float_half(
        "Hsin",
        ["Hsin"],
        "Epsilon",
        0.0008,
        [["NaN", "-Inf", "-denorm", "-0", "0", "denorm", "Inf", "1", "-1"]],
        [["NaN", "-Inf", "0.0", "0.0", "0.0", "0.0", "Inf", "1.175201", "-1.175201"]],
        "unary float",
        "sinh",
    )
    add_test_case_float_half(
        "Htan",
        ["Htan"],
        "Epsilon",
        0.0008,
        [["NaN", "-Inf", "-denorm", "-0", "0", "denorm", "Inf", "1", "-1"]],
        [["NaN", "-1", "-0.0", "-0.0", "0.0", "0.0", "1", "0.761594", "-0.761594"]],
        "unary float",
        "tanh",
        warp_version=16202,
    )
    add_test_case_float_half(
        "Acos",
        ["Acos"],
        "Epsilon",
        0.0008,
        [
            [
                "NaN",
                "-Inf",
                "-denorm",
                "-0",
                "0",
                "denorm",
                "Inf",
                "1",
                "-1",
                "1.5",
                "-1.5",
            ]
        ],
        [
            [
                "NaN",
                "NaN",
                "1.570796",
                "1.570796",
                "1.570796",
                "1.570796",
                "NaN",
                "0",
                "3.1415926",
                "NaN",
                "NaN",
            ]
        ],
        "unary float",
        "acos",
    )
    add_test_case_float_half(
        "Asin",
        ["Asin"],
        "Epsilon",
        0.0008,
        [
            [
                "NaN",
                "-Inf",
                "-denorm",
                "-0",
                "0",
                "denorm",
                "Inf",
                "1",
                "-1",
                "1.5",
                "-1.5",
            ]
        ],
        [
            [
                "NaN",
                "NaN",
                "0.0",
                "0.0",
                "0.0",
                "0.0",
                "NaN",
                "1.570796",
                "-1.570796",
                "NaN",
                "NaN",
            ]
        ],
        "unary float",
        "asin",
    )
    add_test_case_float_half(
        "Atan",
        ["Atan"],
        "Epsilon",
        0.0008,
        [["NaN", "-Inf", "-denorm", "-0", "0", "denorm", "Inf", "1", "-1"]],
        [
            [
                "NaN",
                "-1.570796",
                "0.0",
                "0.0",
                "0.0",
                "0.0",
                "1.570796",
                "0.785398163",
                "-0.785398163",
            ]
        ],
        "unary float",
        "atan",
        warp_version=16202,
    )
    add_test_case_float_half(
        "Exp",
        ["Exp"],
        "Relative",
        21,
        [["NaN", "-Inf", "-denorm", "-0", "0", "denorm", "Inf", "-1", "10"]],
        [["NaN", "0", "1", "1", "1", "1", "Inf", "0.367879441", "22026.46579"]],
        "unary float",
        "exp",
    )
    add_test_case_float_half(
        "Frc",
        ["Frc"],
        "Epsilon",
        0.0008,
        [
            [
                "NaN",
                "-Inf",
                "-denorm",
                "-0",
                "0",
                "denorm",
                "Inf",
                "-1",
                "2.718280",
                "1000.599976",
                "-7.389",
            ]
        ],
        [
            [
                "NaN",
                "NaN",
                "0",
                "0",
                "0",
                "0",
                "NaN",
                "0",
                "0.718280",
                "0.599976",
                "0.611",
            ]
        ],
        "unary float",
        "frac",
        half_inputs=[
            [
                "NaN",
                "-Inf",
                "0x03FF",
                "-0",
                "0",
                "Inf",
                "-1",
                "2.719",
                "1000.5",
                "0xC764",
            ]
        ],
        half_outputs=[
            ["NaN", "NaN", "0x03FF", "0", "0", "NaN", "0", "0.719", "0.5", "0x38E1"]
        ],
    )
    add_test_case_float_half(
        "Log",
        ["Log"],
        "Relative",
        21,
        [
            [
                "NaN",
                "-Inf",
                "-denorm",
                "-0",
                "0",
                "denorm",
                "Inf",
                "-1",
                "2.718281828",
                "7.389056",
                "100",
            ]
        ],
        [
            [
                "NaN",
                "NaN",
                "-Inf",
                "-Inf",
                "-Inf",
                "-Inf",
                "Inf",
                "NaN",
                "1.0",
                "1.99999998",
                "4.6051701",
            ]
        ],
        "unary float",
        "log",
        half_inputs=[
            [
                "NaN",
                "-Inf",
                "-denorm",
                "-0",
                "0",
                "denorm",
                "Inf",
                "-1",
                "2.719",
                "7.39",
                "100",
            ]
        ],
        half_outputs=[
            [
                "NaN",
                "NaN",
                "-Inf",
                "-Inf",
                "-Inf",
                "-Inf",
                "Inf",
                "NaN",
                "1.0",
                "2",
                "4.605",
            ]
        ],
    )
    add_test_case_float_half(
        "Sqrt",
        ["Sqrt"],
        "ulp",
        1,
        [
            [
                "NaN",
                "-Inf",
                "-denorm",
                "-0",
                "0",
                "denorm",
                "Inf",
                "-1",
                "2",
                "16.0",
                "256.0",
            ]
        ],
        [
            [
                "NaN",
                "NaN",
                "-0",
                "-0",
                "0",
                "0",
                "Inf",
                "NaN",
                "1.41421356237",
                "4.0",
                "16.0",
            ]
        ],
        "unary float",
        "sqrt",
        half_inputs=[
            [
                "NaN",
                "-Inf",
                "-denorm",
                "-0",
                "0",
                "0x03FF",
                "Inf",
                "-1",
                "2",
                "16.0",
                "256.0",
            ]
        ],
        half_outputs=[
            [
                "NaN",
                "NaN",
                "NaN",
                "-0",
                "0",
                "0x1FFF",
                "Inf",
                "NaN",
                "1.41421",
                "4.0",
                "16.0",
            ]
        ],
    )
    add_test_case_float_half(
        "Rsqrt",
        ["Rsqrt"],
        "ulp",
        1,
        [
            [
                "NaN",
                "-Inf",
                "-denorm",
                "-0",
                "0",
                "denorm",
                "Inf",
                "-1",
                "16.0",
                "256.0",
                "65536.0",
            ]
        ],
        [
            [
                "NaN",
                "NaN",
                "-Inf",
                "-Inf",
                "Inf",
                "Inf",
                "0",
                "NaN",
                "0.25",
                "0.0625",
                "0.00390625",
            ]
        ],
        "unary float",
        "rsqrt",
        half_inputs=[
            [
                "NaN",
                "-Inf",
                "-denorm",
                "-0",
                "0",
                "0x03FF",
                "Inf",
                "-1",
                "16.0",
                "256.0",
                "0x7bff",
            ]
        ],
        half_outputs=[
            [
                "NaN",
                "NaN",
                "NaN",
                "-Inf",
                "Inf",
                "0x5801",
                "0",
                "NaN",
                "0.25",
                "0.0625",
                "0x1C00",
            ]
        ],
    )
    add_test_case_float_half(
        "Round_ne",
        ["Round_ne"],
        "Epsilon",
        0,
        [
            [
                "NaN",
                "-Inf",
                "-denorm",
                "-0",
                "0",
                "denorm",
                "Inf",
                "10.0",
                "10.4",
                "10.5",
                "10.6",
                "11.5",
                "-10.0",
                "-10.4",
                "-10.5",
                "-10.6",
            ]
        ],
        [
            [
                "NaN",
                "-Inf",
                "-0",
                "-0",
                "0",
                "0",
                "Inf",
                "10.0",
                "10.0",
                "10.0",
                "11.0",
                "12.0",
                "-10.0",
                "-10.0",
                "-10.0",
                "-11.0",
            ]
        ],
        "unary float",
        "round",
    )
    add_test_case_float_half(
        "Round_ni",
        ["Round_ni"],
        "Epsilon",
        0,
        [
            [
                "NaN",
                "-Inf",
                "-denorm",
                "-0",
                "0",
                "denorm",
                "Inf",
                "10.0",
                "10.4",
                "10.5",
                "10.6",
                "-10.0",
                "-10.4",
                "-10.5",
                "-10.6",
            ]
        ],
        [
            [
                "NaN",
                "-Inf",
                "-0",
                "-0",
                "0",
                "0",
                "Inf",
                "10.0",
                "10.0",
                "10.0",
                "10.0",
                "-10.0",
                "-11.0",
                "-11.0",
                "-11.0",
            ]
        ],
        "unary float",
        "floor",
        half_inputs=[
            [
                "NaN",
                "-Inf",
                "-denorm",
                "-0",
                "0",
                "denorm",
                "Inf",
                "10.0",
                "10.4",
                "10.5",
                "10.6",
                "-10.0",
                "-10.4",
                "-10.5",
                "-10.6",
            ]
        ],
        half_outputs=[
            [
                "NaN",
                "-Inf",
                "-1",
                "-0",
                "0",
                "0",
                "Inf",
                "10.0",
                "10.0",
                "10.0",
                "10.0",
                "-10.0",
                "-11.0",
                "-11.0",
                "-11.0",
            ]
        ],
    )
    add_test_case_float_half(
        "Round_pi",
        ["Round_pi"],
        "Epsilon",
        0,
        [
            [
                "NaN",
                "-Inf",
                "-denorm",
                "-0",
                "0",
                "denorm",
                "Inf",
                "10.0",
                "10.4",
                "10.5",
                "10.6",
                "-10.0",
                "-10.4",
                "-10.5",
                "-10.6",
            ]
        ],
        [
            [
                "NaN",
                "-Inf",
                "-0",
                "-0",
                "0",
                "0",
                "Inf",
                "10.0",
                "11.0",
                "11.0",
                "11.0",
                "-10.0",
                "-10.0",
                "-10.0",
                "-10.0",
            ]
        ],
        "unary float",
        "ceil",
        half_inputs=[
            [
                "NaN",
                "-Inf",
                "-denorm",
                "-0",
                "0",
                "denorm",
                "Inf",
                "10.0",
                "10.4",
                "10.5",
                "10.6",
                "-10.0",
                "-10.4",
                "-10.5",
                "-10.6",
            ]
        ],
        half_outputs=[
            [
                "NaN",
                "-Inf",
                "-0",
                "-0",
                "0",
                "1",
                "Inf",
                "10.0",
                "11.0",
                "11.0",
                "11.0",
                "-10.0",
                "-10.0",
                "-10.0",
                "-10.0",
            ]
        ],
    )
    add_test_case_float_half(
        "Round_z",
        ["Round_z"],
        "Epsilon",
        0,
        [
            [
                "NaN",
                "-Inf",
                "-denorm",
                "-0",
                "0",
                "denorm",
                "Inf",
                "10.0",
                "10.4",
                "10.5",
                "10.6",
                "-10.0",
                "-10.4",
                "-10.5",
                "-10.6",
            ]
        ],
        [
            [
                "NaN",
                "-Inf",
                "-0",
                "-0",
                "0",
                "0",
                "Inf",
                "10.0",
                "10.0",
                "10.0",
                "10.0",
                "-10.0",
                "-10.0",
                "-10.0",
                "-10.0",
            ]
        ],
        "unary float",
        "trunc",
    )
    add_test_case_float_half(
        "IsNaN",
        ["IsNaN"],
        "Epsilon",
        0,
        [["NaN", "-Inf", "-denorm", "-0", "0", "denorm", "Inf", "1.0", "-1.0"]],
        [["1", "0", "0", "0", "0", "0", "0", "0", "0"]],
        "unary float bool",
        "isnan",
    )
    add_test_case_float_half(
        "IsInf",
        ["IsInf"],
        "Epsilon",
        0,
        [["NaN", "-Inf", "-denorm", "-0", "0", "denorm", "Inf", "1.0", "-1.0"]],
        [["0", "1", "0", "0", "0", "0", "1", "0", "0"]],
        "unary float bool",
        "isinf",
    )
    add_test_case_float_half(
        "IsFinite",
        ["IsFinite"],
        "Epsilon",
        0,
        [["NaN", "-Inf", "-denorm", "-0", "0", "denorm", "Inf", "1.0", "-1.0"]],
        [["0", "0", "1", "1", "1", "1", "0", "1", "1"]],
        "unary float bool",
        "isfinite",
        warp_version=16202,
    )
    add_test_case_float_half(
        "FAbs",
        ["FAbs"],
        "Epsilon",
        0,
        [["NaN", "-Inf", "-denorm", "-0", "0", "denorm", "Inf", "1.0", "-1.0"]],
        [["NaN", "Inf", "denorm", "0", "0", "denorm", "Inf", "1", "1"]],
        "unary float",
        "abs",
    )
    # Binary Float
    add_test_case(
        "FMin",
        ["FMin", "FMax"],
        "epsilon",
        0,
        [
            [
                "-inf",
                "-inf",
                "-inf",
                "-inf",
                "inf",
                "inf",
                "inf",
                "inf",
                "NaN",
                "NaN",
                "NaN",
                "NaN",
                "1.0",
                "1.0",
                "-1.0",
                "-1.0",
                "1.0",
            ],
            [
                "-inf",
                "inf",
                "1.0",
                "NaN",
                "-inf",
                "inf",
                "1.0",
                "NaN",
                "-inf",
                "inf",
                "1.0",
                "NaN",
                "-inf",
                "inf",
                "1.0",
                "NaN",
                "-1.0",
            ],
        ],
        [
            [
                "-inf",
                "-inf",
                "-inf",
                "-inf",
                "-inf",
                "inf",
                "1.0",
                "inf",
                "-inf",
                "inf",
                "1.0",
                "NaN",
                "-inf",
                "1.0",
                "-1.0",
                "-1.0",
                "-1.0",
            ],
            [
                "-inf",
                "inf",
                "1.0",
                "-inf",
                "inf",
                "inf",
                "inf",
                "inf",
                "-inf",
                "inf",
                "1.0",
                "NaN",
                "1.0",
                "inf",
                "1.0",
                "-1.0",
                "1.0",
            ],
        ],
        "cs_6_0",
        """ struct SBinaryFPOp {
                float input1;
                float input2;
                float output1;
                float output2;
            };
            RWStructuredBuffer<SBinaryFPOp> g_buf : register(u0);
            [numthreads(8,8,1)]
            void main(uint GI : SV_GroupIndex) {
                SBinaryFPOp l = g_buf[GI];
                l.output1 = min(l.input1, l.input2);
                l.output2 = max(l.input1, l.input2);
                g_buf[GI] = l;
            };""",
    )
    add_test_case(
        "FMinHalf",
        ["FMin", "FMax"],
        "epsilon",
        0,
        [
            [
                "-inf",
                "-inf",
                "-inf",
                "-inf",
                "inf",
                "inf",
                "inf",
                "inf",
                "NaN",
                "NaN",
                "NaN",
                "NaN",
                "1.0",
                "1.0",
                "-1.0",
                "-1.0",
                "1.0",
            ],
            [
                "-inf",
                "inf",
                "1.0",
                "NaN",
                "-inf",
                "inf",
                "1.0",
                "NaN",
                "-inf",
                "inf",
                "1.0",
                "NaN",
                "-inf",
                "inf",
                "1.0",
                "NaN",
                "-1.0",
            ],
        ],
        [
            [
                "-inf",
                "-inf",
                "-inf",
                "-inf",
                "-inf",
                "inf",
                "1.0",
                "inf",
                "-inf",
                "inf",
                "1.0",
                "NaN",
                "-inf",
                "1.0",
                "-1.0",
                "-1.0",
                "-1.0",
            ],
            [
                "-inf",
                "inf",
                "1.0",
                "-inf",
                "inf",
                "inf",
                "inf",
                "inf",
                "-inf",
                "inf",
                "1.0",
                "NaN",
                "1.0",
                "inf",
                "1.0",
                "-1.0",
                "1.0",
            ],
        ],
        "cs_6_2",
        """ struct SBinaryHalfOp {
                half input1;
                half input2;
                half output1;
                half output2;
            };
            RWStructuredBuffer<SBinaryHalfOp> g_buf : register(u0);
            [numthreads(8,8,1)]
            void main(uint GI : SV_GroupIndex) {
                SBinaryHalfOp l = g_buf[GI];
                l.output1 = min(l.input1, l.input2);
                l.output2 = max(l.input1, l.input2);
                g_buf[GI] = l;
            };""",
        shader_arguments="-enable-16bit-types",
    )
    add_test_case_float_half(
        "FAdd",
        ["FAdd"],
        "ulp",
        1,
        [
            ["-1.0", "1.0", "32.5", "1.0000001000"],
            ["4", "5.5", "334.7", "0.5000001000"],
        ],
        [["3.0", "6.5", "367.2", "1.5000002000"]],
        "binary float",
        "+",
    )
    add_test_case_float_half(
        "FSub",
        ["FSub"],
        "ulp",
        1,
        [
            ["-1.0", "5.5", "32.5", "1.0000001000"],
            ["4", "1.25", "334.7", "0.5000001000"],
        ],
        [["-5", "4.25", "-302.2", "0.5000"]],
        "binary float",
        "-",
    )
    add_test_case_float_half(
        "FMul",
        ["FMul"],
        "ulp",
        1,
        [["-1.0", "5.5", "1.0000001"], ["4", "1.25", "2.0"]],
        [["-4.0", "6.875", "2.0000002"]],
        "binary float",
        "*",
    )
    add_test_case_float_half(
        "FDiv",
        ["FDiv"],
        "ulp",
        1,
        [["-1.0", "5.5", "1.0000001"], ["4", "1.25", "2.0"]],
        [["-0.25", "4.4", "0.50000006"]],
        "binary float",
        "/",
    )

    # Denorm Binary Float
    add_test_case_denorm(
        "FAddDenorm",
        ["FAdd"],
        "ulp",
        1,
        [
            ["0x007E0000", "0x00200000", "0x007E0000", "0x007E0000"],
            ["0x007E0000", "0x00200000", "0x807E0000", "0x800E0000"],
        ],
        [["0", "0", "0", "0"]],
        [["0x00FC0000", "0x00400000", "0", "0x00700000"]],
        "cs_6_2",
        get_shader_text("binary float", "+"),
    )
    add_test_case_denorm(
        "FSubDenorm",
        ["FSub"],
        "ulp",
        1,
        [
            ["0x007E0000", "0x007F0000", "0x00FF0000", "0x007A0000"],
            ["0x007E0000", "0x807F0000", "0x00800000", "0"],
        ],
        [["0x0", "0", "0", "0"]],
        [["0x0", "0x00FE0000", "0x007F0000", "0x007A0000"]],
        "cs_6_2",
        get_shader_text("binary float", "-"),
    )
    add_test_case_denorm(
        "FDivDenorm",
        ["FDiv"],
        "ulp",
        1,
        [
            ["0x007F0000", "0x807F0000", "0x20000000", "0x00800000"],
            ["1", "4", "0x607F0000", "0x40000000"],
        ],
        [["0", "0", "0", "0"]],
        [["0x007F0000", "0x801FC000", "0x00101010", "0x00400000"]],
        "cs_6_2",
        get_shader_text("binary float", "/"),
    )
    add_test_case_denorm(
        "FMulDenorm",
        ["FMul"],
        "ulp",
        1,
        [
            ["0x00000300", "0x007F0000", "0x007F0000", "0x001E0000", "0x00000300"],
            ["128", "1", "0x007F0000", "20", "0x78000000"],
        ],
        [["0", "0", "0", "0", "0"]],
        [["0x00018000", "0x007F0000", "0", "0x01960000", "0x32400000"]],
        "cs_6_2",
        get_shader_text("binary float", "*"),
    )
    # Tertiary Float
    add_test_case_float_half(
        "FMad",
        ["FMad"],
        "ulp",
        1,
        [
            [
                "NaN",
                "-Inf",
                "-denorm",
                "-0",
                "0",
                "denorm",
                "Inf",
                "1.0",
                "-1.0",
                "0",
                "1",
                "1.5",
            ],
            [
                "NaN",
                "-Inf",
                "-denorm",
                "-0",
                "0",
                "denorm",
                "Inf",
                "1.0",
                "-1.0",
                "0",
                "1",
                "10",
            ],
            [
                "NaN",
                "-Inf",
                "-denorm",
                "-0",
                "0",
                "denorm",
                "Inf",
                "1.0",
                "-1.0",
                "1",
                "0",
                "-5.5",
            ],
        ],
        [["NaN", "NaN", "0", "0", "0", "0", "Inf", "2", "0", "1", "1", "9.5"]],
        "tertiary float",
        "mad",
        half_inputs=[
            ["NaN", "-Inf", "0x03FF", "-0", "0", "Inf", "1.0", "-1.0", "0", "1", "1.5"],
            ["NaN", "-Inf", "1", "-0", "0", "Inf", "1.0", "-1.0", "0", "1", "10"],
            [
                "NaN",
                "-Inf",
                "0x03FF",
                "-0",
                "0",
                "Inf",
                "1.0",
                "-1.0",
                "1",
                "0",
                "-5.5",
            ],
        ],
        half_outputs=[
            ["NaN", "NaN", "0x07FE", "0", "0", "Inf", "2", "0", "1", "1", "9.5"]
        ],
    )
    # Denorm Tertiary Float
    add_test_case_denorm(
        "FMadDenorm",
        ["FMad"],
        "ulp",
        1,
        [
            ["0x80780000", "0x80780000", "0x00780000"],
            ["1", "2", "2"],
            ["0x80780000", "0x00800000", "0x00800000"],
        ],
        [["0", "0x00800000", "0x00800000"]],
        [["0x80F00000", "0x80700000", "0x01380000"]],
        "cs_6_2",
        get_shader_text("tertiary float", "mad"),
    )

    # Unary Int
    int8_min, int8_max = "-128", "127"
    int16_min, int16_max = "-32768", "32767"
    int32_min, int32_max = "-2147483648", "2147483647"
    uint16_max = "65535"
    uint32_max = "4294967295"
    add_test_case_int(
        "Bfrev",
        ["Bfrev"],
        "Epsilon",
        0,
        [[int32_min, "-65536", "-8", "-1", "0", "1", "8", "65536", int32_max]],
        [["1", "65535", "536870911", "-1", "0", int32_min, "268435456", "32768", "-2"]],
        "unary int",
        "reversebits",
        input_16=[[int16_min, "-256", "-8", "-1", "0", "1", "8", "256", int16_max]],
        output_16=[["1", "255", "8191", "-1", "0", int16_min, "4096", "128", "-2"]],
    )
    # firstbit_shi (s for signed) returns the
    # first 0 from the MSB if the number is negative,
    # else the first 1 from the MSB.
    # all the variants of the instruction return ~0 if no match was found
    add_test_case_int(
        "FirstbitSHi",
        ["FirstbitSHi"],
        "Epsilon",
        0,
        [[int32_min, "-65536", "-8", "-1", "0", "1", "8", "65536", int32_max]],
        [["30", "15", "2", "-1", "-1", "0", "3", "16", "30"]],
        "unary int",
        "firstbithigh",
        input_16=[[int16_min, "-256", "-8", "-1", "0", "1", "8", "256", int16_max]],
        output_16=[["14", "7", "2", "-1", "-1", "0", "3", "8", "14"]],
    )
    add_test_case_int(
        "FirstbitLo",
        ["FirstbitLo"],
        "Epsilon",
        0,
        [[int32_min, "-65536", "-8", "-1", "0", "1", "8", "65536", int32_max]],
        [["31", "16", "3", "0", "-1", "0", "3", "16", "0"]],
        "unary int",
        "firstbitlow",
        input_16=[[int16_min, "-256", "-8", "-1", "0", "1", "8", "256", int16_max]],
        output_16=[["15", "8", "3", "0", "-1", "0", "3", "8", "0"]],
    )
    # TODO: there is a known bug in countbits when passing in immediate values.
    # Fix this later
    add_test_case(
        "Countbits",
        ["Countbits"],
        "Epsilon",
        0,
        [[int32_min, "-65536", "-8", "-1", "0", "1", "8", "65536", int32_max]],
        [["1", "16", "29", "32", "0", "1", "1", "1", "31"]],
        "cs_6_0",
        get_shader_text("unary int", "countbits"),
    )
    # Unary uint
    add_test_case_int(
        "FirstbitHi",
        ["FirstbitHi"],
        "Epsilon",
        0,
        [["0", "1", "8", "65536", int32_max, uint32_max]],
        [["-1", "0", "3", "16", "30", "31"]],
        "unary uint",
        "firstbithigh",
        input_16=[["0", "1", "8", uint16_max]],
        output_16=[["-1", "0", "3", "15"]],
    )
    # Binary Int
    add_test_case_int(
        "IAdd",
        ["Add"],
        "Epsilon",
        0,
        [
            [int32_min, "-10", "0", "0", "10", int32_max, "486"],
            ["0", "10", "-10", "10", "10", "0", "54238"],
        ],
        [[int32_min, "0", "-10", "10", "20", int32_max, "54724"]],
        "binary int",
        "+",
        input_16=[
            [int16_min, "-10", "0", "0", "10", int16_max],
            ["0", "10", "-3114", "272", "15", "0"],
        ],
        output_16=[[int16_min, "0", "-3114", "272", "25", int16_max]],
    )
    add_test_case_int(
        "ISub",
        ["Sub"],
        "Epsilon",
        0,
        [
            [int32_min, "-10", "0", "0", "10", int32_max, "486"],
            ["0", "10", "-10", "10", "10", "0", "54238"],
        ],
        [[int32_min, "-20", "10", "-10", "0", int32_max, "-53752"]],
        "binary int",
        "-",
        input_16=[
            [int16_min, "-10", "0", "0", "10", int16_max],
            ["0", "10", "-3114", "272", "15", "0"],
        ],
        output_16=[[int16_min, "-20", "3114", "-272", "-5", int16_max]],
    )
    add_test_case_int(
        "IMax",
        ["IMax"],
        "Epsilon",
        0,
        [
            [int32_min, "-10", "0", "0", "10", int32_max],
            ["0", "10", "-10", "10", "10", "0"],
        ],
        [["0", "10", "0", "10", "10", int32_max]],
        "binary int call",
        "max",
        input_16=[
            [int16_min, "-10", "0", "0", "10", int16_max],
            ["0", "10", "-3114", "272", "15", "0"],
        ],
        output_16=[["0", "10", "0", "272", "15", int16_max]],
    )
    add_test_case_int(
        "IMin",
        ["IMin"],
        "Epsilon",
        0,
        [
            [int32_min, "-10", "0", "0", "10", int32_max],
            ["0", "10", "-10", "10", "10", "0"],
        ],
        [[int32_min, "-10", "-10", "0", "10", "0"]],
        "binary int call",
        "min",
        input_16=[
            [int16_min, "-10", "0", "0", "10", int16_max],
            ["0", "10", "-3114", "272", "15", "0"],
        ],
        output_16=[[int16_min, "-10", "-3114", "0", "10", "0"]],
    )
    add_test_case_int(
        "IMul",
        ["Mul"],
        "Epsilon",
        0,
        [
            [int32_min, "-10", "-1", "0", "1", "10", "10000", int32_max, int32_max],
            ["-10", "-10", "10", "0", "256", "4", "10001", "0", int32_max],
        ],
        [["0", "100", "-10", "0", "256", "40", "100010000", "0", "1"]],
        "binary int",
        "*",
        input_16=[
            [int16_min, "-10", "-1", "0", "1", "10", int16_max],
            ["-10", "-10", "10", "0", "256", "4", "0"],
        ],
        output_16=[["0", "100", "-10", "0", "256", "40", "0"]],
    )
    add_test_case(
        "IDiv",
        ["SDiv", "SRem"],
        "Epsilon",
        0,
        [
            ["1", "1", "10", "10000", int32_max, int32_max, "-1"],
            ["1", "256", "4", "10001", "2", int32_max, "1"],
        ],
        [
            ["1", "0", "2", "0", "1073741823", "1", "-1"],
            ["0", "1", "2", "10000", "1", "0", "0"],
        ],
        "cs_6_0",
        """ struct SBinaryIntOp {
                int input1;
                int input2;
                int output1;
                int output2;
            };
            RWStructuredBuffer<SBinaryIntOp> g_buf : register(u0);
            [numthreads(8,8,1)]
            void main(uint GI : SV_GroupIndex) {
                SBinaryIntOp l = g_buf[GI];
                l.output1 = l.input1 / l.input2;
                l.output2 = l.input1 % l.input2;
                g_buf[GI] = l;
            };""",
    )
    add_test_case_int(
        "Shl",
        ["Shl"],
        "Epsilon",
        0,
        [
            ["1", "1", "0x1010", "0xa", "-1", "0x12341234", "-1"],
            ["0", "259", "4", "2", "0", "15", "3"],
        ],
        [["0x1", "0x8", "0x10100", "0x28", "-1", "0x091a0000", "-8"]],
        "binary int",
        "<<",
        input_16=[
            ["1", "1", "0x0101", "0xa", "-1", "0x1234", "-1"],
            ["0", "259", "4", "2", "0", "13", "3"],
        ],
        output_16=[["0x1", "0x8", "0x1010", "0x28", "-1", "0x8000", "-8"]],
    )
    add_test_case_int(
        "LShr",
        ["LShr"],
        "Epsilon",
        0,
        [
            [
                "1",
                "1",
                "0xffff",
                "0x7fffffff",
                "0x70001234",
                "0x12340ab3",
                "0x7fffffff",
            ],
            ["0", "1", "4", "30", "15", "16", "1"],
        ],
        [["1", "0", "0xfff", "1", "0xe000", "0x1234", "0x3fffffff"]],
        "binary int",
        ">>",
        input_16=[["1", "1", "0x7fff", "0x7fff"], ["0", "1", "4", "14"]],
        output_16=[["1", "0", "0x07ff", "1"]],
    )
    add_test_case_int(
        "And",
        ["And"],
        "Epsilon",
        0,
        [
            [
                "0x1",
                "0x01",
                "0x7fff0000",
                "0x33333333",
                "0x137f",
                "0x12345678",
                "0xa341",
                "-1",
            ],
            ["0x1", "0xf0", "0x0000ffff", "0x22222222", "0xec80", "-1", "0x3471", "-1"],
        ],
        [["0x1", "0x00", "0x0", "0x22222222", "0x0", "0x12345678", "0x2041", "-1"]],
        "binary int",
        "&",
        input_16=[
            ["0x1", "0x01", "0x7fff", "0x3333", "0x137f", "0x1234", "0xa341", "-1"],
            ["0x1", "0xf0", "0x0000", "0x2222", "0xec80", "-1", "0x3471", "-1"],
        ],
        output_16=[["0x1", "0x00", "0x0", "0x2222", "0x0", "0x1234", "0x2041", "-1"]],
    )
    add_test_case_int(
        "Or",
        ["Or"],
        "Epsilon",
        0,
        [
            [
                "0x1",
                "0x01",
                "0x7fff0000",
                "0x11111111",
                "0x137f",
                "0x0",
                "0x12345678",
                "0xa341",
                "-1",
            ],
            [
                "0x1",
                "0xf0",
                "0x0000ffff",
                "0x22222222",
                "0xec80",
                "0x0",
                "0x00000000",
                "0x3471",
                "-1",
            ],
        ],
        [
            [
                "0x1",
                "0xf1",
                "0x7fffffff",
                "0x33333333",
                "0xffff",
                "0x0",
                "0x12345678",
                "0xb771",
                "-1",
            ]
        ],
        "binary int",
        "|",
        input_16=[
            ["0x1", "0x01", "0x7fff", "0x3333", "0x137f", "0x1234", "0xa341", "-1"],
            ["0x1", "0xf0", "0x0000", "0x2222", "0xec80", "0xffff", "0x3471", "-1"],
        ],
        output_16=[
            ["0x1", "0xf1", "0x7fff", "0x3333", "0xffff", "0xffff", "0xb771", "-1"]
        ],
    )
    add_test_case_int(
        "Xor",
        ["Xor"],
        "Epsilon",
        0,
        [
            [
                "0x1",
                "0x01",
                "0x7fff0000",
                "0x11111111",
                "0x137f",
                "0x0",
                "0x12345678",
                "0xa341",
                "-1",
            ],
            [
                "0x1",
                "0xf0",
                "0x0000ffff",
                "0x22222222",
                "0xec80",
                "0x0",
                "0x00000000",
                "0x3471",
                "-1",
            ],
        ],
        [
            [
                "0x0",
                "0xf1",
                "0x7fffffff",
                "0x33333333",
                "0xffff",
                "0x0",
                "0x12345678",
                "0x9730",
                "0x00000000",
            ]
        ],
        "binary int",
        "^",
        input_16=[
            [
                "0x1",
                "0x01",
                "0x7fff",
                "0x1111",
                "0x137f",
                "0x0",
                "0x1234",
                "0xa341",
                "-1",
            ],
            [
                "0x1",
                "0xf0",
                "0x0000",
                "0x2222",
                "0xec80",
                "0x0",
                "0x0000",
                "0x3471",
                "-1",
            ],
        ],
        output_16=[
            [
                "0x0",
                "0xf1",
                "0x7fff",
                "0x3333",
                "0xffff",
                "0x0",
                "0x1234",
                "0x9730",
                "0x0000",
            ]
        ],
    )

    # Binary Uint
    add_test_case_int(
        "UAdd",
        ["Add"],
        "Epsilon",
        0,
        [
            ["2147483648", "4294967285", "0", "0", "10", int32_max, "486"],
            ["0", "10", "0", "10", "10", "0", "54238"],
        ],
        [["2147483648", uint32_max, "0", "10", "20", int32_max, "54724"]],
        "binary uint",
        "+",
        input_16=[
            ["323", "0xfff5", "0", "0", "10", uint16_max, "486"],
            ["0", "10", "0", "10", "10", "0", "334"],
        ],
        output_16=[["323", uint16_max, "0", "10", "20", uint16_max, "820"]],
    )
    add_test_case_int(
        "USub",
        ["Sub"],
        "Epsilon",
        0,
        [
            ["2147483648", uint32_max, "0", "0", "30", int32_max, "54724"],
            ["0", "10", "0", "10", "10", "0", "54238"],
        ],
        [["2147483648", "4294967285", "0", "4294967286", "20", int32_max, "486"]],
        "binary uint",
        "-",
        input_16=[
            ["323", uint16_max, "0", "0", "10", uint16_max, "486"],
            ["0", "10", "0", "10", "10", "0", "334"],
        ],
        output_16=[["323", "0xfff5", "0", "-10", "0", uint16_max, "152"]],
    )
    add_test_case_int(
        "UMax",
        ["UMax"],
        "Epsilon",
        0,
        [
            ["0", "0", "10", "10000", int32_max, uint32_max],
            ["0", "256", "4", "10001", "0", uint32_max],
        ],
        [["0", "256", "10", "10001", int32_max, uint32_max]],
        "binary uint call",
        "max",
        input_16=[
            ["0", "0", "10", "10000", int16_max, uint16_max],
            ["0", "256", "4", "10001", "0", uint16_max],
        ],
        output_16=[["0", "256", "10", "10001", int16_max, uint16_max]],
    )
    add_test_case_int(
        "UMin",
        ["UMin"],
        "Epsilon",
        0,
        [
            ["0", "0", "10", "10000", int32_max, uint32_max],
            ["0", "256", "4", "10001", "0", uint32_max],
        ],
        [["0", "0", "4", "10000", "0", uint32_max]],
        "binary uint call",
        "min",
        input_16=[
            ["0", "0", "10", "10000", int16_max, uint16_max],
            ["0", "256", "4", "10001", "0", uint16_max],
        ],
        output_16=[["0", "0", "4", "10000", "0", uint16_max]],
    )
    add_test_case_int(
        "UMul",
        ["Mul"],
        "Epsilon",
        0,
        [["0", "1", "10", "10000", int32_max], ["0", "256", "4", "10001", "0"]],
        [["0", "256", "40", "100010000", "0"]],
        "binary uint",
        "*",
        input_16=[["0", "0", "10", "100", int16_max], ["0", "256", "4", "101", "0"]],
        output_16=[["0", "0", "40", "10100", "0"]],
    )
    add_test_case(
        "UDiv",
        ["UDiv", "URem"],
        "Epsilon",
        0,
        [
            ["1", "1", "10", "10000", int32_max, int32_max, "0xffffffff"],
            ["0", "256", "4", "10001", "0", int32_max, "1"],
        ],
        [
            ["0xffffffff", "0", "2", "0", "0xffffffff", "1", "0xffffffff"],
            ["0xffffffff", "1", "2", "10000", "0xffffffff", "0", "0"],
        ],
        "cs_6_0",
        """ struct SBinaryUintOp {
                uint input1;
                uint input2;
                uint output1;
                uint output2;
            };
            RWStructuredBuffer<SBinaryUintOp> g_buf : register(u0);
            [numthreads(8,8,1)]
            void main(uint GI : SV_GroupIndex) {
                SBinaryUintOp l = g_buf[GI];
                l.output1 = l.input1 / l.input2;
                l.output2 = l.input1 % l.input2;
                g_buf[GI] = l;
            };""",
    )
    add_test_case(
        "UAddc",
        ["UAddc"],
        "Epsilon",
        0,
        [
            ["1", "1", "10000", "0x80000000", "0x7fffffff", "0xffffffff"],
            ["0", "256", "10001", "1", "0x7fffffff", "0x7fffffff"],
        ],
        [
            ["2", "2", "20000", "0", "0xfffffffe", "0xfffffffe"],
            ["0", "512", "20002", "3", "0xfffffffe", "0xffffffff"],
        ],
        "cs_6_0",
        """ struct SBinaryUintOp {
                uint input1;
                uint input2;
                uint output1;
                uint output2;
            };
            RWStructuredBuffer<SBinaryUintOp> g_buf : register(u0);
            [numthreads(8,8,1)]
            void main(uint GI : SV_GroupIndex) {
                SBinaryUintOp l = g_buf[GI];
                uint2 x = uint2(l.input1, l.input2);
                uint2 y = AddUint64(x, x);
                l.output1 = y.x;
                l.output2 = y.y;
                g_buf[GI] = l;
            };""",
    )

    # Tertiary Int
    add_test_case_int(
        "IMad",
        ["IMad"],
        "epsilon",
        0,
        [
            [
                "-2147483647",
                "-256",
                "-1",
                "0",
                "1",
                "2",
                "16",
                int32_max,
                "1",
                "-1",
                "1",
                "10",
            ],
            ["1", "-256", "-1", "0", "1", "3", "16", "0", "1", "-1", "10", "100"],
            [
                "0",
                "0",
                "0",
                "0",
                "1",
                "3",
                "1",
                "255",
                "2147483646",
                "-2147483647",
                "-10",
                "-2000",
            ],
        ],
        [
            [
                "-2147483647",
                "65536",
                "1",
                "0",
                "2",
                "9",
                "257",
                "255",
                int32_max,
                "-2147483646",
                "0",
                "-1000",
            ]
        ],
        "tertiary int",
        "mad",
        input_16=[
            [int16_min, "-256", "-1", "0", "1", "2", "16", int16_max],
            ["1", "8", "-1", "0", "1", "3", "16", "1"],
            ["0", "0", "1", "3", "250", "-30", int16_min, "-50"],
        ],
        output_16=[[int16_min, "-2048", "2", "3", "251", "-24", "-32512", "32717"]],
    )

    add_test_case_int(
        "UMad",
        ["UMad"],
        "epsilon",
        0,
        [
            ["0", "1", "2", "16", int32_max, "0", "10"],
            ["0", "1", "2", "16", "1", "0", "10"],
            ["0", "0", "1", "15", "0", "10", "10"],
        ],
        [["0", "1", "5", "271", int32_max, "10", "110"]],
        "tertiary uint",
        "mad",
        input_16=[
            ["0", "1", "2", "16", int16_max, "0", "10"],
            ["0", "1", "2", "16", "1", "0", "10"],
            ["0", "0", "1", "15", "0", "10", "10"],
        ],
        output_16=[["0", "1", "5", "271", int16_max, "10", "110"]],
    )

    # Dot
    add_test_case(
        "Dot",
        ["Dot2", "Dot3", "Dot4"],
        "epsilon",
        0.008,
        [
            [
                "NaN,NaN,NaN,NaN",
                "-Inf,-Inf,-Inf,-Inf",
                "-denorm,-denorm,-denorm,-denorm",
                "-0,-0,-0,-0",
                "0,0,0,0",
                "denorm,denorm,denorm,denorm",
                "Inf,Inf,Inf,Inf",
                "1,1,1,1",
                "-10,0,0,10",
                "Inf,Inf,Inf,-Inf",
            ],
            [
                "NaN,NaN,NaN,NaN",
                "-Inf,-Inf,-Inf,-Inf",
                "-denorm,-denorm,-denorm,-denorm",
                "-0,-0,-0,-0",
                "0,0,0,0",
                "denorm,denorm,denorm,denorm",
                "Inf,Inf,Inf,Inf",
                "1,1,1,1",
                "10,0,0,10",
                "Inf,Inf,Inf,Inf",
            ],
        ],
        [
            [nan, p_inf, 0, 0, 0, 0, p_inf, 2, -100, p_inf],
            [nan, p_inf, 0, 0, 0, 0, p_inf, 3, -100, p_inf],
            [nan, p_inf, 0, 0, 0, 0, p_inf, 4, 0, nan],
        ],
        "cs_6_0",
        """ struct SDotOp {
                   float4 input1;
                   float4 input2;
                   float o_dot2;
                   float o_dot3;
                   float o_dot4;
                };
                RWStructuredBuffer<SDotOp> g_buf : register(u0);
                [numthreads(8,8,1)]
                void main(uint GI : SV_GroupIndex) {
                    SDotOp l = g_buf[GI];
                    l.o_dot2 = dot(l.input1.xy, l.input2.xy);
                    l.o_dot3 = dot(l.input1.xyz, l.input2.xyz);
                    l.o_dot4 = dot(l.input1.xyzw, l.input2.xyzw);
                    g_buf[GI] = l;
                };""",
    )

    # Dot2AddHalf
    add_test_case(
        "Dot2AddHalf",
        ["Dot2AddHalf"],
        "epsilon",
        0.008,
        [
            [
                "1,2",
                "1,-2",
                "1,2",
                "-1,2",
                "1,2",
                "-1,2",
                "1,2",
                "-1,-2",
                "65504,1",
                "-65504,1",
                "1,65504",
                "1,-65504",
                "inf,inf",
                "denorm,denorm",
                "-denorm,-denorm",
                "nan,nan",
            ],
            [
                "3,4",
                "-3,4",
                "3,4",
                "3,-4",
                "3,4",
                "-3,4",
                "3,4",
                "-3,-4",
                "1,65504",
                "1,-65504",
                "65504,1",
                "-65504,1",
                "inf,inf",
                "denorm,denorm",
                "-denorm,-denorm",
                "nan,nan",
            ],
            [
                "0",
                "0",
                "10",
                "10",
                "-5",
                "-5",
                "-30",
                "-30",
                "0",
                "0",
                "10000000",
                "-10000000",
                "inf",
                "denorm",
                "-denorm",
                "nan",
            ],
        ],
        [
            [
                11,
                -11,
                21,
                -1,
                6,
                6,
                -19,
                -19,
                131008,
                -131008,
                10131008,
                -10131008,
                p_inf,
                0,
                0,
                nan,
            ],
        ],
        "cs_6_4",
        """ struct SDot2AddHalfOp {
                   half2 input1;
                   half2 input2;
                   float acc;
                   float result;
                };
                RWStructuredBuffer<SDot2AddHalfOp> g_buf : register(u0);
                [numthreads(8,8,1)]
                void main(uint GI : SV_GroupIndex) {
                    SDot2AddHalfOp l = g_buf[GI];
                    l.result = dot2add(l.input1, l.input2, l.acc);
                    g_buf[GI] = l;
                };""",
        shader_arguments="-enable-16bit-types",
    )

    # Dot4AddI8Packed
    add_test_case(
        "Dot4AddI8Packed",
        ["Dot4AddI8Packed"],
        "epsilon",
        0,
        [
            [
                "0x00000102",
                "0x00000102",
                "0x00000102",
                "0x00000102",
                "0XFFFFFFFF",
                "0x80808080",
                "0x80808080",
                "0x807F807F",
                "0x7F7F7F7F",
                "0x80808080",
            ],
            [
                "0x00000304",
                "0x00000304",
                "0x00000304",
                "0x00000304",
                "0xFFFFFFFF",
                "0x01010101",
                "0x7F7F7F7F",
                "0x807F807F",
                "0x7F7F7F7F",
                "0x80808080",
            ],
            ["0", "10", "-5", "-30", "0", "0", "0", "0", "0", "0"],
        ],
        [
            [11, 21, 6, -19, 4, -512, -65024, 65026, 64516, 65536],
        ],
        "cs_6_4",
        """ struct SDot4AddI8PackedOp {
                   dword input1;
                   dword input2;
                   int acc;
                   int result;
                };
                RWStructuredBuffer<SDot4AddI8PackedOp> g_buf : register(u0);
                [numthreads(8,8,1)]
                void main(uint GI : SV_GroupIndex) {
                    SDot4AddI8PackedOp l = g_buf[GI];
                    l.result = dot4add_i8packed(l.input1, l.input2, l.acc);
                    g_buf[GI] = l;
                };""",
    )

    # Dot4AddU8Packed
    add_test_case(
        "Dot4AddU8Packed",
        ["Dot4AddU8Packed"],
        "epsilon",
        0,
        [
            ["0x00000102", "0x00000102", "0x01234567", "0xFFFFFFFF", "0xFFFFFFFF"],
            ["0x00000304", "0x00000304", "0x23456789", "0xFFFFFFFF", "0xFFFFFFFF"],
            ["0", "10", "10000", "0", "3000000000"],
        ],
        [
            [11, 21, 33668, 260100, 3000260100],
        ],
        "cs_6_4",
        """ struct SDot4AddU8PackedOp {
                   dword input1;
                   dword input2;
                   dword acc;
                   dword result;
                };
                RWStructuredBuffer<SDot4AddU8PackedOp> g_buf : register(u0);
                [numthreads(8,8,1)]
                void main(uint GI : SV_GroupIndex) {
                    SDot4AddU8PackedOp l = g_buf[GI];
                    l.result = dot4add_u8packed(l.input1, l.input2, l.acc);
                    g_buf[GI] = l;
                };""",
    )

    # Quaternary
    # Msad4 intrinsic calls both Bfi and Msad. Currently this is the only way to call bfi instruction from HLSL
    add_test_case(
        "Bfi",
        ["Bfi", "Msad"],
        "epsilon",
        0,
        [
            ["0xA100B2C3", "0x00000000", "0xFFFF01C1", "0xFFFFFFFF"],
            [
                "0xD7B0C372, 0x4F57C2A3",
                "0xFFFFFFFF, 0x00000000",
                "0x38A03AEF, 0x38194DA3",
                "0xFFFFFFFF, 0x00000000",
            ],
            ["1,2,3,4", "1,2,3,4", "0,0,0,0", "10,10,10,10"],
        ],
        [["153,6,92,113", "1,2,3,4", "397,585,358,707", "10,265,520,775"]],
        "cs_6_0",
        """ struct SMsad4 {
                        uint ref;
                        uint2 source;
                        uint4 accum;
                        uint4 result;
                    };
                    RWStructuredBuffer<SMsad4> g_buf : register(u0);
                    [numthreads(8,8,1)]
                    void main(uint GI : SV_GroupIndex) {
                        SMsad4 l = g_buf[GI];
                        l.result = msad4(l.ref, l.source, l.accum);
                        g_buf[GI] = l;
                    };""",
    )

    # Wave Active Tests
    add_test_case(
        "WaveActiveSum",
        ["WaveActiveOp", "WaveReadLaneFirst", "WaveReadLaneAt"],
        "Epsilon",
        0,
        [["1", "2", "3", "4"], ["0"], ["2", "4", "8", "-64"]],
        [],
        "cs_6_0",
        get_shader_text("wave op int", "WaveActiveSum"),
    )
    add_test_case(
        "WaveActiveProduct",
        ["WaveActiveOp", "WaveReadLaneFirst", "WaveReadLaneAt"],
        "Epsilon",
        0,
        [["1", "2", "3", "4"], ["0"], ["1", "2", "4", "-64"]],
        [],
        "cs_6_0",
        get_shader_text("wave op int", "WaveActiveProduct"),
    )

    add_test_case(
        "WaveActiveCountBits",
        ["WaveAllBitCount", "WaveReadLaneFirst", "WaveReadLaneAt"],
        "Epsilon",
        0,
        [
            ["1", "2", "3", "4"],
            ["0"],
            ["1", "10", "-4", "-64"],
            ["-100", "-1000", "300"],
        ],
        [],
        "cs_6_0",
        get_shader_text("wave op int count", "WaveActiveCountBits"),
    )
    add_test_case(
        "WaveActiveMax",
        ["WaveActiveOp", "WaveReadLaneFirst", "WaveReadLaneAt"],
        "Epsilon",
        0,
        [
            ["1", "2", "3", "4"],
            ["0"],
            ["1", "10", "-4", "-64"],
            ["-100", "-1000", "300"],
        ],
        [],
        "cs_6_0",
        get_shader_text("wave op int", "WaveActiveMax"),
    )
    add_test_case(
        "WaveActiveMin",
        ["WaveActiveOp", "WaveReadLaneFirst", "WaveReadLaneAt"],
        "Epsilon",
        0,
        [
            ["1", "2", "3", "4", "5", "6", "7", "8", "9", "10"],
            ["0"],
            ["1", "10", "-4", "-64"],
            ["-100", "-1000", "300"],
        ],
        [],
        "cs_6_0",
        get_shader_text("wave op int", "WaveActiveMin"),
    )
    add_test_case(
        "WaveActiveAllEqual",
        ["WaveActiveAllEqual"],
        "Epsilon",
        0,
        [["1", "2", "3", "4", "1", "1", "1", "1"], ["3"], ["-10"]],
        [],
        "cs_6_0",
        get_shader_text("wave op int", "WaveActiveAllEqual"),
    )
    add_test_case(
        "WaveActiveAnyTrue",
        ["WaveAnyTrue", "WaveReadLaneFirst", "WaveReadLaneAt"],
        "Epsilon",
        0,
        [["1", "0", "1", "0", "1"], ["1"], ["0"]],
        [],
        "cs_6_0",
        get_shader_text("wave op int", "WaveActiveAnyTrue"),
    )
    add_test_case(
        "WaveActiveAllTrue",
        ["WaveAllTrue", "WaveReadLaneFirst", "WaveReadLaneAt"],
        "Epsilon",
        0,
        [["1", "0", "1", "0", "1"], ["1"], ["1"]],
        [],
        "cs_6_0",
        get_shader_text("wave op int", "WaveActiveAllTrue"),
    )

    add_test_case(
        "WaveActiveUSum",
        ["WaveActiveOp", "WaveReadLaneFirst", "WaveReadLaneAt"],
        "Epsilon",
        0,
        [["1", "2", "3", "4"], ["0"], ["2", "4", "8", "64"]],
        [],
        "cs_6_0",
        get_shader_text("wave op uint", "WaveActiveSum"),
    )
    add_test_case(
        "WaveActiveUProduct",
        ["WaveActiveOp", "WaveReadLaneFirst", "WaveReadLaneAt"],
        "Epsilon",
        0,
        [["1", "2", "3", "4"], ["0"], ["1", "2", "4", "64"]],
        [],
        "cs_6_0",
        get_shader_text("wave op uint", "WaveActiveProduct"),
    )
    add_test_case(
        "WaveActiveUMax",
        ["WaveActiveOp", "WaveReadLaneFirst", "WaveReadLaneAt"],
        "Epsilon",
        0,
        [["1", "2", "3", "4"], ["0"], ["1", "10", "4", "64"]],
        [],
        "cs_6_0",
        get_shader_text("wave op uint", "WaveActiveMax"),
    )
    add_test_case(
        "WaveActiveUMin",
        ["WaveActiveOp", "WaveReadLaneFirst", "WaveReadLaneAt"],
        "Epsilon",
        0,
        [
            ["1", "2", "3", "4", "5", "6", "7", "8", "9", "10"],
            ["0"],
            ["1", "10", "4", "64"],
        ],
        [],
        "cs_6_0",
        get_shader_text("wave op uint", "WaveActiveMin"),
    )
    add_test_case(
        "WaveActiveBitOr",
        ["WaveActiveBit"],
        "Epsilon",
        0,
        [
            [
                "0xe0000000",
                "0x0d000000",
                "0x00b00000",
                "0x00070000",
                "0x0000e000",
                "0x00000d00",
                "0x000000b0",
                "0x00000007",
            ],
            ["0xedb7edb7", "0xdb7edb7e", "0xb7edb7ed", "0x7edb7edb"],
            ["0x12481248", "0x24812481", "0x48124812", "0x81248124"],
            ["0x00000000", "0xffffffff"],
        ],
        [],
        "cs_6_0",
        get_shader_text("wave op uint", "WaveActiveBitOr"),
    )
    add_test_case(
        "WaveActiveBitAnd",
        ["WaveActiveBit"],
        "Epsilon",
        0,
        [
            [
                "0xefffffff",
                "0xfdffffff",
                "0xffbfffff",
                "0xfff7ffff",
                "0xffffefff",
                "0xfffffdff",
                "0xffffffbf",
                "0xfffffff7",
            ],
            ["0xedb7edb7", "0xdb7edb7e", "0xb7edb7ed", "0x7edb7edb"],
            ["0x12481248", "0x24812481", "0x48124812", "0x81248124"],
            ["0x00000000", "0xffffffff"],
        ],
        [],
        "cs_6_0",
        get_shader_text("wave op uint", "WaveActiveBitAnd"),
    )
    add_test_case(
        "WaveActiveBitXor",
        ["WaveActiveBit"],
        "Epsilon",
        0,
        [
            [
                "0xe0000000",
                "0x0d000000",
                "0x00b00000",
                "0x00070000",
                "0x0000e000",
                "0x00000d00",
                "0x000000b0",
                "0x00000007",
            ],
            ["0xedb7edb7", "0xdb7edb7e", "0xb7edb7ed", "0x7edb7edb"],
            ["0x12481248", "0x24812481", "0x48124812", "0x81248124"],
            ["0x00000000", "0xffffffff"],
        ],
        [],
        "cs_6_0",
        get_shader_text("wave op uint", "WaveActiveBitXor"),
    )
    add_test_case(
        "WavePrefixCountBits",
        ["WavePrefixBitCount"],
        "Epsilon",
        0,
        [
            ["1", "2", "3", "4", "5"],
            ["0"],
            ["1", "10", "-4", "-64"],
            ["-100", "-1000", "300"],
        ],
        [],
        "cs_6_0",
        get_shader_text("wave op int count", "WavePrefixCountBits"),
    )
    add_test_case(
        "WavePrefixSum",
        ["WavePrefixOp"],
        "Epsilon",
        0,
        [["1", "2", "3", "4", "5"], ["0", "1"], ["1", "2", "4", "-64", "128"]],
        [],
        "cs_6_0",
        get_shader_text("wave op int", "WavePrefixSum"),
    )
    add_test_case(
        "WavePrefixProduct",
        ["WavePrefixOp"],
        "Epsilon",
        0,
        [["1", "2", "3", "4", "5"], ["0", "1"], ["1", "2", "4", "-64", "128"]],
        [],
        "cs_6_0",
        get_shader_text("wave op int", "WavePrefixProduct"),
    )
    add_test_case(
        "WavePrefixUSum",
        ["WavePrefixOp"],
        "Epsilon",
        0,
        [["1", "2", "3", "4", "5"], ["0", "1"], ["1", "2", "4", "128"]],
        [],
        "cs_6_0",
        get_shader_text("wave op uint", "WavePrefixSum"),
    )
    add_test_case(
        "WavePrefixUProduct",
        ["WavePrefixOp"],
        "Epsilon",
        0,
        [["1", "2", "3", "4", "5"], ["0", "1"], ["1", "2", "4", "128"]],
        [],
        "cs_6_0",
        get_shader_text("wave op uint", "WavePrefixProduct"),
    )

    # Wave Multi Prefix Tests
    add_test_case(
        "WaveMultiPrefixBitAnd",
        ["WaveMultiPrefixOp"],
        "Epsilon",
        0,
        [
            ["0", "3", "1", "5", "4"],
            ["10", "42", "1", "64", "11", "76", "90", "111", "9", "6", "79", "34"],
        ],
        [],
        "cs_6_5",
        get_shader_text("wave op multi prefix int", "WaveMultiPrefixBitAnd"),
    )
    add_test_case(
        "WaveMultiPrefixBitOr",
        ["WaveMultiPrefixOp"],
        "Epsilon",
        0,
        [
            ["0", "3", "1", "5", "4"],
            ["10", "42", "1", "64", "11", "76", "90", "111", "9", "6", "79", "34"],
        ],
        [],
        "cs_6_5",
        get_shader_text("wave op multi prefix int", "WaveMultiPrefixBitOr"),
    )
    add_test_case(
        "WaveMultiPrefixBitXor",
        ["WaveMultiPrefixOp"],
        "Epsilon",
        0,
        [
            ["0", "3", "1", "5", "4"],
            ["10", "42", "1", "64", "11", "76", "90", "111", "9", "6", "79", "34"],
        ],
        [],
        "cs_6_5",
        get_shader_text("wave op multi prefix int", "WaveMultiPrefixBitXor"),
    )
    add_test_case(
        "WaveMultiPrefixSum",
        ["WaveMultiPrefixOp"],
        "Epsilon",
        0,
        [
            ["0", "3", "1", "5", "4"],
            ["10", "42", "1", "64", "11", "76", "90", "111", "9", "6", "79", "34"],
        ],
        [],
        "cs_6_5",
        get_shader_text("wave op multi prefix int", "WaveMultiPrefixSum"),
    )
    add_test_case(
        "WaveMultiPrefixProduct",
        ["WaveMultiPrefixOp"],
        "Epsilon",
        0,
        [
            ["0", "3", "1", "5", "4"],
            ["10", "42", "1", "64", "11", "76", "90", "111", "9", "6", "79", "34"],
        ],
        [],
        "cs_6_5",
        get_shader_text("wave op multi prefix int", "WaveMultiPrefixProduct"),
    )
    add_test_case(
        "WaveMultiPrefixCountBits",
        ["WaveMultiPrefixOp"],
        "Epsilon",
        0,
        [
            ["0", "3", "1", "5", "4"],
            ["0", "42", "0", "64", "11", "76", "90", "111", "0", "0", "79", "34"],
        ],
        [],
        "cs_6_5",
        get_shader_text("wave op multi prefix int", "WaveMultiPrefixCountBits"),
    )

    add_test_case(
        "WaveMultiPrefixUBitAnd",
        ["WaveMultiPrefixOp"],
        "Epsilon",
        0,
        [
            ["0", "3", "1", "5", "4"],
            ["10", "42", "1", "64", "11", "76", "90", "111", "9", "6", "79", "34"],
        ],
        [],
        "cs_6_5",
        get_shader_text("wave op multi prefix uint", "WaveMultiPrefixBitAnd"),
    )
    add_test_case(
        "WaveMultiPrefixUBitOr",
        ["WaveMultiPrefixOp"],
        "Epsilon",
        0,
        [
            ["0", "3", "1", "5", "4"],
            ["10", "42", "1", "64", "11", "76", "90", "111", "9", "6", "79", "34"],
        ],
        [],
        "cs_6_5",
        get_shader_text("wave op multi prefix uint", "WaveMultiPrefixBitOr"),
    )
    add_test_case(
        "WaveMultiPrefixUBitXor",
        ["WaveMultiPrefixOp"],
        "Epsilon",
        0,
        [
            ["0", "3", "1", "5", "4"],
            ["10", "42", "1", "64", "11", "76", "90", "111", "9", "6", "79", "34"],
        ],
        [],
        "cs_6_5",
        get_shader_text("wave op multi prefix uint", "WaveMultiPrefixBitXor"),
    )
    add_test_case(
        "WaveMultiPrefixUSum",
        ["WaveMultiPrefixOp"],
        "Epsilon",
        0,
        [
            ["0", "3", "1", "5", "4"],
            ["10", "42", "1", "64", "11", "76", "90", "111", "9", "6", "79", "34"],
        ],
        [],
        "cs_6_5",
        get_shader_text("wave op multi prefix uint", "WaveMultiPrefixSum"),
    )
    add_test_case(
        "WaveMultiPrefixUProduct",
        ["WaveMultiPrefixOp"],
        "Epsilon",
        0,
        [
            ["0", "3", "1", "5", "4"],
            ["10", "42", "1", "64", "11", "76", "90", "111", "9", "6", "79", "34"],
        ],
        [],
        "cs_6_5",
        get_shader_text("wave op multi prefix uint", "WaveMultiPrefixProduct"),
    )
    add_test_case(
        "WaveMultiPrefixUCountBits",
        ["WaveMultiPrefixOp"],
        "Epsilon",
        0,
        [
            ["0", "3", "1", "5", "4"],
            ["0", "42", "0", "64", "11", "76", "90", "111", "0", "0", "79", "34"],
        ],
        [],
        "cs_6_5",
        get_shader_text("wave op multi prefix uint", "WaveMultiPrefixCountBits"),
    )


# generating xml file for execution test using data driven method
# TODO: ElementTree is not generating formatted XML. Currently xml file is checked in after VS Code formatter.
# Implement xml formatter or import formatter library and use that instead.


def generate_parameter_types(
    table, num_inputs, num_outputs, has_known_warp_issue=False
):
    param_types = ET.SubElement(table, "ParameterTypes")
    ET.SubElement(
        param_types, "ParameterType", attrib={"Name": "ShaderOp.Target"}
    ).text = "String"
    ET.SubElement(
        param_types, "ParameterType", attrib={"Name": "ShaderOp.Arguments"}
    ).text = "String"
    ET.SubElement(
        param_types, "ParameterType", attrib={"Name": "ShaderOp.Text"}
    ).text = "String"
    ET.SubElement(
        param_types, "ParameterType", attrib={"Name": "Validation.Type"}
    ).text = "String"
    ET.SubElement(
        param_types, "ParameterType", attrib={"Name": "Validation.Tolerance"}
    ).text = "double"
    for i in range(0, num_inputs):
        ET.SubElement(
            param_types,
            "ParameterType",
            attrib={"Name": "Validation.Input{}".format(i + 1), "Array": "true"},
        ).text = "String"
    for i in range(0, num_outputs):
        ET.SubElement(
            param_types,
            "ParameterType",
            attrib={"Name": "Validation.Expected{}".format(i + 1), "Array": "true"},
        ).text = "String"
    if has_known_warp_issue:
        ET.SubElement(
            param_types, "ParameterType", attrib={"Name": "Warp.Version"}
        ).text = "unsigned int"


def generate_parameter_types_wave(table):
    param_types = ET.SubElement(table, "ParameterTypes")
    ET.SubElement(
        param_types, "ParameterType", attrib={"Name": "ShaderOp.Target"}
    ).text = "String"
    ET.SubElement(
        param_types, "ParameterType", attrib={"Name": "ShaderOp.Text"}
    ).text = "String"
    ET.SubElement(
        param_types, "ParameterType", attrib={"Name": "Validation.NumInputSet"}
    ).text = "String"
    ET.SubElement(
        param_types,
        "ParameterType",
        attrib={"Name": "Validation.InputSet1", "Array": "true"},
    ).text = "String"
    ET.SubElement(
        param_types,
        "ParameterType",
        attrib={"Name": "Validation.InputSet2", "Array": "true"},
    ).text = "String"
    ET.SubElement(
        param_types,
        "ParameterType",
        attrib={"Name": "Validation.InputSet3", "Array": "true"},
    ).text = "String"
    ET.SubElement(
        param_types,
        "ParameterType",
        attrib={"Name": "Validation.InputSet4", "Array": "true"},
    ).text = "String"


def generate_parameter_types_wave_multi_prefix(table):
    param_types = ET.SubElement(table, "ParameterTypes")

    ET.SubElement(
        param_types, "ParameterType", attrib={"Name": "ShaderOp.Target"}
    ).text = "String"
    ET.SubElement(
        param_types, "ParameterType", attrib={"Name": "ShaderOp.Text"}
    ).text = "String"
    ET.SubElement(
        param_types,
        "ParameterType",
        attrib={"Name": "Validation.Keys", "Array": "true"},
    ).text = "String"
    ET.SubElement(
        param_types,
        "ParameterType",
        attrib={"Name": "Validation.Values", "Array": "true"},
    ).text = "String"


def generate_parameter_types_msad(table):
    param_types = ET.SubElement(table, "ParameterTypes")
    ET.SubElement(
        param_types, "ParameterType", attrib={"Name": "ShaderOp.Text"}
    ).text = "String"
    ET.SubElement(
        param_types, "ParameterType", attrib={"Name": "Validation.Tolerance"}
    ).text = "int"
    ET.SubElement(
        param_types,
        "ParameterType",
        attrib={"Name": "Validation.Input1", "Array": "true"},
    ).text = "unsigned int"
    ET.SubElement(
        param_types,
        "ParameterType",
        attrib={"Name": "Validation.Input2", "Array": "true"},
    ).text = "String"
    ET.SubElement(
        param_types,
        "ParameterType",
        attrib={"Name": "Validation.Input3", "Array": "true"},
    ).text = "String"
    ET.SubElement(
        param_types,
        "ParameterType",
        attrib={"Name": "Validation.Expected1", "Array": "true"},
    ).text = "String"


def generate_row(table, case):
    row = ET.SubElement(table, "Row", {"Name": case.test_name})
    ET.SubElement(
        row, "Parameter", {"Name": "Validation.Type"}
    ).text = case.validation_type
    ET.SubElement(row, "Parameter", {"Name": "Validation.Tolerance"}).text = str(
        case.validation_tolerance
    )
    ET.SubElement(row, "Parameter", {"Name": "ShaderOp.Text"}).text = case.shader_text
    ET.SubElement(
        row, "Parameter", {"Name": "ShaderOp.Target"}
    ).text = case.shader_target
    for i in range(len(case.input_lists)):
        inputs = ET.SubElement(
            row, "Parameter", {"Name": "Validation.Input{}".format(i + 1)}
        )
        for val in case.input_lists[i]:
            ET.SubElement(inputs, "Value").text = str(val)
    for i in range(len(case.output_lists)):
        outputs = ET.SubElement(
            row, "Parameter", {"Name": "Validation.Expected{}".format(i + 1)}
        )
        for val in case.output_lists[i]:
            ET.SubElement(outputs, "Value").text = str(val)
    # Optional parameters
    if case.warp_version > 0:
        ET.SubElement(row, "Parameter", {"Name": "Warp.Version"}).text = str(
            case.warp_version
        )
    if case.shader_arguments != "":
        ET.SubElement(
            row, "Parameter", {"Name": "ShaderOp.Arguments"}
        ).text = case.shader_arguments


def generate_row_wave(table, case):
    row = ET.SubElement(table, "Row", {"Name": case.test_name})
    ET.SubElement(row, "Parameter", {"Name": "ShaderOp.Name"}).text = case.test_name
    ET.SubElement(row, "Parameter", {"Name": "ShaderOp.Text"}).text = case.shader_text
    ET.SubElement(row, "Parameter", {"Name": "Validation.NumInputSet"}).text = str(
        len(case.input_lists)
    )
    for i in range(len(case.input_lists)):
        inputs = ET.SubElement(
            row, "Parameter", {"Name": "Validation.InputSet{}".format(i + 1)}
        )
        for val in case.input_lists[i]:
            ET.SubElement(inputs, "Value").text = str(val)


def generate_row_wave_multi(table, case):
    row = ET.SubElement(table, "Row", {"Name": case.test_name})
    ET.SubElement(row, "Parameter", {"Name": "ShaderOp.Name"}).text = case.test_name
    ET.SubElement(
        row, "Parameter", {"Name": "ShaderOp.Target"}
    ).text = case.shader_target
    ET.SubElement(row, "Parameter", {"Name": "ShaderOp.Text"}).text = case.shader_text
    inputs = ET.SubElement(row, "Parameter", {"Name": "Validation.Keys"})
    for val in case.input_lists[0]:
        ET.SubElement(inputs, "Value").text = str(val)
    inputs = ET.SubElement(row, "Parameter", {"Name": "Validation.Values"})
    for val in case.input_lists[1]:
        ET.SubElement(inputs, "Value").text = str(val)


def generate_table_for_taef():
    with open(
        "..\\..\\tools\\clang\\unittests\\HLSL\\ShaderOpArithTable.xml", "w"
    ) as f:
        tree = ET.ElementTree()
        root = ET.Element("Data")
        # Create tables
        generate_parameter_types(
            ET.SubElement(root, "Table", attrib={"Id": "UnaryFloatOpTable"}), 1, 1, True
        )
        generate_parameter_types(
            ET.SubElement(root, "Table", attrib={"Id": "BinaryFloatOpTable"}), 2, 2
        )
        generate_parameter_types(
            ET.SubElement(root, "Table", attrib={"Id": "TertiaryFloatOpTable"}), 3, 1
        )
        generate_parameter_types(
            ET.SubElement(root, "Table", attrib={"Id": "UnaryHalfOpTable"}), 1, 1, True
        )
        generate_parameter_types(
            ET.SubElement(root, "Table", attrib={"Id": "BinaryHalfOpTable"}), 2, 2
        )
        generate_parameter_types(
            ET.SubElement(root, "Table", attrib={"Id": "TertiaryHalfOpTable"}), 3, 1
        )
        generate_parameter_types(
            ET.SubElement(root, "Table", attrib={"Id": "UnaryIntOpTable"}), 1, 1
        )
        generate_parameter_types(
            ET.SubElement(root, "Table", attrib={"Id": "BinaryIntOpTable"}), 2, 2
        )
        generate_parameter_types(
            ET.SubElement(root, "Table", attrib={"Id": "TertiaryIntOpTable"}), 3, 1
        )
        generate_parameter_types(
            ET.SubElement(root, "Table", attrib={"Id": "UnaryInt16OpTable"}), 1, 1
        )
        generate_parameter_types(
            ET.SubElement(root, "Table", attrib={"Id": "BinaryInt16OpTable"}), 2, 2
        )
        generate_parameter_types(
            ET.SubElement(root, "Table", attrib={"Id": "TertiaryInt16OpTable"}), 3, 1
        )
        generate_parameter_types(
            ET.SubElement(root, "Table", attrib={"Id": "UnaryUintOpTable"}), 1, 1
        )
        generate_parameter_types(
            ET.SubElement(root, "Table", attrib={"Id": "BinaryUintOpTable"}), 2, 2
        )
        generate_parameter_types(
            ET.SubElement(root, "Table", attrib={"Id": "TertiaryUintOpTable"}), 3, 1
        )
        generate_parameter_types(
            ET.SubElement(root, "Table", attrib={"Id": "UnaryUint16OpTable"}), 1, 1
        )
        generate_parameter_types(
            ET.SubElement(root, "Table", attrib={"Id": "BinaryUint16OpTable"}), 2, 2
        )
        generate_parameter_types(
            ET.SubElement(root, "Table", attrib={"Id": "TertiaryUint16OpTable"}), 3, 1
        )
        generate_parameter_types(
            ET.SubElement(root, "Table", attrib={"Id": "DotOpTable"}), 2, 3
        )
        generate_parameter_types(
            ET.SubElement(root, "Table", attrib={"Id": "Dot2AddHalfOpTable"}), 3, 1
        )
        generate_parameter_types(
            ET.SubElement(root, "Table", attrib={"Id": "Dot4AddI8PackedOpTable"}), 3, 1
        )
        generate_parameter_types(
            ET.SubElement(root, "Table", attrib={"Id": "Dot4AddU8PackedOpTable"}), 3, 1
        )
        generate_parameter_types_msad(
            ET.SubElement(root, "Table", attrib={"Id": "Msad4Table"})
        )
        generate_parameter_types_wave(
            ET.SubElement(root, "Table", attrib={"Id": "WaveIntrinsicsActiveIntTable"})
        )
        generate_parameter_types_wave(
            ET.SubElement(root, "Table", attrib={"Id": "WaveIntrinsicsActiveUintTable"})
        )
        generate_parameter_types_wave(
            ET.SubElement(root, "Table", attrib={"Id": "WaveIntrinsicsPrefixIntTable"})
        )
        generate_parameter_types_wave(
            ET.SubElement(root, "Table", attrib={"Id": "WaveIntrinsicsPrefixUintTable"})
        )
        generate_parameter_types_wave_multi_prefix(
            ET.SubElement(
                root, "Table", attrib={"Id": "WaveIntrinsicsMultiPrefixIntTable"}
            )
        )
        generate_parameter_types_wave_multi_prefix(
            ET.SubElement(
                root, "Table", attrib={"Id": "WaveIntrinsicsMultiPrefixUintTable"}
            )
        )
        generate_parameter_types(
            ET.SubElement(root, "Table", attrib={"Id": "DenormBinaryFloatOpTable"}),
            2,
            2,
        )  # 2 sets of expected values for any mode
        generate_parameter_types(
            ET.SubElement(root, "Table", attrib={"Id": "DenormTertiaryFloatOpTable"}),
            3,
            2,
        )

        for case in g_test_cases.values():
            cur_inst = case.insts[0]
            if cur_inst.is_cast or cur_inst.category.startswith("Unary"):
                if "f" in cur_inst.oload_types and not "Half" in case.test_name:
                    generate_row(root.find("./Table[@Id='UnaryFloatOpTable']"), case)
                if "h" in cur_inst.oload_types and "Half" in case.test_name:
                    generate_row(root.find("./Table[@Id='UnaryHalfOpTable']"), case)
                if "i" in cur_inst.oload_types and "Bit16" not in case.test_name:
                    if cur_inst.category.startswith("Unary int"):
                        generate_row(root.find("./Table[@Id='UnaryIntOpTable']"), case)
                    elif cur_inst.category.startswith("Unary uint"):
                        generate_row(root.find("./Table[@Id='UnaryUintOpTable']"), case)
                    else:
                        print("unknown op: " + cur_inst.name)
                        print(cur_inst.dxil_class)
                if "w" in cur_inst.oload_types and "Bit16" in case.test_name:
                    if cur_inst.category.startswith("Unary int"):
                        generate_row(
                            root.find("./Table[@Id='UnaryInt16OpTable']"), case
                        )
                    elif cur_inst.category.startswith("Unary uint"):
                        generate_row(
                            root.find("./Table[@Id='UnaryUint16OpTable']"), case
                        )
                    else:
                        print("unknown op: " + cur_inst.name)
                        print(cur_inst.dxil_class)

            elif cur_inst.is_binary or cur_inst.category.startswith("Binary"):
                if "f" in cur_inst.oload_types and not "Half" in case.test_name:
                    if case.test_name in g_denorm_tests:  # for denorm tests
                        generate_row(
                            root.find("./Table[@Id='DenormBinaryFloatOpTable']"), case
                        )
                    else:
                        generate_row(
                            root.find("./Table[@Id='BinaryFloatOpTable']"), case
                        )
                if "h" in cur_inst.oload_types and "Half" in case.test_name:
                    generate_row(root.find("./Table[@Id='BinaryHalfOpTable']"), case)
                if "i" in cur_inst.oload_types and "Bit16" not in case.test_name:
                    if cur_inst.category.startswith("Binary int"):
                        if case.test_name in [
                            "UAdd",
                            "USub",
                            "UMul",
                        ]:  # Add, Sub, Mul use same operations for int and uint.
                            generate_row(
                                root.find("./Table[@Id='BinaryUintOpTable']"), case
                            )
                        else:
                            generate_row(
                                root.find("./Table[@Id='BinaryIntOpTable']"), case
                            )
                    elif cur_inst.category.startswith("Binary uint"):
                        generate_row(
                            root.find("./Table[@Id='BinaryUintOpTable']"), case
                        )
                    else:
                        print("unknown op: " + cur_inst.name)
                        print(cur_inst.dxil_class)
                if "w" in cur_inst.oload_types and "Bit16" in case.test_name:
                    if cur_inst.category.startswith("Binary int"):
                        if case.test_name in [
                            "UAdd",
                            "USub",
                            "UMul",
                        ]:  # Add, Sub, Mul use same operations for int and uint.
                            generate_row(
                                root.find("./Table[@Id='BinaryUint16OpTable']"), case
                            )
                        else:
                            generate_row(
                                root.find("./Table[@Id='BinaryInt16OpTable']"), case
                            )
                    elif cur_inst.category.startswith("Binary uint"):
                        generate_row(
                            root.find("./Table[@Id='BinaryUint16OpTable']"), case
                        )
                    else:
                        print("unknown op: " + cur_inst.name)
                        print(cur_inst.dxil_class)

            elif cur_inst.category.startswith("Tertiary"):
                if "f" in cur_inst.oload_types and not "Half" in case.test_name:
                    if case.test_name in g_denorm_tests:  # for denorm tests
                        generate_row(
                            root.find("./Table[@Id='DenormTertiaryFloatOpTable']"), case
                        )
                    else:
                        generate_row(
                            root.find("./Table[@Id='TertiaryFloatOpTable']"), case
                        )
                if "h" in cur_inst.oload_types and "Half" in case.test_name:
                    generate_row(root.find("./Table[@Id='TertiaryHalfOpTable']"), case)
                if "i" in cur_inst.oload_types and "Bit16" not in case.test_name:
                    if cur_inst.category.startswith("Tertiary int"):
                        generate_row(
                            root.find("./Table[@Id='TertiaryIntOpTable']"), case
                        )
                    elif cur_inst.category.startswith("Tertiary uint"):
                        generate_row(
                            root.find("./Table[@Id='TertiaryUintOpTable']"), case
                        )
                    else:
                        print("unknown op: " + cur_inst.name)
                        print(cur_inst.dxil_class)
                if "w" in cur_inst.oload_types and "Bit16" in case.test_name:
                    if cur_inst.category.startswith("Tertiary int"):
                        generate_row(
                            root.find("./Table[@Id='TertiaryInt16OpTable']"), case
                        )
                    elif cur_inst.category.startswith("Tertiary uint"):
                        generate_row(
                            root.find("./Table[@Id='TertiaryUint16OpTable']"), case
                        )
                    else:
                        print("unknown op: " + cur_inst.name)
                        print(cur_inst.dxil_class)
            elif cur_inst.category.startswith("Quaternary"):
                if cur_inst.name == "Bfi":
                    generate_row(root.find("./Table[@Id='Msad4Table']"), case)
                else:
                    print("unknown op: " + cur_inst.name)
                    print(cur_inst.dxil_class)
            elif cur_inst.category == "Dot":
                generate_row(root.find("./Table[@Id='DotOpTable']"), case)
            elif cur_inst.category == "Dot product with accumulate":
                if cur_inst.name == "Dot2AddHalf":
                    generate_row(root.find("./Table[@Id='Dot2AddHalfOpTable']"), case)
                elif cur_inst.name == "Dot4AddI8Packed":
                    generate_row(
                        root.find("./Table[@Id='Dot4AddI8PackedOpTable']"), case
                    )
                elif cur_inst.name == "Dot4AddU8Packed":
                    generate_row(
                        root.find("./Table[@Id='Dot4AddU8PackedOpTable']"), case
                    )
                else:
                    print("unknown op: " + cur_inst.name)
                    print(cur_inst.dxil_class)
            elif cur_inst.dxil_class in [
                "WaveActiveOp",
                "WaveAllOp",
                "WaveActiveAllEqual",
                "WaveAnyTrue",
                "WaveAllTrue",
            ]:
                if case.test_name.startswith("WaveActiveU"):
                    generate_row_wave(
                        root.find("./Table[@Id='WaveIntrinsicsActiveUintTable']"), case
                    )
                else:
                    generate_row_wave(
                        root.find("./Table[@Id='WaveIntrinsicsActiveIntTable']"), case
                    )
            elif cur_inst.dxil_class == "WaveActiveBit":
                generate_row_wave(
                    root.find("./Table[@Id='WaveIntrinsicsActiveUintTable']"), case
                )
            elif cur_inst.dxil_class == "WavePrefixOp":
                if case.test_name.startswith("WavePrefixU"):
                    generate_row_wave(
                        root.find("./Table[@Id='WaveIntrinsicsPrefixUintTable']"), case
                    )
                else:
                    generate_row_wave(
                        root.find("./Table[@Id='WaveIntrinsicsPrefixIntTable']"), case
                    )
            elif cur_inst.dxil_class == "WaveMultiPrefixOp":
                if case.test_name.startswith("WaveMultiPrefixU"):
                    generate_row_wave_multi(
                        root.find("./Table[@Id='WaveIntrinsicsMultiPrefixUintTable']"),
                        case,
                    )
                else:
                    generate_row_wave_multi(
                        root.find("./Table[@Id='WaveIntrinsicsMultiPrefixIntTable']"),
                        case,
                    )
            else:
                print("unknown op: " + cur_inst.name)
                print(cur_inst.dxil_class)
        tree._setroot(root)
        from xml.dom.minidom import parseString

        pretty_xml = parseString(ET.tostring(root)).toprettyxml(indent="    ")
        f.write(pretty_xml)

        print("Saved file at: " + f.name)
        f.close()


def print_untested_inst():
    lst = []
    for name in [
        node.inst.name
        for node in g_instruction_nodes.values()
        if len(node.test_cases) == 0
    ]:
        lst += [name]
    lst.sort()
    print("Untested dxil ops: ")
    for name in lst:
        print(name)
    print("Total uncovered dxil ops: " + str(len(lst)))
    print("Total covered dxil ops: " + str(len(g_instruction_nodes) - len(lst)))


# inst name to instruction dict
g_instruction_nodes = {}
# test name to test case dict
g_test_cases = {}

if __name__ == "__main__":
    db = get_db_dxil()
    for inst in db.instr:
        g_instruction_nodes[inst.name] = inst_node(inst)
    add_test_cases()

    args = vars(parser.parse_args())
    mode = args["mode"]
    if mode == "info":
        print_untested_inst()
    elif mode == "gen-xml":
        generate_table_for_taef()
    else:
        print("unknown mode: " + mode)
        exit(1)
    exit(0)
