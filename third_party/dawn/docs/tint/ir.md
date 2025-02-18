# Intermediate Representation

As Tint has grown the number of transforms on the AST has grown. This
growth has lead to several issues:

1. Transforms rebuild the AST and SEM which causes slowness
1. Transforming in AST can be difficult as the AST is hard to work with

In order to address these goals, an IR was introduced into Tint. The IR
is mutable, it holds the needed state in order to be transformed. The IR
is also translatable back into AST. It will be possible to generate an
AST, convert to IR, transform, and then rebuild a new AST. This
round-trip ability provides a few features:

1. Easy to integrate into current system by replacing AST transforms
   piecemeal
1. Easier to test as the resulting AST can be emitted as WGSL and
   compared.

The IR helps with the complexity of the AST transforms by limiting the
representations in the IR form. For example, instead of `for`, `while`
and `loop` constructs there is a single `loop` construct. `alias` and
`const_assert` nodes are not emitted into IR. Dead code maybe eliminated
during IR construction.

As the IR can convert into AST, we could potentially simplify the
SPIRV-Reader by generating IR directly. The IR is closer to what SPIR-V
looks like, so maybe a simpler transform.


## Design

The IR is composed of three primary concepts, `blocks`, `values`, and
`instructions`.

The `blocks` provide lists of `instructions` which are executed
sequentially. An `instruction` may contain `blocks`, `value` operands
and `value` results. A `value` is a block argument, function parameter,
constant result, or a result computed by an `instruction`.

Each `block` ends in a `terminator` expression. There are a few blocks
which maybe empty, `if-false block`, `loop-initializer` and
`loop-continuing`. Those blocks are optional. The `terminators` may hold
`values` which are returned out of the block being terminated.

Control flow is handled through `ControlInstructions`. These are the
`if`, `switch` and `loop` instructions. Each control instruction
contains `blocks` for the various control flow paths.

Transforming from AST to IR and back to AST is a lossy operation.
The resulting AST when converting back may not be the same as the
AST being provided. This transformation is intentional as it greatly
simplifies the number of things to consider in the IR. For instance:

* No `alias` nodes
* No `const_assert` nodes


### Code Structure
The code is contained in the `src/tint/ir` folder and is broken down
into several classes. Note, the IR is a Tint _internal_ representation
and these files should _never_ appear in the public API.


#### Builder
The `Builder` class provides useful helper routines for creating IR
content. The Builder references an `ir::Module`.


#### Module
The top level of the IR is the `Module`. The module stores a list of
`functions`, constant manager, type manager, allocators and various other
bits of information needed by the IR. The `module` _may_ contain a
disassembled representation of the module if the disassembler has been
called.

A free `ir::FromProgram` method is provided to convert from a
`Program` to an `ir::Module`. A similar `ToProgram` free method is
provided for the reverse conversion.


### Transforms
Similar to the AST a transform system is available for IR. The transform
has the same setup as the AST (and inherits from the same base transform
class.)

The IR transforms live in `src/tint/lang/core/ir/transform`. These transforms are
for use by the various IR generator backends.

Unlike with the AST transforms, the IR transforms know which transforms
must have run before themselves. The transform manager will then verify
that all the required transforms for a given transform have executed.


### Validation
The IR contains a validator. The validator checks for common errors
encountered when building IR. The validator is _not_ run in production
as any error is an internal coding error. The validator is executed by
the generators before starting, and after each transform executes in
debug mode.


### Naming
The instructions are internal to the IR, and the values in the IR do not
contain names. The IR module has a symbol table which can be used to
retrieve names for the instructions and values but those names may not
be the names provided originally by the AST.


### Control Flow

#### Block
A block contains the instruction lists for a section of code. A block
always ends in a terminator instruction.

The instructions in a block can be walked in linear scoped fashion. The
control instructions do not need to be entered to walk a blocks
instructions.


#### Control Instructions
The control instructions provide containers for other blocks. These
include the `if`, `loop` and `switch` instruction. A control instruction
is a self contained scope on top of the containing block scope.


##### Control Instruction -- If
The if instruction is an `if-else` structure. There are no `else-if`
entries, they get moved into the `else` of the `if`. The if instruction
has two internal blocks, the `True` and `False` blocks.

A if instruction will _always_ have a `True` target. The `False` target
maybe empty.

The sub-blocks under the `if` can be terminated with `ExitIf`,
`ExitSwitch`, `ExitLoop`, `Continue` or `Return`.


#### Control Instruction -- Loop
All of the loop structures in AST merge down to a single loop
instruction.  The loop contains the `Body`, which is required and
optional `Initializer` and `Continuing` blocks.

The `loop` body can be terminated with a `ExitLoop`, `Continue`,
`NextIteration` or `Return`.

The `loop` continuing can be terminated with a `NextIteration`, or a
`BreakIf` terminator.

A while loop is decomposed as listed in the WGSL spec:

```
while (a < b) {
  c += 1;
}
```

becomes:

```
loop {
  if (!(a < b)) {
    break;
  }
  c += 1;
}
```

A for loop is decomposed as listed in the WGSL spec:
```
for (var i = 0; i < 10; i++) {
  c += 1;
}
```

becomes:

```
var i = 0;
loop {
  if (!(i < 10)) {
    break;
  }

  c += 1;

  continuing {
    i++;
  }
}
```

#### Control Instruction -- Switch
The switch instruction has a block for each of the `case/default` labels.

The `switch` case blocks can be terminated with an `ExitSwitch`,
`Continue`, or `Return`.


#### Expressions
All expressions in IR are single operations. There are no complex
expressions. Any complex expression in the AST is broke apart into the
simpler single operation components.

```
var a = b + c - (4 * k);
```

becomes:

```
%0:i32 = add %b, %c
%1:i32 = mul 4u, %k
%2:i32 = sub %0, %1
%v:ptr<function, i32, read_write> = var %2
```

This also means that many of the short forms `i += 1`, `i++` get
expanded into the longer form of `i = i + 1`.


##### Short-Circuit Expressions
The short-circuit expressions (e.g. `a && b`) are convert into an `if`
structure control flow.

```
let c = a() && b()
```

becomes

```
let c = a();
if (c) {
  c = b();
}
```

#### Values
There are several types of values used in the SSA form.

1. `Constant` Value
1. `InstructionResult` Value
1. `Function` Value
1. `FunctionParam` Value
1. `BlockParam` Value


##### Constant Value
All values in IR are concrete, there are no abstract values as
materialization has already happened. Each constant holds a lower level
`constant::Value` from the `src/tint/lang/core/ir/constant` system.


##### InstructionResult Value
The `InstructionResult` is the result of an instruction. The
temporaries are created as complex expressions are broken down into
pieces. The result tracks the usage for the value so you can determine
which instructions use this value. The `InstructionResult` also points
back to its `Source` instruction.


### Function Value
All `Functions` are values in the IR. This allows the function to be
provided as a Value argument to things like the `Call` instructions.


##### FunctionParam Value
The function param values are used to store information on the values
being passed into a function call.

### BlockParam Values
The `BlockParam` values are used to pass information for a
`MultiInBlock` (e.g. Loop body, and loop continuing).

## Alternatives
Instead of going to a custom IR there are several possible other roads
that could be travelled.

### Mutable AST
Tint originally contained a mutable AST. This was converted to immutable
in order to allow processing over multiple threads and for safety
properties. Those desires still hold, the AST is public API, and we want
it to be as safe as possible, so keeping it immutable provides that
guarantee.

### Multiple Transforms With One Program Builder
Instead of generating an immutable AST after each transform, running
multiple transforms on the single program builder would remove some of
the performance penalties of going to and from immutable AST. While this
is true, the transforms use a combination of AST and SEM information.
When they transform they _do not_ create new SEM information. That
means, after a given transform, the SEM is out of date. In order to
re-generate the SEM the resolver needs to be rerun. Supporting this
would require being very careful on what transforms run together and
how they modify the AST.

### Adopt An Existing IR
There are already several IRs in the while, Mesa has NIR, LLVM has
LLVM IR. There are others, adopting one of those would remove the
requirements of writing and maintaining our own IR. While that is true,
there are several downsides to this re-use. The IRs are internal to the
library, so the API isn't public, LLVM IR changes with each iteration of
LLVM. This would require us to adapt the AST -> IR -> AST transform for
each modification of the IR.

They also end up being lower level then is strictly useful for us. While
the IR in Tint is a simplified form, we still have to be able to go back
to the high level structured form in order to emit the resulting HLSL,
MSL, GLSL, etc. (Only SPIR-V is a good match for the lowered IR form).
This transformation back is not a direction other IRs maybe interested
in so may have lost information, or require re-determining (determining
variables from SSA and PHI nodes for example).

Other technical reasons are the maintenance of BUILD.gn and CMake files
in order to integrate into our build systems, along with resulting
binary size questions from pulling in external systems.

