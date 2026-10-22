// Copyright (c) 2020 The Khronos Group Inc.
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

const spirvTools = require("../../out/web/spirv-tools");
const fs = require("fs");
const util = require("util");
const readFile = util.promisify(fs.readFile);

const SPV_PATH = "./test/fuzzers/corpora/spv/simple.spv";

const test = async () => {
  const spv = await spirvTools();

  // disassemble from file
  const buffer = await readFile(SPV_PATH);
  const disFileResult = spv.dis(
    buffer,
    spv.SPV_ENV_UNIVERSAL_1_3,
    spv.SPV_BINARY_TO_TEXT_OPTION_INDENT |
      spv.SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES |
      spv.SPV_BINARY_TO_TEXT_OPTION_COLOR
  );
  console.log("dis from file:\n", disFileResult);

  // assemble
  const source = `
             OpCapability Linkage 
             OpCapability Shader 
             OpMemoryModel Logical GLSL450 
             OpSource GLSL 450 
             OpDecorate %spec SpecId 1 
      %int = OpTypeInt 32 1 
     %spec = OpSpecConstant %int 0 
    %const = OpConstant %int 42`;
  const asResult = spv.as(
    source,
    spv.SPV_ENV_UNIVERSAL_1_3,
    spv.SPV_TEXT_TO_BINARY_OPTION_NONE
  );
  console.log(`as returned ${asResult.byteLength} bytes`);

  // re-disassemble
  const disResult = spv.dis(
    asResult,
    spv.SPV_ENV_UNIVERSAL_1_3,
    spv.SPV_BINARY_TO_TEXT_OPTION_INDENT |
      spv.SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES |
      spv.SPV_BINARY_TO_TEXT_OPTION_COLOR
  );
  console.log("dis:\n", disResult);
};

test();
