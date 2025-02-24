# Generated Code

There is a lot of code generated in `layers/vulkan/generated/`. This is done to prevent errors forgetting to add support for new
values when the Vulkan Headers or SPIR-V Grammar is updated.

- [How to generate the code](#how-to-generate-the-code)
- [Adding and Editing code generation](#adding-and-editing-code-generation)
- [How it works](#how-it-works)

# Dependency

`pyparsing` is required, so if it is not installed, you will need to call `pip install pyparsing`

`clang-format` is required, we format the code after it is generated

# How to generate the code

When making change to the `scripts/` folder, make sure to run `generate_source.py` and check in both the changes to
`scripts/` and `layers/vulkan/generated/` in any PR.

A helper CMake target `vvl_codegen` is provided to simplify the invocation of `scripts/generate_source.py` from the build directory:

```bash
cmake -S . -B build -D VVL_CODEGEN=ON
cmake --build build --target vvl_codegen
```

NOTE: `VVL_CODEGEN` is `OFF` by default to allow users to build `VVL` via `add_subdirectory` and to avoid potential issues for system/language package managers.

## Tips

If only dealing with a single file,  run `scripts/generate_source.py` with `--target`

```bash
# Example - only generates chassis.cpp
scripts/generate_source.py external/Vulkan-Headers/registry/ external/SPIRV-Headers/include/spirv/unified1/ --target chassis.cpp
```

# Adding and Editing code generation

> Make sure to look at the [python coding style guide](python_scripts_code_style.md)

The `base_generator.py` and `vulkan_object.py` are the core of all generated code

- `BaseGenerator`
  - This is the only file that understands the `reg.py` flow in the `registry`
  - most developers will never need to touch this file
- `VulkanObject`
  - Can be accessed with `self.vk`
  - "C Header" like file that describes what information can be used when generating code
  - Uses the [Python 3.7 Dataclasses](https://docs.python.org/3/library/dataclasses.html) to enforce a schema so developers

Every "Generator" that extends `BaseGenerator` has a `def generate(self)` which is the "main" function

## Using VulkanObject

The following are examples of helpful things that can be done with VulkanObject

```python
#
# Loop structs that have a sType
for struct in [x for x in self.vk.structs.values() if x.sType]:
    print(struct.name)

#
# Print each command parameter C string
for command in self.vk.commands.value():
    for param in command.params:
        print(param.cDeclaration)

#
# Loop commands with Transfer Queues
for command in [x for x in self.vk.commands.value() if Queues.TRANSFER & x.queues]:
    print(command.name)

#
# Find enums that are extended with an Instance extension
for enum in self.vk.enum.values():
    for extension in [x for x in enum.extensions if x.instance]:
        print(f'{enum.name} - {extension.name}')

#
# List all VkImageViewType enum flags
for field in self.vk.enums['VkImageViewType'].fields:
    print(field.name)
```

## Design philosophy

> Written by someone who has written bad Vulkan code gen, debugged other's bad code gen, and rewrote all the scripts.

### Avoid functions when possible

All code gen has a single function that outputs one large string, there is zero dynamic control flow that occurs. (Generators don't take any runtime arguments other then file locations)

While it seems useful to group your logic into a single function, it because hard to debug where all the sub-strings are appearing from in the final file.

The few times it is good to use functions is

- It returns a non-string value. (ex. sorting logic)
- There is recursion needed (ex. walking down structs with more structs in it)

### Avoid writting C/C++ code in python strings

If you find yourself having many functions that are just written as python strings, it might be worth looking into having the file live in the actual layer code.

### Choose code readability over clever python

Code generation is **not** a bottleneck for performance, but trying add/edit/debug code generation scripts **is a bottleneck** for developer time. The main goal is make any python generating code as easy to understand as possible.

# How it works

`generate_source.py` sets up the environment and then calls into `run_generator.py` where each file is generated at a time. Many of the generation scripts will generate both the `.cpp` source and `.h` header.

The Vulkan code is generated from [vk.xml](https://github.com/KhronosGroup/Vulkan-Headers/blob/main/registry/vk.xml) and uses the python helper functions in the `Vulkan-Headers/registry` folder.

The SPIR-V code is generated from [SPIR-V Grammar](https://github.com/KhronosGroup/SPIRV-Headers/blob/main/include/spirv/unified1/spirv.core.grammar.json)

## Implementation Details

The `Vulkan-Headers/registry` generation scripts biggest issue is it's designed to generate one file at a time.
The Validation Layers became very messy as each generated file had to re-parse this and try to create its own containers.
The new flow was designed to still make use of the `registry` generation file, but allow a more maintainable way to find data when one only wants to add a little extra code to generation.

The `base_generator.py` and `vulkan_object.py` are were added to help reduce the work needed for each script.

Before the workflow was:

1. `SomethingOutputGenerator::beginFile()` (in `./scripts/`)
2. `OutputGenerator::beginFile()` (in `./external/Vulkan-Headers/registry/`)
3. `SomethingOutputGenerator::beginFeatures()`
4. `OutputGenerator::beginFeatures()`
5. `SomethingOutputGenerator::genCmd()` (or `genGroup`,`genStruc`,`genTyp`,`genEnum`, etc)
6. `OutputGenerator::genCmd()`
7. repeat step 3-6
8. `SomethingOutputGenerator::endFile()`
9. `OutputGenerator::endFile()`

This is an issue because having to decide to write things out to the file during a `genCmd` or `endFile` call gets messy.

The new flow creates a seperate base class so the workflow now looks like:

1. `BaseGenerator::beginFile()`
2. `OutputGenerator::beginFile()` (in `./external/Vulkan-Headers/registry/`)
3. `BaseGenerator::beginFeatures()`
4. `OutputGenerator::beginFeatures()`
5. `BaseGenerator::genCmd()`
6. `OutputGenerator::genCmd()`
7. repeat step 3-6
8. `SomethingOutputGenerator::generate()` (single function per generator)
9. `OutputGenerator::endFile()`

The big difference is `SomethingOutputGenerator` (ex. `CommandValidationOutputGenerator`) only has to be called  **once** at the end.
This means each generator script doesn't have to worry about understanding how the `registry` file works.

This is possible because of the new `class VulkanObject()`

The `VulkanObject` makes use of the [Python 3.7 Dataclasses](https://docs.python.org/3/library/dataclasses.html) to enforce a schema so developers don't have to go guessing what each random object from the various `registry` is. Most of the schema is derived from the [Spec's registry.rnc](https://github.com/KhronosGroup/Vulkan-Docs/blob/main/xml/registry.rnc) file. This file is used to enforce the `vk.xml` schema and serves as a good understanding of what will be in each element of the XML.

If a developer needs something new, it can be added in the `VulkanObject` class. This provides 2 large advantages

1. Code to create a container around the XML is properly reused between scripts
2. Only one file (`base_generator.py`) use to understand the inner working of the `registry`. A developer can just view the `vulkan_object.py` file to see what it can grab in the single pass
