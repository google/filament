=====================================
HLSL to SPIR-V Feature Mapping Manual
=====================================

.. contents::
   :local:
   :depth: 3

Introduction
============

This document describes the mappings from HLSL features to SPIR-V for Vulkan
adopted by the SPIR-V codegen. For how to build, use, or contribute to the
SPIR-V codegen and its internals, please see the
`wiki <https://github.com/Microsoft/DirectXShaderCompiler/wiki/SPIR%E2%80%90V-CodeGen>`_
page.

`SPIR-V <https://www.khronos.org/registry/spir-v/>`_ is a binary intermediate
language for representing graphical-shader stages and compute kernels for
multiple Khronos APIs, such as Vulkan, OpenGL, and OpenCL. At the moment we
only intend to support the Vulkan flavor of SPIR-V.

DirectXShaderCompiler is the reference compiler for HLSL. Adding SPIR-V codegen
in DirectXShaderCompiler will enable the usage of HLSL as a frontend language
for Vulkan shader programming. Sharing the same code base also means we can
track the evolution of HLSL more closely and always deliver the best of HLSL to
developers. Moreover, developers will also have a unified compiler toolchain for
targeting both DirectX and Vulkan. We believe this effort will benefit the
general graphics ecosystem.

Note that this document is expected to be an ongoing effort and grow as we
implement more and more HLSL features.

Overview
========

Although they share the same basic concepts, DirectX and Vulkan are still
different graphics APIs with semantic gaps. HLSL is the native shading language
for DirectX, so certain HLSL features do not have corresponding mappings in
Vulkan, and certain Vulkan specific information does not have native ways to
express in HLSL source code. This section describes the general translation
paradigms and how we close some of the major semantic gaps.

Note that the term "semantic" is overloaded. In HLSL, it can mean the string
attached to shader input or output. For such cases, we refer it as "HLSL
semantic" or "semantic string". For other cases, we just use the normal
"semantic" term.

Shader entry function
---------------------

HLSL entry functions can read data from the previous shader stage and write
data to the next shader stage via function parameters and return value. On the
contrary, Vulkan requires all SPIR-V entry functions taking no parameters and
returning void. All data passing between stages should use global variables
in the ``Input`` and ``Output`` storage class.

To handle this difference, we emit a wrapper function as the SPIR-V entry
function around the HLSL source code entry function. The wrapper function is
responsible to read data from SPIR-V ``Input`` global variables and prepare
them to the types required in the source code entry function signature, call
the source code entry function, and then decompose the contents in return value
(and ``out``/``inout`` parameters) to the types required by the SPIR-V
``Output`` global variables, and then write out. For details about the wrapper
function, please refer to the `entry function wrapper`_ section.

Shader stage IO interface matching
----------------------------------

HLSL leverages semantic strings to link variables and pass data between shader
stages. Great flexibility is allowed as for how to use the semantic strings.
They can appear on function parameters, function returns, and struct members.
In Vulkan, linking variables and passing data between shader stages is done via
numeric ``Location`` decorations on SPIR-V global variables in the ``Input`` and
``Output`` storage class.

To help handling such differences, we provide `Vulkan specific attributes`_ to
let the developer to express precisely their intents. The compiler will also try
its best to deduce the mapping from semantic strings to SPIR-V ``Location``
numbers when such explicit Vulkan specific attributes are absent. Please see the
`HLSL semantic and Vulkan Location`_ section for more details about the mapping
and ``Location`` assignment.

What makes the story complicated is Vulkan's strict requirements on interface
matching. Basically, a variable in the previous stage is considered a match to
a variable in the next stage if and only if they are decorated with the same
``Location`` number and with the exact same type, except for the outermost
arrayness in hull/domain/geometry shader, which can be ignored regarding
interface matching. This is causing problems together with the flexibility of
HLSL semantic strings.

Some HLSL system-value (SV) semantic strings will be mapped into SPIR-V
variables with builtin decorations, some are not. HLSL non-SV semantic strings
should all be mapped to SPIR-V variables without builtin decorations (but with
``Location`` decorations).

With these complications, if we are grouping multiple semantic strings in a
struct in the HLSL source code, that struct should be flattened and each of
its members should be mapped separately. For example, for the following:

.. code:: hlsl

  struct T {
    float2 clip0 : SV_ClipDistance0;
    float3 cull0 : SV_CullDistance0;
    float4 foo   : FOO;
  };

  struct S {
    float4 pos   : SV_Position;
    float2 clip1 : SV_ClipDistance1;
    float3 cull1 : SV_CullDistance1;
    float4 bar   : BAR;
    T      t;
  };

If we have an ``S`` input parameter in pixel shader, we should flatten it
recursively to generate five SPIR-V ``Input`` variables. Three of them are
decorated by the ``Position``, ``ClipDistance``, ``CullDistance`` builtin,
and two of them are decorated by the ``Location`` decoration. (Note that
``clip0`` and ``clip1`` are concatenated, also ``cull0`` and ``cull1``.
The ``ClipDistance`` and ``CullDistance`` builtins are special and explained
in the `ClipDistance & CullDistance`_ section.)

Flattening is infective because of Vulkan interface matching rules. If we
flatten a struct in the output of a previous stage, which may create multiple
variables decorated with different ``Location`` numbers, we also need to
flatten it in the input of the next stage. otherwise we may have ``Location``
mismatch even if we share the same definition of the struct. Because
hull/domain/geometry shader is optional, we can have different chains of shader
stages, which means we need to flatten all shader stage interfaces. For
hull/domain/geometry shader, their inputs/outputs have an additional arrayness.
So if we are seeing an array of structs in these shaders, we need to flatten
them into arrays of its fields.

Vulkan specific features
------------------------

We try to implement Vulkan specific features using the most intuitive and
non-intrusive ways in HLSL, which means we will prefer native language
constructs when possible. If that is inadequate, we then consider attaching
`Vulkan specific attributes`_ to them, or introducing new syntax.

Descriptors
~~~~~~~~~~~

The compiler provides multiple mechanisms to specify which Vulkan descriptor
a particular resource binds to.

In the source code, you can use the ``[[vk::binding(X[, Y])]]`` and
``[[vk::counter_binding(X)]]`` attribute. The native ``:register()`` attribute
is also respected.

On the command-line, you can use the ``-fvk-{b|s|t|u}-shift`` or
``-fvk-bind-register`` option.

If you can modify the source code, the ``[[vk::binding(X[, Y])]]`` and
``[[vk::counter_binding(X)]]`` attribute gives you find-grained control over
descriptor assignment.

If you cannot modify the source code, you can use command-line options to change
how ``:register()`` attribute is handled by the compiler. ``-fvk-bind-register``
lets you to specify the descriptor for the source at a certain register.
``-fvk-{b|s|t|u}-shift`` lets you to apply shifts to all register numbers
of a certain register type. They cannot be used together, though.

When the ``[[vk::combinedImageSampler]]`` attribute is applied, only the
``-fvk-t-shift`` value will be used to apply shifts to combined texture and
sampler resource bindings and any ``-fvk-s-shift`` value will be ignored.

Without attribute and command-line option, ``:register(xX, spaceY)`` will be
mapped to binding ``X`` in descriptor set ``Y``. Note that register type ``x``
is ignored, so this may cause overlap.

The more specific a mechanism is, the higher precedence it has, and command-line
option has higher precedence over source code attribute.

For more details, see `HLSL register and Vulkan binding`_, `Vulkan specific
attributes`_, and `Vulkan-specific options`_.

Subpass inputs
~~~~~~~~~~~~~~

Within a Vulkan `rendering pass <https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#renderpass>`_,
a subpass can write results to an output target that can then be read by the
next subpass as an input subpass. The "Subpass Input" feature regards the
ability to read an output target.

Subpasses are read through two new builtin resource types, available only in
pixel shader:

.. code:: hlsl

  class SubpassInput<T> {
    T SubpassLoad();
  };

  class SubpassInputMS<T> {
    T SubpassLoad(int sampleIndex);
  };

In the above, ``T`` is a scalar or vector type. If omitted, it will defaults to
``float4``.

Subpass inputs are implicitly addressed by the pixel's (x, y, layer) coordinate.
These objects support reading the subpass input through the methods as shown
in the above.

A subpass input is selected by using a new attribute ``vk::input_attachment_index``.
For example:

.. code:: hlsl

  [[vk::input_attachment_index(i)]] SubpassInput input;

A ``vk::input_attachment_index`` of ``i`` selects the ith entry in the input
pass list. A subpass input without a ``vk::input_attachment_index`` will be
associated with the depth/stencil attachment. (See Vulkan API spec for more
information.)

Push constants
~~~~~~~~~~~~~~

Vulkan push constant blocks are represented using normal global variables of
struct types in HLSL. The variables (not the underlying struct types) should be
annotated with the ``[[vk::push_constant]]`` attribute.

Please note as per the requirements of Vulkan, "there must be no more than one
push constant block statically used per shader entry point."

Specialization constants
~~~~~~~~~~~~~~~~~~~~~~~~

To use Vulkan specialization constants, annotate global constants with the
``[[vk::constant_id(X)]]`` attribute. For example,

.. code:: hlsl

  [[vk::constant_id(1)]] const bool  specConstBool  = true;
  [[vk::constant_id(2)]] const int   specConstInt   = 42;
  [[vk::constant_id(3)]] const float specConstFloat = 1.5;

Shader Record Buffer
~~~~~~~~~~~~~~~~~~~~

SPV_NV_ray_tracing exposes user managed buffer in shader binding table by
using storage class ShaderRecordBufferNV. ConstantBuffer or cbuffer blocks
can now be mapped to this storage class under HLSL by using
``[[vk::shader_record_nv]]`` annotation. It is applicable only on ConstantBuffer
and cbuffer declarations.

Please note as per the requirements of VK_NV_ray_tracing, "there must be no
more than one shader_record_nv block statically used per shader entry point
otherwise results are undefined."

The official Khronos ray tracing extension also comes with a SPIR-V storage class
that has the same functionality. The ``[[vk::shader_record_ext]]`` annotation can
be used when targeting the SPV_KHR_ray_tracing extension.

Builtin variables
~~~~~~~~~~~~~~~~~

Some of the Vulkan builtin variables have no equivalents in native HLSL
language. To support them, ``[[vk::builtin("<builtin>")]]`` is introduced.
Right now the following ``<builtin>`` are supported:

* ``PointSize``: The GLSL equivalent is ``gl_PointSize``.
* ``HelperInvocation``: For Vulkan 1.3 or above, we use its GLSL equivalent
  ``gl_HelperInvocation`` and decorate it with ``HelperInvocation`` builtin
  since Vulkan 1.3 or above supports ``Volatile`` decoration for builtin
  variables. For Vulkan 1.2 or earlier, we do not create a builtin variable for
  ``HelperInvocation``. Instead, we create a variable with ``Private`` storage
  class and set its value as the result of `OpIsHelperInvocationEXT <https://htmlpreview.github.io/?https://github.com/KhronosGroup/SPIRV-Registry/blob/master/extensions/EXT/SPV_EXT_demote_to_helper_invocation.html#OpIsHelperInvocationEXT>`_
  instruction.
* ``BaseVertex``: The GLSL equivalent is ``gl_BaseVertexARB``.
  Need ``SPV_KHR_shader_draw_parameters`` extension.
* ``BaseInstance``: The GLSL equivalent is ``gl_BaseInstanceARB``.
  Need ``SPV_KHR_shader_draw_parameters`` extension.
* ``DrawIndex``: The GLSL equivalent is ``gl_DrawIDARB``.
  Need ``SPV_KHR_shader_draw_parameters`` extension.
* ``DeviceIndex``: The GLSL equivalent is ``gl_DeviceIndex``.
  Need ``SPV_KHR_device_group`` extension.
* ``ViewportMaskNV``: The GLSL equivalent is ``gl_ViewportMask``.

Please see Vulkan spec. `15.9. Built-In Variables <https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#interfaces-builtin-variables>`_
for detailed explanation of these builtins.

Supported extensions
~~~~~~~~~~~~~~~~~~~~

* SPV_KHR_16bit_storage
* SPV_KHR_device_group
* SPV_KHR_fragment_shading_rate
* SPV_KHR_multivew
* SPV_KHR_post_depth_coverage
* SPV_KHR_non_semantic_info
* SPV_KHR_shader_draw_parameters
* SPV_KHR_ray_tracing
* SPV_KHR_shader_clock
* SPV_EXT_demote_to_helper_invocation
* SPV_EXT_descriptor_indexing
* SPV_EXT_fragment_fully_covered
* SPV_EXT_fragment_invocation_density
* SPV_EXT_fragment_shader_interlock
* SPV_EXT_mesh_shader
* SPV_EXT_shader_stencil_support
* SPV_EXT_shader_viewport_index_layer
* SPV_AMD_shader_early_and_late_fragment_tests
* SPV_GOOGLE_hlsl_functionality1
* SPV_GOOGLE_user_type
* SPV_NV_ray_tracing
* SPV_NV_mesh_shader
* SPV_KHR_ray_query
* SPV_EXT_shader_image_int64
* SPV_KHR_fragment_shader_barycentric
* SPV_KHR_physical_storage_buffer
* SPV_KHR_vulkan_memory_model
* SPV_KHR_compute_shader_derivatives
* SPV_NV_compute_shader_derivatives
* SPV_KHR_maximal_reconvergence
* SPV_KHR_float_controls
* SPV_NV_shader_subgroup_partitioned
* SPV_KHR_quad_control

Vulkan specific attributes
--------------------------

`C++ attribute specifier sequence <http://en.cppreference.com/w/cpp/language/attributes>`_
is a non-intrusive way of providing Vulkan specific information in HLSL.

The namespace ``vk`` will be used for all Vulkan attributes:

- ``location(X)``: For specifying the location (``X``) numbers for stage
  input/output variables. Allowed on function parameters, function returns,
  and struct fields.
- ``binding(X[, Y])``: For specifying the descriptor set (``Y``) and binding
  (``X``) numbers for resource variables. The descriptor set (``Y``) is
  optional; if missing, it will be set to 0. Allowed on global variables.
- ``counter_binding(X)``: For specifying the binding number (``X``) for the
  associated counter for RW/Append/Consume structured buffer. The descriptor
  set number for the associated counter is always the same as the main resource.
- ``push_constant``: For marking a variable as the push constant block. Allowed
  on global variables of struct type. At most one variable can be marked as
  ``push_constant`` in a shader.
- ``offset(X)``: For manually layout struct members. Annotating a struct member
  with this attribute will force the compiler to put the member at offset ``X``
  w.r.t. the beginning of the struct. Only allowed on struct members.
- ``constant_id(X)``: For marking a global constant as a specialization constant.
  Allowed on global variables of boolean/integer/float types.
- ``input_attachment_index(X)``: To associate the Xth entry in the input pass
  list to the annotated object. Only allowed on objects whose type are
  ``SubpassInput`` or ``SubpassInputMS``.
- ``builtin("X")``: For specifying an entity should be translated into a certain
  Vulkan builtin variable. Allowed on function parameters, function returns,
  and struct fields.
- ``index(X)``: For specifying the index at a specific pixel shader output
  location. Used for dual-source blending.
- ``post_depth_coverage``: The input variable decorated with SampleMask will
  reflect the result of the EarlyFragmentTests. Only valid on pixel shader entry points.
- ``combinedImageSampler``: For specifying a Texture (e.g., ``Texture2D``,
  ``Texture1DArray``, ``TextureCube``) and ``SamplerState`` to use the combined image
  sampler (or sampled image) type with the same descriptor set and binding numbers (see
  `wiki page <https://github.com/microsoft/DirectXShaderCompiler/wiki/Vulkan-combined-image-sampler-type>`_
  for more detail).
- ``early_and_late_tests``: Marks an entry point as enabling early and late depth
  tests. If depth is written via ``SV_Depth``, ``depth_unchanged`` must also be specified
  (``SV_DepthLess`` and ``SV_DepthGreater`` can be written freely). If a stencil reference
  value is written via ``SV_StencilRef``, one of ``stencil_ref_unchanged_front``,
  ``stencil_ref_greater_equal_front``, or ``stencil_ref_less_equal_front`` and
  one of ``stencil_ref_unchanged_back``, ``stencil_ref_greater_equal_back``, or
  ``stencil_ref_less_equal_back`` must be specified.
- ``depth_unchanged``: Specifies that any depth written to ``SV_Depth`` will not
  invalidate the result of early depth tests. Sets the ``DepthUnchanged`` execution
  mode in SPIR-V. Only valid on pixel shader entry points.
- ``stencil_ref_unchanged_front``: Specifies that any stencil ref written to
  ``SV_StencilRef`` will not invalidate the result of early stencil tests when
  the fragment is front facing. Sets the ``StencilRefUnchangedFrontAMD`` execution
  mode in SPIR-V. Only valid on pixel shader entry points.
- ``stencil_ref_greater_equal_front``: Specifies that any stencil ref written to
  ``SV_StencilRef`` will be greater than or equal to the stencil reference value
  set by the API when the fragment is front facing. Sets the ``StencilRefGreaterFrontAMD``
  execution mode in SPIR-V. Only valid on pixel shader entry points.
- ``stencil_ref_less_equal_front``: Specifies that any stencil ref written to
  ``SV_StencilRef`` will be less than or equal to the stencil reference value
  set by the API when the fragment is front facing. Sets the ``StencilRefLessFrontAMD``
  execution mode in SPIR-V. Only valid on pixel shader entry points.
- ``stencil_ref_unchanged_back``: Specifies that any stencil ref written to
  ``SV_StencilRef`` will not invalidate the result of early stencil tests when
  the fragment is back facing. Sets the ``StencilRefUnchangedBackAMD`` execution
  mode in SPIR-V. Only valid on pixel shader entry points.
- ``stencil_ref_greater_equal_back``: Specifies that any stencil ref written to
  ``SV_StencilRef`` will be greater than or equal to the stencil reference value
  set by the API when the fragment is back facing. Sets the ``StencilRefGreaterBackAMD``
  execution mode in SPIR-V. Only valid on pixel shader entry points.
- ``stencil_ref_less_equal_back``: Specifies that any stencil ref written to
  ``SV_StencilRef`` will be less than or equal to the stencil reference value
  set by the API when the fragment is back facing. Sets the ``StencilRefLessBackAMD``
  execution mode in SPIR-V. Only valid on pixel shader entry points.

Only ``vk::`` attributes in the above list are supported. Other attributes will
result in warnings and be ignored by the compiler. All C++11 attributes will
only trigger warnings and be ignored if not compiling towards SPIR-V.

For example, to specify the layout of resource variables and the location of
interface variables:

.. code:: hlsl

  struct S { ... };

  [[vk::binding(X, Y), vk::counter_binding(Z)]]
  RWStructuredBuffer<S> mySBuffer;

  [[vk::location(M)]] float4
  main([[vk::location(N)]] float4 input: A) : B
  { ... }

Macros for SPIR-V
-----------------

If SPIR-V CodeGen is enabled and ``-spirv`` flag is used as one of the command
line options (meaning that "generates SPIR-V code"), it defines an implicit
macro ``__spirv__``. For example, this macro definition can be used for SPIR-V
specific part of the HLSL code:

.. code:: hlsl

  #ifdef __spirv__
  [[vk::binding(X, Y), vk::counter_binding(Z)]]
  #endif
  RWStructuredBuffer<S> mySBuffer;

When the ``-spirv`` flag is used, the ``-fspv-target-env`` option will
implicitly define the macros ``__SPIRV_MAJOR_VERSION__`` and
``__SPIRV_MINOR_VERSION__``, which will be integers representing the major and
minor version of the SPIR-V being generated. This can be used to enable code that uses a feature
only for environments where that feature is available.

SPIR-V version and extension
----------------------------

SPIR-V CodeGen provides two command-line options for fine-grained SPIR-V target
environment (hence SPIR-V version) and SPIR-V extension control:

- ``-fspv-target-env=``: for specifying SPIR-V target environment
- ``-fspv-extension=``: for specifying allowed SPIR-V extensions

``-fspv-target-env=`` accepts a Vulkan target environment (see ``-help`` for
supported values). If such an option is not given, the CodeGen defaults to
``vulkan1.0``. When targeting ``vulkan1.0``, trying to use features that are only
available in Vulkan 1.1 (SPIR-V 1.3), like `Shader Model 6.0 wave intrinsic <https://github.com/microsoft/directxshadercompiler/wiki/wave-intrinsics>`_,
will trigger a compiler error.

If ``-fspv-extension=`` is not specified, the CodeGen will select suitable
SPIR-V extensions to translate the source code. Otherwise, only extensions
supplied via ``-fspv-extension=`` will be used. If that does not suffice, errors
will be emitted explaining what additional extensions are required to translate
what specific feature in the source code. If you want to allow all KHR
extensions, you can use ``-fspv-extension=KHR``.

Legalization, optimization, validation
--------------------------------------

After initial translation of the HLSL source code, SPIR-V CodeGen will further
conduct legalization (if needed), optimization (if requested), and validation
(if not turned off). All these three stages are outsourced to `SPIRV-Tools <https://github.com/KhronosGroup/SPIRV-Tools>`_.
Here are the options controlling these stages:

* ``-fcgl``: turn off legalization and optimization
* ``-Od``: turn off optimization
* ``-Vd``: turn off validation

Legalization
~~~~~~~~~~~~

HLSL is a fairly permissive language considering the flexibility it provides for
manipulating resource objects. The developer can create local copies, pass
them around as function parameters and return values, as long as after certain
transformations (function inlining, constant evaluation and propagating, dead
code elimination, etc.), the compiler can remove all temporary copies and
pinpoint all uses to unique global resource objects.

Resulting from the above property of HLSL, if we translate into SPIR-V for
Vulkan literally from the input HLSL source code, we will sometimes generate
illegal SPIR-V. Certain transformations are needed to legalize the literally
translated SPIR-V. Performing such transformations at the frontend AST level
is cumbersome or impossible (e.g., function inlining). They are better to be
conducted at SPIR-V level. Therefore, legalization is delegated to SPIRV-Tools.

Specifically, we need to legalize the following HLSL source code patterns:

* Using resource types in struct types
* Creating aliases of global resource objects
* Control flows invovling the above cases

Legalization transformations will not run unless the above patterns are
encountered in the source code.

For more details, please see the `SPIR-V cookbook <https://github.com/Microsoft/DirectXShaderCompiler/tree/main/docs/SPIRV-Cookbook.rst>`_,
which contains examples of what HLSL code patterns will be accepted and
generate valid SPIR-V for Vulkan.

Optimization
~~~~~~~~~~~~

Optimization is also delegated to SPIRV-Tools. Right now there are no difference
between optimization levels greater than zero; they will all invoke the same
optimization recipe. That is, the recipe behind ``spirv-opt -O``.  If you want to
run a custom optimization recipe, you can do so using the command line option
``-Oconfig=`` and specifying a comma-separated list of your desired passes.
The passes are invoked in the specified order.

For example, you can specify ``-Oconfig=--loop-unroll,--scalar-replacement=300,--eliminate-dead-code-aggressive``
to firstly invoke loop unrolling, then invoke scalar replacement of aggregates,
lastly invoke aggressive dead code elimination. All valid options to
``spirv-opt`` are accepted as components to the comma-separated list.

Here are the typical passes in alphabetical order:

* ``--ccp``
* ``--cfg-cleanup``
* ``--convert-local-access-chains``
* ``--copy-propagate-arrays``
* ``--eliminate-dead-branches``
* ``--eliminate-dead-code-aggressive``
* ``--eliminate-dead-functions``
* ``--eliminate-local-multi-store``
* ``--eliminate-local-single-block``
* ``--eliminate-local-single-store``
* ``--flatten-decorations``
* ``--if-conversion``
* ``--inline-entry-points-exhaustive``
* ``--local-redundancy-elimination``
* ``--loop-fission``
* ``--loop-fusion``
* ``--loop-unroll``
* ``--loop-unroll-partial=[<n>]``
* ``--loop-peeling`` (requires ``--loop-peeling-threshold``)
* ``--merge-blocks``
* ``--merge-return``
* ``--loop-unswitch``
* ``--private-to-local``
* ``--reduce-load-size``
* ``--redundancy-elimination``
* ``--remove-duplicates``
* ``--replace-invalid-opcode``
* ``--ssa-rewrite``
* ``--scalar-replacement[=<n>]``
* ``--simplify-instructions``
* ``--vector-dce``


Besides, there are two special batch options; each stands for a recommended
recipe by itself:

* ``-O``: A bunch of passes in an appropriate order that attempt to improve
  performance of generated code. Same as ``spirv-opt -O``. Also same as SPIR-V
  CodeGen's default recipe.
* ``-Os``: A bunch of passes in an appropriate order that attempt to reduce the
  size of the generated code. Same as ``spirv-opt -Os``.

So if you want to run loop unrolling additionally after the default optimization
recipe, you can specify ``-Oconfig=-O,--loop-unroll``.

For the whole list of accepted passes and details about each one, please see
``spirv-opt``'s help manual (``spirv-opt --help``), or the SPIRV-Tools `optimizer header file <https://github.com/KhronosGroup/SPIRV-Tools/blob/main/include/spirv-tools/optimizer.hpp>`_.

Validation
~~~~~~~~~~

Validation is turned on by default as the last stage of SPIR-V CodeGen. Failing
validation, which indicates there is a CodeGen bug, will trigger a fatal error.
Please file an issue if you see that.

Debugging
---------

By default, the compiler will only emit names for types and variables as debug
information, to aid reading of the generated SPIR-V. The ``-Zi`` option will
let the compiler emit the following additional debug information:

* Full path of the main source file using ``OpSource``
* Preprocessed source code using ``OpSource`` and ``OpSourceContinued``
* Line information for certain instructions using ``OpLine`` (WIP)
* DXC Git commit hash using ``OpModuleProcessed`` (requires Vulkan 1.1)
* DXC command-line options used to compile the shader using ``OpModuleProcessed``
  (requires Vulkan 1.1)

We chose to embed preprocessed source code instead of original source code to
avoid pulling in lots of contents unrelated to the current entry point, and
boilerplate contents generated by engines. We may add a mode for selecting
between preprocessed single source code and original separated source code in
the future.

One thing to note is that to keep the line numbers in consistent with the
embedded source, the compiler is invoked twice; the first time is for
preprocessing the source code, and the second time is for feeding the
preprocessed source code as input for a whole compilation. So using ``-Zi``
means performance penality.

If you want to have fine-grained control over the categories of emitted debug
information, you can use ``-fspv-debug=``. It accepts:

* ``file``: for emitting full path of the main source file
* ``source``: for emitting preprocessed source code (turns on ``file`` implicitly)
* ``line``: for emitting line information (turns on ``source`` implicitly)
* ``tool``: for emitting DXC Git commit hash and command-line options

These ``-fspv-debug=`` options overrule ``-Zi``. And you can provide multiple
instances of ``-fspv-debug=``. For example, you can use ``-fspv-debug=file
-fspv-debug=tool`` to turn on emitting file path and DXC information; source
code and line information will not be emitted.

If you want to generate `NonSemantic.Shader.DebugInfo.100 <http://htmlpreview.github.io/?https://github.com/KhronosGroup/SPIRV-Registry/blob/main/nonsemantic/NonSemantic.Shader.DebugInfo.100.html>`_ extended instructions, you can use
``-fspv-debug=vulkan-with-source``. These instructions support source-level
shader debugging with tools such as RenderDoc, even if the SPIR-V is optimized.
This option overrules the other ``-fspv-debug`` options above.

Reflection
----------

Making reflection easier is one of the goals of SPIR-V CodeGen. This section
provides guidelines about how to reflect on certain facts.

Note that we generate ``OpName``/``OpMemberName`` instructions for various
types/variables both explicitly defined in the source code and interally created
by the compiler. These names are primarily for debugging purposes in the
compiler. They have "no semantic impact and can safely be removed" according
to the SPIR-V spec. And they are subject to changes without notice. So we do
not suggest to use them for reflection.

Source code shader profile
~~~~~~~~~~~~~~~~~~~~~~~~~~

The source code shader profile version can be re-discovered by the "Version"
operand in ``OpSource`` instruction. For ``*s_<major>_<minor>``, the "Verison"
operand in ``OpSource`` will be set as ``<major>`` * 100 + ``<minor>`` * 10.
For example, ``vs_5_1`` will have 510, ``ps_6_2`` will have 620.

HLSL Semantic
~~~~~~~~~~~~~

HLSL semantic strings are by default not emitted into the SPIR-V binary module.
If you need them, by specifying ``-fspv-reflect``, the compiler will use
the ``Op*DecorateStringGOOGLE`` instruction in `SPV_GOOGLE_hlsl_funtionality1 <https://github.com/KhronosGroup/SPIRV-Registry/blob/main/extensions/GOOGLE/SPV_GOOGLE_hlsl_functionality1.asciidoc>`_
extension to emit them.

HLSL User Types
~~~~~~~~~~~~~~~

HLSL type information is by default not emitted into the SPIR-V binary module.
If you need them, by specifying ``-fspv-reflect``, the compiler will emit
``OpDecorateString*`` instructions with a ``UserTypeGOOGLE`` decoration and the
`SPV_GOOGLE_user_type <https://github.com/KhronosGroup/SPIRV-Registry/blob/main/extensions/GOOGLE/SPV_GOOGLE_user_type.asciidoc>`_
extension. A string name for the unambiguous type of the decorated object will
be included in the user's source using the lowercase type name followed by
template params. For example, ``Texture2DMSArray<float4, 64> arr`` would be
decorated with ``OpDecorateString %arr UserTypeGOOGLE "texture2dmsarray:<float4,64>"``.

Counter buffers for RW/Append/Consume StructuredBuffer
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The association between a counter buffer and its main RW/Append/Consume
StructuredBuffer is conveyed by ``OpDecorateId <structured-buffer-id>
HLSLCounterBufferGOOGLE <counter-buffer-id>`` instruction from the
`SPV_GOOGLE_hlsl_funtionality1 <https://github.com/KhronosGroup/SPIRV-Registry/blob/main/extensions/GOOGLE/SPV_GOOGLE_hlsl_functionality1.asciidoc>`_
extension. This information is by default missing; you need to specify
``-fspv-reflect`` to direct the compiler to emit them.

Read-only vs. read-write resource types
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

There are no clear and consistent decorations in the SPIR-V to show whether a
resource type is translated from a read-only (RO) or read-write (RW) HLSL
resource type. Instead, you need to use different checks for reflecting different
resource types:

* HLSL samplers: RO.
* HLSL ``Buffer``/``RWBuffer``/``Texture*``/``RWTexture*``: Check the "Sampled"
  operand in the ``OpTypeImage`` instruction they translated into. "2" means RW,
  "1" means RO.
* HLSL constant/texture/structured/byte buffers: Check both ``Block``/``BufferBlock``
  and ``NonWritable`` decoration. If decorated with ``Block`` (``cbuffer`` &
  ``ConstantBuffer``), then RO; if decorated with ``BufferBlock`` and ``NonWritable``
  (``tbuffer``, ``TextureBuffer``, ``StructuredBuffer``), then RO; Otherwise, RW.


HLSL Types
==========

This section lists how various HLSL types are mapped.

Normal scalar types
-------------------

`Normal scalar types <https://msdn.microsoft.com/en-us/library/windows/desktop/bb509646(v=vs.85).aspx>`_
in HLSL are relatively easy to handle and can be mapped directly to SPIR-V
type instructions:

============================== ======================= ================== ===========
      HLSL                      Command Line Option           SPIR-V       Capability
============================== ======================= ================== ===========
``bool``                                               ``OpTypeBool``
``int``/``int32_t``                                    ``OpTypeInt 32 1``
``int16_t``                    ``-enable-16bit-types`` ``OpTypeInt 16 1`` ``Int16``
``uint``/``dword``/``uin32_t``                         ``OpTypeInt 32 0``
``uint16_t``                   ``-enable-16bit-types`` ``OpTypeInt 16 0`` ``Int16``
``half``                                               ``OpTypeFloat 32``
``half``/``float16_t``         ``-enable-16bit-types`` ``OpTypeFloat 16`` ``Float16``
``float``/``float32_t``                                ``OpTypeFloat 32``
``snorm float``                                        ``OpTypeFloat 32``
``unorm float``                                        ``OpTypeFloat 32``
``double``/``float64_t``                               ``OpTypeFloat 64`` ``Float64``
============================== ======================= ================== ===========

Please note that ``half`` is translated into 32-bit floating point numbers
if without ``-enable-16bit-types`` because MSDN says that "this data type
is provided only for language compatibility. Direct3D 10 shader targets map
all ``half`` data types to ``float`` data types."

Minimal precision scalar types
------------------------------

HLSL also supports various
`minimal precision scalar types <https://msdn.microsoft.com/en-us/library/windows/desktop/bb509646(v=vs.85).aspx>`_,
which graphics drivers can implement by using any precision greater than or
equal to their specified bit precision.
There are no direct mappings in SPIR-V for these types. We translate them into
the corresponding 16-bit or 32-bit scalar types with the ``RelaxedPrecision`` decoration.
We use the 16-bit variants if '-enable-16bit-types' command line option is present.
For more information on these types, please refer to:
https://github.com/Microsoft/DirectXShaderCompiler/wiki/16-Bit-Scalar-Types

============== ======================= ================== ==================== ============
    HLSL        Command Line Option          SPIR-V            Decoration       Capability 
============== ======================= ================== ==================== ============
``min16float``                         ``OpTypeFloat 32`` ``RelaxedPrecision``
``min10float``                         ``OpTypeFloat 32`` ``RelaxedPrecision``
``min16int``                           ``OpTypeInt 32 1`` ``RelaxedPrecision``
``min12int``                           ``OpTypeInt 32 1`` ``RelaxedPrecision``
``min16uint``                          ``OpTypeInt 32 0`` ``RelaxedPrecision``
``min16float`` ``-enable-16bit-types`` ``OpTypeFloat 16``                      ``Float16`` 
``min10float`` ``-enable-16bit-types`` ``OpTypeFloat 16``                      ``Float16`` 
``min16int``   ``-enable-16bit-types`` ``OpTypeInt 16 1``                      ``Int16``
``min12int``   ``-enable-16bit-types`` ``OpTypeInt 16 1``                      ``Int16``
``min16uint``  ``-enable-16bit-types`` ``OpTypeInt 16 0``                      ``Int16``
============== ======================= ================== ==================== ============

Vectors and matrices
--------------------

`Vectors <https://msdn.microsoft.com/en-us/library/windows/desktop/bb509707(v=vs.85).aspx>`_
and `matrices <https://msdn.microsoft.com/en-us/library/windows/desktop/bb509623(v=vs.85).aspx>`_
are translated into:

==================================== ====================================================
              HLSL                                         SPIR-V
==================================== ====================================================
``|type|N`` (``N`` > 1)              ``OpTypeVector |type| N``
``|type|1``                          The scalar type for ``|type|``
``|type|MxN`` (``M`` > 1, ``N`` > 1) ``%v = OpTypeVector |type| N`` ``OpTypeMatrix %v M``
``|type|Mx1`` (``M`` > 1)            ``OpTypeVector |type| M``
``|type|1xN`` (``N`` > 1)            ``OpTypeVector |type| N``
``|type|1x1``                        The scalar type for ``|type|``
==================================== ====================================================

The above table is for float matrices.

A MxN HLSL float matrix is translated into a SPIR-V matrix with M vectors, each with
N elements. Conceptually HLSL matrices are row-major while SPIR-V matrices are
column-major, thus all HLSL matrices are represented by their transposes.
Doing so may require special handling of certain matrix operations:

- **Indexing**: no special handling required. ``matrix[m][n]`` will still access
  the correct element since ``m``/``n`` means the ``m``-th/``n``-th row/column
  in HLSL but ``m``-th/``n``-th vector/element in SPIR-V.
- **Per-element operation**: no special handling required.
- **Matrix multiplication**: need to swap the operands. ``mat1 x mat2`` should
  be translated as ``transpose(mat2) x transpose(mat1)``. Then the result is
  ``transpose(mat1 x mat2)``.
- **Storage layout**: ``row_major``/``column_major`` will be translated into
  SPIR-V ``ColMajor``/``RowMajor`` decoration. This is because HLSL matrix
  row/column becomes SPIR-V matrix column/row. If elements in a row/column are
  packed together, they should be loaded into a column/row correspondingly.

See `Appendix A. Matrix Representation`_ for further explanation regarding these design choices.

Since the ``Shader`` capability in SPIR-V does not allow to parameterize matrix
types with non-floating-point types, a non-floating-point MxN matrix is translated
into an array with M elements, with each element being a vector with N elements.

Structs
-------

`Structs <https://msdn.microsoft.com/en-us/library/windows/desktop/bb509668(v=vs.85).aspx>`_
in HLSL are defined in the a format similar to C structs. They are translated
into SPIR-V ``OpTypeStruct``. Depending on the storage classes of the instances,
a single struct definition may generate multiple ``OpTypeStruct`` instructions
in SPIR-V. For example, for the following HLSL source code:

.. code:: hlsl

  struct S { ... }

  ConstantBuffer<S>   myCBuffer;
  StructuredBuffer<S> mySBuffer;

  float4 main() : A {
    S myLocalVar;
    ...
  }

There will be three different ``OpTypeStruct`` generated, one for each variable
defined in the above source code. This is because the ``OpTypeStruct`` for
both ``myCBuffer`` and ``mySBuffer`` will have layout decorations (``Offset``,
``MatrixStride``, ``ArrayStride``, ``RowMajor``, ``ColMajor``). However, their
layout rules are different (by default); ``myCBuffer`` will use vector-relaxed
OpenGL ``std140`` while ``mySBuffer`` will use vector-relaxed OpenGL ``std430``.
``myLocalVar`` will have its ``OpTypeStruct`` without layout decorations.
Read more about storage classes in the `Constant/Texture/Structured/Byte Buffers`_
section.

Structs used as stage inputs/outputs will have semantics attached to their
members. These semantics are handled in the `entry function wrapper`_.

Structs used as pixel shader inputs can have optional interpolation modifiers
for their members, which will be translated according to the following table:

=========================== ================= =====================
HLSL Interpolation Modifier SPIR-V Decoration   SPIR-V Capability
=========================== ================= =====================
``linear``                  <none>
``centroid``                ``Centroid``
``nointerpolation``         ``Flat``
``noperspective``           ``NoPerspective``
``sample``                  ``Sample``        ``SampleRateShading``
=========================== ================= =====================

Arrays
------

Sized (either explicitly or implicitly) arrays are translated into SPIR-V
`OpTypeArray`. Unsized arrays are translated into `OpTypeRuntimeArray`.

Arrays, if used for external resources (residing in SPIR-V `Uniform` or
`UniformConstant` storage class), will need layout decorations like SPIR-V
`ArrayStride` decoration. For arrays of opaque types, e.g., HLSL textures
or samplers, we don't decorate with `ArrayStride` decorations since there is
no meaningful strides. Similarly for arrays of structured/byte buffers.

User-defined types
------------------

`User-defined types <https://msdn.microsoft.com/en-us/library/windows/desktop/bb509702(v=vs.85).aspx>`_
are type aliases introduced by typedef. No new types are introduced and we can
rely on Clang to resolve to the original types.

Samplers
--------

All `sampler types <https://msdn.microsoft.com/en-us/library/windows/desktop/bb509644(v=vs.85).aspx>`_
will be translated into SPIR-V ``OpTypeSampler``.

SPIR-V ``OpTypeSampler`` is an opaque type that cannot be parameterized;
therefore state assignments on sampler types is not supported (yet).

Textures
--------

`Texture types <https://msdn.microsoft.com/en-us/library/windows/desktop/bb509700(v=vs.85).aspx>`_
are translated into SPIR-V ``OpTypeImage``, with parameters:

======================= ==================== ===== =================== ========== ===== ======= == ======= ================ =================
       HLSL                   Vulkan                                        SPIR-V
----------------------- -------------------------- ------------------------------------------------------------------------------------------
     Texture Type         Descriptor Type    RO/RW    Storage Class        Dim    Depth Arrayed MS Sampled   Image Format      Capability
======================= ==================== ===== =================== ========== ===== ======= == ======= ================ =================
``Texture1D``           Sampled Image         RO   ``UniformConstant`` ``1D``      2       0    0    1     ``Unknown``
``Texture2D``           Sampled Image         RO   ``UniformConstant`` ``2D``      2       0    0    1     ``Unknown``
``Texture3D``           Sampled Image         RO   ``UniformConstant`` ``3D``      2       0    0    1     ``Unknown``
``TextureCube``         Sampled Image         RO   ``UniformConstant`` ``Cube``    2       0    0    1     ``Unknown``
``Texture1DArray``      Sampled Image         RO   ``UniformConstant`` ``1D``      2       1    0    1     ``Unknown``
``Texture2DArray``      Sampled Image         RO   ``UniformConstant`` ``2D``      2       1    0    1     ``Unknown``
``Texture2DMS``         Sampled Image         RO   ``UniformConstant`` ``2D``      2       0    1    1     ``Unknown``
``Texture2DMSArray``    Sampled Image         RO   ``UniformConstant`` ``2D``      2       1    1    1     ``Unknown``
``TextureCubeArray``    Sampled Image         RO   ``UniformConstant`` ``3D``      2       1    0    1     ``Unknown``
``Buffer<T>``           Uniform Texel Buffer  RO   ``UniformConstant`` ``Buffer``  2       0    0    1     Depends on ``T`` ``SampledBuffer``
``RWBuffer<T>``         Storage Texel Buffer  RW   ``UniformConstant`` ``Buffer``  2       0    0    2     Depends on ``T`` ``SampledBuffer``
``RWTexture1D<T>``      Storage Image         RW   ``UniformConstant`` ``1D``      2       0    0    2     Depends on ``T``
``RWTexture2D<T>``      Storage Image         RW   ``UniformConstant`` ``2D``      2       0    0    2     Depends on ``T``
``RWTexture3D<T>``      Storage Image         RW   ``UniformConstant`` ``3D``      2       0    0    2     Depends on ``T``
``RWTexture1DArray<T>`` Storage Image         RW   ``UniformConstant`` ``1D``      2       1    0    2     Depends on ``T``
``RWTexture2DArray<T>`` Storage Image         RW   ``UniformConstant`` ``2D``      2       1    0    2     Depends on ``T``
======================= ==================== ===== =================== ========== ===== ======= == ======= ================ =================

The meanings of the headers in the above table is explained in ``OpTypeImage``
of the SPIR-V spec.

For storage images (e.g. ``RWTexture2D<T>``) and texel buffers (e.g. ``RWBuffer<T>``),
the image format is typically inferred from the data type ``T``. However, the
``-fspv-use-unknown-image-format`` command-line option can be used to change
this behavior. When this option is active, the default format for these
resources becomes ``Unknown`` if not otherwise specified by the
``[[vk::image_format]]`` attribute.

Vulkan specific Image Formats
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Since HLSL lacks the syntax for fully specifying image formats for textures in
SPIR-V, we introduce ``[[vk::image_format("FORMAT")]]`` attribute for texture types.
For example,

.. code:: hlsl

  [[vk::image_format("rgba8")]]
  RWBuffer<float4> Buf;

  [[vk::image_format("rg16f")]]
  RWTexture2D<float2> Tex;

  RWTexture2D<float2> Tex2; // Works like before

``rgba8`` means ``Rgba8`` `SPIR-V Image Format <https://registry.khronos.org/SPIR-V/specs/unified1/SPIRV.html#Image_Format>`_.
The following table lists the mapping between ``FORMAT`` of
``[[vk::image_format("FORMAT")]]`` and its corresponding SPIR-V Image Format.

======================= ============================================
       FORMAT                   SPIR-V Image Format
======================= ============================================
``unknown``             ``Unknown``
``rgba32f``             ``Rgba32f``
``rgba16f``             ``Rgba16f``
``r32f``                ``R32f``
``rgba8``               ``Rgba8``
``rgba8snorm``          ``Rgba8Snorm``
``rg32f``               ``Rg32f``
``rg16f``               ``Rg16f``
``r11g11b10f``          ``R11fG11fB10f``
``r16f``                ``R16f``
``rgba16``              ``Rgba16``
``rgb10a2``             ``Rgb10A2``
``rg16``                ``Rg16``
``rg8``                 ``Rg8``
``r16``                 ``R16``
``r8``                  ``R8``
``rgba16snorm``         ``Rgba16Snorm``
``rg16snorm``           ``Rg16Snorm``
``rg8snorm``            ``Rg8Snorm``
``r16snorm``            ``R16Snorm``
``r8snorm``             ``R8Snorm``
``rgba32i``             ``Rgba32i``
``rgba16i``             ``Rgba16i``
``rgba8i``              ``Rgba8i``
``r32i``                ``R32i``
``rg32i``               ``Rg32i``
``rg16i``               ``Rg16i``
``rg8i``                ``Rg8i``
``r16i``                ``R16i``
``r8i``                 ``R8i``
``rgba32ui``            ``Rgba32ui``
``rgba16ui``            ``Rgba16ui``
``rgba8ui``             ``Rgba8ui``
``r32ui``               ``R32ui``
``rgb10a2ui``           ``Rgb10a2ui``
``rg32ui``              ``Rg32ui``
``rg16ui``              ``Rg16ui``
``rg8ui``               ``Rg8ui``
``r16ui``               ``R16ui``
``r8ui``                ``R8ui``
``r64ui``               ``R64ui``
``r64i``                ``R64i``
======================= ============================================

Constant/Texture/Structured/Byte Buffers
----------------------------------------

There are serveral buffer types in HLSL:

- ``cbuffer`` and ``ConstantBuffer``
- ``tbuffer`` and ``TextureBuffer``
- ``StructuredBuffer`` and ``RWStructuredBuffer``
- ``AppendStructuredBuffer`` and ``ConsumeStructuredBuffer``
- ``ByteAddressBuffer`` and ``RWByteAddressBuffer``

Note that ``Buffer`` and ``RWBuffer`` are considered as texture object in HLSL.
They are listed in the above section.

Please see the following sections for the details of each type. As a summary:

=========================== ================== ================================ ==================== =================
         HLSL Type          Vulkan Buffer Type    Default Memory Layout Rule    SPIR-V Storage Class SPIR-V Decoration
=========================== ================== ================================ ==================== =================
``cbuffer``                   Uniform Buffer   Vector-relaxed OpenGL ``std140``      ``Uniform``     ``Block``
``ConstantBuffer``            Uniform Buffer   Vector-relaxed OpenGL ``std140``      ``Uniform``     ``Block``
``tbuffer``                   Storage Buffer   Vector-relaxed OpenGL ``std430``      ``Uniform``     ``BufferBlock``
``TextureBuffer``             Storage Buffer   Vector-relaxed OpenGL ``std430``      ``Uniform``     ``BufferBlock``
``StructuredBuffer``          Storage Buffer   Vector-relaxed OpenGL ``std430``      ``Uniform``     ``BufferBlock``
``RWStructuredBuffer``        Storage Buffer   Vector-relaxed OpenGL ``std430``      ``Uniform``     ``BufferBlock``
``AppendStructuredBuffer``    Storage Buffer   Vector-relaxed OpenGL ``std430``      ``Uniform``     ``BufferBlock``
``ConsumeStructuredBuffer``   Storage Buffer   Vector-relaxed OpenGL ``std430``      ``Uniform``     ``BufferBlock``
``ByteAddressBuffer``         Storage Buffer   Vector-relaxed OpenGL ``std430``      ``Uniform``     ``BufferBlock``
``RWByteAddressBuffer``       Storage Buffer   Vector-relaxed OpenGL ``std430``      ``Uniform``     ``BufferBlock``
=========================== ================== ================================ ==================== =================

To know more about the Vulkan buffer types, please refer to the Vulkan spec
`14.1 Descriptor Types <https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#descriptorsets-types>`_.

Memory layout rules
~~~~~~~~~~~~~~~~~~~

SPIR-V CodeGen supports four sets of memory layout rules for buffer resources
right now:

1. Vector-relaxed OpenGL ``std140`` for uniform buffers and vector-relaxed
   OpenGL ``std430`` for storage buffers: these rules satisfy Vulkan `"Standard
   Uniform Buffer Layout" and "Standard Storage Buffer Layout" <https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#interfaces-resources-layout>`_,
   respectively.
   They are the default.
2. DirectX memory layout rules for uniform buffers and storage buffers:
   they allow packing data on the application side that can be shared with
   DirectX. They can be enabled by ``-fvk-use-dx-layout``.
   
   NOTE: This requires ``VK_EXT_scalar_block_layout`` to be enabled on the
   application side.
3. Strict OpenGL ``std140`` for uniform buffers and strict OpenGL ``std430``
   for storage buffers: they allow packing data on the application side that
   can be shared with OpenGL. They can be enabled by ``-fvk-use-gl-layout``.
4. Scalar layout rules introduced via `VK_EXT_scalar_block_layout`, which
   basically aligns all aggregrate types according to their elements'
   natural alignment. They can be enabled by ``-fvk-use-scalar-layout``.
   
   NOTE: This requires ``VK_EXT_scalar_block_layout`` to be enabled on the
   application side.

In the above, "vector-relaxed OpenGL ``std140``/``std430``" rules mean OpenGL
``std140``/``std430`` rules with the following modification for vector type
alignment:

1. The alignment of a vector type is set to be the alignment of its element type
2. If the above causes an `improper straddle <https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#interfaces-resources-layout>`_,
   the alignment will be set to 16 bytes.

As an example, for the following HLSL definition:

.. code:: hlsl

  struct S {
      float3 f;
  };

  struct T {
                float    a_float;
                float3   b_float3;
                S        c_S_float3;
                float2x3 d_float2x3;
      row_major float2x3 e_float2x3;
                int      f_int_3[3];
                float2   g_float2_2[2];
  };

We will have the following offsets for each member:

============== ====== ====== ====== ========== ====== ====== ====== ==========
     HLSL             Uniform Buffer                Storage Buffer
-------------- ------------------------------- -------------------------------
    Member     1 (VK) 2 (DX) 3 (GL) 4 (Scalar) 1 (VK) 2 (DX) 3 (GL) 4 (Scalar)
============== ====== ====== ====== ========== ====== ====== ====== ==========
``a_float``      0      0      0        0        0      0     0        0
``b_float3``     4      4      16       4        4      4     16       4
``c_S_float3``   16     16     32       16       16     16    32       16
``d_float2x3``   32     32     48       28       32     28    48       28
``e_float2x3``   80     80     96       52       64     52    80       52
``f_int_3``      112    112    128      76       96     76    112      76
``g_float2_2``   160    160    176      88       112    88    128      88
============== ====== ====== ====== ========== ====== ====== ====== ==========

If the above layout rules do not satisfy your needs and you want to manually
control the layout of struct members, you can use either

* The native HLSL ``:packoffset()`` attribute: only available for cbuffers; or
* The Vulkan-specific ``[[vk::offset()]]`` attribute: applies to all resources.

``[[vk::offset]]`` overrules ``:packoffset``. Attaching ``[[vk::offset]]``
to a struct memeber affects all variables of the struct type in question. So
sharing the same struct definition having ``[[vk::offset]]`` annotations means
also sharing the layout.

For global variables (which are collected into the ``$Globals`` cbuffer), you
can use the native HLSL ``:register(c#)`` attribute. Note that ``[[vk::offset]]``
and ``:packoffset`` cannot be applied to these variables.

If ``register(cX)`` is used on any global variable, the offset for that variable
is set to ``X * 16``, and the offset for all other global variables without the
``register(c#)`` annotation will be set to the next available address after
the highest explicit address. For example:

.. code:: hlsl

  float x : register(c10);   // Offset = 160 (10 * 16)
  float y;                   // Offset = 164 (160 + 4)
  float z: register(c1);     // Offset = 16  (1  * 16)


These attributes give great flexibility but also responsibility to the
developer; the compiler will just take in what is specified in the source code
and emit it to SPIR-V with no error checking.

``cbuffer`` and ``ConstantBuffer``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

These two buffer types are treated as uniform buffers using Vulkan's
terminology. They are translated into an ``OpTypeStruct`` with the
necessary layout decorations (``Offset``, ``ArrayStride``, ``MatrixStride``,
``RowMajor``, ``ColMajor``) and the ``Block`` decoration. The layout rule
used is vector-relaxed OpenGL ``std140`` (by default). A variable declared as
one of these types will be placed in the ``Uniform`` storage class.

For example, for the following HLSL source code:

.. code:: hlsl

  struct T {
    float  a;
    float3 b;
  };

  ConstantBuffer<T> myCBuffer;

will be translated into

.. code:: spirv

  ; Layout decoration
  OpMemberDecorate %type_ConstantBuffer_T 0 Offset 0
  OpMemberDecorate %type_ConstantBuffer_T 0 Offset 4
  ; Block decoration
  OpDecorate %type_ConstantBuffer_T Block

  ; Types
  %type_ConstantBuffer_T = OpTypeStruct %float %v3float
  %_ptr_Uniform_type_ConstantBuffer_T = OpTypePointer Uniform %type_ConstantBuffer_T

  ; Variable
  %myCbuffer = OpVariable %_ptr_Uniform_type_ConstantBuffer_T Uniform

``tbuffer`` and ``TextureBuffer``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

These two buffer types are treated as storage buffers using Vulkan's
terminology. They are translated into an ``OpTypeStruct`` with the
necessary layout decorations (``Offset``, ``ArrayStride``, ``MatrixStride``,
``RowMajor``, ``ColMajor``) and the ``BufferBlock`` decoration. All the struct
members are also decorated with ``NonWritable`` decoration. The layout rule
used is vector-relaxed OpenGL ``std430`` (by default). A variable declared as
one of these types will be placed in the ``Uniform`` storage class.


``StructuredBuffer`` and ``RWStructuredBuffer``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``StructuredBuffer<T>``/``RWStructuredBuffer<T>`` is treated as storage buffer
using Vulkan's terminology. It is translated into an ``OpTypeStruct`` containing
an ``OpTypeRuntimeArray`` of type ``T``, with necessary layout decorations
(``Offset``, ``ArrayStride``, ``MatrixStride``, ``RowMajor``, ``ColMajor``) and
the ``BufferBlock`` decoration.  The default layout rule used is vector-relaxed
OpenGL ``std430``. A variable declared as one of these types will be placed in
the ``Uniform`` storage class.

For ``RWStructuredBuffer<T>``, each variable will have an associated counter
variable generated. The counter variable will be of ``OpTypeStruct`` type, which
only contains a 32-bit integer. The counter variable takes its own binding
number. ``.IncrementCounter()``/``.DecrementCounter()`` will modify this counter
variable.

For example, for the following HLSL source code:

.. code:: hlsl

  struct T {
    float  a;
    float3 b;
  };

  StructuredBuffer<T> mySBuffer;

will be translated into

.. code:: spirv

  ; Layout decoration
  OpMemberDecorate %T 0 Offset 0
  OpMemberDecorate %T 1 Offset 4
  OpDecorate %_runtimearr_T ArrayStride 16
  OpMemberDecorate %type_StructuredBuffer_T 0 Offset 0
  OpMemberDecorate %type_StructuredBuffer_T 0 NoWritable
  ; BufferBlock decoration
  OpDecorate %type_StructuredBuffer_T BufferBlock

  ; Types
  %T = OpTypeStruct %float %v3float
  %_runtimearr_T = OpTypeRuntimeArray %T
  %type_StructuredBuffer_T = OpTypeStruct %_runtimearr_T
  %_ptr_Uniform_type_StructuredBuffer_T = OpTypePointer Uniform %type_StructuredBuffer_T

  ; Variable
  %myCbuffer = OpVariable %_ptr_Uniform_type_ConstantBuffer_T Uniform

``AppendStructuredBuffer`` and ``ConsumeStructuredBuffer``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``AppendStructuredBuffer<T>``/``ConsumeStructuredBuffer<T>`` is treated as
storage buffer using Vulkan's terminology. It is translated into an
``OpTypeStruct`` containing an ``OpTypeRuntimeArray`` of type ``T``, with
necessary layout decorations (``Offset``, ``ArrayStride``, ``MatrixStride``,
``RowMajor``, ``ColMajor``) and the ``BufferBlock`` decoration. The default
layout rule used is vector-relaxed OpenGL ``std430``.

A variable declared as one of these types will be placed in the ``Uniform``
storage class. Besides, each variable will have an associated counter variable
generated. The counter variable will be of ``OpTypeStruct`` type, which only
contains a 32-bit integer. The integer is the total number of elements in the
buffer. The counter variable takes its own binding number.
``.Append()``/``.Consume()`` will use the counter variable as the index and
adjust it accordingly.

For example, for the following HLSL source code:

.. code:: hlsl

  struct T {
    float  a;
    float3 b;
  };

  AppendStructuredBuffer<T> mySBuffer;

will be translated into

.. code:: spirv

  ; Layout decorations
  OpMemberDecorate %T 0 Offset 0
  OpMemberDecorate %T 1 Offset 4
  OpDecorate %_runtimearr_T ArrayStride 16
  OpMemberDecorate %type_AppendStructuredBuffer_T 0 Offset 0
  OpDecorate %type_AppendStructuredBuffer_T BufferBlock
  OpMemberDecorate %type_ACSBuffer_counter 0 Offset 0
  OpDecorate %type_ACSBuffer_counter BufferBlock

  ; Binding numbers
  OpDecorate %myASbuffer DescriptorSet 0
  OpDecorate %myASbuffer Binding 0
  OpDecorate %counter_var_myASbuffer DescriptorSet 0
  OpDecorate %counter_var_myASbuffer Binding 1

  ; Types
  %T = OpTypeStruct %float %v3float
  %_runtimearr_T = OpTypeRuntimeArray %T
  %type_AppendStructuredBuffer_T = OpTypeStruct %_runtimearr_T
  %_ptr_Uniform_type_AppendStructuredBuffer_T = OpTypePointer Uniform %type_AppendStructuredBuffer_T
  %type_ACSBuffer_counter = OpTypeStruct %int
  %_ptr_Uniform_type_ACSBuffer_counter = OpTypePointer Uniform %type_ACSBuffer_counter

  ; Variables
  %myASbuffer = OpVariable %_ptr_Uniform_type_AppendStructuredBuffer_T Uniform
  %counter_var_myASbuffer = OpVariable %_ptr_Uniform_type_ACSBuffer_counter Uniform

``ByteAddressBuffer`` and ``RWByteAddressBuffer``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``ByteAddressBuffer``/``RWByteAddressBuffer`` is treated as storage buffer using
Vulkan's terminology. It is translated into an ``OpTypeStruct`` containing an
``OpTypeRuntimeArray`` of 32-bit unsigned integers, with ``BufferBlock``
decoration.

A variable declared as one of these types will be placed in the ``Uniform``
storage class.

For example, for the following HLSL source code:

.. code:: hlsl

  ByteAddressBuffer   myBuffer1;
  RWByteAddressBuffer myBuffer2;

will be translated into

.. code:: spirv

  ; Layout decorations

  OpDecorate %_runtimearr_uint ArrayStride 4

  OpDecorate %type_ByteAddressBuffer BufferBlock
  OpMemberDecorate %type_ByteAddressBuffer 0 Offset 0
  OpMemberDecorate %type_ByteAddressBuffer 0 NonWritable

  OpDecorate %type_RWByteAddressBuffer BufferBlock
  OpMemberDecorate %type_RWByteAddressBuffer 0 Offset 0

  ; Types

  %_runtimearr_uint = OpTypeRuntimeArray %uint

  %type_ByteAddressBuffer = OpTypeStruct %_runtimearr_uint
  %_ptr_Uniform_type_ByteAddressBuffer = OpTypePointer Uniform %type_ByteAddressBuffer

  %type_RWByteAddressBuffer = OpTypeStruct %_runtimearr_uint
  %_ptr_Uniform_type_RWByteAddressBuffer = OpTypePointer Uniform %type_RWByteAddressBuffer

  ; Variables

  %myBuffer1 = OpVariable %_ptr_Uniform_type_ByteAddressBuffer Uniform
  %myBuffer2 = OpVariable %_ptr_Uniform_type_RWByteAddressBuffer Uniform

Rasterizer Ordered Views
------------------------

The following types are rasterizer ordered views:

* ``RasterizerOrderedBuffer``
* ``RasterizerOrderedByteAddressBuffer``
* ``RasterizerOrderedStructuredBuffer``
* ``RasterizerOrderedTexture1D``
* ``RasterizerOrderedTexture1DArray``
* ``RasterizerOrderedTexture2D``
* ``RasterizerOrderedTexture2DArray``
* ``RasterizerOrderedTexture3D``

These are translated to the same types as their equivalent RW* types - for
example, a ``RasterizerOrderedBuffer`` is translated to the same SPIR-V type as
an ``RWBuffer``. The sole difference lies in how loads and stores to these
values are treated.

The access order guarantee made by ROVs is implemented in SPIR-V using the
`SPV_EXT_fragment_shader_interlock <https://github.com/KhronosGroup/SPIRV-Registry/blob/main/extensions/EXT/SPV_EXT_fragment_shader_interlock.asciidoc>`_.
When you load or store a value from or to a rasterizer ordered view, using
either the ``Load*()`` or ``Store*()`` methods or the indexing operator,
``OpBeginInvocationInterlockEXT`` will be inserted before the first access and
``OpEndInvocationInterlockEXT`` will be inserted after the last access.

An execution mode will be added to the entry point, depending on the sample
frequency, which will be deduced based on the semantics inputted by the entry
point. ``PixelInterlockOrderedEXT`` will be selected by default,
``SampleInterlockOrderedEXT`` will be selected if the ``SV_SampleIndex``
semantic is input, and ``ShadingRateInterlockOrderedEXT`` will be selected if
the ``SV_ShadingRate`` semantic is input.

HLSL Variables and Resources
============================

This section lists how various HLSL variables and resources are mapped.

According to `Shader Constants <https://msdn.microsoft.com/en-us/library/windows/desktop/bb509581(v=vs.85).aspx>`_,

  There are two default constant buffers available, $Global and $Param. Variables
  that are placed in the global scope are added implicitly to the $Global cbuffer,
  using the same packing method that is used for cbuffers. Uniform parameters in
  the parameter list of a function appear in the $Param constant buffer when a
  shader is compiled outside of the effects framework.

So all global externally-visible non-resource-type stand-alone variables will
be collected into a cbuffer named as ``$Globals``, no matter whether they are
statically referenced by the entry point or not. The ``$Globals`` cbuffer
follows the layout rules like normal cbuffer.

Storage class
-------------

Normal local variables (without any modifier) will be placed in the ``Function``
SPIR-V storage class. Normal global variables (without any modifer) will be
placed in the ``Uniform`` or ``UniformConstant`` storage class.

- ``static``

  - Global variables with ``static`` modifier will be placed in the ``Private``
    SPIR-V storage class. Initalizers of such global variables will be translated
    into SPIR-V ``OpVariable`` initializers if possible; otherwise, they will be
    initialized at the very beginning of the `entry function wrapper`_ using
    SPIR-V ``OpStore``.
  - Local variables with ``static`` modifier will also be placed in the
    ``Private`` SPIR-V storage class. initializers of such local variables will
    also be translated into SPIR-V ``OpVariable`` initializers if possible;
    otherwise, they will be initialized at the very beginning of the enclosing
    function. To make sure that such a local variable is only initialized once,
    a second boolean variable of the ``Private`` SPIR-V storage class will be
    generated to mark its initialization status.

- ``groupshared``

  - Global variables with ``groupshared`` modifier will be placed in the
    ``Workgroup`` storage class.
  - Note that this modifier overrules ``static``; if both ``groupshared`` and
    ``static`` are applied to a variable, ``static`` will be ignored.

- ``uniform``

  - This does not affect codegen. Variables will be treated like normal global
    variables.

- ``extern``

  - This does not affect codegen. Variables will be treated like normal global
    variables.

- ``shared``

  - This is a hint to the compiler. It will be ingored.

- ``volatile``

  - This is a hint to the compiler. It will be ingored.

HLSL semantic and Vulkan ``Location``
-------------------------------------

Direct3D uses HLSL "`semantics <https://msdn.microsoft.com/en-us/library/windows/desktop/bb509647(v=vs.85).aspx>`_"
to compose and match the interfaces between subsequent stages. These semantic
strings can appear after struct members, function parameters and return
values. E.g.,

.. code:: hlsl

  struct VSInput {
    float4 pos  : POSITION;
    float3 norm : NORMAL;
  };

  float4 VSMain(in  VSInput input,
                in  float4  tex   : TEXCOORD,
                out float4  pos   : SV_Position) : TEXCOORD {
    pos = input.pos;
    return tex;
  }

In contrary, Vulkan stage input and output interface matching is via explicit
``Location`` numbers. Details can be found `here <https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#interfaces-iointerfaces>`_.

To translate HLSL to SPIR-V for Vulkan, semantic strings need to be mapped to
Vulkan ``Location`` numbers properly. This can be done either explicitly via
information provided by the developer or implicitly by the compiler.

Explicit ``Location`` number assignment
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``[[vk::location(X)]]`` can be attached to the entities where semantic are
allowed to attach (struct fields, function parameters, and function returns).
For the above exmaple we can have:

.. code:: hlsl

  struct VSInput {
    [[vk::location(0)]] float4 pos  : POSITION;
    [[vk::location(1)]] float3 norm : NORMAL;
  };

  [[vk::location(1)]]
  float4 VSMain(in  VSInput input,
                [[vk::location(2)]]
                in  float4  tex     : TEXCOORD,
                out float4  pos     : SV_Position) : TEXCOORD {
    pos = input.pos;
    return tex;
  }

In the above, input ``POSITION``, ``NORMAL``, and ``TEXCOORD`` will be mapped to
``Location`` 0, 1, and 2, respectively, and output ``TEXCOORD`` will be mapped
to ``Location`` 1.

[TODO] Another explicit way: using command-line options

Please note that the compiler prohibits mixing the explicit and implicit
approach for the same SigPoint to avoid complexity and fallibility. However,
for a certain shader stage, one SigPoint using the explicit approach while the
other adopting the implicit approach is permitted.

Implicit ``Location`` number assignment
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Without hints from the developer, the compiler will try its best to map
semantics to ``Location`` numbers. However, there is no single rule for this
mapping; semantic strings should be handled case by case.

Firstly, under certain `SigPoints <https://github.com/Microsoft/DirectXShaderCompiler/blob/main/docs/DXIL.rst#hlsl-signatures-and-semantics>`_,
some system-value (SV) semantic strings will be translated into SPIR-V
``BuiltIn`` decorations:

.. table:: Mapping from HLSL SV semantic to SPIR-V builtin and execution mode

+---------------------------+-------------+----------------------------------------+-----------------------+-----------------------------+
| HLSL Semantic             | SigPoint    | SPIR-V ``BuiltIn``                     | SPIR-V Execution Mode |   SPIR-V Capability         |
+===========================+=============+========================================+=======================+=============================+
|                           | VSOut       | ``Position``                           | N/A                   | ``Shader``                  |
|                           +-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | HSCPIn      | ``Position``                           | N/A                   | ``Shader``                  |
|                           +-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | HSCPOut     | ``Position``                           | N/A                   | ``Shader``                  |
|                           +-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | DSCPIn      | ``Position``                           | N/A                   | ``Shader``                  |
|                           +-------------+----------------------------------------+-----------------------+-----------------------------+
| SV_Position               | DSOut       | ``Position``                           | N/A                   | ``Shader``                  |
|                           +-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | GSVIn       | ``Position``                           | N/A                   | ``Shader``                  |
|                           +-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | GSOut       | ``Position``                           | N/A                   | ``Shader``                  |
|                           +-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | PSIn        | ``FragCoord``                          | N/A                   | ``Shader``                  |
|                           +-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | MSOut       | ``Position``                           | N/A                   | ``Shader``                  |
+---------------------------+-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | VSOut       | ``ClipDistance``                       | N/A                   | ``ClipDistance``            |
|                           +-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | HSCPIn      | ``ClipDistance``                       | N/A                   | ``ClipDistance``            |
|                           +-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | HSCPOut     | ``ClipDistance``                       | N/A                   | ``ClipDistance``            |
|                           +-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | DSCPIn      | ``ClipDistance``                       | N/A                   | ``ClipDistance``            |
|                           +-------------+----------------------------------------+-----------------------+-----------------------------+
| SV_ClipDistance           | DSOut       | ``ClipDistance``                       | N/A                   | ``ClipDistance``            |
|                           +-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | GSVIn       | ``ClipDistance``                       | N/A                   | ``ClipDistance``            |
|                           +-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | GSOut       | ``ClipDistance``                       | N/A                   | ``ClipDistance``            |
|                           +-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | PSIn        | ``ClipDistance``                       | N/A                   | ``ClipDistance``            |
|                           +-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | MSOut       | ``ClipDistance``                       | N/A                   | ``ClipDistance``            |
+---------------------------+-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | VSOut       | ``CullDistance``                       | N/A                   | ``CullDistance``            |
|                           +-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | HSCPIn      | ``CullDistance``                       | N/A                   | ``CullDistance``            |
|                           +-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | HSCPOut     | ``CullDistance``                       | N/A                   | ``CullDistance``            |
|                           +-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | DSCPIn      | ``CullDistance``                       | N/A                   | ``CullDistance``            |
|                           +-------------+----------------------------------------+-----------------------+-----------------------------+
| SV_CullDistance           | DSOut       | ``CullDistance``                       | N/A                   | ``CullDistance``            |
|                           +-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | GSVIn       | ``CullDistance``                       | N/A                   | ``CullDistance``            |
|                           +-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | GSOut       | ``CullDistance``                       | N/A                   | ``CullDistance``            |
|                           +-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | PSIn        | ``CullDistance``                       | N/A                   | ``CullDistance``            |
|                           +-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | MSOut       | ``CullDistance``                       | N/A                   | ``CullDistance``            |
+---------------------------+-------------+----------------------------------------+-----------------------+-----------------------------+
| SV_VertexID               | VSIn        | ``VertexIndex``                        | N/A                   | ``Shader``                  |
+---------------------------+-------------+----------------------------------------+-----------------------+-----------------------------+
| SV_InstanceID             | VSIn        | ``InstanceIndex`` or                   | N/A                   | ``Shader``                  |
|                           |             | ``InstanceIndex - BaseInstance``       |                       |                             |
|                           |             | with                                   |                       |                             |
|                           |             | ``-fvk-support-nonzero-base-instance`` |                       |                             |
+---------------------------+-------------+----------------------------------------+-----------------------+-----------------------------+
| SV_StartVertexLocation    | VSIn        | ``BaseVertex``                         | N/A                   | ``Shader``                  |
+---------------------------+-------------+----------------------------------------+-----------------------+-----------------------------+
| SV_StartInstanceLocation  | VSIn        | ``BaseInstance``                       | N/A                   | ``Shader``                  |
+---------------------------+-------------+----------------------------------------+-----------------------+-----------------------------+
| SV_Depth                  | PSOut       | ``FragDepth``                          | N/A                   | ``Shader``                  |
+---------------------------+-------------+----------------------------------------+-----------------------+-----------------------------+
| SV_DepthGreaterEqual      | PSOut       | ``FragDepth``                          | ``DepthGreater``      | ``Shader``                  |
+---------------------------+-------------+----------------------------------------+-----------------------+-----------------------------+
| SV_DepthLessEqual         | PSOut       | ``FragDepth``                          | ``DepthLess``         | ``Shader``                  |
+---------------------------+-------------+----------------------------------------+-----------------------+-----------------------------+
| SV_IsFrontFace            | PSIn        | ``FrontFacing``                        | N/A                   | ``Shader``                  |
+---------------------------+-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | CSIn        | ``GlobalInvocationId``                 | N/A                   | ``Shader``                  |
|                           +-------------+----------------------------------------+-----------------------+-----------------------------+
| SV_DispatchThreadID       | MSIn        | ``GlobalInvocationId``                 | N/A                   | ``Shader``                  |
|                           +-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | ASIn        | ``GlobalInvocationId``                 | N/A                   | ``Shader``                  |
+---------------------------+-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | CSIn        | ``WorkgroupId``                        | N/A                   | ``Shader``                  |
|                           +-------------+----------------------------------------+-----------------------+-----------------------------+
| SV_GroupID                | MSIn        | ``WorkgroupId``                        | N/A                   | ``Shader``                  |
|                           +-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | ASIn        | ``WorkgroupId``                        | N/A                   | ``Shader``                  |
+---------------------------+-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | CSIn        | ``LocalInvocationId``                  | N/A                   | ``Shader``                  |
|                           +-------------+----------------------------------------+-----------------------+-----------------------------+
| SV_GroupThreadID          | MSIn        | ``LocalInvocationId``                  | N/A                   | ``Shader``                  |
|                           +-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | ASIn        | ``LocalInvocationId``                  | N/A                   | ``Shader``                  |
+---------------------------+-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | CSIn        | ``LocalInvocationIndex``               | N/A                   | ``Shader``                  |
|                           +-------------+----------------------------------------+-----------------------+-----------------------------+
| SV_GroupIndex             | MSIn        | ``LocalInvocationIndex``               | N/A                   | ``Shader``                  |
|                           +-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | ASIn        | ``LocalInvocationIndex``               | N/A                   | ``Shader``                  |
+---------------------------+-------------+----------------------------------------+-----------------------+-----------------------------+
| SV_OutputControlPointID   | HSIn        | ``InvocationId``                       | N/A                   | ``Tessellation``            |
+---------------------------+-------------+----------------------------------------+-----------------------+-----------------------------+
| SV_GSInstanceID           | GSIn        | ``InvocationId``                       | N/A                   | ``Geometry``                |
+---------------------------+-------------+----------------------------------------+-----------------------+-----------------------------+
| SV_DomainLocation         | DSIn        | ``TessCoord``                          | N/A                   | ``Tessellation``            |
+---------------------------+-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | HSIn        | ``PrimitiveId``                        | N/A                   | ``Tessellation``            |
|                           +-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | PCIn        | ``PrimitiveId``                        | N/A                   | ``Tessellation``            |
|                           +-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | DsIn        | ``PrimitiveId``                        | N/A                   | ``Tessellation``            |
|                           +-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | GSIn        | ``PrimitiveId``                        | N/A                   | ``Geometry``                |
| SV_PrimitiveID            +-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | GSOut       | ``PrimitiveId``                        | N/A                   | ``Geometry``                |
|                           +-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | PSIn        | ``PrimitiveId``                        | N/A                   | ``Geometry``                |
|                           +-------------+----------------------------------------+-----------------------+-----------------------------+
|                           |             |                                        |                       | ``MeshShadingNV``           |
|                           | MSOut       | ``PrimitiveId``                        | N/A                   |                             |
|                           |             |                                        |                       | ``MeshShadingEXT``          |
+---------------------------+-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | PCOut       | ``TessLevelOuter``                     | N/A                   | ``Tessellation``            |
| SV_TessFactor             +-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | DSIn        | ``TessLevelOuter``                     | N/A                   | ``Tessellation``            |
+---------------------------+-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | PCOut       | ``TessLevelInner``                     | N/A                   | ``Tessellation``            |
| SV_InsideTessFactor       +-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | DSIn        | ``TessLevelInner``                     | N/A                   | ``Tessellation``            |
+---------------------------+-------------+----------------------------------------+-----------------------+-----------------------------+
| SV_SampleIndex            | PSIn        | ``SampleId``                           | N/A                   | ``SampleRateShading``       |
+---------------------------+-------------+----------------------------------------+-----------------------+-----------------------------+
| SV_StencilRef             | PSOut       | ``FragStencilRefEXT``                  | N/A                   | ``StencilExportEXT``        |
+---------------------------+-------------+----------------------------------------+-----------------------+-----------------------------+
| SV_Barycentrics           | PSIn        | ``BaryCoord*KHR``                      | N/A                   | ``FragmentBarycentricKHR``  |
+---------------------------+-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | GSOut       | ``Layer``                              | N/A                   | ``Geometry``                |
|                           +-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | PSIn        | ``Layer``                              | N/A                   | ``Geometry``                |
| SV_RenderTargetArrayIndex +-------------+----------------------------------------+-----------------------+-----------------------------+
|                           |             |                                        |                       | ``MeshShadingNV``           |
|                           | MSOut       | ``Layer``                              | N/A                   |                             |
|                           |             |                                        |                       | ``MeshShadingEXT``          |
+---------------------------+-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | GSOut       | ``ViewportIndex``                      | N/A                   | ``MultiViewport``           |
|                           +-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | PSIn        | ``ViewportIndex``                      | N/A                   | ``MultiViewport``           |
| SV_ViewportArrayIndex     +-------------+----------------------------------------+-----------------------+-----------------------------+
|                           |             |                                        |                       | ``MeshShadingNV``           |
|                           | MSOut       | ``ViewportIndex``                      | N/A                   |                             |
|                           |             |                                        |                       | ``MeshShadingEXT``          |
+---------------------------+-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | PSIn        | ``SampleMask``                         | N/A                   | ``Shader``                  |
| SV_Coverage               +-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | PSOut       | ``SampleMask``                         | N/A                   | ``Shader``                  |
+---------------------------+-------------+----------------------------------------+-----------------------+-----------------------------+
| SV_InnerCoverage          | PSIn        | ``FullyCoveredEXT``                    | N/A                   | ``FragmentFullyCoveredEXT`` |
+---------------------------+-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | VSIn        | ``ViewIndex``                          | N/A                   | ``MultiView``               |
|                           +-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | HSIn        | ``ViewIndex``                          | N/A                   | ``MultiView``               |
|                           +-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | DSIn        | ``ViewIndex``                          | N/A                   | ``MultiView``               |
| SV_ViewID                 +-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | GSIn        | ``ViewIndex``                          | N/A                   | ``MultiView``               |
|                           +-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | PSIn        | ``ViewIndex``                          | N/A                   | ``MultiView``               |
|                           +-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | MSIn        | ``ViewIndex``                          | N/A                   | ``MultiView``               |
+---------------------------+-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | VSOut       | ``PrimitiveShadingRateKHR``            | N/A                   | ``FragmentShadingRate``     |
|                           +-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | GSOut       | ``PrimitiveShadingRateKHR``            | N/A                   | ``FragmentShadingRate``     |
| SV_ShadingRate            +-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | PSIn        | ``ShadingRateKHR``                     | N/A                   | ``FragmentShadingRate``     |
|                           +-------------+----------------------------------------+-----------------------+-----------------------------+
|                           | MSOut       | ``PrimitiveShadingRateKHR``            | N/A                   | ``FragmentShadingRate``     |
+---------------------------+-------------+----------------------------------------+-----------------------+-----------------------------+
| SV_CullPrimitive          | MSOut       | ``CullPrimitiveEXT``                   | N/A                   | ``MeshShadingEXT``          |
+---------------------------+-------------+----------------------------------------+-----------------------+-----------------------------+


For entities (function parameters, function return values, struct fields) with
the above SV semantic strings attached, SPIR-V variables of the
``Input``/``Output`` storage class will be created. They will have the
corresponding SPIR-V ``Builtin``  decorations according to the above table.

SV semantic strings not translated into SPIR-V ``BuiltIn`` decorations will be
handled similarly as non-SV (arbitrary) semantic strings: a SPIR-V variable
of the ``Input``/``Output`` storage class will be created for each entity with
such semantic string. Then sort all semantic strings according to declaration
(the default, or if ``-fvk-stage-io-order=decl`` is given) or alphabetical
(if ``-fvk-stage-io-order=alpha`` is given) order, and assign ``Location``
numbers sequentially to the corresponding SPIR-V variables. Note that this means
flattening all structs if structs are used as function parameters or returns.

There is an exception to the above rule for SV_Target[N]. It will always be
mapped to ``Location`` number N.

``ClipDistance & CullDistance``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Variables decorated with ``SV_ClipDistanceX`` can be float or vector of float
type. To map them into one float array in the struct, we firstly sort them
asecendingly according to ``X``, and then concatenate them tightly. For example,

.. code:: hlsl

  struct T {
    float clip0: SV_ClipDistance0,
  };

  struct S {
    float3 clip5: SV_ClipDistance5;
    ...
  };

  void main(T t, S s, float2 clip2 : SV_ClipDistance2) { ... }

Then we have an float array of size (1 + 2 + 3 =) 6 for ``ClipDistance``, with
``clip0`` at offset 0, ``clip2`` at offset 1, ``clip5`` at offset 3.

Decorating a variable or struct member with the ``ClipDistance`` builtin but not
requiring the ``ClipDistance`` capability is legal as long as we don't read or
write the variable or struct member. But as per the way we handle `shader entry
function`_, this is not satisfied because we need to read their contents to
prepare for the source code entry function call or write back them after the
call. So annotating a variable or struct member with ``SV_ClipDistanceX`` means
requiring the ``ClipDistance`` capability in the generated SPIR-V.

Variables decorated with ``SV_CullDistanceX`` are mapped similarly as above.

Signature packing
~~~~~~~~~~~~~~~~~

In usual, Vulkan drivers have a limitation of the number of available locations.
It varies depending on the device. To avoid the driver crash caused by the
limitation, we added an experimental signature packing support using Component
decoration (see the Vulkan spec "15.1.5. Component Assignment").
``-pack-optimized`` is the command line option to enable it.

In a high level, for a stage variable that needs ``M`` components in ``N``
locations e.g., stage variable ``float3 foo[2]`` needs 3 components in 2
locations, we find a minimum ``K`` where each of ``N`` continuous locations in
``[K, K + N)`` has ``M`` continuous unused Component slots. We create a Location
decoration instruction for the stage variable with ``K`` and a Component
decoration instruction with the first unused component number of the
``M`` continuous unused Component slots.

HLSL register and Vulkan binding
--------------------------------

In shaders for DirectX, resources are accessed via registers; while in shaders
for Vulkan, it is done via descriptor set and binding numbers. The developer
can explicitly annotate variables in HLSL to specify descriptor set and binding
numbers, or leave it to the compiler to derive implicitly from registers.

Explicit binding number assignment
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``[[vk::binding(X[, Y])]]`` can be attached to global variables to specify the
descriptor set as ``Y`` and binding number as ``X``. The descriptor set number
is optional; if missing, it will be zero (If ``-auto-binding-space N`` command
line option is used, then descriptor set #N will be used instead of descriptor
set #0). RW/append/consume structured buffers have associated counters, which
will occupy their own Vulkan descriptors. ``[vk::counter_binding(Z)]`` can be
attached to a RW/append/consume structured buffers to specify the binding number
for the associated counter to ``Z``. Note that the set number of the counter is
always the same as the main buffer.

.. warning::
   When a RW/append/consume structured buffer is accessed through a resource
   heap, its associated counter is in its own binding, but shares the same
   index in the binding as its associated resource.

   Example:
    - ResourceDescriptorHeap -> binding 0, set 0
    - No other resources are used.

    - RWStructuredBuffer buff = ResourceDescriptorHeap[3]
    - buff.IncrementCounter()

    - buff will be at index 3 of the array at binding 0, set 0.
      buff.counter will be at index 3 of the array at binding 1, set 0


Implicit binding number assignment
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Without explicit annotations, the compiler will try to deduce descriptor sets
and binding numbers in the following way:

If there is ``:register(xX, spaceY)`` specified for the given global variable,
the corresponding resource will be assigned to descriptor set ``Y`` and binding
number ``X``, regardless of the register type ``x``. Note that this will cause
binding number collision if, say, two resources are of different register
type but the same register number. To solve this problem, four command-line
options, ``-fvk-b-shift N M``, ``-fvk-s-shift N M``, ``-fvk-t-shift N M``, and
``-fvk-u-shift N M``, are provided to shift by ``N`` all binding numbers
inferred for register type ``b``, ``s``, ``t``, and ``u`` in space ``M``,
respectively.

If there is no register specification, the corresponding resource will be
assigned to the next available binding number, starting from 0, in descriptor
set #0 (If ``-auto-binding-space N`` command line option is used, then
descriptor set #N will be used instead of descriptor set #0).

If there is no register specification AND ``-fvk-auto-shift-bindings`` is specified,
then the register type will be automatically identified based on the resource
type (according to the following table), and the appropriate shift will
automatically be applied according to ``-fvk-*shift N M``.

.. code:: spirv

  t - for shader resource views (SRV)
      TEXTURE1D
      TEXTURE1DARRAY
      TEXTURE2D
      TEXTURE2DARRAY
      TEXTURE3D
      TEXTURECUBE
      TEXTURECUBEARRAY
      TEXTURE2DMS
      TEXTURE2DMSARRAY
      STRUCTUREDBUFFER
      BYTEADDRESSBUFFER
      BUFFER
      TBUFFER

  s - for samplers
      SAMPLER
      SAMPLER1D
      SAMPLER2D
      SAMPLER3D
      SAMPLERCUBE
      SAMPLERSTATE
      SAMPLERCOMPARISONSTATE

  u - for unordered access views (UAV)
      RWBYTEADDRESSBUFFER
      RWSTRUCTUREDBUFFER
      APPENDSTRUCTUREDBUFFER
      CONSUMESTRUCTUREDBUFFER
      RWBUFFER
      RWTEXTURE1D
      RWTEXTURE1DARRAY
      RWTEXTURE2D
      RWTEXTURE2DARRAY
      RWTEXTURE3D

  b - for constant buffer views (CBV)
      CBUFFER
      CONSTANTBUFFER

Binding number assignment for resources in cbuffer
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Basically, we use the same binding assignment rule described above for a
cbuffer, but when a cbuffer contains one or more resources, it is inevitable
to use multiple binding numbers for a single cbuffer. For this type of
cbuffers, we first assign the next available binding number to the resources.
Based the order of the appearance in the cbuffer, a resource that appears early
uses a smaller (earlier available) binding number than a resource that appears
later. After assigning binding numbers to all resource members, if the cbuffer
contains one or more members with non-resource types, it creates a struct for
the remaining members and assign the next available binding number to the
variable with the struct type.

For example, the binding numbers for the following resources and cbuffers

.. code:: hlsl

  cbuffer buf0 : register(b0) {
    float4 non_resource0;
  };
  cbuffer buf1 : register(b4) {
    float4 non_resource1;
  };
  cbuffer buf2 {
    float4 non_resource2;
    Texture2D resource0;
    SamplerState resource1;
  };
  cbuffer buf3 : register(b2) {
    SamplerState resource2;
  }

will be

- ``buf0``: 0 because of ``register(b0)``

- ``buf1``: 4 because of ``register(b4)``

- ``resource2``: 2 because of ``register(b2)``. Note that ``buf3`` is empty
  without ``resource2``. We do not assign a binding number to an empty struct.

- ``resource0``: 1 because it is the next available binding number.

- ``resource1``: 3 because it is the next available binding number.

- ``buf2`` including only ``non_resource2``: 5 because it is the next available
  binding number.

Summary
~~~~~~~

In summary, the compiler essentially assigns binding numbers in three passes.

- Firstly it handles all declarations with explicit ``[[vk::binding(X[, Y])]]``
  annotation.

- Then the compiler processes all remaining declarations with
  ``:register(xX, spaceY)`` annotation, by applying the shift passed in using
  command-line option ``-fvk-{b|s|t|u}-shift N M``, if provided.

  - If ``:register`` assignment is missing and ``-fvk-auto-shift-bindings`` is
    specified, the register type will be automatically detected based on the
    resource type, and the ``-fvk-{b|s|t|u}-shift N M`` will be applied.

- Finally, the compiler assigns next available binding numbers to the rest in
  the declaration order.

As an example, for the following code:

.. code:: hlsl

  struct S { ... };

  ConstantBuffer<S> cbuffer1 : register(b0);
  Texture2D<float4> texture1 : register(t0);
  Texture2D<float4> texture2 : register(t1, space1);
  SamplerState      sampler1;
  [[vk::binding(3)]]
  RWBuffer<float4> rwbuffer1 : register(u5, space2);

If we compile with ``-fvk-t-shift 10 0 -fvk-t-shift 20 1``:

- ``rwbuffer1`` will take binding #3 in set #0, since explicit binding
  assignment has precedence over the rest.
- ``cbuffer1`` will take binding #0 in set #0, since that's what deduced from
  the register assignment, and there is no shift requested from command line.
- ``texture1`` will take binding #10 in set #0, and ``texture2`` will take
  binding #21 in set #1, since we requested an 10 shift on t-type registers.
- ``sampler1`` will take binding 1 in set #0, since that's the next available
  binding number in set #0.

HLSL global variables and Vulkan binding
----------------------------------------
As mentioned above, all global externally-visible non-resource-type stand-alone
variables will be collected into a cbuffer named ``$Globals``. By default,
the ``$Globals`` cbuffer is placed in descriptor set #0, and the binding number
would be the next available binding number in that set. Meaning, the binding number
depends on where the very first global variable is in the code.

Example 1:

.. code:: hlsl

  float4 someColors;
    // $Globals cbuffer placed at DescriptorSet #0, Binding #0
  Texture2D<float4> texture1;
    // texture1         placed at DescriptorSet #0, Binding #1

Example 2:

.. code:: hlsl

  Texture2D<float4> texture1;
    // texture1         placed at DescriptorSet #0, Binding #0
  float4 someColors;
    // $Globals cbuffer placed at DescriptorSet #0, Binding #1

In order provide more control over the descriptor set and binding number of the
``$Globals`` cbuffer, you can use the ``-fvk-bind-globals B S`` command line
option, which will place this cbuffer at descriptor set ``S``, and binding number ``B``.

Example 3: (compiled with ``-fvk-bind-globals 2 1``)

.. code:: hlsl

  Texture2D<float4> texture1;
    // texture1         placed at DescriptorSet #0, Binding #0
  float4 someColors;
    // $Globals cbuffer placed at DescriptorSet #1, Binding #2

Note that if the developer chooses to use this command line option, it is their
responsibility to provide proper numbers and avoid binding overlaps.

ResourceDescriptorHeaps & SamplerDescriptorHeaps
------------------------------------------------

The SPIR-V backend supported SM6.6 resource heaps, using 2 extensions:
- `SPV_EXT_descriptor_indexing`
- `VK_EXT_mutable_descriptor_type`

Each type loaded from a heap is considered to be an unbounded RuntimeArray
bound to the descriptor set 0.

Each heap uses at most 1 binding in that set. Meaning if 2 types are loaded
from the same heap, DXC will generate 2 RuntimeArray, one for each type, and
will bind them to the same binding/set.
(This requires `VK_EXT_mutable_descriptor_type`).

For resources with counters, like RW/Append/Consume structured buffers,
DXC generates another RuntimeArray of counters, and binds it to a new
binding in the set 0.

This means Resource/Sampler heaps can use at most 3 bindings:
    - 1 for all RuntimeArrays associated with the ResourceDescriptorHeap.
    - 1 for all RuntimeArrays associated with the SamplerDescriptorHeaps.
    - 1 for UAV counters.

The index of a counter in the counters RuntimeArray matches the index of the
associated ResourceDescriptorHeap RuntimeArray.

The selection of the binding indices for those RuntimeArrays is done once all
other resources are bound to their respective bindings/sets.
DXC takes the first 3 unused bindings in the set 0, and distributes them in
that order:
    1. Resource heap.
    2. Sampler heap.
    3. Resouce heap counters.

Bindings are lazily allocated: if only the sampler heap is used,
1 binding will be used.

.. code:: hlsl
   Texture2D tex = ResourceDescriptorHeap[10];
   // tex is in the descriptor set 0, binding 0.

.. code:: hlsl
   [[vk::binding(0, 0)]]
   Texture2D Texture;
   // Texture is using set=0, binding=0

   Texture2D tex = ResourceDescriptorHeap[0];
   // tex is in the descriptor set 0, binding 1.

.. code:: hlsl
   [[vk::binding(0, 0)]]
   RWStructuredBuffer<int> buffer;
   // Texture is using set=0, binding=0

   RWStructuredBuffer<int> tmp = ResourceDescriptorHeap[0];
   tmp.IncrementCounter();
   // tmp is in the descriptor set 0, binding 1.
   // tmp.counter is in the descriptor set 0, binding 2

.. code:: hlsl
   [[vk::binding(1, 0)]]
   RWStructuredBuffer<int> buffer;
   // Texture is using set=0, binding=1

   RWStructuredBuffer<int> tmp = ResourceDescriptorHeap[0];
   tmp.IncrementCounter();
   // tmp is in the descriptor set 0, binding 0.
   // tmp.counter is in the descriptor set 0, binding 2

.. code:: hlsl
   RWStructuredBuffer buffer = ResourceDescriptorHeap[2];
   // buffer is in the descriptor set 0, binding 0.
   // Counter not generated, because unused.

HLSL Expressions
================

Unless explicitly noted, matrix per-element operations will be conducted on
each component vector and then collected into the result matrix. The following
sections lists the SPIR-V opcodes for scalars and vectors.

Arithmetic operators
--------------------

`Arithmetic operators <https://msdn.microsoft.com/en-us/library/windows/desktop/bb509631(v=vs.85).aspx#Additive_and_Multiplicative_Operators>`_
(``+``, ``-``, ``*``, ``/``, ``%``) are translated into their corresponding
SPIR-V opcodes according to the following table.

+-------+-----------------------------+-------------------------------+--------------------+
|       | (Vector of) Signed Integers | (Vector of) Unsigned Integers | (Vector of) Floats |
+=======+=============================+===============================+====================+
| ``+`` |                         ``OpIAdd``                          |     ``OpFAdd``     |
+-------+-------------------------------------------------------------+--------------------+
| ``-`` |                         ``OpISub``                          |     ``OpFSub``     |
+-------+-------------------------------------------------------------+--------------------+
| ``*`` |                         ``OpIMul``                          |     ``OpFMul``     |
+-------+-----------------------------+-------------------------------+--------------------+
| ``/`` |    ``OpSDiv``               |       ``OpUDiv``              |     ``OpFDiv``     |
+-------+-----------------------------+-------------------------------+--------------------+
| ``%`` |    ``OpSRem``               |       ``OpUMod``              |     ``OpFRem``     |
+-------+-----------------------------+-------------------------------+--------------------+

Note that for modulo operation, SPIR-V has two sets of instructions: ``Op*Rem``
and ``Op*Mod``. For ``Op*Rem``, the sign of a non-0 result comes from the first
operand; while for ``Op*Mod``, the sign of a non-0 result comes from the second
operand. HLSL doc does not mandate which set of instructions modulo operations
should be translated into; it only says "the % operator is defined only in cases
where either both sides are positive or both sides are negative." So technically
it's undefined behavior to use the modulo operation with operands of different
signs. But considering HLSL's C heritage and the behavior of Clang frontend, we
translate modulo operators into ``Op*Rem`` (there is no ``OpURem``).

For multiplications of float vectors and float scalars, the dedicated SPIR-V
operation ``OpVectorTimesScalar`` will be used. Similarly, for multiplications
of float matrices and float scalars, ``OpMatrixTimesScalar`` will be generated.

Bitwise operators
-----------------

`Bitwise operators <https://msdn.microsoft.com/en-us/library/windows/desktop/bb509631(v=vs.85).aspx#Bitwise_Operators>`_
(``~``, ``&``, ``|``, ``^``, ``<<``, ``>>``) are translated into their
corresponding SPIR-V opcodes according to the following table.

+--------+-----------------------------+-------------------------------+
|        | (Vector of) Signed Integers | (Vector of) Unsigned Integers |
+========+=============================+===============================+
| ``~``  |                         ``OpNot``                           |
+--------+-------------------------------------------------------------+
| ``&``  |                      ``OpBitwiseAnd``                       |
+--------+-------------------------------------------------------------+
| ``|``  |                      ``OpBitwiseOr``                        |
+--------+-----------------------------+-------------------------------+
| ``^``  |                      ``OpBitwiseXor``                       |
+--------+-----------------------------+-------------------------------+
| ``<<`` |                   ``OpShiftLeftLogical``                    |
+--------+-----------------------------+-------------------------------+
| ``>>`` | ``OpShiftRightArithmetic``  | ``OpShiftRightLogical``       |
+--------+-----------------------------+-------------------------------+

Note that for ``<<``/``>>``, the right hand side will be culled: only the ``n``
- 1 least significant bits are considered, where ``n`` is the bitwidth of the
left hand side.

Comparison operators
--------------------

`Comparison operators <https://msdn.microsoft.com/en-us/library/windows/desktop/bb509631(v=vs.85).aspx#Comparison_Operators>`_
(``<``, ``<=``, ``>``, ``>=``, ``==``, ``!=``) are translated into their
corresponding SPIR-V opcodes according to the following table.

+--------+-----------------------------+-------------------------------+------------------------------+
|        | (Vector of) Signed Integers | (Vector of) Unsigned Integers |     (Vector of) Floats       |
+========+=============================+===============================+==============================+
| ``<``  |  ``OpSLessThan``            |  ``OpULessThan``              |  ``OpFOrdLessThan``          |
+--------+-----------------------------+-------------------------------+------------------------------+
| ``<=`` |  ``OpSLessThanEqual``       |  ``OpULessThanEqual``         |  ``OpFOrdLessThanEqual``     |
+--------+-----------------------------+-------------------------------+------------------------------+
| ``>``  |  ``OpSGreaterThan``         |  ``OpUGreaterThan``           |  ``OpFOrdGreaterThan``       |
+--------+-----------------------------+-------------------------------+------------------------------+
| ``>=`` |  ``OpSGreaterThanEqual``    |  ``OpUGreaterThanEqual``      |  ``OpFOrdGreaterThanEqual``  |
+--------+-----------------------------+-------------------------------+------------------------------+
| ``==`` |                     ``OpIEqual``                            |  ``OpFOrdEqual``             |
+--------+-------------------------------------------------------------+------------------------------+
| ``!=`` |                     ``OpINotEqual``                         |  ``OpFOrdNotEqual``          |
+--------+-------------------------------------------------------------+------------------------------+

Note that for comparison of (vectors of) floats, SPIR-V has two sets of
instructions: ``OpFOrd*``, ``OpFUnord*``. We translate into ``OpFOrd*`` ones.

Boolean math operators
----------------------

`Boolean match operators <https://msdn.microsoft.com/en-us/library/windows/desktop/bb509631(v=vs.85).aspx#Boolean_Math_Operators>`_
(``&&``, ``||``, ``?:``) are translated into their corresponding SPIR-V opcodes
according to the following table.

+--------+----------------------+
|        | (Vector of) Booleans |
+========+======================+
| ``&&`` |  ``OpLogicalAnd``    |
+--------+----------------------+
| ``||`` |  ``OpLogicalOr``     |
+--------+----------------------+
| ``?:`` |  ``OpSelect``        |
+--------+----------------------+

Please note that "unlike short-circuit evaluation of ``&&``, ``||``, and ``?:``
in C, HLSL expressions never short-circuit an evaluation because they are vector
operations. All sides of the expression are always evaluated."

Unary operators
---------------

For `unary operators <https://msdn.microsoft.com/en-us/library/windows/desktop/bb509631(v=vs.85).aspx#Unary_Operators>`_:

- ``!`` is translated into ``OpLogicalNot``. Parsing will gurantee the operands
  are of boolean types by inserting necessary casts.
- ``+`` requires no additional SPIR-V instructions.
- ``-`` is translated into ``OpSNegate`` and ``OpFNegate`` for (vectors of)
  integers and floats, respectively.

Casts
-----

Casting between (vectors) of scalar types is translated according to the following table:

+------------+-------------------+-------------------+-------------------+-------------------+
| From \\ To |        Bool       |       SInt        |      UInt         |       Float       |
+============+===================+===================+===================+===================+
|   Bool     |       no-op       |                 select between one and zero               |
+------------+-------------------+-------------------+-------------------+-------------------+
|   SInt     |                   |     no-op         |  ``OpBitcast``    | ``OpConvertSToF`` |
+------------+                   +-------------------+-------------------+-------------------+
|   UInt     | compare with zero |   ``OpBitcast``   |      no-op        | ``OpConvertUToF`` |
+------------+                   +-------------------+-------------------+-------------------+
|   Float    |                   | ``OpConvertFToS`` | ``OpConvertFToU`` |      no-op        |
+------------+-------------------+-------------------+-------------------+-------------------+

It is also feasible in HLSL to cast a float matrix to another float matrix with a smaller size.
This is known as matrix truncation cast. For instance, the following code casts a 3x4 matrix
into a 2x3 matrix.

.. code:: hlsl

  float3x4 m = { 1,  2,  3, 4,
                 5,  6,  7, 8,
                 9, 10, 11, 12 };

  float2x3 a = (float2x3)m;

Such casting takes the upper-left most corner of the original matrix to generate the result.
In the above example, matrix ``a`` will have 2 rows, with 3 columns each. First row will be
``1, 2, 3`` and the second row will be ``5, 6, 7``.

Indexing operator
-----------------

The ``[]`` operator can also be used to access elements in a matrix or vector.
A matrix whose row and/or column count is 1 will be translated into a vector or
scalar. If a variable is used as the index for the dimension whose count is 1,
that variable will be ignored in the generated SPIR-V code. This is because
out-of-bound indexing triggers undefined behavior anyway. For example, for a
1xN matrix ``mat``, ``mat[index][0]`` will be translated into
``OpAccessChain ... %mat %uint_0``. Similarly, variable index into a size 1
vector will also be ignored and the only element will be always returned.

Assignment operators
--------------------

Assigning to struct object may involve decomposing the source struct object and
assign each element separately and recursively. This happens when the source
struct object is of different memory layout from the destination struct object.
For example, for the following source code:

.. code:: hlsl

  struct S {
    float    a;
    float2   b;
    float2x3 c;
  };

      ConstantBuffer<S> cbuf;
  RWStructuredBuffer<S> sbuf;

  ...
  sbuf[0] = cbuf[0];
  ...

We need to assign each element because ``ConstantBuffer`` and
``RWStructuredBuffer`` has different memory layout.

HLSL Control Flows
==================

This section lists how various HLSL control flows are mapped.

Switch statement
----------------

HLSL `switch statements <https://msdn.microsoft.com/en-us/library/windows/desktop/bb509669(v=vs.85).aspx>`_
are translated into SPIR-V using:

- **OpSwitch**: if (all case values are integer literals or constant integer
  variables) and (no attribute or the ``forcecase`` attribute is specified)
- **A series of if statements**: for all other scenarios (e.g., when
  ``flatten``, ``branch``, or ``call`` attribute is specified)

Loops (for, while, do)
----------------------

HLSL `for statements <https://msdn.microsoft.com/en-us/library/windows/desktop/bb509602(v=vs.85).aspx>`_,
`while statements <https://msdn.microsoft.com/en-us/library/windows/desktop/bb509708(v=vs.85).aspx>`_,
and `do statements <https://msdn.microsoft.com/en-us/library/windows/desktop/bb509593(v=vs.85).aspx>`_
are translated into SPIR-V by constructing all necessary basic blocks and using
``OpLoopMerge`` to organize as structured loops.

The HLSL attributes for these statements are translated into SPIR-V loop control
masks according to the following table:

+-------------------------+--------------------------------------------------+
|   HLSL loop attribute   |            SPIR-V Loop Control Mask              |
+=========================+==================================================+
|        ``unroll(x)``    |                ``Unroll``                        |
+-------------------------+--------------------------------------------------+
|         ``loop``        |              ``DontUnroll``                      |
+-------------------------+--------------------------------------------------+
|        ``fastopt``      |              ``DontUnroll``                      |
+-------------------------+--------------------------------------------------+
| ``allow_uav_condition`` |           Currently Unimplemented                |
+-------------------------+--------------------------------------------------+

HLSL Functions
==============

All functions reachable from the entry-point function will be translated into
SPIR-V code. Functions not reachable from the entry-point function will be
ignored.

Entry function wrapper
----------------------

HLSL entry functions takes in parameters and returns values. These parameters
and return values can have semantics attached or if they are struct type,
the struct fields can have semantics attached. However, in Vulkan, the entry
function must be of the ``void(void)`` signature. To handle this difference,
for a given entry function ``main``, we will emit a wrapper function for it.

The wrapper function will take the name of the source code entry function,
while the source code entry function will have its name prefixed with "src.".
The wrapper function reads in stage input/builtin variables created according
to semantics and groups them into composites meeting the requirements of the
source code entry point. Then the wrapper calls the source code entry point.
The return value is extracted and components of it will be written to stage
output/builtin variables created according to semantics. For example:


.. code:: hlsl

  // HLSL source code

  struct S {
    bool a : A;
    uint2 b: B;
    float2x3 c: C;
  };

  struct T {
    S x;
    int y: D;
  };

  T main(T input) {
    return input;
  }


.. code:: spirv

  ; SPIR-V code

  %in_var_A = OpVariable %_ptr_Input_bool Input
  %in_var_B = OpVariable %_ptr_Input_v2uint Input
  %in_var_C = OpVariable %_ptr_Input_mat2v3float Input
  %in_var_D = OpVariable %_ptr_Input_int Input

  %out_var_A = OpVariable %_ptr_Output_bool Output
  %out_var_B = OpVariable %_ptr_Output_v2uint Output
  %out_var_C = OpVariable %_ptr_Output_mat2v3float Output
  %out_var_D = OpVariable %_ptr_Output_int Output

  ; Wrapper function starts

  %main    = OpFunction %void None ...
  ...      = OpLabel

  %param_var_input = OpVariable %_ptr_Function_T Function

  ; Load stage input variables and group into the expected composite

  %inA = OpLoad %bool %in_var_A
  %inB = OpLoad %v2uint %in_var_B
  %inC = OpLoad %mat2v3float %in_var_C
  %inS = OpCompositeConstruct %S %inA %inB %inC
  %inD = OpLoad %int %in_var_D
  %inT = OpCompositeConstruct %T %inS %inD
         OpStore %param_var_input %inT

  %ret = OpFunctionCall %T %src_main %param_var_input

  ; Extract component values from the composite and store into stage output variables

  %outS = OpCompositeExtract %S %ret 0
  %outA = OpCompositeExtract %bool %outS 0
          OpStore %out_var_A %outA
  %outB = OpCompositeExtract %v2uint %outS 1
          OpStore %out_var_B %outB
  %outC = OpCompositeExtract %mat2v3float %outS 2
          OpStore %out_var_C %outC
  %outD = OpCompositeExtract %int %ret 1
          OpStore %out_var_D %outD

  OpReturn
  OpFunctionEnd

  ; Source code entry point starts

  %src_main = OpFunction %T None ...

In this way, we can concentrate all stage input/output/builtin variable
manipulation in the wrapper function and handle the source code entry function
just like other nomal functions.

Function parameter
------------------

For a function ``f`` which has a parameter of type ``T``, the generated SPIR-V
signature will use type ``T*`` for the parameter. At every call site of ``f``,
additional local variables will be allocated to hold the actual arguments.
The local variables are passed in as direct function arguments. For example:

.. code:: hlsl

  // HLSL source code

  float4 f(float a, int b) { ... }

  void caller(...) {
    ...
    float4 result = f(...);
    ...
  }

.. code:: spirv

  ; SPIR-V code

                ...
  %i32PtrType = OpTypePointer Function %int
  %f32PtrType = OpTypePointer Function %float
      %fnType = OpTypeFunction %v4float %f32PtrType %i32PtrType
                ...

           %f = OpFunction %v4float None %fnType
           %a = OpFunctionParameter %f32PtrType
           %b = OpFunctionParameter %i32PtrType
                ...

      %caller = OpFunction ...
                ...
     %aAlloca = OpVariable %_ptr_Function_float Function
     %bAlloca = OpVariable %_ptr_Function_int Function
                ...
                OpStore %aAlloca ...
                OpStore %bAlloca ...
      %result = OpFunctioncall %v4float %f %aAlloca %bAlloca
                ...

This approach gives us unified handling of function parameters and local
variables: both of them are accessed via load/store instructions.

Intrinsic functions
-------------------

The following intrinsic HLSL functions have no direct SPIR-V opcode or GLSL
extended instruction mapping, so they are handled with additional steps:

- ``dot`` : performs dot product of two vectors, each containing floats or
  integers. If the two parameters are vectors of floats, we use SPIR-V's
  ``OpDot`` instruction to perform the translation. If the two parameters are
  vectors of integers, we multiply corresponding vector elements using
  ``OpIMul`` and accumulate the results using ``OpIAdd`` to compute the dot
  product.
- ``mul``: performs multiplications. Each argument may be a scalar, vector,
  or matrix. Depending on the argument type, this will be translated into
  one of the multiplication instructions.
- ``all``: returns true if all components of the given scalar, vector, or
  matrix are true. Performs conversions to boolean where necessary. Uses SPIR-V
  ``OpAll`` for scalar arguments and vector arguments. For matrix arguments,
  performs ``OpAll`` on each row, and then again on the vector containing the
  results of all rows.
- ``any``: returns true if any component of the given scalar, vector, or matrix
  is true. Performs conversions to boolean where necessary. Uses SPIR-V
  ``OpAny`` for scalar arguments and vector arguments. For matrix arguments,
  performs ``OpAny`` on each row, and then again on the vector containing the
  results of all rows.
- ``asfloat``: converts the component type of a scalar/vector/matrix from float,
  uint, or int into float. Uses ``OpBitcast``. This method currently does not
  support taking non-float matrix arguments.
- ``asint``: converts the component type of a scalar/vector/matrix from float or
  uint into int. Uses ``OpBitcast``. This method currently does not support
  conversion into integer matrices.
- ``asuint``: converts the component type of a scalar/vector/matrix from float
  or int into uint. Uses ``OpBitcast``. This method currently does not support
- ``asuint``: Converts a double into two 32-bit unsigned integers. Uses SPIR-V ``OpBitCast``.
- ``asdouble``: Converts two 32-bit unsigned integers into a double, or four 32-bit unsigned
  integers into two doubles. Uses SPIR-V ``OpVectorShuffle`` and ``OpBitCast``.
  conversion into unsigned integer matrices.
- ``isfinite`` : Determines if the specified value is finite. Since ``OpIsFinite``
  requires the ``Kernel`` capability, translation is done using ``OpIsNan`` and
  ``OpIsInf``.  A given value is finite iff it is not NaN and not infinite.
- ``clip``: Discards the current pixel if the specified value is less than zero.
  Uses conditional control flow as well as SPIR-V ``OpKill``.
- ``rcp``: Calculates a fast, approximate, per-component reciprocal.
  Uses SIR-V ``OpFDiv``.
- ``lit``: Returns a lighting coefficient vector. This vector is a float4 with
  components of (ambient, diffuse, specular, 1). How ``diffuse`` and ``specular``
  are calculated are explained `here <https://msdn.microsoft.com/en-us/library/windows/desktop/bb509619(v=vs.85).aspx>`_.
- ``D3DCOLORtoUBYTE4``: Converts a floating-point, 4D vector set by a D3DCOLOR to a UBYTE4.
  This is achieved by performing ``int4(input.zyxw * 255.002)`` using SPIR-V ``OpVectorShuffle``,
  ``OpVectorTimesScalar``, and ``OpConvertFToS``, respectively.
- ``dst``: Calculates a distance vector. The resulting vector, ``dest``, has the following specifications:
  ``dest.x = 1.0``, ``dest.y = src0.y * src1.y``, ``dest.z = src0.z``, and ``dest.w = src1.w``.
  Uses SPIR-V ``OpCompositeExtract`` and ``OpFMul``.

Using SPIR-V opcode
~~~~~~~~~~~~~~~~~~~

The following intrinsic HLSL functions have direct SPIR-V opcodes for them:

==================================== =================================
   HLSL Intrinsic Function                   SPIR-V Opcode
==================================== =================================
``AllMemoryBarrier``                 ``OpMemoryBarrier``
``AllMemoryBarrierWithGroupSync``    ``OpControlBarrier``
``countbits``                        ``OpBitCount``
``DeviceMemoryBarrier``              ``OpMemoryBarrier``
``DeviceMemoryBarrierWithGroupSync`` ``OpControlBarrier``
``ddx``                              ``OpDPdx``
``ddy``                              ``OpDPdy``
``ddx_coarse``                       ``OpDPdxCoarse``
``ddy_coarse``                       ``OpDPdyCoarse``
``ddx_fine``                         ``OpDPdxFine``
``ddy_fine``                         ``OpDPdyFine``
``fmod``                             ``OpFRem``
``fwidth``                           ``OpFwidth``
``GroupMemoryBarrier``               ``OpMemoryBarrier``
``GroupMemoryBarrierWithGroupSync``  ``OpControlBarrier``
``InterlockedAdd``                   ``OpAtomicIAdd``
``InterlockedAnd``                   ``OpAtomicAnd``
``InterlockedOr``                    ``OpAtomicOr``
``InterlockedXor``                   ``OpAtomicXor``
``InterlockedMin``                   ``OpAtomicUMin``/``OpAtomicSMin``
``InterlockedMax``                   ``OpAtomicUMax``/``OpAtomicSMax``
``InterlockedExchange``              ``OpAtomicExchange``
``InterlockedCompareExchange``       ``OpAtomicCompareExchange``
``InterlockedCompareStore``          ``OpAtomicCompareExchange``
``isnan``                            ``OpIsNan``
``isInf``                            ``OpIsInf``
``reversebits``                      ``OpBitReverse``
``transpose``                        ``OpTranspose``
``CheckAccessFullyMapped``           ``OpImageSparseTexelsResident``
==================================== =================================

Using GLSL extended instructions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The following intrinsic HLSL functions are translated using their equivalent
instruction in the `GLSL extended instruction set <https://www.khronos.org/registry/spir-v/specs/1.0/GLSL.std.450.html>`_.

======================= ===================================
HLSL Intrinsic Function   GLSL Extended Instruction
======================= ===================================
``abs``                 ``SAbs``/``FAbs``
``acos``                ``Acos``
``asin``                ``Asin``
``atan``                ``Atan``
``atan2``               ``Atan2``
``ceil``                ``Ceil``
``clamp``               ``SClamp``/``UClamp``/``FClamp``
``cos``                 ``Cos``
``cosh``                ``Cosh``
``cross``               ``Cross``
``degrees``             ``Degrees``
``distance``            ``Distance``
``radians``             ``Radian``
``determinant``         ``Determinant``
``exp``                 ``Exp``
``exp2``                ``exp2``
``f16tof32``            ``UnpackHalf2x16``
``f32tof16``            ``PackHalf2x16``
``faceforward``         ``FaceForward``
``firstbithigh``        ``FindSMsb`` / ``FindUMsb``
``firstbitlow``         ``FindILsb``
``floor``               ``Floor``
``fma``                 ``Fma``
``frac``                ``Fract``
``frexp``               ``FrexpStruct``
``ldexp``               ``Ldexp``
``length``              ``Length``
``lerp``                ``FMix``
``log``                 ``Log``
``log10``               ``Log2`` (scaled by ``1/log2(10)``)
``log2``                ``Log2``
``mad``                 ``Fma``
``max``                 ``SMax``/``UMax``/``NMax``/``FMax``
``min``                 ``SMin``/``UMin``/``NMin``/``FMin``
``modf``                ``ModfStruct``
``normalize``           ``Normalize``
``pow``                 ``Pow``
``reflect``             ``Reflect``
``refract``             ``Refract``
``round``               ``RoundEven``
``rsqrt``               ``InverseSqrt``
``saturate``            ``FClamp``
``sign``                ``SSign``/``FSign``
``sin``                 ``Sin``
``sincos``              ``Sin`` and ``Cos``
``sinh``                ``Sinh``
``smoothstep``          ``SmoothStep``
``sqrt``                ``Sqrt``
``step``                ``Step``
``tan``                 ``Tan``
``tanh``                ``Tanh``
``trunc``               ``Trunc``
======================= ===================================

Note on NMax,Nmin,FMax & FMin:

This compiler supports the ``--ffinite-math-only`` option, which allows
assuming non-NaN parameters to some operations. ``min`` & ``max`` intrinsics
will by default generate ``NMin`` & ``NMax`` instructions, but if this option
is enabled, ``FMin`` & ``FMax`` can be generated instead.

Synchronization intrinsics
~~~~~~~~~~~~~~~~~~~~~~~~~~

Synchronization intrinsics are translated into ``OpMemoryBarrier`` (for those
non-``WithGroupSync`` variants) or ``OpControlBarrier`` (for those ``WithGroupSync``
variants) instructions with parameters:

======================= ============ ===== ======= ========= ==============
       HLSL                SPIR-V          SPIR-V Memory Semantics
----------------------- ------------ --------------------------------------
     Intrinsic          Memory Scope Image Uniform Workgroup AcquireRelease
======================= ============ ===== ======= ========= ==============
``AllMemoryBarrier``    Device       ✓       ✓         ✓          ✓
``DeviceMemoryBarrier`` Device       ✓       ✓                    ✓
``GroupMemoryBarrier``  Workgroup                       ✓          ✓
======================= ============ ===== ======= ========= ==============

For the ``*WithGroupSync`` intrinsics, SPIR-V memory scope and semantics are the
same as their counterparts in the above. They have an additional execution
scope:

==================================== ======================
       HLSL Intrinsic                SPIR-V Execution Scope
==================================== ======================
``AllMemoryBarrierWithGroupSync``    Workgroup
``DeviceMemoryBarrierWithGroupSync`` Workgroup
``GroupMemoryBarrierWithGroupSync``  Workgroup
==================================== ======================

HLSL OO features
================

A HLSL struct/class member method is translated into a normal SPIR-V function,
whose signature has an additional first parameter for the struct/class called
upon. Every calling site of the method is generated to pass in the object as
the first argument.

HLSL struct/class static member variables are translated into SPIR-V variables
in the ``Private`` storage class.

HLSL Methods
============

This section lists how various HLSL methods are mapped.

Buffers
-------

``Buffer``
~~~~~~~~~~

``.Load()``
+++++++++++
Since Buffers are represented as ``OpTypeImage`` with ``Sampled`` set to 1
(meaning to be used with a sampler), ``OpImageFetch`` is used to perform this
operation. The return value of ``OpImageFetch`` is always a four-component
vector; so proper additional instructions are generated to truncate the vector
and return the desired number of elements.
If an output unsigned integer ``status`` argument is present, ``OpImageSparseFetch``
is used instead. The resulting SPIR-V ``Residency Code`` will be written to ``status``.

``operator[]``
++++++++++++++
Handled similarly as ``.Load()``.

``.GetDimensions()``
++++++++++++++++++++
Since Buffers are represented as ``OpTypeImage`` with dimension of ``Buffer``,
``OpImageQuerySize`` is used to perform this operation.

``RWBuffer``
~~~~~~~~~~~~

``.Load()``
+++++++++++
Since RWBuffers are represented as ``OpTypeImage`` with ``Sampled`` set to 2
(meaning to be used without a sampler), ``OpImageRead`` is used to perform this
operation. If an output unsigned integer ``status`` argument is present, ``OpImageSparseRead``
is used instead. The resulting SPIR-V ``Residency Code`` will be written to ``status``.

``operator[]``
++++++++++++++
Using ``operator[]`` for reading is handled similarly as ``.Load()``, while for
writing, the ``OpImageWrite`` instruction is generated.

``.GetDimensions()``
++++++++++++++++++++
Since RWBuffers are represented as ``OpTypeImage`` with dimension of ``Buffer``,
``OpImageQuerySize`` is used to perform this operation.

``StructuredBuffer`` and ``RWStructuredBuffer``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``.GetDimensions()``
++++++++++++++++++++
Since StructuredBuffers/RWStructuredBuffers are represented as a struct with one
member that is a runtime array of structures, ``OpArrayLength`` is invoked on
the runtime array in order to find the dimension.

``ByteAddressBuffer``
~~~~~~~~~~~~~~~~~~~~~

``.GetDimensions()``
++++++++++++++++++++
Since ByteAddressBuffers are represented as a struct with one member that is a
runtime array of unsigned integers, ``OpArrayLength`` is invoked on the runtime array
in order to find the number of unsigned integers. This is then multiplied by 4 to find
the number of bytes.

``.Load()``, ``.Load2()``, ``.Load3()``, ``.Load4()``
+++++++++++++++++++++++++++++++++++++++++++++++++++++
ByteAddressBuffers are represented as a struct with one member that is a runtime array of
unsigned integers. The ``address`` argument passed to the function is first divided by 4
in order to find the offset into the array (because each array element is 4 bytes). The
SPIR-V ``OpAccessChain`` instruction is then used to access that offset, and ``OpLoad`` is
used to load a 32-bit unsigned integer. For ``Load2``, ``Load3``, and ``Load4``, this is
done 2, 3, and 4 times, respectively. Each time the word offset is incremented by 1 before
performing ``OpAccessChain``. After all ``OpLoad`` operations are performed, a vector is
constructed with all the resulting values.

``RWByteAddressBuffer``
~~~~~~~~~~~~~~~~~~~~~~~

``.GetDimensions()``
++++++++++++++++++++
Since RWByteAddressBuffers are represented as a struct with one member that is a
runtime array of unsigned integers, ``OpArrayLength`` is invoked on the runtime array
in order to find the number of unsigned integers. This is then multiplied by 4 to find
the number of bytes.

``.Load()``, ``.Load2()``, ``.Load3()``, ``.Load4()``
+++++++++++++++++++++++++++++++++++++++++++++++++++++
RWByteAddressBuffers are represented as a struct with one member that is a runtime array of
unsigned integers. The ``address`` argument passed to the function is first divided by 4
in order to find the offset into the array (because each array element is 4 bytes). The
SPIR-V ``OpAccessChain`` instruction is then used to access that offset, and ``OpLoad`` is
used to load a 32-bit unsigned integer. For ``Load2``, ``Load3``, and ``Load4``, this is
done 2, 3, and 4 times, respectively. Each time the word offset is incremented by 1 before
performing ``OpAccessChain``. After all ``OpLoad`` operations are performed, a vector is
constructed with all the resulting values.

``.Store()``, ``.Store2()``, ``.Store3()``, ``.Store4()``
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
RWByteAddressBuffers are represented as a struct with one member that is a runtime array of
unsigned integers. The ``address`` argument passed to the function is first divided by 4
in order to find the offset into the array (because each array element is 4 bytes). The
SPIR-V ``OpAccessChain`` instruction is then used to access that offset, and ``OpStore`` is
used to store a 32-bit unsigned integer. For ``Store2``, ``Store3``, and ``Store4``, this is
done 2, 3, and 4 times, respectively. Each time the word offset is incremented by 1 before
performing ``OpAccessChain``.

``.Interlocked*()``
+++++++++++++++++++

================================= =================================
     HLSL Intrinsic Method                SPIR-V Opcode
================================= =================================
``.InterlockedAdd()``             ``OpAtomicIAdd``
``.InterlockedAnd()``             ``OpAtomicAnd``
``.InterlockedOr()``              ``OpAtomicOr``
``.InterlockedXor()``             ``OpAtomicXor``
``.InterlockedMin()``             ``OpAtomicUMin``/``OpAtomicSMin``
``.InterlockedMax()``             ``OpAtomicUMax``/``OpAtomicSMax``
``.InterlockedExchange()``        ``OpAtomicExchange``
``.InterlockedCompareExchange()`` ``OpAtomicCompareExchange``
``.InterlockedCompareStore()``    ``OpAtomicCompareExchange``
================================= =================================

``AppendStructuredBuffer``
~~~~~~~~~~~~~~~~~~~~~~~~~~

``.Append()``
+++++++++++++

The associated counter number will be increased by 1 using ``OpAtomicIAdd``.
The return value of ``OpAtomicIAdd``, which is the original count number, will
be used as the index for storing the new element. E.g., for ``buf.Append(vec)``:

.. code:: spirv

  %counter = OpAccessChain %_ptr_Uniform_int %counter_var_buf %uint_0
    %index = OpAtomicIAdd %uint %counter %uint_1 %uint_0 %uint_1
      %ptr = OpAccessChain %_ptr_Uniform_v4float %buf %uint_0 %index
      %val = OpLoad %v4float %vec
             OpStore %ptr %val

``.GetDimensions()``
++++++++++++++++++++
Since AppendStructuredBuffers are represented as a struct with one member that
is a runtime array, ``OpArrayLength`` is invoked on the runtime array in order
to find the number of elements. The stride is also calculated based on GLSL
``std430`` as explained above.

``ConsumeStructuredBuffer``
~~~~~~~~~~~~~~~~~~~~~~~~~~~

``.Consume()``
++++++++++++++

The associated counter number will be decreased by 1 using ``OpAtomicISub``.
The return value of ``OpAtomicISub`` minus 1, which is the new count number,
will be used as the index for reading the new element. E.g., for
``buf.Consume(vec)``:

.. code:: spirv

  %counter = OpAccessChain %_ptr_Uniform_int %counter_var_buf %uint_0
     %prev = OpAtomicISub %uint %counter %uint_1 %uint_0 %uint_1
    %index = OpISub %uint %prev %uint_1
      %ptr = OpAccessChain %_ptr_Uniform_v4float %buf %uint_0 %index
      %val = OpLoad %v4float %vec
             OpStore %ptr %val

``.GetDimensions()``
++++++++++++++++++++
Since ConsumeStructuredBuffers are represented as a struct with one member that
is a runtime array, ``OpArrayLength`` is invoked on the runtime array in order
to find the number of elements. The stride is also calculated based on GLSL
``std430`` as explained above.

Read-only textures
------------------

Methods common to all texture types are explained in the "common texture methods"
section. Methods unique to a specific texture type is explained in the section
for that texture type.

Common texture methods
~~~~~~~~~~~~~~~~~~~~~~

``.Sample(sampler, location[, offset][, clamp][, Status])``
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Not available to ``Texture2DMS`` and ``Texture2DMSArray``.

The ``OpImageSampleImplicitLod`` instruction is used to translate ``.Sample()``
since texture types are represented as ``OpTypeImage``. An ``OpSampledImage`` is
created based on the ``sampler`` passed to the function. The resulting sampled
image and the ``location`` passed to the function are used as arguments to
``OpImageSampleImplicitLod``, with the optional ``offset`` tranlated into
addtional SPIR-V image operands ``ConstOffset`` or ``Offset`` on it. The optional
``clamp`` argument will be translated to the ``MinLod`` image operand.

If an output unsigned integer ``status`` argument is present,
``OpImageSparseSampleImplicitLod`` is used instead. The resulting SPIR-V
``Residency Code`` will be written to ``status``.

``.SampleLevel(sampler, location, lod[, offset][, Status])``
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Not available to ``Texture2DMS`` and ``Texture2DMSArray``.

The ``OpImageSampleExplicitLod`` instruction is used to translate this method.
An ``OpSampledImage`` is created based on the ``sampler`` passed to the function.
The resulting sampled image and the ``location`` passed to the function are used
as arguments to ``OpImageSampleExplicitLod``. The ``lod`` passed to the function
is attached to the instruction as an SPIR-V image operands ``Lod``. The optional
``offset`` is also tranlated into addtional SPIR-V image operands ``ConstOffset``
or ``Offset`` on it.

If an output unsigned integer ``status`` argument is present,
``OpImageSparseSampleExplicitLod`` is used instead. The resulting SPIR-V
``Residency Code`` will be written to ``status``.

``.SampleGrad(sampler, location, ddx, ddy[, offset][, clamp][, Status])``
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Not available to ``Texture2DMS`` and ``Texture2DMSArray``.

Similarly to ``.SampleLevel``, the ``ddx`` and ``ddy`` parameter are attached to
the ``OpImageSampleExplicitLod`` instruction as an SPIR-V image operands
``Grad``. The optional ``clamp`` argument will be translated into the ``MinLod``
image operand.

If an output unsigned integer ``status`` argument is present,
``OpImageSparseSampleExplicitLod`` is used instead. The resulting SPIR-V
``Residency Code`` will be written to ``status``.

``.SampleBias(sampler, location, bias[, offset][, clamp][, Status])``
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Not available to ``Texture2DMS`` and ``Texture2DMSArray``.

The translation is similar to ``.Sample()``, with the ``bias`` parameter
attached to the ``OpImageSampleImplicitLod`` instruction as an SPIR-V image
operands ``Bias``.

If an output unsigned integer ``status`` argument is present,
``OpImageSparseSampleImplicitLod`` is used instead. The resulting SPIR-V
``Residency Code`` will be written to ``status``.

``.SampleCmp(sampler, location, comparator[, offset][, clamp][, Status])``
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Not available to ``Texture3D``, ``Texture2DMS``, and ``Texture2DMSArray``.

The translation is similar to ``.Sample()``, but the
``OpImageSampleDrefImplicitLod`` instruction are used.

If an output unsigned integer ``status`` argument is present,
``OpImageSparseSampleDrefImplicitLod`` is used instead. The resulting SPIR-V
``Residency Code`` will be written to ``status``.

``.SampleCmpLevelZero(sampler, location, comparator[, offset][, Status])``
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Not available to ``Texture3D``, ``Texture2DMS``, and ``Texture2DMSArray``.

The translation is similar to ``.Sample()``, but the
``OpImageSampleDrefExplicitLod`` instruction are used, with the additional
``Lod`` image operands set to 0.0.

If an output unsigned integer ``status`` argument is present,
``OpImageSparseSampleDrefExplicitLod`` is used instead. The resulting SPIR-V
``Residency Code`` will be written to ``status``.

``.SampleCmpBias(sampler, location, bias, comparator[, offset][, clamp][, Status])``
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Not available to ``Texture3D``, ``Texture2DMS``, and ``Texture2DMSArray``.

The translation is similar to ``.SampleBias()``, but the
``OpImageSampleDrefImplicitLod`` instruction is used.

If an output unsigned integer ``status`` argument is present,
``OpImageSparseSampleDrefImplicitLod`` is used instead. The resulting SPIR-V
``Residency Code`` will be written to ``status``.

``.SampleCmpGrad(sampler, location, ddx, ddy, comparator[, offset][, clamp][, Status])``
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Not available to ``Texture3D``, ``Texture2DMS``, and ``Texture2DMSArray``.

The translation is similar to ``.SampleGrad()``, but the
``OpImageSampleDrefExplicitLod`` instruction are used.

If an output unsigned integer ``status`` argument is present,
``OpImageSparseSampleDrefExplicitLod`` is used instead. The resulting SPIR-V
``Residency Code`` will be written to ``status``.

``.Gather()``
+++++++++++++

Available to ``Texture2D``, ``Texture2DArray``, ``TextureCube``, and
``TextureCubeArray``.

The translation is similar to ``.Sample()``, but the ``OpImageGather``
instruction is used, with component setting to 0.

If an output unsigned integer ``status`` argument is present,
``OpImageSparseGather`` is used instead. The resulting SPIR-V
``Residency Code`` will be written to ``status``.

``.GatherRed()``, ``.GatherGreen()``, ``.GatherBlue()``, ``.GatherAlpha()``
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Available to ``Texture2D``, ``Texture2DArray``, ``TextureCube``, and
``TextureCubeArray``.

The ``OpImageGather`` instruction is used to translate these functions, with
component setting to 0, 1, 2, and 3 respectively.

There are a few overloads for these functions:

- For those overloads taking 4 offset parameters, those offset parameters will
  be conveyed as an additional ``ConstOffsets`` image operands to the
  instruction if those offset parameters are all constants. Otherwise,
  4 separate ``OpImageGather`` instructions will be emitted to get each texel
  from each offset, using the ``Offset`` image operands.
- For those overloads with the ``status`` parameter, ``OpImageSparseGather``
  is used instead, and the resulting SPIR-V ``Residency Code`` will be
  written to ``status``.

``.GatherCmp()``
++++++++++++++++

Available to ``Texture2D``, ``Texture2DArray``, ``TextureCube``, and
``TextureCubeArray``.

The translation is similar to ``.Sample()``, but the ``OpImageDrefGather``
instruction is used.

For the overload with the output unsigned integer ``status`` argument,
``OpImageSparseDrefGather`` is used instead. The resulting SPIR-V
``Residency Code`` will be written to ``status``.


``.GatherCmpRed()``
+++++++++++++++++++

Available to ``Texture2D``, ``Texture2DArray``, ``TextureCube``, and
``TextureCubeArray``.

The translation is the same as ``.GatherCmp()``.

``.Load(location[, sampleIndex][, offset])``
++++++++++++++++++++++++++++++++++++++++++++

The ``OpImageFetch`` instruction is used for translation because texture types
are represented as ``OpTypeImage``. The last element in the ``location``
parameter will be used as arguments to the ``Lod`` SPIR-V image operand attached
to the ``OpImageFetch`` instruction, and the rest are used as the coordinate
argument to the instruction. ``offset`` is handled similarly to ``.Sample()``.
The return value of ``OpImageFetch`` is always a four-component vector; so
proper additional instructions are generated to truncate the vector and return
the desired number of elements.

For the overload with the output unsigned integer ``status`` argument,
``OpImageSparseFetch`` is used instead. The resulting SPIR-V
``Residency Code`` will be written to ``status``.

``operator[]``
++++++++++++++
Handled similarly as ``.Load()``.

``.mips[lod][position]``
++++++++++++++++++++++++

Not available to ``TextureCube``, ``TextureCubeArray``, ``Texture2DMS``, and
``Texture2DMSArray``.

This method is translated into the ``OpImageFetch`` instruction. The ``lod``
parameter is attached to the instruction as the parameter to the ``Lod`` SPIR-V
image operands. The ``position`` parameter are used as the coordinate to the
instruction directly.

``.CalculateLevelOfDetail()`` and ``.CalculateLevelOfDetailUnclamped()``
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Not available to ``Texture2DMS`` and ``Texture2DMSArray``.

Since texture types are represented as ``OpTypeImage``, the ``OpImageQueryLod``
instruction is used for translation. An ``OpSampledImage`` is created based on
the ``SamplerState`` or ``SamplerComparisonState`` passed to the function. The
resulting sampled image and the coordinate passed to the function are used to
invoke ``OpImageQueryLod``. The result of ``OpImageQueryLod`` is a ``float2``.
The first element contains the mipmap array layer. The second element contains
the unclamped level of detail.

``Texture1D``
~~~~~~~~~~~~~

``.GetDimensions(width)`` or ``.GetDimensions(MipLevel, width, NumLevels)``
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Since Texture1D is represented as ``OpTypeImage``, the ``OpImageQuerySizeLod`` instruction
is used for translation. If a ``MipLevel`` argument is passed to ``GetDimensions``, it will
be used as the ``Lod`` parameter of the query instruction. Otherwise, ``Lod`` of ``0`` be used.

``Texture1DArray``
~~~~~~~~~~~~~~~~~~

``.GetDimensions(width, elements)`` or ``.GetDimensions(MipLevel, width, elements, NumLevels)``
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Since Texture1DArray is represented as ``OpTypeImage``, the ``OpImageQuerySizeLod`` instruction
is used for translation. If a ``MipLevel`` argument is present, it will be used as the
``Lod`` parameter of the query instruction. Otherwise, ``Lod`` of ``0`` be used.

``Texture2D``
~~~~~~~~~~~~~

``.GetDimensions(width, height)`` or ``.GetDimensions(MipLevel, width, height, NumLevels)``
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Since Texture2D is represented as ``OpTypeImage``, the ``OpImageQuerySizeLod`` instruction
is used for translation. If a ``MipLevel`` argument is present, it will be used as the
``Lod`` parameter of the query instruction. Otherwise, ``Lod`` of ``0`` be used.

``Texture2DArray``
~~~~~~~~~~~~~~~~~~

``.GetDimensions(width, height, elements)`` or ``.GetDimensions(MipLevel, width, height, elements, NumLevels)``
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Since Texture2DArray is represented as ``OpTypeImage``, the ``OpImageQuerySizeLod`` instruction
is used for translation. If a ``MipLevel`` argument is present, it will be used as the
``Lod`` parameter of the query instruction. Otherwise, ``Lod`` of ``0`` be used.

``Texture3D``
~~~~~~~~~~~~~

``.GetDimensions(width, height, depth)`` or ``.GetDimensions(MipLevel, width, height, depth, NumLevels)``
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Since Texture3D is represented as ``OpTypeImage``, the ``OpImageQuerySizeLod`` instruction
is used for translation. If a ``MipLevel`` argument is present, it will be used as the
``Lod`` parameter of the query instruction. Otherwise, ``Lod`` of ``0`` be used.

``Texture2DMS``
~~~~~~~~~~~~~~~

``.sample[sample][position]``
+++++++++++++++++++++++++++++
This method is translated into the ``OpImageFetch`` instruction. The ``sample``
parameter is attached to the instruction as the parameter to the ``Sample``
SPIR-V image operands. The ``position`` parameter are used as the coordinate to
the instruction directly.

``.GetDimensions(width, height, numSamples)``
+++++++++++++++++++++++++++++++++++++++++++++
Since Texture2DMS is represented as ``OpTypeImage`` with ``MS`` of ``1``, the ``OpImageQuerySize`` instruction
is used to get the width and the height. Furthermore, ``OpImageQuerySamples`` is used to get the numSamples.

``.GetSamplePosition(index)``
+++++++++++++++++++++++++++++
There are no direct mapping SPIR-V instructions for this method. Right now, it
is translated into the SPIR-V code for the following HLSL source code:

.. code:: hlsl

  // count is the number of samples in the Texture2DMS(Array)
  // index is the index of the sample we are trying to get the position

  static const float2 pos2[] = {
      { 4.0/16.0,  4.0/16.0 }, {-4.0/16.0, -4.0/16.0 },
  };

  static const float2 pos4[] = {
      {-2.0/16.0, -6.0/16.0 }, { 6.0/16.0, -2.0/16.0 }, {-6.0/16.0,  2.0/16.0 }, { 2.0/16.0,  6.0/16.0 },
  };

  static const float2 pos8[] = {
      { 1.0/16.0, -3.0/16.0 }, {-1.0/16.0,  3.0/16.0 }, { 5.0/16.0,  1.0/16.0 }, {-3.0/16.0, -5.0/16.0 },
      {-5.0/16.0,  5.0/16.0 }, {-7.0/16.0, -1.0/16.0 }, { 3.0/16.0,  7.0/16.0 }, { 7.0/16.0, -7.0/16.0 },
  };

  static const float2 pos16[] = {
      { 1.0/16.0,  1.0/16.0 }, {-1.0/16.0, -3.0/16.0 }, {-3.0/16.0,  2.0/16.0 }, { 4.0/16.0, -1.0/16.0 },
      {-5.0/16.0, -2.0/16.0 }, { 2.0/16.0,  5.0/16.0 }, { 5.0/16.0,  3.0/16.0 }, { 3.0/16.0, -5.0/16.0 },
      {-2.0/16.0,  6.0/16.0 }, { 0.0/16.0, -7.0/16.0 }, {-4.0/16.0, -6.0/16.0 }, {-6.0/16.0,  4.0/16.0 },
      {-8.0/16.0,  0.0/16.0 }, { 7.0/16.0, -4.0/16.0 }, { 6.0/16.0,  7.0/16.0 }, {-7.0/16.0, -8.0/16.0 },
  };

  float2 position = float2(0.0f, 0.0f);

  if (count == 2) {
      position = pos2[index];
  } else if (count == 4) {
      position = pos4[index];
  } else if (count == 8) {
      position = pos8[index];
  } else if (count == 16) {
      position = pos16[index];
  }

From the above, it's clear that the current implementation only supports standard
sample settings, i.e., with 1, 2, 4, 8, or 16 samples. For other cases, the
implementation will just return `(float2)0`.

``Texture2DMSArray``
~~~~~~~~~~~~~~~~~~~~

``.sample[sample][position]``
+++++++++++++++++++++++++++++
This method is translated into the ``OpImageFetch`` instruction. The ``sample``
parameter is attached to the instruction as the parameter to the ``Sample``
SPIR-V image operands. The ``position`` parameter are used as the coordinate to
the instruction directly.

``.GetDimensions(width, height, elements, numSamples)``
+++++++++++++++++++++++++++++++++++++++++++++++++++++++
Since Texture2DMS is represented as ``OpTypeImage`` with ``MS`` of ``1``, the ``OpImageQuerySize`` instruction
is used to get the width, the height, and the elements. Furthermore, ``OpImageQuerySamples`` is used to get the numSamples.

``.GetSamplePosition(index)``
+++++++++++++++++++++++++++++
Similar to Texture2D.

``TextureCube``
~~~~~~~~~~~~~~~

``TextureCubeArray``
~~~~~~~~~~~~~~~~~~~~

Read-write textures
-------------------

Methods common to all texture types are explained in the "common texture methods"
section. Methods unique to a specific texture type is explained in the section
for that texture type.

Common texture methods
~~~~~~~~~~~~~~~~~~~~~~

``.Load()``
+++++++++++
Since read-write texture types are represented as ``OpTypeImage`` with
``Sampled`` set to 2 (meaning to be used without a sampler), ``OpImageRead`` is
used to perform this operation.

For the overload with the output unsigned integer ``status`` argument,
``OpImageSparseRead`` is used instead. The resulting SPIR-V
``Residency Code`` will be written to ``status``.

``operator[]``
++++++++++++++
Using ``operator[]`` for reading is handled similarly as ``.Load()``, while for
writing, the ``OpImageWrite`` instruction is generated.

``RWTexture1D``
~~~~~~~~~~~~~~~

``.GetDimensions(width)``
+++++++++++++++++++++++++
The ``OpImageQuerySize`` instruction is used to find the width.

``RWTexture1DArray``
~~~~~~~~~~~~~~~~~~~~

``.GetDimensions(width, elements)``
+++++++++++++++++++++++++++++++++++
The ``OpImageQuerySize`` instruction is used to get a uint2. The first element
is the width, and the second is the elements.

``RWTexture2D``
~~~~~~~~~~~~~~~

``.GetDimensions(width, height)``
+++++++++++++++++++++++++++++++++
The ``OpImageQuerySize`` instruction is used to get a uint2. The first element is the width, and the second
element is the height.

``RWTexture2DArray``
~~~~~~~~~~~~~~~~~~~~

``.GetDimensions(width, height, elements)``
+++++++++++++++++++++++++++++++++++++++++++
The ``OpImageQuerySize`` instruction is used to get a uint3. The first element is the width, the second
element is the height, and the third is the elements.

``RWTexture3D``
~~~~~~~~~~~~~~~

``.GetDimensions(width, height, depth)``
++++++++++++++++++++++++++++++++++++++++
The ``OpImageQuerySize`` instruction is used to get a uint3. The first element is the width, the second
element is the height, and the third element is the depth.

HLSL Shader Stages
==================

Hull Shaders
------------

Hull shaders corresponds to Tessellation Control Shaders (TCS) in Vulkan.
This section describes how Hull shaders are translated to SPIR-V for Vulkan.

Hull Entry Point Attributes
~~~~~~~~~~~~~~~~~~~~~~~~~~~
The following HLSL attributes are attached to the main entry point of hull shaders
and are translated to SPIR-V execution modes according to the table below:

.. table:: Mapping from HLSL attribute to SPIR-V execution mode

+-------------------------+---------------------+--------------------------+
| HLSL Attribute          |   value             | SPIR-V Execution Mode    |
+=========================+=====================+==========================+
|                         | ``quad``            | ``Quads``                |
|                         +---------------------+--------------------------+
|    ``domain``           | ``tri``             | ``Triangles``            |
|                         +---------------------+--------------------------+
|                         | ``isoline``         | ``Isoline``              |
+-------------------------+---------------------+--------------------------+
|                         | ``integer``         | ``SpacingEqual``         |
|                         +---------------------+--------------------------+
|                         | ``fractional_even`` | ``SpacingFractionalEven``|
|    ``partitioning``     +---------------------+--------------------------+
|                         | ``fractional_odd``  | ``SpacingFractionalOdd`` |
|                         +---------------------+--------------------------+
|                         | ``pow2``            |           N/A            |
+-------------------------+---------------------+--------------------------+
|                         | ``point``           | ``PointMode``            |
|                         +---------------------+--------------------------+
|                         | ``line``            |           N/A            |
|  ``outputtopology``     +---------------------+--------------------------+
|                         | ``triangle_cw``     | ``VertexOrderCw``        |
|                         +---------------------+--------------------------+
|                         | ``triangle_ccw``    | ``VertexOrderCcw``       |
+-------------------------+---------------------+--------------------------+
|``outputcontrolpoints``  | ``n``               | ``OutputVertices n``     |
+-------------------------+---------------------+--------------------------+

The ``patchconstfunc`` attribute does not have a direct equivalent in SPIR-V.
It specifies the name of the Patch Constant Function. This function is run only
once per patch. This is further described below.

InputPatch and OutputPatch
~~~~~~~~~~~~~~~~~~~~~~~~~~
Both of ``InputPatch<T, N>`` and ``OutputPatch<T, N>`` are translated to an array
of constant size ``N`` where each element is of type ``T``.

InputPatch can be passed to the Hull shader main entry function as well as the
patch constant function. This would include information about each of the ``N``
vertices that are input to the tessellation control shader.

OutputPatch is an array containing ``N`` elements (where ``N`` is the number of
output vertices). Each element of the array is the hull shader output for each
output vertex. For example, each element of ``OutputPatch<HSOutput, 3>`` is each
output value of the hull shader function for each ``SV_OutputControlPointID``.
It is shared between threads i.e., in the patch constant function, threads for
the same patch must see the same values for the elements of
``OutputPatch<HSOutput, 3>``.

The SPIR-V ``InvocationID`` (``SV_OutputControlPointID`` in HLSL) is used to index
into the InputPatch and OutputPatch arrays to read/write information for the given
vertex.

The hull main entry function in HLSL returns only one value (say, of type ``T``), but
that function is in fact executed once for each control point. The Vulkan spec requires that
"Tessellation control shader per-vertex output variables and blocks, and tessellation control,
tessellation evaluation, and geometry shader per-vertex input variables and blocks are required
to be declared as arrays, with each element representing input or output values for a single vertex
of a multi-vertex primitive". Therefore, we need to create a stage output variable that is an array
with elements of type ``T``. The number of elements of the array is equal to the number of
output control points. Each final output control point is written into the corresponding element in
the array using SV_OutputControlPointID as the index.

Patch Constant Function
~~~~~~~~~~~~~~~~~~~~~~~
As mentioned above, the patch constant function is to be invoked only once per patch.
As a result, in the SPIR-V module, the `entry function wrapper`_ will first invoke the
main entry function, and then use an ``OpControlBarrier`` to wait for all vertex
processing to finish. After the barrier, *only* the first thread (with InvocationID of 0)
will invoke the patch constant function. Since the first thread has to see the
OutputPatch that contains output of the hull shader function for other threads,
we have to use the output stage variable (with Output storage class) of the
hull shader function for OutputPatch that can be an input to the patch constant
function.

The information resulting from the patch constant function will also be returned
as stage output variables. The output struct of the patch constant function must include
``SV_TessFactor`` and ``SV_InsideTessFactor`` fields which will translate to
``TessLevelOuter`` and ``TessLevelInner`` builtin variables, respectively. And the rest
will be flattened and translated into normal stage output variables, one for each field.

Geometry Shaders
----------------

This section describes how geometry shaders are translated to SPIR-V for Vulkan.

Geometry Shader Entry Point Attributes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The following HLSL attribute is attached to the main entry point of geometry shaders
and is translated to SPIR-V execution mode as follows:

.. table:: Mapping from geometry shader HLSL attribute to SPIR-V execution mode

+-------------------------+---------------------+--------------------------+
| HLSL Attribute          |   value             | SPIR-V Execution Mode    |
+=========================+=====================+==========================+
|``maxvertexcount``       | ``n``               | ``OutputVertices n``     |
+-------------------------+---------------------+--------------------------+
|``instance``             | ``n``               | ``Invocations n``        |
+-------------------------+---------------------+--------------------------+

Translation for Primitive Types
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Geometry shader vertex inputs may be qualified with primitive types. Only one primitive type
is allowed to be used in a given geometry shader. The following table shows the SPIR-V execution
mode that is used in order to represent the given primitive type.

.. table:: Mapping from geometry shader primitive type to SPIR-V execution mode

+---------------------+-----------------------------+
| HLSL Primitive Type | SPIR-V Execution Mode       |
+=====================+=============================+
|``point``            | ``InputPoints``             |
+---------------------+-----------------------------+
|``line``             | ``InputLines``              |
+---------------------+-----------------------------+
|``triangle``         | ``Triangles``               |
+---------------------+-----------------------------+
|``lineadj``          | ``InputLinesAdjacency``     |
+---------------------+-----------------------------+
|``triangleadj``      | ``InputTrianglesAdjacency`` |
+---------------------+-----------------------------+

Translation of Output Stream Types
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Supported output stream types in geometry shaders are: ``PointStream<T>``,
``LineStream<T>``, and ``TriangleStream<T>``. These types are translated as the underlying
type ``T``, which is recursively flattened into stand-alone variables for each field.

Furthermore, output stream objects passed to geometry shader entry points are
required to be annotated with ``inout``, but the generated SPIR-V only contains
stage output variables for them.

The following table shows the SPIR-V execution mode that is used in order to represent the
given output stream.

.. table:: Mapping from geometry shader output stream type to SPIR-V execution mode

+---------------------+-----------------------------+
| HLSL Output Stream  | SPIR-V Execution Mode       |
+=====================+=============================+
|``PointStream``      | ``OutputPoints``            |
+---------------------+-----------------------------+
|``LineStream``       | ``OutputLineStrip``         |
+---------------------+-----------------------------+
|``TriangleStream``   | ``OutputTriangleStrip``     |
+---------------------+-----------------------------+

In other shader stages, stage output variables are only written in the `entry
function wrapper`_ after calling the source code entry function. However,
geometry shaders can output as many vertices as they wish, by calling the
``.Append()`` method on the output stream object. Therefore, it is incorrect to
have only one flush in the entry function wrapper like other stages. Instead,
each time a ``*Stream<T>::Append()`` is encountered, all stage output variables
behind ``T`` will be flushed before SPIR-V ``OpEmitVertex`` instruction is
generated. ``.RestartStrip()`` method calls will be translated into the SPIR-V
``OpEndPrimitive`` instruction.

Raytracing Shader Stages
------------------------

DirectX Raytracing adds six new shader stages for raytracing namely ray generation, intersection, closest-hit,
any-hit, miss and callable.

| Refer to following pages for details:
| https://docs.microsoft.com/en-us/windows/desktop/direct3d12/direct3d-12-raytracing
| https://docs.microsoft.com/en-us/windows/desktop/direct3d12/direct3d-12-raytracing-hlsl-reference


Flow chart for various stages in a raytracing pipeline is as follows:
::

          +---------------------+
          |   Ray generation    |
          +---------------------+
                     |
          TraceRay() |                      +--------------+
                     |      _ _ _ _ _ _ _ _ |   Any Hit    |
                     |     |                +--------------+
                     V     V                       ^
          +---------------------+                  |
          |    Acceleration     |           +--------------+
          |     Structure       |           | Intersection |
          |     Traversal       |           +--------------+
          +---------------------+                  ^
                    |        |                     |
                    |        |_ _ _ _ _ _ _ _ _ _ _|
                    |
                    |
                    V
          +--------------------+            +-------------+
          |      Is Hit ?      |            |  Callable   |
          +--------------------+            +-------------+
              |            |
          Yes |            | No
              V            V
         +---------+    +------+
         | Closest |    | Miss |
         |   Hit   |    |      |
         +---------+    +------+


| *Note : DXC does not add special shader profiles for raytracing under -T option.*
| *All raytracing shaders must be compiled as library using lib_6_3/lib_6_4 profile option.*
| *Note : DXC now targets SPV_KHR_ray_tracing extension by default.*
| *This extension is provisional and subject to change*.
| *To compile for NV extension use -fspv-extension=SPV_NV_ray_tracing.*

Ray Generation Stage
~~~~~~~~~~~~~~~~~~~~

| Ray generation shaders start ray tracing work and work on a compute-like 3D grid of threads.
| Entry functions of this stage type are annotated with **[shader("raygeneration")]** in HLSL source.
| Such entry functions must return void and do not accept any arguments.

| For example:

.. code:: hlsl

  RaytracingAccelerationStructure rs;
  struct Payload
  {
  float4 color;
  };
  [shader("raygeneration")]
  void main() {
    Payload myPayload = { float4(0.0f,0.0f,0.0f,0.0f) };
    RayDesc rayDesc;
    rayDesc.Origin = float3(0.0f, 0.0f, 0.0f);
    rayDesc.Direction = float3(0.0f, 0.0f, -1.0f);
    rayDesc.TMin = 0.0f;
    rayDesc.TMax = 1000.0f;
    TraceRay(rs, 0x0, 0xff, 0, 1, 0, rayDesc, myPayload);
  }

Intersection Stage
~~~~~~~~~~~~~~~~~~

| Intersection shader stage is used to implement arbitrary ray-primitive intersections such spheres or axis-aligned bounding boxes (AABB). Triangle primitives do not require a custom intersection shader.
| Entry functions of this stage are annotated with **[shader("intersection")]** in HLSL source.
| Such entry functions must return void and do not accept any arguments.

| For example:

.. code:: hlsl

  struct Attribute
  {
    float2 bary;
  };

  [shader("intersection")]
  void main() {
  Attribute myHitAttribute = { float2(0.0f,0.0f) };
  ReportHit(0.0f, 0U, myHitAttribute);
  }


Closest-Hit Stage
~~~~~~~~~~~~~~~~~

| Hit shaders are invoked when a ray primitive intersection is found. A closest-hit shader
| is invoked for the closest intersection point along a ray and can be used to compute interactions
| at intersection point or spawn secondary rays.
| Entry functions of this stage are annotated with **[shader("closesthit")]** in HLSL source.
| Such entry functions must return void and accept exactly two arguments. First argument must be an inout
| variable of user defined structure type and second argument must be a in variable of user defined structure type.

| For example:

.. code:: hlsl

  struct Attribute
  {
    float2 bary;
  };
  struct Payload {
    float4 color;
  };
  [shader("closesthit")]
  void main(inout Payload a, in Attribute b) {
    a.color = float4(0.0f,1.0f,0.0f,0.0f);
  }

Any-Hit Stage
~~~~~~~~~~~~~~~~~

| Hit shaders are invoked when a ray primitive intersection is found. An any-hit shader
| is invoked for all intersections along a ray with a primitive.
| Entry functions of this stage are annotated with **[shader("anyhit")]** in HLSL source.
| Such entry functions must return void and accept exactly two arguments. First argument must be an inout
| variable of user defined structure type and second argument must be an in variable of user defined structure type.

| For example:

.. code:: hlsl

  struct Attribute
  {
    float2 bary;
  };
  struct Payload {
    float4 color;
  };
  [shader("anyhit")]
  void main(inout Payload a, in Attribute b) {
    a.color = float4(0.0f,1.0f,0.0f,0.0f);
  }

Miss Stage
~~~~~~~~~~

| Miss shaders are invoked when no intersection is found.
| Entry functions of this stage are annotated with **[shader("miss")]** in HLSL source.
| Such entry functions return void and accept exactly one argument. First argument must be an inout variable of user defined structure type.

| For example:

.. code:: hlsl

  struct Payload {
    float4 color;
  };
  [shader("miss")]
  void main(inout Payload a) {
    a.color = float4(0.0f,1.0f,0.0f,0.0f);
  }

Callable Stage
~~~~~~~~~~~~~~

| Callables are generic function calls which can be invoked from either raygeneration, closest-hit,
| miss or callable shader stages.
| Entry functions of this stage are annotated with **[shader("callable")]** in HLSL source.
| Such entry functions must return void and accept exactly one argument. First argument must be an inout
| variable of user defined structure type.

| For example:

.. code:: hlsl

  struct CallData {
    float4 data;
  };
  [shader("callable")]
  void main(inout CallData a) {
    a.color = float4(0.0f,1.0f,0.0f,0.0f);
  }

Mesh and Amplification Shaders
------------------------------

| DirectX adds 2 new shader stages for using MeshShading pipeline namely Mesh and Amplification.
| Amplification shaders corresponds to Task Shaders in Vulkan.
|
| Refer to following HLSL and SPIR-V specs for details:
| https://microsoft.github.io/DirectX-Specs/d3d/MeshShader.html
| https://github.com/KhronosGroup/SPIRV-Registry/blob/main/extensions/NV/SPV_NV_mesh_shader.asciidoc
|
| This section describes how Mesh and Amplification shaders are translated to SPIR-V for Vulkan.

Entry Point Attributes
~~~~~~~~~~~~~~~~~~~~~~
The following HLSL attributes are attached to the main entry point of Mesh and/or Amplification
shaders and are translated to SPIR-V execution modes according to the table below:

.. table:: Mapping from HLSL attribute to SPIR-V execution mode

+-----------------------+--------------------+-------------------------+
|  HLSL Attribute       |   Value            | SPIR-V Execution Mode   |
+=======================+====================+=========================+
|``outputtopology``     | ``point``          | ``OutputPoints``        |
|                       +--------------------+-------------------------+
| (SPV_NV_mesh_shader)  | ``line``           | ``OutputLinesNV``       |
|                       |                    |                         |
|                       +--------------------+-------------------------+
|                       | ``triangle``       | ``OutputTrianglesNV``   |
+-----------------------+--------------------+-------------------------+
|``outputtopology``     | ``point``          | ``OutputPoints``        |
|                       +--------------------+-------------------------+
| (SPV_EXT_mesh_shader) | ``line``           | ``OutputLinesEXT``      |
|                       |                    |                         |
|                       +--------------------+-------------------------+
|                       | ``triangle``       | ``OutputTrianglesEXT``  |
+-----------------------+--------------------+-------------------------+
| ``numthreads``        | ``X, Y, Z``        | ``LocalSize X, Y, Z``   |
|                       |                    |                         |
|                       | ``(X*Y*Z <= 128)`` |                         |
+-----------------------+--------------------+-------------------------+

Intrinsics
~~~~~~~~~~
The following HLSL intrinsics are used in Mesh or Amplification shaders
and are translated to SPIR-V intrinsics according to the table below:

.. table:: Mapping from HLSL intrinsics to SPIR-V intrinsics for SPV_NV_mesh_shader

+---------------------------+--------------------+-----------------------------------------+
|  HLSL Intrinsic           |  Parameters        | SPIR-V Intrinsic                        |
+===========================+====================+=========================================+
| ``SetMeshOutputCounts``   | ``numVertices``    | ``PrimitiveCountNV numPrimitives``      |
|                           |                    |                                         |
| ``(Mesh shader)``         | ``numPrimitives``  |                                         |
+---------------------------+--------------------+-----------------------------------------+
| ``DispatchMesh``          | ``ThreadX``        | ``OpControlBarrier``                    |
|                           |                    |                                         |
| ``(Amplification shader)``| ``ThreadY``        | ``TaskCountNV ThreadX*ThreadY*ThreadZ`` |
|                           |                    |                                         |
|                           | ``ThreadZ``        |                                         |
|                           |                    |                                         |
|                           | ``MeshPayload``    |                                         |
+---------------------------+--------------------+-----------------------------------------+

.. table:: Mapping from HLSL intrinsics to SPIR-V intrinsics for SPV_EXT_mesh_shader

+---------------------------+--------------------+--------------------------------------------------------------+
|  HLSL Intrinsic           |  Parameters        | SPIR-V Intrinsic                                             |
+===========================+====================+==============================================================+
| ``SetMeshOutputCounts``   | ``numVertices``    | ``OpSetMeshOutputsEXT``                                      |
|                           |                    |                                                              |
| ``(Mesh shader)``         | ``numPrimitives``  |                                                              |
+---------------------------+--------------------+--------------------------------------------------------------+
| ``DispatchMesh``          | ``ThreadX``        | ``OpEmitMeshTasksEXT ThreadX ThreadY ThreadZ MeshPayload``   |
|                           |                    |                                                              |
| ``(Amplification shader)``| ``ThreadY``        | ``TaskCountNV ThreadX*ThreadY*ThreadZ``                      |
|                           |                    |                                                              |
|                           | ``ThreadZ``        |                                                              |
|                           |                    |                                                              |
|                           | ``MeshPayload``    |                                                              |
+---------------------------+--------------------+--------------------------------------------------------------+

| Note : For ``DispatchMesh`` intrinsic, we also emit ``MeshPayload`` as output block with ``PerTaskNV`` decoration

Mesh Interface Variables
~~~~~~~~~~~~~~~~~~~~~~~~
| Interface variables are defined for Mesh shaders using HLSL modifiers.
| Following table gives high level overview of the mapping:
|

.. table:: Mapping from HLSL modifiers to SPIR-V definitions

+-----------------+-------------------------------------------------------------------------+
|  HLSL modifier  | SPIR-V definition                                                       |
+=================+=========================================================================+
| ``indices``     | Maps to SPIR-V intrinsic ``PrimitiveIndicesNV``                         |
|                 |                                                                         |
|                 | Defines SPIR-V Execution Mode ``OutputPrimitivesNV <array-size>``       |
+-----------------+-------------------------------------------------------------------------+
| ``vertices``    | Maps to per-vertex out attributes                                       |
|                 |                                                                         |
|                 | Defines existing SPIR-V Execution Mode ``OutputVertices <array-size>``  |
+-----------------+-------------------------------------------------------------------------+
| ``primitives``  | Maps to per-primitive out attributes with ``PerPrimitiveNV`` decoration |
+-----------------+-------------------------------------------------------------------------+
| ``payload``     | Maps to per-task in attributes with ``PerTaskNV`` decoration            |
+-----------------+-------------------------------------------------------------------------+


Raytracing in Vulkan and SPIRV
==============================

| SPIR-V codegen is currently supported for NVIDIA platforms via SPV_NV_ray_tracing extension or
| on other platforms via provisional cross vendor SPV_KHR_ray_tracing extension.
| SPIR-V specification for reference:
| https://github.com/KhronosGroup/SPIRV-Registry/blob/main/extensions/NV/SPV_NV_ray_tracing.asciidoc
| https://github.com/KhronosGroup/SPIRV-Registry/blob/main/extensions/KHR/SPV_KHR_ray_tracing.asciidoc

| Vulkan ray tracing samples:
| https://developer.nvidia.com/rtx/raytracing/vkray


Raytracing Mapping to SPIR-V
----------------------------

Intrinsics
~~~~~~~~~~


| Following table provides mapping for system value intrinsics along with supported shader stages.

============================    ===============================    ====== ============ =========== ======= ======== ========
        HLSL                               SPIR-V                               HLSL Shader Stage
----------------------------    -------------------------------    ---------------------------------------------------------
  System Value Intrinsic               Builtin                     Raygen Intersection Closest Hit Any Hit   Miss   Callable
============================    ===============================    ====== ============ =========== ======= ======== ========
``DispatchRaysIndex()``         ``LaunchId{NV/KHR}``               ✓       ✓             ✓          ✓       ✓       ✓
``DispatchRaysDimensions()``    ``LaunchSize{NV/KHR}``             ✓       ✓             ✓          ✓       ✓       ✓
``WorldRayOrigin()``            ``WorldRayOrigin{NV/KHR}``                 ✓             ✓          ✓       ✓
``WorldRayDirection()``         ``WorldRayDirection{NV/KHR}``              ✓             ✓          ✓       ✓
``RayTMin()``                   ``RayTmin{NV/KHR}``                        ✓             ✓          ✓       ✓
``RayTCurrent()``               ``RayTmax{NV/KHR}``                        ✓             ✓          ✓       ✓
``RayFlags()``                  ``IncomingRayFlags{NV/KHR}``               ✓             ✓          ✓       ✓
``InstanceIndex()``             ``InstanceId``                             ✓             ✓          ✓
``GeometryIndex()``             ``RayGeometryIndexKHR``                    ✓             ✓          ✓
``InstanceID()``                ``InstanceCustomIndex{NV/KHR}``            ✓             ✓          ✓
``PrimitiveIndex()``            ``PrimitiveId``                            ✓             ✓          ✓
``ObjectRayOrigin()``           ``ObjectRayOrigin{NV/KHR}``                ✓             ✓          ✓
``ObjectRayDirection()``        ``ObjectRayDirection{NV/KHR}``             ✓             ✓          ✓
``ObjectToWorld3x4()``          ``ObjectToWorld{NV/KHR}``                  ✓             ✓          ✓
``ObjectToWorld4x3()``          ``ObjectToWorld{NV/KHR}``                  ✓             ✓          ✓
``WorldToObject3x4()``          ``WorldToObject{NV/KHR}``                  ✓             ✓          ✓
``WorldToObject4x3()``          ``WorldToObject{NV/KHR}``                  ✓             ✓          ✓
``HitKind()``                   ``HitKind{NV/KHR}``                        ✓             ✓          ✓
============================    ===============================    ====== ============ =========== ======= ======== ========

| *There is no separate builtin for transposed matrices ObjectToWorld3x4 and WorldToObject3x4 in SPIR-V hence we internally transpose during translation*
| *GeometryIndex() is only supported under SPV_KHR_ray_tracing extension.*

| Following table provides mapping for other intrinsics along with supported shader stages.


===========================     =================================     ====== ============ =========== ======= ===== ========
        HLSL                               SPIR-V                                 HLSL Shader Stage
---------------------------     ---------------------------------     ------------------------------------------------------
   Intrinsic                              Opcode                      Raygen Intersection Closest Hit Any Hit  Miss Callable
===========================     =================================     ====== ============ =========== ======= ===== ========
``TraceRay``                    ``OpTrace{NV/KHR}``                     ✓                    ✓                    ✓
``ReportHit``                   ``OpReportIntersection{NV/KHR}``        ✓         ✓
``IgnoreHit``                   ``OpIgnoreIntersection{NV/KHR}``        ✓                             ✓
``AcceptHitAndEndSearch``       ``OpTerminateRay{NV/KHR}``              ✓                             ✓
``CallShader``                  ``OpExecuteCallable{NV/KHR}``           ✓                    ✓             ✓     ✓
===========================     =================================     ====== ============ =========== ======= ===== ========


Resource Types
~~~~~~~~~~~~~~

| Following table provides mapping for new resource types supported in all raytracing shaders.


===================================     =======================================
        HLSL Type                                   SPIR-V Opcode
-----------------------------------     ---------------------------------------
``RaytracingAccelerationStructure``     ``OpTypeAccelerationStructure{NV/KHR}``
===================================     =======================================

Interface Variables
~~~~~~~~~~~~~~~~~~~

| Interface variables are created for various ray tracing storage classes based on intrinsic/shader stage
| Following table gives high level overview of the mapping.


=================================       ===========================================================
   SPIR-V Storage Class                        Created For
---------------------------------       -----------------------------------------------------------
``RayPayload{NV/KHR}``                  Last argument to TraceRay
``IncomingRayPayload{NV/KHR}``          First argument of entry for AnyHit/ClosestHit & Miss stage
``HitAttribute{NV/KHR}``                Last argument to ReportHit
``CallableData{NV/KHR}``                Last argument to CallShader
``IncomingCallableData{NV/KHR}``        First argument of entry for Callable stage
=================================       ===========================================================

RayQuery
--------

Ray Query is subfeature of the DirectX ray tracing and belongs to the DirectX ray tracing spec 1.1 (DXR 1.1).
DirectX add RayQuery object type and its member TraceRayInline() to do the TraceRay() that doesn't
use any seperate ray-tracing shader stages.
Shaders can instantiate RayQuery objects as local variables, the RayQuery object acts as a state
machine for ray query. The shader interacts with the RayQuery object's methods to advance the
query through an acceleration structure and query traversal information

Refer to following pages for details:
https://microsoft.github.io/DirectX-Specs/d3d/Raytracing.html

A flow chart for a simple ray query process

::

          +------------------------------+
          |   RayQuery<RAY_FLAG_NONE> q  |
          +------------------------------+
                         |
                         V
          +------------------------------+
          |      q.TraceRayInline()      |
          +------------------------------+
                  |               — — — — — — — — — — — — —
                  |              |                         |
                  |              |              +------------------------+
                  |              |              | Your intersection code |
                  |              |              +------------------------+
                  |              |                         ^
                  V              V                         |
          +------------------------------+      +---------------------+
          |  q.Proceed() // AS traversal |      |  q.CandidateType()  |
          +------------------------------+      +---------------------+
               |                   |                       ^
           No  |                   | Yes                   |
               |                   |_ _ _ _ _ _ _ _ _ _ _ _|
               V
         +------------------------------+
         |     q.CommittedStatus()      |
         +------------------------------+
                       |
                       V
        +----------------------------------+
        | Your Intersection/shader code    |
        +----------------------------------+


Example:

.. code:: hlsl

  void main() {
    RayQuery<RAY_FLAG_CULL_NON_OPAQUE | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> q;
    q.TraceRayInline(myAccelerationStructure, 0 , 0xff, myRay);

    // Proceed() is AccelerationStructure traversal loop take places
    while(q.Proceed()) {
      switch(q.CandidateType()) {
        // retrieve intersection information/Do the shadering
      }
    }

    // AccelerationStructure traversal end
    // Get the Committed status
    switch(q.CommittedStatus()) {
      // retrieve intersection information/ Do the shadering
    }
  }

Ray Query in SPIRV
~~~~~~~~~~~~~~~~~~
RayQuery SPIR-V codegen is currently supported via SPV_KHR_ray_query extension
SPIR-V specification for reference:
https://github.com/KhronosGroup/SPIRV-Registry/blob/main/extensions/KHR/SPV_KHR_ray_query.asciidoc

Object Type
~~~~~~~~~~~
RayQuery<RAY_FLAGS>

RayQuery represents the state of an inline ray tracing call into an acceleration structure.


============ ================================
 HLSL Type            SPIR-V Opcode
------------ --------------------------------
``RayQuery`` ``OpTypeRayQueryKHR``
============ ================================

RayQuery Mapping to SPIR-V
~~~~~~~~~~~~~~~~~~~~~~~~~~

+---------------------------------------------------+-------------------------------------------------------------------------+
|      HLSL  RayQuery member Intrinsic              |             SPIR-V Opcode                                               |
+===================================================+=========================================================================+
|``.Abort``                                         | ``OpRayQueryTerminateKHR``                                              |
+---------------------------------------------------+-------------------------------------------------------------------------+
|``.CandidateType``                                 | ``OpRayQueryGetIntersectionTypeKHR``                                    |
+---------------------------------------------------+-------------------------------------------------------------------------+
|``.CandidateProceduralPrimitiveNonOpaque``         | ``OpRayQueryGetIntersectionCandidateAABBOpaqueKHR``                     |
+---------------------------------------------------+-------------------------------------------------------------------------+
|``.CandidateInstanceIndex``                        | ``OpRayQueryGetIntersectionInstanceIdKHR``                              |
+---------------------------------------------------+-------------------------------------------------------------------------+
|``.CandidateInstanceID``                           | ``OpRayQueryGetIntersectionInstanceCustomIndexKHR``                     |
+---------------------------------------------------+-------------------------------------------------------------------------+
| ``.CandidateInstanceContributionToHitGroupIndex`` | ``OpRayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR``  |
+---------------------------------------------------+-------------------------------------------------------------------------+
|``.CandidateGeometryIndex``                        | ``OpRayQueryGetIntersectionGeometryIndexKHR``                           |
+---------------------------------------------------+-------------------------------------------------------------------------+
|``.CandidatePrimitiveIndex``                       | ``OpRayQueryGetIntersectionPrimitiveIndexKHR``                          |
+---------------------------------------------------+-------------------------------------------------------------------------+
|``.CandidateObjectRayOrigin``                      | ``OpRayQueryGetIntersectionObjectRayOriginKHR``                         |
+---------------------------------------------------+-------------------------------------------------------------------------+
|``.CandidateObjectRayDirection``                   | ``OpRayQueryGetIntersectionObjectRayDirectionKHR``                      |
+---------------------------------------------------+-------------------------------------------------------------------------+
|``.CandidateObjectToWorld3x4``                     | ``OpRayQueryGetIntersectionObjectToWorldKHR``                           |
+---------------------------------------------------+-------------------------------------------------------------------------+
|``.CandidateObjectToWorld4x3``                     | ``OpRayQueryGetIntersectionObjectToWorldKHR``                           |
+---------------------------------------------------+-------------------------------------------------------------------------+
|``.CandidateWorldToObject3x4``                     | ``OpRayQueryGetIntersectionWorldToObjectKHR``                           |
+---------------------------------------------------+-------------------------------------------------------------------------+
|``.CandidateWorldToObject4x3``                     | ``OpRayQueryGetIntersectionWorldToObjectKHR``                           |
+---------------------------------------------------+-------------------------------------------------------------------------+
|``.CandidateTriangleBarycentrics``                 | ``OpRayQueryGetIntersectionBarycentricsKHR``                            |
+---------------------------------------------------+-------------------------------------------------------------------------+
|``.CandidateTriangleFrontFace``                    | ``OpRayQueryGetIntersectionFrontFaceKHR``                               |
+---------------------------------------------------+-------------------------------------------------------------------------+
|``.CommittedStatus``                               | ``OpRayQueryGetIntersectionTypeKHR``                                    |
+---------------------------------------------------+-------------------------------------------------------------------------+
|``.CommittedInstanceIndex``                        | ``OpRayQueryGetIntersectionInstanceIdKHR``                              |
+---------------------------------------------------+-------------------------------------------------------------------------+
|``.CommittedInstanceID``                           | ``OpRayQueryGetIntersectionInstanceCustomIndexKHR``                     |
+---------------------------------------------------+-------------------------------------------------------------------------+
| ``.CommittedInstanceContributionToHitGroupIndex`` |  ``OpRayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR`` |
+---------------------------------------------------+-------------------------------------------------------------------------+
|``.CommittedGeometryIndex``                        | ``OpRayQueryGetIntersectionGeometryIndexKHR``                           |
+---------------------------------------------------+-------------------------------------------------------------------------+
|``.CommittedPrimitiveIndex``                       | ``OpRayQueryGetIntersectionPrimitiveIndexKHR``                          |
+---------------------------------------------------+-------------------------------------------------------------------------+
|``.CommittedRayT``                                 | ``OpRayQueryGetIntersectionTKHR``                                       |
+---------------------------------------------------+-------------------------------------------------------------------------+
|``.CommittedObjectRayOrigin``                      | ``OpRayQueryGetIntersectionObjectRayOriginKHR``                         |
+---------------------------------------------------+-------------------------------------------------------------------------+
|``.CommittedObjectRayDirection``                   | ``OpRayQueryGetIntersectionObjectRayDirectionKHR``                      |
+---------------------------------------------------+-------------------------------------------------------------------------+
|``.CommittedObjectToWorld3x4``                     | ``OpRayQueryGetIntersectionObjectToWorldKHR``                           |
+---------------------------------------------------+-------------------------------------------------------------------------+
|``.CommittedObjectToWorld4x3``                     | ``OpRayQueryGetIntersectionObjectToWorldKHR``                           |
+---------------------------------------------------+-------------------------------------------------------------------------+
|``.CommittedWorldToObject3x4``                     | ``OpRayQueryGetIntersectionWorldToObjectKHR``                           |
+---------------------------------------------------+-------------------------------------------------------------------------+
|``.CommittedWorldToObject4x3``                     | ``OpRayQueryGetIntersectionWorldToObjectKHR``                           |
+---------------------------------------------------+-------------------------------------------------------------------------+
|``.CommittedTriangleBarycentrics``                 | ``OpRayQueryGetIntersectionBarycentricsKHR``                            |
+---------------------------------------------------+-------------------------------------------------------------------------+
|``.CommittedTriangleFrontFace``                    | ``OpRayQueryGetIntersectionFrontFaceKHR``                               |
+---------------------------------------------------+-------------------------------------------------------------------------+
|``.CommitNonOpaqueTriangleHit``                    | ``OpRayQueryConfirmIntersectionKHR``                                    |
+---------------------------------------------------+-------------------------------------------------------------------------+
|``.CommitProceduralPrimitiveHit``                  | ``OpRayQueryGenerateIntersectionKHR``                                   |
+---------------------------------------------------+-------------------------------------------------------------------------+
|``.Proceed``                                       | ``OpRayQueryProceedKHR``                                                |
+---------------------------------------------------+-------------------------------------------------------------------------+
|``.RayFlags``                                      | ``OpRayQueryGetRayFlagsKHR``                                            |
+---------------------------------------------------+-------------------------------------------------------------------------+
|``.RayTMin``                                       | ``OpRayQueryGetRayTMinKHR``                                             |
+---------------------------------------------------+-------------------------------------------------------------------------+
|``.TraceRayInline``                                | ``OpRayQueryInitializeKHR``                                             |
+---------------------------------------------------+-------------------------------------------------------------------------+
|``.WorldRayDirection``                             | ``OpRayQueryGetWorldRayDirectionKHR``                                   |
+---------------------------------------------------+-------------------------------------------------------------------------+
|``.WorldRayOrigin``                                | ``OpRayQueryGetWorldRayOriginKHR``                                      |
+---------------------------------------------------+-------------------------------------------------------------------------+

Shader Model 6.0+ Wave Intrinsics
=================================


Note that Wave intrinsics requires SPIR-V 1.3, which is supported by Vulkan 1.1.
If you use wave intrinsics in your source code, you will need to specify
-fspv-target-env=vulkan1.1 via the command line to target Vulkan 1.1.

Shader model 6.0 introduces a set of wave operations. Apart from
``WaveGetLaneCount()`` and ``WaveGetLaneIndex()``, which are translated into
loading from SPIR-V builtin variable ``SubgroupSize`` and
``SubgroupLocalInvocationId`` respectively, the rest are translated into SPIR-V
group operations with ``Subgroup`` scope according to the following chart:

============= ============================ =================================== ==============================
Wave Category       Wave Intrinsics               SPIR-V Opcode                SPIR-V Group Operation
============= ============================ =================================== ==============================
Query         ``WaveIsFirstLane()``        ``OpGroupNonUniformElect``
Vote          ``WaveActiveAnyTrue()``      ``OpGroupNonUniformAny``
Vote          ``WaveActiveAllTrue()``      ``OpGroupNonUniformAll``
Vote          ``WaveActiveBallot()``       ``OpGroupNonUniformBallot``
Reduction     ``WaveActiveAllEqual()``     ``OpGroupNonUniformAllEqual``       ``Reduction``
Reduction     ``WaveActiveCountBits()``    ``OpGroupNonUniformBallotBitCount`` ``Reduction``
Reduction     ``WaveActiveSum()``          ``OpGroupNonUniform*Add``           ``Reduction``
Reduction     ``WaveActiveProduct()``      ``OpGroupNonUniform*Mul``           ``Reduction``
Reduction     ``WaveActiveBitAdd()``       ``OpGroupNonUniformBitwiseAnd``     ``Reduction``
Reduction     ``WaveActiveBitOr()``        ``OpGroupNonUniformBitwiseOr``      ``Reduction``
Reduction     ``WaveActiveBitXor()``       ``OpGroupNonUniformBitwiseXor``     ``Reduction``
Reduction     ``WaveActiveMin()``          ``OpGroupNonUniform*Min``           ``Reduction``
Reduction     ``WaveActiveMax()``          ``OpGroupNonUniform*Max``           ``Reduction``
Scan/Prefix   ``WavePrefixSum()``          ``OpGroupNonUniform*Add``           ``ExclusiveScan``
Scan/Prefix   ``WavePrefixProduct()``      ``OpGroupNonUniform*Mul``           ``ExclusiveScan``
Scan/Prefix   ``WavePrefixCountBits()``    ``OpGroupNonUniformBallotBitCount`` ``ExclusiveScan``
Broadcast     ``WaveReadLaneAt()``         ``OpGroupNonUniformBroadcast``
Broadcast     ``WaveReadLaneFirst()``      ``OpGroupNonUniformBroadcastFirst``
Quad          ``QuadReadAcrossX()``        ``OpGroupNonUniformQuadSwap``
Quad          ``QuadReadAcrossY()``        ``OpGroupNonUniformQuadSwap``
Quad          ``QuadReadAcrossDiagonal()`` ``OpGroupNonUniformQuadSwap``
Quad          ``QuadReadLaneAt()``         ``OpGroupNonUniformQuadBroadcast``
Quad          ``QuadAny()``                ``OpGroupNonUniformQuadAnyKHR``
Quad          ``QuadAll()``                ``OpGroupNonUniformQuadAllKHR``
N/A           ``WaveMatch()``              ``OpGroupNonUniformPartitionNV``
Multiprefix   ``WaveMultiPrefixSum()``     ``OpGroupNonUniform*Add``           ``PartitionedExclusiveScanNV``
Multiprefix   ``WaveMultiPrefixProduct()`` ``OpGroupNonUniform*Mul``           ``PartitionedExclusiveScanNV``
Multiprefix   ``WaveMultiPrefixBitAnd()``  ``OpGroupNonUniformLogicalAnd``     ``PartitionedExclusiveScanNV``
Multiprefix   ``WaveMultiPrefixBitOr()``   ``OpGroupNonUniformLogicalOr``      ``PartitionedExclusiveScanNV``
Multiprefix   ``WaveMultiPrefixBitXor()``  ``OpGroupNonUniformLogicalXor``     ``PartitionedExclusiveScanNV``
============= ============================ =================================== ==============================

``QuadAny`` and ``QuadAll`` will use the ``OpGroupNonUniformQuadAnyKHR`` and
``OpGroupNonUniformQuadAllKHR`` instructions if the ``SPV_KHR_quad_control``
extension is enabled. If it is not, they will fall back to constructing the
value using multiple calls to ``OpGroupNonUniformQuadBroadcast``.

The Implicit ``vk`` Namespace
=============================

Overview
--------
We have introduced an implicit namepace (called ``vk``) that will be home to all
Vulkan-specific functions, enums, etc. Given the similarity between HLSL and
C++, developers are likely familiar with namespaces -- and implicit namespaces
(e.g. ``std::`` in C++). The ``vk`` namespace provides an interface for expressing
Vulkan-specific features (core spec and KHR extensions).

**The compiler will generate the proper error message (** ``unknown 'vk' identifier`` **)
if** ``vk::`` **is used for compiling to DXIL.**

Any intrinsic function or enum in the vk namespace will be deprecated if an
equivalent one is added to the default namepsace.

Current Features
----------------
The following intrinsic functions and constants are currently defined in the
implicit ``vk`` namepsace.

.. code:: hlsl

  // Implicitly defined when compiling to SPIR-V.
  namespace vk {

    const uint CrossDeviceScope = 0;
    const uint DeviceScope      = 1;
    const uint WorkgroupScope   = 2;
    const uint SubgroupScope    = 3;
    const uint InvocationScope  = 4;
    const uint QueueFamilyScope = 5;

    uint64_t ReadClock(in uint scope);
    T        RawBufferLoad<T = uint>(in uint64_t deviceAddress,
                                     in uint alignment = 4);
  } // end namespace


Intrinsic Constants
-------------------
The following constants are currently defined:

========================  ============================================
  Constant                value   (SPIR-V constant equivalent, if any)
========================  ============================================
``vk::CrossDeviceScope``    ``0`` (``CrossDevice``)
``vk::DeviceScope``         ``1`` (``Device``)
``vk::WorkgroupScope``      ``2`` (``Workgroup``)
``vk::SubgroupScope``       ``3`` (``Subgroup``)
``vk::InvocationScope``     ``4`` (``Invocation``)
``vk::QueueFamilyScope``    ``5`` (``QueueFamily``)
========================  ============================================

Intrinsic Functions
-------------------

ReadClock
~~~~~~~~~
This intrinsic funcion has the following signature:

.. code:: hlsl

  uint64_t ReadClock(in uint scope);

It translates to performing ``OpReadClockKHR`` defined in `VK_KHR_shader_clock <https://registry.khronos.org/vulkan/specs/latest/man/html/VK_KHR_shader_clock.html>`_.
One can use the predefined scopes in the ``vk`` namepsace to specify the scope argument.
For example:

.. code:: hlsl

  uint64_t clock = vk::ReadClock(vk::SubgroupScope);

RawBufferLoad and RawBufferStore
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The Vulkan extension `VK_KHR_buffer_device_address <https://registry.khronos.org/vulkan/specs/latest/man/html/VK_KHR_buffer_device_address.html>`_
supports getting the 64-bit address of a buffer and passing it to SPIR-V as a
Uniform buffer. SPIR-V can use the address to load and store data without a descriptor.
We add the following intrinsic functions to expose a subset of the
`VK_KHR_buffer_device_address <https://registry.khronos.org/vulkan/specs/latest/man/html/VK_KHR_buffer_device_address.html>`_
and `SPV_KHR_physical_storage_buffer <https://github.com/KhronosGroup/SPIRV-Registry/blob/main/extensions/KHR/SPV_KHR_physical_storage_buffer.asciidoc>`_
functionality to HLSL:

.. code:: hlsl

  // RawBufferLoad and RawBufferStore use 'uint' for the default template argument.
  // The default alignment is 4. Note that 'alignment' must be a constant integer.
  T RawBufferLoad<T = uint>(in uint64_t deviceAddress, in uint alignment = 4);
  void RawBufferStore<T = uint>(in uint64_t deviceAddress, in T value, in uint alignment = 4);


These intrinsics allow the shader program to load and store a single value with type T (int, float2, struct, etc...)
from GPU accessible memory at given address, similar to ``ByteAddressBuffer.Load()``.
Additionally, these intrinsics allow users to set the memory alignment for the underlying data.
We assume a 'uint' type when the template argument is missing, and we use a value of '4' for the default alignment.
Note that the alignment argument must be a constant integer if it is given.

Though we do support setting the `alignment` of the data load and store, we do not currently
support setting the memory layout for the data. Since these intrinsics are supposed to load
"arbitrary" data to or from a random device address, we assume that the program loads/stores some "bytes of data",
but that its format or layout is unknown. Therefore, keep in mind that these intrinsics
load or store ``sizeof(T)`` bytes of data, and that loading/storing data with a struct
with a custom memory alignment may yield undefined behavior due to the missing custom memory layout support.
Loading data with customized memory layouts is future work.

Using either of these intrinsics adds ``PhysicalStorageBufferAddresses`` capability and
``SPV_KHR_physical_storage_buffer`` extension requirements as well as changing
the addressing model to ``PhysicalStorageBuffer64``.

Example:

.. code:: hlsl

  uint64_t address;
  [numthreads(32, 1, 1)]
  void main(uint3 tid : SV_DispatchThreadID) {
    double foo = vk::RawBufferLoad<double>(address, 8);
    uint bar = vk::RawBufferLoad(address + 8);
    ...
    vk::RawBufferStore<uint>(address + tid.x, bar + tid.x);
  }

Inline SPIR-V (HLSL version of GL_EXT_spirv_intrinsics)
=======================================================

GL_EXT_spirv_intrinsics is an extension of GLSL that allows users to embed
arbitrary SPIR-V instructions in the GLSL code similar to the concept of
inline assembly in the C code. We support the HLSL version of
GL_EXT_spirv_intrinsics. See
`wiki <https://github.com/microsoft/DirectXShaderCompiler/wiki/Inline-SPIR%E2%80%90V>`_
for the details.

Supported Command-line Options
==============================

Command-line options supported by SPIR-V CodeGen are listed below. They are
also recognized by the library API calls.

General options
---------------

- ``-T``: specifies shader profile
- ``-E``: specifies entry point
- ``-D``: Defines macro
- ``-I``: Adds directory to include search path
- ``-O{|0|1|2|3}``: Specifies optimization level
- ``-enable-16bit-types``: enables 16-bit types and disables min precision types
- ``-Zpc``: Packs matrices in column-major order by deafult
- ``-Zpr``: Packs matrices in row-major order by deafult
- ``-Fc``: outputs SPIR-V disassembly to the given file
- ``-Fe``: outputs warnings and errors to the given file
- ``-Fo``: outputs SPIR-V code to the given file
- ``-Fh``: outputs SPIR-V code as a header file
- ``-Vn``: specifies the variable name for SPIR-V code in generated header file
- ``-Zi``: Emits more debug information (see `Debugging`_)
- ``-Cc``: colorizes SPIR-V disassembly
- ``-No``: adds instruction byte offsets to SPIR-V disassembly
- ``-H``:  Shows header includes and nesting depth
- ``-Vi``: Shows details about the include process
- ``-Vd``: Disables SPIR-V verification
- ``-WX``: Treats warnings as errors
- ``-no-warnings``: Suppresses all warnings
- ``-flegacy-macro-expansion``: expands the operands before performing
  token-pasting operation (fxc behavior)

Vulkan-specific options
-----------------------

The following command line options are added into ``dxc`` to support SPIR-V
codegen for Vulkan:

- ``-spirv``: Generates SPIR-V code.
- ``-fvk-b-shift N M``: Shifts by ``N`` the inferred binding numbers for all
  resources in b-type registers of space ``M``. Specifically, for a resouce
  attached with ``:register(bX, spaceM)`` but not ``[vk::binding(...)]``,
  sets its Vulkan descriptor set to ``M`` and binding number to ``X + N``. If
  you need to shift the inferred binding numbers for more than one space,
  provide more than one such option. If more than one such option is provided
  for the same space, the last one takes effect. If you need to shift the
  inferred binding numbers for all sets, use ``all`` as ``M``.
  See `HLSL register and Vulkan binding`_ for explanation and examples.
- ``-fvk-t-shift N M``, similar to ``-fvk-b-shift``, but for t-type registers.
- ``-fvk-s-shift N M``, similar to ``-fvk-b-shift``, but for s-type registers.
- ``-fvk-u-shift N M``, similar to ``-fvk-b-shift``, but for u-type registers.
- ``-fvk-auto-shift-bindings``: Automatically detects the register type for
  resources that are missing the ``:register`` assignment, so the above shifts
  can be applied to them if needed.
- ``-fvk-bind-register xX Y N M`` (short alias: ``-vkbr``): Binds the resouce
  at ``register(xX, spaceY)`` to descriptor set ``M`` and binding ``N``. This
  option cannot be used together with other binding assignment options.
  It requires all source code resources have ``:register()`` attribute and
  all registers have corresponding Vulkan descriptors specified using this
  option. If the ``$Globals`` cbuffer resource is used, it must also be bound
  with ``-fvk-bind-globals``.
- ``-fvk-bind-globals N M``: Places the ``$Globals`` cbuffer at
  descriptor set #M and binding #N. See `HLSL global variables and Vulkan binding`_
  for explanation and examples.
- ``-fvk-use-gl-layout``: Uses strict OpenGL ``std140``/``std430``
  layout rules for resources.
- ``-fvk-use-dx-layout``: Uses DirectX layout rules for resources.
- ``-fvk-invert-y``: Negates (additively inverts) SV_Position.y before writing
  to stage output. Used to accommodate the difference between Vulkan's
  coordinate system and DirectX's. Only allowed in VS/DS/GS/MS/Lib.
- ``-fvk-use-dx-position-w``: Reciprocates (multiplicatively inverts)
  SV_Position.w after reading from stage input. Used to accommodate the
  difference between Vulkan DirectX: the w component of SV_Position in PS is
  stored as 1/w in Vulkan. Only recognized in PS; applying to other stages
  is no-op.
- ``-fvk-stage-io-order={alpha|decl}``: Assigns the stage input/output variable
  location number according to alphabetical order or declaration order. See
  `HLSL semantic and Vulkan Location`_ for more details.
- ``-fspv-reflect``: Emits additional SPIR-V instructions to aid reflection.
- ``-fspv-debug=<category>``: Controls what category of debug information
  should be emitted. Accepted values are ``file``, ``source``, ``line``, and
  ``tool``. See `Debugging`_ for more details.
- ``-fspv-extension=<extension>``: Only allows using ``<extension>`` in CodeGen.
  If you want to allow multiple extensions, provide more than one such option. If you
  want to allow *all* KHR extensions, use ``-fspv-extension=KHR``.
- ``-fspv-target-env=<env>``: Specifies the target environment for this compilation.
  The current valid options are ``vulkan1.0`` and ``vulkan1.1``. If no target
  environment is provided, ``vulkan1.0`` is used as default.
- ``-fspv-flatten-resource-arrays``: Flattens arrays of textures and samplers
  into individual resources, each taking one binding number. For example, an
  array of 3 textures will become 3 texture resources taking 3 binding numbers.
  This makes the behavior similar to DX. Without this option, you would get 1
  array object taking 1 binding number. Note that arrays of
  {RW|Append|Consume}StructuredBuffers are currently not supported in the
  SPIR-V backend. Also note that this requires the optimizer to be able to
  resolve all array accesses with constant indeces. Therefore, all loops using
  the resource arrays must be marked with ``[unroll]``.
- ``-fspv-entrypoint-name=<name>``: Specify the SPIR-V entry point name. Defaults
  to the HLSL entry point name.
- ``-fspv-use-legacy-buffer-matrix-order``: Assumes the legacy matrix order (row
  major) when accessing raw buffers (e.g., ByteAdddressBuffer).
- ``-fspv-preserve-interface``: Preserves all interface variables in the entry
  point, even when those variables are unused.
- ``-Wno-vk-ignored-features``: Does not emit warnings on ignored features
  resulting from no Vulkan support, e.g., cbuffer member initializer.

Unsupported HLSL Features
=========================

The following HLSL language features are not supported in SPIR-V codegen,
either because of no Vulkan equivalents at the moment, or because of deprecation.

* Literal/immediate sampler state: deprecated feature. The compiler will
  emit a warning and ignore it.
* ``abort()`` intrinsic function: no Vulkan equivalent. The compiler will emit
  an error.
* ``GetRenderTargetSampleCount()`` intrinsic function: no Vulkan equivalent.
  (Its GLSL counterpart is ``gl_NumSamples``, which is not available in GLSL for
  Vulkan.) The compiler will emit an error.
* ``GetRenderTargetSamplePosition()`` intrinsic function: no Vulkan equivalent.
  (``gl_SamplePosition`` provides similar functionality but it's only for the
  sample currently being processed.) The compiler will emit an error.
* ``tex*()`` intrinsic functions: deprecated features. The compiler will
  emit errors.
* ``.GatherCmpGreen()``, ``.GatherCmpBlue()``, ``.GatherCmpAlpha()`` intrinsic
  method: no Vulkan equivalent. (SPIR-V ``OpImageDrefGather`` instruction does
  not take component as input.) The compiler will emit an error.
* Since ``StructuredBuffer``, ``RWStructuredBuffer``, ``ByteAddressBuffer``, and
  ``RWByteAddressBuffer`` are not represented as image types in SPIR-V, using the
  output unsigned integer ``status`` argument in their ``Load*`` methods is not
  supported. Using these methods with the ``status`` argument will cause a compiler error.
* Applying ``row_major`` or ``column_major`` attributes to a stand-alone matrix will be
  ignored by the compiler because ``RowMajor`` and ``ColMajor`` decorations in SPIR-V are
  only allowed to be applied to members of structures. A warning will be issued by the compiler.
* The Hull shader ``partitioning`` attribute may not have the ``pow2`` value. The compiler
  will emit an error. Other attribute values are supported and described in the
  `Hull Entry Point Attributes`_ section.
* ``cbuffer``/``tbuffer`` member initializer: no Vulkan equivalent. The compiler
  will emit an warning and ignore it.

Appendix
==========

Appendix A. Matrix Representation
---------------------------------
Consider a matrix in HLSL defined as ``float2x3 m;``. Conceptually, this is a matrix with 2 rows and 3 columns.
This means that you can access its elements via expressions such as ``m[i][j]``, where ``i`` can be ``{0, 1}`` and ``j`` can be ``{0, 1, 2}``.

Now let's look how matrices are defined in SPIR-V:

.. code:: spirv

  %columnType = OpTypeVector %float      <number of rows>
     %matType = OpTypeMatrix %columnType <number of columns>

As you can see, SPIR-V conceptually represents matrices as a collection of vectors where each vector is a *column*.

Now, let's represent our float2x3 matrix in SPIR-V. If we choose a naive translation (3 columns, each of which is a vector of size 2), we get:

.. code:: spirv

      %v2float = OpTypeVector %float 2
  %mat3v2float = OpTypeMatrix %v2float 3

Now, let's use this naive translation to access into the matrix (e.g. ``m[0][2]``). This is evaluated by first finding ``n = m[0]``, and then finding ``n[2]``.
Notice that in HLSL, ``m[0]`` represents a row, which is a vector of size 3. But accessing the first dimension of the SPIR-V matrix give us
the first column which is a vector of size 2.

.. code:: spirv

  ; n is a vector of size 2
  %n = OpAccessChain %v2float %m %int_0

Notice that in HLSL access ``m[i][j]``, ``i`` can be ``{0, 1}`` and ``j`` can be ``{0, 1, 2}``.
But in SPIR-V OpAccessChain access, the first index (``i``) can be ``{0, 1, 2}`` and the second index (``j``) can be ``{1, 0}``.
Therefore, the naive translation does not work well with indexing.

As a result, we must translate a given HLSL float2x3 matrix (with 2 rows and 3 columns) as a SPIR-V matrix with 3 rows and 2 columns:

.. code:: spirv

      %v3float = OpTypeVector %float 3
  %mat2v3float = OpTypeMatrix %v3float 2

This way, all accesses into the matrix can be naturally handled correctly.

Packing
~~~~~~~
The HLSL ``row_major`` and ``column_major`` type modifiers change the way packing is done.
The following table provides an example which should make our translation more clear:

+------------------+---------------------------+---------------------------+-----------------------------+-------------------+
| Host CPU Data    | HLSL Variable             | GPU (HLSL Representation) | GPU (SPIR-V Representation) | SPIR-V Decoration |
+==================+===========================+===========================+=============================+===================+
|``{1,2,3,4,5,6}`` |          ``float2x3``     |  ``[1 3 5]``              |  ``[1 2]``                  |                   |
|                  |                           |                           |                             |                   |
|                  |                           |  ``[2 4 6]``              |  ``[3 4]``                  |  ``RowMajor``     |
|                  |                           |                           |                             |                   |
|                  |                           |                           |  ``[5 6]``                  |                   |
+------------------+---------------------------+---------------------------+-----------------------------+-------------------+
|``{1,2,3,4,5,6}`` | ``column_major float2x3`` |  ``[1 3 5]``              |  ``[1 2]``                  |                   |
|                  |                           |                           |                             |                   |
|                  |                           |  ``[2 4 6]``              |  ``[3 4]``                  | ``RowMajor``      |
|                  |                           |                           |                             |                   |
|                  |                           |                           |  ``[5 6]``                  |                   |
+------------------+---------------------------+---------------------------+-----------------------------+-------------------+
|``{1,2,3,4,5,6}`` |    ``row_major float2x3`` |  ``[1 2 3]``              |  ``[1 4]``                  |                   |
|                  |                           |                           |                             |                   |
|                  |                           |  ``[4 5 6]``              |  ``[2 5]``                  | ``ColMajor``      |
|                  |                           |                           |                             |                   |
|                  |                           |                           |  ``[3 6]``                  |                   |
+------------------+---------------------------+---------------------------+-----------------------------+-------------------+
