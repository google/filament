# Intrinsic definition files

Tint uses intrinsic definition to generate intrinsic tables, end-to-end tests and C++ enums.

These definition files have a `.def` extension and exist under the language subdirectories of `src/tint/lang/`. \
For example, the WGSL definition file can be found at `src/tint/lang/wgsl/wgsl.def`.

A definition file can contain the following declarations:

* [Imports](#imports)
* [Enumerators](#enumerators)
* [Types](#types)
* [Type matchers](#type-matchers)
* [Overload declarations](#overload-declarations)
  * [Builtin Functions](#builtin-functions)
  * [Value constructors](#value-constructors)
  * [Value conversions](#value-conversions)
  * [Operators](#operators)

## Imports

Syntax: `import "` _**project-relative-path**_ `"`

An import is similar to a C++ `#include` statement. It imports the content of the project-relative
file into this definition file so that declarations can be shared between languages.

Example: `import "src/tint/lang/core/access.def"`

## Enumerators

An named enumerator of symbols can be declared with `enum`.

Example:

```c
enum diagnostic_severity {
  error
  warning
  info
  off
}
```

These enum declarations can be used by templates to generate an equivalent C++ enum, along with a parser and printer helper method, using the helper template [`src/tint/utils/templates/enums.tmpl.inc`](../../src/tint/utils/templates/enums.tmpl.inc). \
See [`src/tint/lang/wgsl/extension.h.tmpl`](../../src/tint/lang/wgsl/extension.h.tmpl) as an example.

Enum declarations can also be used as constraints on type template parameters and overload template parameters.

## Types

Syntax: `type` _**name**_ _**[**_ `<` _**type-or-number-templates**_ `>` _**]**_

Type declarations form the basic primitives are used to match intrinsic overloads. A type can be referenced by type matchers and by builtin overloads.

The simplest type declaration has the form: `type name`

A type can be templated with any number of sub-types and numbers, using a `<`...`>` suffix.

Examples:

| Example               | Description                      |
|-----------------------|----------------------------------|
| `type my_type`        | Declares a type called `my_type` |
| `type vec3<T>`        | Declares a type called `vec3` which accepts a templated sub-type |
| `type vec<N: num, T>` | Declares a type called `vec` which accepts a templated number `N` and sub-type `T` |

### Annotations

Types may be annotated with `@precedence(N)` to prioritize which type will be picked when multiple types of a matcher match. This is typically used to ensure that abstract numerical types materialize to the concrete type with the lowest conversion rank. Types with higher precedence values will be matched first.

Example:

```ts
@precedence(5) type ia // This is preferred over all others
@precedence(4) type fa
@precedence(3) type i32
@precedence(2) type u32
@precedence(1) type f32
@precedence(0) type f16
```

Types may also be annotated with `@display("`_string_`")` to customize how the type is printed in diagnostic message. The string may contain curly-brace references to the templated types.

Example:

```ts
@display("vec{N}<{T}>") type vec<N: num, T>
```

Would display as `vec2<i32>` if `N=2` and `T=i32`.

## Type Matchers

Syntax: `matcher` _**name**_ `:` _**type_1 [**_ `|`  _**type_2 [ ...**_ `|` _**type_n ] ]**_

A type matcher can be used by overloads to match one or more types.

## Overload declarations

The overload declarations declare all the built-in functions, Value constructors, type
conversions and unary and binary operators supported by the target language.

An overload declaration can declare a simple static-type declarations, as well as overload
declarations that can match a number of different argument types via the use of template types,
template numbers and type matchers.

All overload declarations share the same declaration patterns, but the kind of declaration has a different prefix.

* [Builtin Functions](#builtin-functions) have a `fn` prefix
* [Value constructors](#value-constructors) have a `ctor` prefix
* [Value conversions](#value-conversions) have a `conv` prefix
* [Operators](#operators) have a `op` prefix

For example: `fn isInf(f32) -> bool` declares an overload of the function `isInf` that accepts a single parameter of type `f32` and returns a `bool`.

### Parameters

All overload declarations use the same syntax for a parameter.

A parameter can be named or unnamed.

Unnamed parameters just use the name of constraint (type, type-matcher or template parameter).
Named parameters have a name and constraint, separated with a colon. Example `fn F(count : i32)`

| Code               | Description                                                                 |
|--------------------|-----------------------------------------------------------------------------|
| `fn F(i32)`        | function `F` has a single unnamed parameter, of type or matcher `i32`       |
| `fn F(count: i32)` | function `F` has a single parameter named `count`, of type or matcher `i32` |

Note: Parameters may use a type-matcher directly, but for `wgsl.def` it is encouraged to use an implicit template parameter. This helps the readability of the diagnostics produced. For example, instead of `fn F(scalar)`, prefer: `fn F[S: scalar](S)`.

### Templates

Builtin function overload declarations can support **explicit** and **implicit** template parameters.

Each template parameter has the syntax: _**name**_ _**[**_ `:` _**constraint ]**_
Where the optional _**constrant**_ can be a [type](#types), [type matcher](#type-matchers), or `: num` for a number template.

**Explicit template parameters:**

* Are optional in the overload declaration.
* Have the syntax: `<` _**template_parameter_list**_ `>`
* Must be declared before implicit template parameters (if declared) and parameter list.
* Must be provided by the call of the function, with exactly one template argument provided for each template parameter.
* May reference implicit template parameters.
* Can only be declared on [Builtin Functions](#builtin-functions).
* Do not support number templates.

**Implicit template parameters:**

* Are optional in the overload declaration.
* Are inferred from the arguments passed to the intrinsic.
* Have the syntax: `[` _**template_parameter_list**_ `]`
* Must be declared after explicit template parameters (if declared), and before the parameter list.
* Cannot reference explicit template parameters.
* Support number templates.

Examples:

```ts
// A function overload that has no explicit or implicit template parameters
fn F(i32) -> i32
```

```ts
// A function overload that accepts an explicit template parameter 'T', which matches any type.
// The overload takes a single parameter of type 'T' and returns a value of type 'T'
fn F<T>(T) -> T
```

```ts
// A function overload that accepts an implicit (inferred) template parameter 'T', which must be of
// the type or type-matcher 'scalar'.
// The overload takes a single parameter of type 'T' and returns a value of type 'T'
fn F[T: scalar](T) -> T
```

```ts
// A function overload that accepts an explicit template parameter 'T', which must be of a 'vec'
// type of the inferred element type 'E' and number 'N'.
// 'E' is an implicit template type parameter, and must match the type or type-matcher 'scalar'
// 'N' is an implicit template number parameter.
// The overload takes a single parameter of type 'T' and returns a value of type 'T'
fn F< T: vec<N, E> >[E: scalar, N: num](T) -> T
```

### Matching algorithm for a single overload

The goal of overload matching is to compare a function call's arguments and any
explicitly provided template types in the program source against an overload
declaration, to determine if the call satisfies the form and type constraints
of the overload. If the call matches an overload, then the overload is added
to the list of 'overload candidates' used for overload resolution (described below).

Prior to matching an overload, all template types are undefined.

Implicit template types are first defined with the type of the leftmost argument
that matches against that template type name. Subsequent arguments that attempt
to match against the template type name will either reject the overload or
refine the template, in one of 3 ways:

* (a) Fail to match, causing the overload to be immediately rejected.
* (b) Match the existing implicit template type, either exactly or via implicit
      conversion, and overload resolution continues.
* (c) Match via implicit conversion of the currently defined template type
      to the argument type. In this situation, the template type is refined
      with the more constrained argument type, and overload resolution
      continues.

To better understand, let's consider the following hypothetical overload declaration:

```ts
matcher scalar: f32 | i32 | u32 | bool
fn foo[T: scalar](T, T)
```

Where:


| Symbol        | Description                                                                      |
|---------------|----------------------------------------------------------------------------------|
|  `T`          | is the template type name                                                        |
| `scalar`      | is a matcher for the types `f32`, `i32`, `u32` or `bool`                         |
| `[T: scalar]` | declares the implicit template parameter `T`, with the constraint that `T` must match one of `f32`, `i32`, `u32` or `bool`. |

The process for resolving this overload is as follows:

1. The overload resolver begins by attempting to match the parameter types from left to right.
   The first argument type is compared against the parameter type `T`.
   As the implicit template type `T` has not been defined yet, `T` is defined as the type of the first argument.
   There's no verification that the `T` type is a scalar at this stage.
2. The second argument is then compared against the second parameter.
   As the template type `T` is now defined as the first argument type, the second argument type is
   compared against the type of `T`. Depending on the comparison of the argument type to the
   template type, either the actions of (a), (b) or (c) from above will occur.
3. If all the parameters matched, constraints on the template types need to be checked next.
   If the defined type does not match the constraint, then the overload is no longer considered.

This algorithm for matching a single overload is less general than the algorithm described in the
WGSL spec, but it makes the same decisions because the overloads defined by WGSL are monotonic
in the sense that once a template parameter has been refined, there is never a need to backtrack
and un-refine it to match a later argument.

The algorithm for matching template numbers is similar to matching template types, except numbers
need to exactly match across all uses - there is no implicit conversion. Template numbers may
match integer numbers or enumerators.

### Overload resolution for candidate overloads

If multiple candidate overloads match a given set of arguments, then a final overload resolution
pass needs to be performed. The arguments and overload parameter types for each candidate overload
are compared, following the algorithm described at: https:www.w3.org/TR/WGSL/#overload-resolution-section

If the candidate list contains a single entry, then that single candidate is picked, and no
overload resolution needs to be performed.

If the candidate list is empty, then the call fails to resolve and an error diagnostic is raised.

### Common attributes

Intrinsics that can be used in constant evaluation expressions are annotated with `@const` or `@const(` _**name**_ `)`.

The templates use this attribute to associate the overload with a function in [`src/tint/lang/core/constant/eval.h`](../../src/tint/lang/core/constant/eval.h).

## Builtin functions

Syntax: `fn` _**name [**_ `<` _**explicit-template-parameters**_ `>` _**] [**_ `[` _**implicit-template-parameters**_ `]` _**]**_`(` _**parameters**_ `) ->` _**return_type**_

Builtin function overloads support both implicit and explicit template parameters.

Builtin functions may be annotated with `@must_use` to prevent the function being used as a call-statement in WGSL.

### Examples

| Code                         | Description                                                                        |
|------------------------------|------------------------------------------------------------------------------------|
| `fn F()`                     | Function called `F`. No template types or numbers, no parameters, no return value` |
| `fn F() -> R`                | Function with `R` as the return type                                               |
| `fn F(f32, i32)`             | Two fixed-type, anonymous parameters                                               |
| `fn F(U : f32)`              | Single parameter with name `U`                                                     |
| `fn F[T](T)`                 | Single parameter of unconstrained implicit template type `T` (any type)            |
| `fn F[T: scalar](T)`         | Single parameter of constrained implict template type `T` (must match `scalar`)    |
| `fn F[T: fiu32](T) -> T`     | Single parameter of constrained implicit template type T (must match `fiu32`)<br>Return type is of the parameter type |
| `fn F[T, N: num](vec<N, T>)` | Single parameter of `vec` type with template number `N` and element template type `T` |
| `fn F[A: access](texture_storage_1d<f32_texel_format, A>)` | Single parameter of type `texture_storage_1d` type with implicit template number  `A` constrained to the enum `access`, and a texel format that that must match `f32_texel_format` |

## Value constructors

Syntax: `ctor` _**type-name [**_ `[` _**implicit-template-parameters**_ `]` _**]**_`(` _**parameters**_ `) ->` _**return_type**_

Value constructors construct a value of the given type.

## Value conversions

Syntax: `conv` _**type-name [**_ `[` _**implicit-template-parameters**_ `]` _**]**_`(` _**parameters**_ `) ->` _**return_type**_

Value conversions convert from one type to another.

## Operators

Unary operator syntax: `op` _**operator [**_ `[` _**implicit-template-parameters**_ `]` _**]**_`(` _**parameter**_ `) ->` _**return_type**_

Binary operator syntax: `op` _**operator [**_ `[` _**implicit-template-parameters**_ `]` _**]**_`(` _**lhs-parameter**_ `,` _**rhs-parameter**_ `) ->` _**return_type**_

