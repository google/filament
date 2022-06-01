# beamsplitter

- [Description](#description)
- [Instructions](#instructions)
- [Emitter Flags](#emitter-flags)
- [Source Files](#source-files)
- [Output Files](#output-files)
- [Input Format](#input-format)

## Description

This Go program consumes C++ header file(s) and generates Java bindings, JavaScript bindings, and
C++ code that performs JSON serialization.

## Instructions

To install the Go compiler on macOS, just do:

    brew install go

To build and invoke the code generator, do:

    cd tools/beamsplitter ; go run .

## Emitter Flags

Special directives in the form `%codegen_foo%` are called *emitter flags*. They are typically
embedded in a comment associated with a particular struct field.

flag                        | description
--------------------------- | ----
**codegen_skip_json**       | Field is skipped when generating JSON serialization code.
**codegen_skip_javascript** | Field is skipped when generating JavaScript and TypeScript bindings.
**codegen_java_flatten**    | Field is replaced with constituent sub-fields.
**codegen_java_float**      | Field will be forced to have a `float` representation in Java.

## Source Files

- `filament/include/filament/Options.h`

## Output Files

 The following files are created:

- `libs/viewer/src/Settings_generated.h`
- `libs/viewer/src/Settings_generated.cpp`
- `web/filament-js/jsbindings_generated.cpp`
- `web/filament-js/jsenums_generated.cpp`
- `web/filament-js/extensions_generated.js`

Additionally, in-place edits are made to the following files:

- `web/filament-js/filament.d.ts`
- `android/filament-android/src/main/java/.../View.java`

## Input Format

There are many ways in which the source file format is more restrictive than the full C++
language, but here are some of the highlights:

- All enums must be class enums.
- External headers pulled in with `#include` files are ignored.
- Expressions in the RHS of default value assignments are not parsed, they are just exposed by
  the lexer as blobs.
- Struct fields, class fields, and method arguments must have fairly simple types. e.g. they cannot
  have parentheses. If a type is C style callback, then it should be specified with an alias.
- Multiline strings and macro definitions are not allowed.
- Enum values must be sequential and cannot have custom values.

The following formal grammar describes the above limitations in greater detail, but with some
caveats:

- All C preprocessor directives are discarded during lexical analysis; they do not exist in the AST.
- Whitespace is similarly discarded, so there is no "space" concept in the AST.
- Macro invocations are also removed by the lexer if they are known Filament-specific macros (e.g.
  `UTILS_PUBLIC` and `UTILS_DEPRECATED`).
- Comments are removed by the lexer and are generally not part of the resulting AST. However
  the lexer proffers a mapping from line numbers to comments to allow for docstring extraction.
- Emitter flags in the form `%codegen_foo%` are detected in a post-processing phase and removed from
  all comments.

### Grammar

```eBNF
root = namespace ;
namespace = "namespace" , [ ident ] , "{" , { block } , "}" ;
block = class | struct | enum | namespace | using | forward_declaration;
forward_declaration = ("class" | "struct" ) , ident , ";" ;
template = "template" , "TemplateArgs" ;
class = [template] , "class" , ident , [ ":" , [ "public" ] , "SimpleType" ]
    , "{" , struct_body  , "}" , ";" ;
struct = [template] , "struct" , ident , "{" , struct_body , "}" , ";" ;
enum = "enum" , "class" , ident , [ ":" , type ]
    , "{" , , ident , { "," , ident } , [ "," ] , "}" , ";" ;
using = "using" , ident , "=", type , ";" ;
struct_body = { access_specifier | field | method | block } ;
access_specifier = ("public" | "private" | "protected" ) , ":" ;
method = [template] , { "constexpr" , "friend" } ,
    , type , ident , "MethodArgs" , specifiers , ( ";" | "MethodBody" ) ;
specifiers = { "const" | "noexcept" } ;
field = type , ident , [ array ] , [ "=" , "DefaultValue" ] ";" ;
array = "[" , "ArrayLength", "]" ;
type = "SimpleType" ;
ident = "Identifier" ;
```
The above grammar uses the following notation:
- `" ... "` denotes a terminal
- `{ ... }` denotes zero or more repetition
- `[ ... ]` denotes an optional quantity
- `( ... )` is used for grouping
- `a | b` denotes a choice
- `a , b` denotes concatenation
- `;` terminates a production

Terminal name               | Description
--------------------------- | ----
SimpleType (*)              | examples: `Texture* const`, `uint8_t`, `BlendMode`
MethodBody                  | unparsed implementation of a function or method, including outer `{}`
MethodArgs                  | similar to above; an unparsed blob, but delimited with `()`
TemplateArgs                | similar to above; an unparsed blob, but delimited with `<>`
DefaultValue (**)           | an unparsed expression with certain restrictions
Identifier                  | `[A-Za-z_][A-Za-z0-9_]*`
ArrayLength                 | `[1-9][0-9]*`

(*) `SimpleType` should not contain parentheses or commas, so C callbacks are not allowed unless
you alias them first.

(**) If `DefaultValue` is a vector, it must be in the form: `{ x, y, z }`.

## References

Initially inspired by the following Rob Pike talk.
- https://www.youtube.com/watch?v=HxaD_trXwRE

Beamsplitter does not use the state machine described in the above prezo, but it does use a channel
for separating the parser from the lexer. The beamsplitter lexer is actually a recursive descent
parser with simple lookahead functionality. This makes it easy for the "real" parser to create a
coarse-grained AST.

The companion to the above talk is Go's template lexer, which can be studied here:
- https://cs.opensource.google/go/go/+/master:src/text/template/parse/lex.go

Wikipedia has a good example of recursive descent:
- https://en.wikipedia.org/wiki/Recursive_descent_parser
