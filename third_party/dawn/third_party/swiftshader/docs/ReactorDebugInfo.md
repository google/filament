# Reactor Debug Info Generation

## Introduction

Reactor produces Just In Time compiled dynamic executable code and can be used to JIT high performance functions specialized for runtime
configurations, or to even build a compiler.

In order to debug executable code at a higher level than disassembly, source code files are required.

Reactor has two potential sources of source code:

1. The C++ source code of the program that calls into Reactor.
2. External source files read by the program and passed to Reactor.

While case (2) is preferable for implementing a compiler, this is currently not
implemented.

Reactor implements case (1) and this can be used by GDB to single line step and
inspect variables.

## Supported Platforms

Currently:

* Debug info generation is only supported on Linux with the LLVM 7
backend.
* GDB is the only supported debugger.
* The program must be compiled with debug info iteself.

## Enabling

Debug generation is enabled with `REACTOR_EMIT_DEBUG_INFO` CMake flag (defaults
to disabled).

## Implementation details

### Source Location

All Reactor functions begin with a call to `RR_DEBUG_INFO_UPDATE_LOC()`, which calls into `rr::DebugInfo::EmitLocation()`.

`rr::DebugInfo::EmitLocation()` calls `rr::DebugInfo::getCallerBacktrace()`,
which in turn uses [`libbacktrace`](https://github.com/ianlancetaylor/libbacktrace)
to unwind the stack and find the file, function and line of the caller.

This information is passed to `llvm::IRBuilder<>::SetCurrentDebugLocation`
to emit source line information for the next LLVM instructions to be built.

### Variables

There are 3 aspects to generating variable debug information:

#### 1. Variable names

Constructing a Reactor `LValue`:

```C++
rr::Int a = 1;
```

Will emit an LLVM `alloca` instruction to allocate the storage of the variable,
and emit another to initialize it to the constant `1`. While fluent, none of the
Reactor calls see the name of the C++ local variable "`a`", and the LLVM `alloca`
value gets a meaningless numerical value.

There are two potential ways that Reactor can obtain the variable name:

1. Use the running executable's own debug information to examine the local
   declaration and extract the local variable's name.
2. Use the backtrace information to parse the name from the source file.

While (1) is arguably a cleaner and more robust solution, (2) is
easier to implement and can work for the majority of use cases.

(2) is the current solution implemented.

`rr::DebugInfo::getOrParseFileTokens()` scans a source file line by line, and
uses a regular expression to look for patterns of `<type> <name>`. Matching is not
precise, but is adequate to find locals constructed with and without assignment.

#### 2. Variable binding

Given that we can find a variable name for a given source line, we need a way of
binding the LLVM values to the name.

Given our trivial example:

```C++
rr::Int a = 1
```

The `rr::Int` constructor calls `RR_DEBUG_INFO_EMIT_VAR()` passing the storage
value as single argument. `RR_DEBUG_INFO_EMIT_VAR()` performs the backtrace
to find the source file and line and uses the token information produced by
`rr::DebugInfo::getOrParseFileTokens()` to identify the variable name.

However, things get a bit more complicated when there are multiple variables
being constructed on the same line.

Take for example:

```C++
rr::Int a = rr::Int(1) + rr::Int(2)
```

Here we have 3 calls to the `rr::Int` constructor, each calling down
to `RR_DEBUG_INFO_EMIT_VAR()`.

To disambiguate which of these should be bound to the variable name "`a`",
`rr::DebugInfo::EmitVariable()` buffers the binding into
`scope.pending` and the last binding for a given line is used by
`DebugInfo::emitPending()`. For variable construction and assignment, C++
guarantees that the LHS is the last value to be constructed.

This solution is not perfect.

Multi-line expressions, multiple assignments on a single line, macro obfuscation
can all break variable bindings - however the majority of typical cases work.

#### 3. Variable scope

`rr::DebugInfo` maintains a stack of `llvm::DIScope`s and `llvm::DILocation`s
that mirrors the current backtrace for function being called.

A synthetic call stack is produced by chaining `llvm::DILocation`s with
`InlinedAt`s.

For example, at the declaration of `i`:

```C++
void B()
{
    rr::Int i; // <- here
}

void A()
{
    B();
}

int main(int argc, const char* argv[])
{
    A();
}
```

The `DIScope` hierarchy would be:

```C++
                              DIFile: "foo.cpp"
rr::DebugInfo::diScope[0].di: ↳ DISubprogram: "main"
rr::DebugInfo::diScope[1].di: ↳ DISubprogram: "A"
rr::DebugInfo::diScope[2].di: ↳ DISubprogram: "B"
```

The `DILocation` hierarchy would be:

```C++
rr::DebugInfo::diRootLocation:      DILocation(DISubprogram: "ReactorFunction")
rr::DebugInfo::diScope[0].location: ↳ DILocation(DISubprogram: "main")
rr::DebugInfo::diScope[1].location:   ↳ DILocation(DISubprogram: "A")
rr::DebugInfo::diScope[2].location:     ↳ DILocation(DISubprogram: "B")
```

Where '↳' represents an `InlinedAt`.


`rr::DebugInfo::diScope` is updated by `rr::DebugInfo::syncScope()`.

`llvm::DIScope`s typically do not nest - there is usually a separate
`llvm::DISubprogram` for each function in the callstack. All local variables
within a function will typically share the same scope, regardless of whether
they are declared within a sub-block.

Loops and jumps within a function add complexity. Consider:

```C++
void B()
{
    rr::Int i = 0;
}

void A()
{
    for (int i = 0; i < 3; i++)
    {
        rr::Int x = 0;
    }
    B();
}

int main(int argc, const char* argv[])
{
    A();
}
```

In this particular example Reactor will not be aware of the `for` loop, and will
attempt to create three variables called "`x`" in the same function scope for `A()`.
Duplicate symbols in the same `llvm::DIScope` result in undefined behavior.

To solve this, `rr::DebugInfo::syncScope()` observes when a function jumps
backwards, and forks the current `llvm::DILexicalBlock` for the function. This
results in a number of `llvm::DILexicalBlock` chains, each declaring variables
that shadow the previous block.

At the declaration of `i`, the `DIScope` hierarchy would be:

```C++
                              DIFile: "foo.cpp"
rr::DebugInfo::diScope[0].di: ↳ DISubprogram: "main"
                              ↳ DISubprogram: "A"
                              | ↳ DILexicalBlock: "A".1
rr::DebugInfo::diScope[1].di: |   ↳ DILexicalBlock: "A".2
rr::DebugInfo::diScope[2].di: ↳ DISubprogram: "B"
```

The `DILocation` hierarchy would be:

```C++
rr::DebugInfo::diRootLocation:      DILocation(DISubprogram: "ReactorFunction")
rr::DebugInfo::diScope[0].location: ↳ DILocation(DISubprogram: "main")
rr::DebugInfo::diScope[1].location:   ↳ DILocation(DILexicalBlock: "A".2)
rr::DebugInfo::diScope[2].location:     ↳ DILocation(DISubprogram: "B")
```

### Debugger integration

Once the debug information has been generated, it needs to be handed to the
debugger.

Reactor uses [`llvm::JITEventListener::createGDBRegistrationListener()`](http://llvm.org/doxygen/classllvm_1_1JITEventListener.html#a004abbb5a0d48ac376dfbe3e3c97c306)
to inform GDB of the JIT'd program and its debugging information.
More information [can be found here](https://llvm.org/docs/DebuggingJITedCode.html).

LLDB should be able to support this same mechanism, but at the time of writing
this does not appear to work.

