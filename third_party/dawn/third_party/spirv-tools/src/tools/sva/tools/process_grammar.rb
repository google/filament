#!/usr/bin/env ruby

# Copyright 2019 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

require 'json'

GRAMMAR = "../../external/spirv-headers/include/spirv/unified1/spirv.core.grammar.json"
GLSL = "../../external/spirv-headers/include/spirv/unified1/extinst.glsl.std.450.grammar.json"

CAPABILITIES = %w(
      Matrix
      Shader
      Sampled1D
      Image1D
      DerivativeControl
      ImageQuery
      VulkanMemoryModel
)

g = JSON.parse(File.open(GRAMMAR).read)
magic = g['magic_number']
vers = [g['major_version'], g['minor_version']]
instructions = {}

g['instructions'].each do |inst|
  if (inst.has_key?('capabilities'))
    skip = true
    inst['capabilities'].each do |cap|
      if CAPABILITIES.include?(cap)
        skip = false
        break
      end
    end
    next if skip
  end

  op = {
    opcode: inst['opcode'],
    operands: []
  }

  if !inst['operands'].nil?
    inst['operands'].each do |operand|
      operand.delete('name')
      op[:operands] << operand
    end
  end

  instructions[inst['opname']] = op
end

operand_kinds = {}
g['operand_kinds'].each do |op_kind|
  next if op_kind['category'] !~ /Enum/

  kind = {
    type: op_kind['category'],
    values: {}
  }

  op_kind['enumerants'].each do |enum|
    if (enum.has_key?('capabilities'))
      skip = true
      enum['capabilities'].each do |cap|
        if CAPABILITIES.include?(cap)
          skip = false
          break
        end
      end
      next if skip
    end

    v = if op_kind['category'] == 'BitEnum'
      enum['value'].to_i(16)
    else
      enum['value'].to_i
    end
    params = []
    if enum.has_key?('parameters')
      enum['parameters'].each do |param|
        params << param['kind']
      end
    end
    kind[:values][enum['enumerant']] = {value: v}
    kind[:values][enum['enumerant']][:params] = params unless params.empty?
  end

  next if kind[:values].empty?
  operand_kinds[op_kind['kind']] = kind
end

# We only support GLSL extensions at the moment.
ext = {}
glsl = JSON.parse(File.open(GLSL).read)
glsl['instructions'].each do |inst|
  ext[inst['opname']] = inst['opcode']
end

puts "/*#{g['copyright'].join("\n")}*/"
puts "\n// THIS FILE IS GENERATED WITH tools/process_grammar.rb\n\n"
puts "export default " + JSON.pretty_generate({
  magic: magic,
  version: vers,
  instructions: instructions,
  operand_kinds: operand_kinds,
  ext: ext
})
