# Tint Architecture

```
                   ┏━━━━━━━━┓                   ┏━━━━━━┓
                   ┃ SPIR━V ┃                   ┃ WGSL ┃
                   ┗━━━━┃━━━┛                   ┗━━━┃━━┛
                        ▼                           ▼
              ┏━━━━━━━━━┃━━━━━━━━━━━━━━━━━━━━━━━━━━━┃━━━━━━━━┓
              ┃         ┃          Reader           ┃        ┃
              ┃         ┃                           ┃        ┃
              ┃ ┏━━━━━━━┻━━━━━━┓             ┏━━━━━━┻━━━━━━┓ ┃
              ┃ ┃ SPIRV-Reader ┃             ┃ WGSL-Reader ┃ ┃
              ┃ ┗━━━━━━━━━━━━━━┛             ┗━━━━━━━━━━━━━┛ ┃
              ┗━━━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━━┛
                                      ▼
                    ┏━━━━━━━━━━━━━━━━━┻━━━━━━━━━━━━━━━━━┓
                    ┃           ProgramBuilder          ┃
                    ┃             (mutable)             ┃
      ┏━━━━━━━━━━━━►┫         ┏━━━━━┓   ┏━━━━━━━━━┓     ┃
      ┃             ┃         ┃ AST ┃   ┃ Symbols ┃     ┃
      ┃             ┃         ┗━━━━━┛   ┗━━━━━━━━━┛     ┃
      ┃             ┗━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━┛
      ┃                               ▼
      ┃             ┌┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┃┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┐
      ▲             ┆ Resolve         ▼                ┆
  ┏━━━┻━━━┓         ┆        ┏━━━━━━━━┻━━━━━━━━┓       ┆
  ┃ Clone ┃         ┆        ┃    Resolver     ┃       ┆
  ┗━━━┳━━━┛         ┆        ┗━━━━━━━━━━━━━━━━━┛       ┆
      ▲             └┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┃┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┘
      ┃                               ▼
      ┃       ┏━━━━━━━━━━━━━━━━━━━━━━━┻━━━━━━━━━━━━━━━━━━━━━━┓
      ┃       ┃                    Program                   ┃
      ┃       ┃                  (immutable)                 ┃
      ┣━━━━━━◄┫       ┏━━━━━┓ ┏━━━━━━━━━━┓ ┏━━━━━━━━━┓       ┃
      ┃       ┃       ┃ AST ┃ ┃ Semantic ┃ ┃ Symbols ┃       ┃
      ┃       ┃       ┗━━━━━┛ ┗━━━━━━━━━━┛ ┗━━━━━━━━━┛       ┃
      ┃       ┗━━━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━━┛
      ▲                               ▼
┏━━━━━┻━━━━━┓                         ┃             ┏━━━━━━━━━━━┓
┃ Transform ┃◄━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━►┃ Inspector ┃
┗━━━━━━━━━━━┛                         ┃             ┗━━━━━━━━━━━┛
                                      ▼
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┻━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                                  Writers                                    ┃
┃                                                                             ┃
┃ ┏━━━━━━━━━━━━━━┓┏━━━━━━━━━━━━━┓┏━━━━━━━━━━━━━┓┏━━━━━━━━━━━━━┓┏━━━━━━━━━━━━┓ ┃
┃ ┃ SPIRV-Writer ┃┃ WGSL-Writer ┃┃ HLSL-Writer ┃┃ GLSL-Writer ┃┃ MSL-Writer ┃ ┃
┃ ┗━━━━━━━┳━━━━━━┛┗━━━━━━┳━━━━━━┛┗━━━━━━┳━━━━━━┛┗━━━━━━┳━━━━━━┛┗━━━━━━┳━━━━━┛ ┃
┗━━━━━━━━━┃━━━━━━━━━━━━━━┃━━━━━━━━━━━━━━┃━━━━━━━━━━━━━━┃━━━━━━━━━━━━━━┃━━━━━━━┛
          ▼              ▼              ▼              ▼              ▼
     ┏━━━━┻━━━┓      ┏━━━┻━━┓       ┏━━━┻━━┓       ┏━━━┻━━┓        ┏━━┻━━┓
     ┃ SPIR-V ┃      ┃ WGSL ┃       ┃ HLSL ┃       ┃ GLSL ┃        ┃ MSL ┃
     ┗━━━━━━━━┛      ┗━━━━━━┛       ┗━━━━━━┛       ┗━━━━━━┛        ┗━━━━━┛
```

## Reader

Readers are responsible for parsing a shader program and populating a
`ProgramBuilder` with the parsed AST and symbol information.

The WGSL reader is a recursive descent parser. It closely follows the WGSL
grammar in the naming of the parse methods.

## ProgramBuilder

A `ProgramBuilder` is the interface to construct an immutable `Program`.
There are a large number of helper methods for simplifying the creation of the
AST nodes. A `ProgramBuilder` can only be used once, and must be discarded after
the `Program` is constructed.

A `Program` is built from the `ProgramBuilder` via a call to
`resolver::Resolve()`, which will perform validation and semantic analysis.
The returned program will contain the semantic information which can be obtained
by calling `Program::Sem()`.

At any time before building the `Program`, `ProgramBuilder::IsValid()` may be
called to ensure that no error diagnostics have been raised during the
construction of the AST. This includes parser syntax errors, but not semantic
validation which happens during the `Resolve` phase.

If further changes to the `Program` are needed (say via a `Transform`) then a
new `ProgramBuilder` can be produced by cloning the `Program` into a new
`ProgramBuilder`.

Unlike `Program`s, `ProgramBuilder`s are not part of the public Tint API.

## AST

The Abstract Syntax Tree is a directed acyclic graph of `ast::Node`s which
encode the syntactic structure of the WGSL program.

The root of the AST is the `ast::Module` class which holds each of the declared
functions, variables and user declared types (type aliases and structures).

Each `ast::Node` represents a **single** part of the program's source, and so
`ast::Node`s are not shared.

The AST does not perform any verification of its content. For example, the
`ast::Array` node has numeric size parameter, which is not validated to be
within the WGSL specification limits until validation of the `Resolver`.

## Semantic information

Semantic information is held by `sem::Node`s which describe the program at
a higher / more abstract level than the AST. This includes information such as
the resolved type of each expression, the resolved overload of a builtin
function call, and the module scoped variables used by each function.

Semantic information is generated by the `Resolver` when the `Program`
is built from a `ProgramBuilder`.

The `sem::Info` class holds a map of `ast::Node`s to `sem::Node`s.
This map is **many-to-one** - i.e. while a AST node might have a single
corresponding semantic node, the reverse may not be true. For example:
many `ast::IdentifierExpression` nodes may map to a single `sem::Variable`,
and so the `sem::Variable` does not have a single corresponding
`ast::Node`.

Unlike `ast::Node`s, semantic nodes may not necessarily form a directed acyclic
graph, and the semantic graph may contain diamonds.

## Types

AST types are regular AST nodes, in that they uniquely represent a single part
of the parsed source code. Unlike semantic types, identical AST types are not
de-duplicated as they refer to the source usage of the type.

Semantic types are constructed during the `Resolver` phase, and are held by
the `Program` or `ProgramBuilder`.

Each `sem::Type` node **uniquely** represents a particular WGSL type within the
program, so you can compare `type::Type*` pointers to check for type
equivalence. For example, a `Program` will only hold one instance of the
`sem::I32` semantic type, no matter how many times an `i32` is mentioned in the
source program.

WGSL type aliases resolve to their target semantic type. For example, given:

```wgsl
type MyI32 = i32;
const a : i32 = 1;
const b : MyI32 = 2;
```

The **semantic** types for the variables `a` and `b` will both be the same
`sem::I32` node pointer.

## Symbols

Symbols represent a unique string identifier in the source program. These string
identifiers are transformed into symbols within the `Reader`s.

During the Writer phase, symbols may be emitted as strings using a `Namer`.
A `Namer` may output the symbol in any form that preserves the uniqueness of
that symbol.

## Resolver

The `Resolver` will automatically run when a `Program` is built.
A `Resolver` creates the `Program`s semantic information by analyzing the
`Program`s AST and type information.

The `Resolver` will validate to make sure the generated `Program` is
semantically valid.

## Program

A `Program` holds an immutable version of the information from the
`ProgramBuilder` along with semantic information generated by the
`Resolver`.

`Program::IsValid()` may be called to ensure the program is structurally correct
**and** semantically valid, and that the `Resolver` did not report any errors
during validation.

Unlike the `ProgramBuilder`, a `Program` is fully immutable, and is part of the
public Tint API. The immutable nature of `Program`s make these entirely safe
to share between multiple threads without the use of synchronization primitives.

## Inspector

The inspectors job is to go through the `Program` and pull out various pieces of
information. The information may be used to pass information into the downstream
compilers (things like specialization constants) or may be used to pass into
transforms to update the AST before generating the resulting code.

The input `Program` to the inspector must be valid (pass validation).

## Transforms

There maybe various transforms we want to run over the `Program`.
This is for things like Vertex Pulling or Robust Buffer Access.

A transform operates by cloning the input `Program` into a new `ProgramBuilder`,
applying the required changes, and then finally building and returning a new
output `Program`. As the resolver is always run when a `Program` is built,
Transforms will always emit a `Program` with semantic information.

The input `Program` to a transform must be valid (pass validation).
If the input `Program` of a transform is valid then the transform must guarantee
that the output program is also valid.

## Writers

A writer is responsible for writing the `Program` in the target shader language.

The input `Program` to a writer must be valid (pass validation).
