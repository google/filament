# Tint changes during Origin Trial

## Changes for M112

### Breaking changes

* Most builtin functions that return a value can no longer be used as a call statement. [tint:1844](crbug.com/tint/1844)
* The `sig` member of the return type of `frexp()` has been renamed to `fract`. [tint:1766](crbug.com/tint/1766)
* Calling a function with multiple pointer arguments that alias each other is now a error. [tint:1675](crbug.com/tint/1675)
* `type` deprecation has been removed. `alias` must be used now. [tint:1812](crbug.com/tint/1812)
* `static_assert` deprecation has been removed. `const_assert` must now be used. [tint:1807](crbug.com/tint/1807)

## Changes for M111

### New features

* The `workgroupUniformLoad` builtin function is now supported. [tint:1780](crbug.com/tint/1780)
* The `diagnostic` directive and `@diagnostic` attribute are now supported. [tint:1809](crbug.com/tint/1809)
  * The attribute is currently only supported on function declarations.

### Breaking changes

* You may need to add parentheses to less-than or greater-than binary expressions that now parse as template lists. For example `a(b<c, d>e)` will need parentheses around `b<c` or `d>e`. [tint:1810](crbug.com/tint/1810).
* Uniformity analysis failures are now an error [tint:880](crbug.com/tint/880)
  * The `derivative_uniformity` diagnostic filter can be used to modify the severity if needed.

## Deprecated Features

* The keyword to alias a type has been renamed from `type` to `alias`. [tint:1812](crbug.com/tint/1812)
* `static_assert` has been renamed to `const_assert`. [tint:1807](crbug.com/tint/1807)

## Changes for M110

### Breaking changes

* The `textureSampleLevel()` overload for `texture_external` has been removed. Use `textureSampleBaseClampToEdge()`. [tint:1671](crbug.com/tint/1671)

### Deprecated Features

* The `sig` member of the return type of `frexp()` has been renamed to `fract`. [tint:1757](crbug.com/tint/1757)
* Calling a function with multiple pointer arguments that alias each other is now a warning, and
  will become an error in a future release. [tint:1675](crbug.com/tint/1675)

## Changes for M109

### Breaking changes

* `textureDimensions()`, `textureNumLayers()` and `textureNumLevels()` now return unsigned integers / vectors. [tint:1526](crbug.com/tint/1526)
* The `@stage` attribute has been removed. The short forms should be used
  instead (`@vertex`, `@fragment`, or `@compute`). [tint:1503](crbug.com/tint/1503)
* Module-scope `let` is now an error. Use module-scope `const` instead. [tint:1580](crbug.com/tint/1584)
* Reserved words are now an error instead of a deprecation. [tint:1463](crbug.com/tint/1463)
* You may no longer use pointer parameters in `workgroup` address space. [tint:1721](crbug.com/tint/1721)

### New features

* Uniformity analysis failures are warnings again [tint:1728](crbug.com/tint/1728)
* You can now call texture builtins with a mix of signed and unsigned integer arguments. [tint:1733](crbug.com/tint/1733)

## Changes for M108

### New features

* `textureSampleBaseClampToEdge()` has been implemented. [tint:1671](crbug.com/tint/1671)

### Deprecated Features

* The `external_texture` overload of `textureSampleLevel()` has been deprecated. Use `textureSampleBaseClampToEdge()` instead. [tint:1671](crbug.com/tint/1671)

### Fixes

* Constant evaluation of type conversions where the value exceeds the limits of the target type have been fixed. [tint:1707](crbug.com/tint/1707)

## Changes for M107

### New features

* `saturate()` has been implemented. [tint:1591](crbug.com/tint/1591)

### Breaking changes

* Uniformity analysis failures are now an error [tint:880](crbug.com/tint/880)
* Indexing an array, vector or matrix with a compile-time expression that's out-of-bounds is now an error [tint:1665](crbug.com/tint/1665)

## Changes for M106

### New features

* `array()` constructor can now infer type and count. [tint:1628](crbug.com/tint/1628)
* `static_assert` statement has been added. [tint:1625](crbug.com/tint/1625)

### Deprecated Features

* The list of reserved words has been sync'd to the WGSL specification. [tint:1463](crbug.com/tint/1463)

## Changes for M105

### New features

* Module-scope `var<private>` can now infer the storage type, like function-scope `var`. [tint:1584](crbug.com/tint/1584)
* The `acosh`, `asinh`, and `atanh` builtin functions are now supported [tint:1465](crbug.com/tint/1465)

### Breaking changes

* The `smoothStep()` builtin has been removed (use `smoothstep` instead). [tint:1483](crbug.com/tint/1483)
* Module-scope `let` has been replaced with module-scope `const`. [tint:1580](crbug.com/tint/1584)
  * Note: Module-scope `const` does not support structure types. Use `var<private>` if you need a module-scope structure type.
* Struct members can no longer be separated with semicolons (use commas instead). [tint:1475](crbug.com/tint/1475)
* Single scalar matrix constructors have been removed. These were never part of the WGSL spec. [tint:1597](crbug.com/tint/1597)

### Deprecated Features

* The `@stage` attribute has been deprecated. The short forms should be used
  instead (`@vertex`, `@fragment`, or `@compute`). [tint:1503](crbug.com/tint/1503)

## Changes for M104

### New features

* Tint now supports abstract-numerics, removing the need to always suffix unsigned integers with `u` [tint:1504](crbug.com/tint/1504)
* Parsing of `@compute`, `@fragment` and `@vertex` added.

## Changes for M103

### New features

* Produce warnings for when calling barriers, textureSample, and derivative
builtins in non-uniform control flow [tint:880](crbug.com/tint/880)
* Matrix identity constructors and constructors for a single scalar value are now supported [tint:1545](crbug.com/tint/1545)

### Breaking changes

* Builtin `atomicCompareExchangeWeak` returns a struct instead of a vec2. [tint:1185](crbug.com/tint/1185)

## Changes for M102

### New Features

* Parentheses are no longer required around expressions for if and switch statements [tint:1424](crbug.com/tint/1424)
* Compound assignment statements are now supported. [tint:1325](https://crbug.com/tint/1325)
* Postfix increment and decrement statements are now supported. [tint:1488](crbug.com/tint/1488)
* The colon in case statements is now optional. [tint:1485](crbug.com/tint/1485)

### Breaking changes

* Struct members are now separated by commas. [tint:1475](crbug.com/tint/1475)
* The `@block` attribute has been removed. [tint:1324](crbug.com/tint/1324)
* The `@stride` attribute has been removed. [tint:1381](crbug.com/tint/1381)
* Attributes using `[[attribute]]` syntax are no longer supported. [tint:1382](crbug.com/tint/1382)
* The `elseif` keyword is no longer supported. [tint:1289](crbug.com/tint/1289)

### Deprecated Features

* The `smoothStep()` builtin has been renamed to `smoothstep()`. [tint:1483](crbug.com/tint/1483)

## Changes for M101

### New Features

* Tint now supports unicode identifiers. [tint:1437](crbug.com/tint/1437)

### Breaking changes

* The `isNan()`, `isInf()`, `isFinite()`, and `isNormal()` builtins have been removed. [tint:1312](https://crbug.com/tint/1312)

## Changes for M100

### Breaking changes

* The `@interpolate(flat)` attribute must now be specified on integral user-defined IO. [tint:1224](crbug.com/tint/1224)
* The `ignore()` intrinsic has been removed. Use phoney-assignment instead: `ignore(expr);` -> `_ = expr;`.
* `break` statements in `continuing` blocks are now correctly validated.

### New Features

* Module-scope declarations can now be declared in any order. [tint:1266](crbug.com/tint/1266)
* The `override` keyword and `@id()` attribute for pipeline-overridable constants are now supported, replacing the `@override` attribute. [tint:1403](crbug.com/tint/1403)

## Changes for M99

### Breaking changes

Obviously infinite loops (no condition, no break) are now a validation error.

### Deprecated Features

The following features have been deprecated and will be removed in M102:

* The `[[block]]` attribute has been deprecated. [tint:1324](https://crbug.com/tint/1324)
* Attributes now use the `@decoration` syntax instead of the `[[decoration]]` syntax. [tint:1382](https://crbug.com/tint/1382)
* `elseif` has been replaced with `else if`. [tint:1289](https://crbug.com/tint/1289)
* The `[[stride]]` attribute has been deprecated. [tint:1381](https://crbug.com/tint/1381)

### New Features

* Vector and matrix element type can now be inferred from constructor argument types. [tint:1334](https://crbug.com/tint/1334)
* Added builtins `degrees()` and `radians()` for converting between degrees and radians. [tint:1329](https://crbug.com/tint/1329)
* `let` arrays and matrices can now be dynamically indexed. [tint:1352](https://crbug.com/tint/1352)
* Storage and Uniform buffer types no longer have to be structures. [tint:1372](crbug.com/tint/1372)
* A struct declaration does not have to be followed by a semicolon. [tint:1380](crbug.com/tint/1380)

### Fixes

* Fixed an issue where for-loops that contain array or structure constructors in the loop initializer statements, condition expressions or continuing statements could fail to compile. [tint:1364](https://crbug.com/tint/1364)

## Changes for M98

### Breaking Changes

* Taking the address of a vector component is no longer allowed.
* Module-scope declarations can no longer alias a builtin name. [tint:1318](https://crbug.com/tint/1318)
* It is now an error to call a function either directly or transitively, from a loop continuing block, that uses `discard`. [tint:1302](https://crbug.com/tint/1302)

### Deprecated Features

* The `isNan()`, `isInf()`, `isFinite()` and `isNormal()` builtins has been deprecated and will be removed in M101. [tint:1312](https://crbug.com/tint/1312)

### New Features

* New texture gather builtins: `textureGather()` and `textureGatherCompare()`. [tint:1330](https://crbug.com/tint/1330)
* Shadowing is now fully supported. [tint:819](https://crbug.com/tint/819)
* The `dot()` builtin now supports integer vector types.
* Identifiers can now start with a single leading underscore.  [tint:1292](https://crbug.com/tint/1292)
* Control flow analysis has been improved, and functions no longer need to `return` if the statement is unreachable. [tint:1302](https://crbug.com/tint/1302)
* Unreachable statements now produce a warning instead of an error, to allow WGSL code to be updated to the new analysis behavior. These warnings may become errors in the future [gpuweb#2378](https://github.com/gpuweb/gpuweb/issues/2378)

### Fixes

* Fixed an issue where using a module-scoped `let` in a `workgroup_size` may result in a compilation error. [tint:1320](https://crbug.com/tint/1320)

## Changes for M97

### Breaking Changes

* Deprecated `modf()` and `frexp()` builtin overloads that take a pointer second parameter have been removed.
* Deprecated texture builtin functions that accepted a `read` access controlled storage texture have been removed.
* Storage textures must now only use the `write` access control.

### Deprecated Features

* The `ignore()` builtin has been replaced with phony-assignment. [gpuweb#2127](https://github.com/gpuweb/gpuweb/pull/2127)

### New Features

* `any()` and `all()` now support a `bool` parameter. These simply return the passed argument. [tint:1253](https://crbug.com/tint/1253)
* Call statements may now include functions that return a value (`ignore()` is no longer needed).
* The `interpolate(flat)` attribute can now be specified on integral user-defined IO. It will eventually become an error to define integral user-defined IO without this attribute.
* Matrix construction from scalar element values is now supported.

### Fixes

* Swizzling of `vec3` types in `storage` and `uniform` buffers has been fixed for Metal 1.x. [tint:1249](https://crbug.com/tint/1249)
* Calling a function that returns an unused value no longer produces an FXC compilation error. [tint:1259](https://crbug.com/tint/1259)
* `abs()` fixed for unsigned integers on SPIR-V backend

## Changes for M95

### New Features

* The size of an array can now be defined using a non-overridable module-scope constant
* The `num_workgroups` builtin is now supported.

### Fixes

* Hex floats: now correctly errors when the magnitude is non-zero, and the exponent would cause overflow. [tint:1150](https://crbug.com/tint/1150), [tint:1166](https://crbug.com/tint/1166)
* Identifiers beginning with an underscore are now correctly rejected.  [tint:1179](https://crbug.com/tint/1179)
