#!/usr/bin/env python3
"""Generate C headers for certain extended instruction sets"""

import subprocess
import os

# Assume we are running from the tools/buildHeaders directory
os.chdir('../../include/spirv/unified1')

def mk_extinst(name, grammar_file, header_basename=None):
  """Generate one C header from a grammar"""
  script = '../../../tools/buildHeaders/bin/generate_language_headers.py'
  header_basename = header_basename if header_basename else name
  subprocess.check_call(['python3',
                         script,
                         '--extinst-name=' + name,
                         '--extinst-grammar=' + grammar_file,
                         '--extinst-output-base=' + header_basename])
  subprocess.check_call(['dos2unix', header_basename + '.h'])


mk_extinst('DebugInfo', 'extinst.debuginfo.grammar.json')
mk_extinst('OpenCLDebugInfo100', 'extinst.opencl.debuginfo.100.grammar.json')
mk_extinst('AMD_gcn_shader', 'extinst.spv-amd-gcn-shader.grammar.json')
mk_extinst('AMD_shader_ballot', 'extinst.spv-amd-shader-ballot.grammar.json')
mk_extinst('AMD_shader_explicit_vertex_parameter', 'extinst.spv-amd-shader-explicit-vertex-parameter.grammar.json')
mk_extinst('AMD_shader_trinary_minmax', 'extinst.spv-amd-shader-trinary-minmax.grammar.json')
mk_extinst('NonSemanticDebugPrintf', 'extinst.nonsemantic.debugprintf.grammar.json')
mk_extinst('NonSemanticClspvReflection', 'extinst.nonsemantic.clspvreflection.grammar.json')
mk_extinst('NonSemanticDebugBreak', 'extinst.nonsemantic.debugbreak.grammar.json')
mk_extinst('NonSemanticVkspReflection', 'extinst.nonsemantic.vkspreflection.grammar.json')
mk_extinst('TOSA', 'extinst.tosa.001000.1.grammar.json', 'TOSA.001000.1')
