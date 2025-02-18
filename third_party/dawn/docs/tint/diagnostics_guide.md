# Tint diagnostic style guide

This guide provides a set of best practices when writing code that emits
diagnostic messages in Tint. These diagnostics are messages presented to the
user in case of error or warning.

The goal of this document is to have our diagnostic messages be clear and
understandable to our users, so that problems are easy to fix, and to try and
keep a consistent style.

## Message style

* Start diagnostic messages with a lower-case letter
* Try to keep the message to a single sentence, if possible
* Do not end the message with punctuation (full stop, exclamation mark, etc)

**Don't:**

```
shader.wgsl:7:1 error: Cannot take the address of expression.
```

**Do:**

```
shader.wgsl:7:1 error: cannot take the address of expression
```

**Justification:**

Succinct messages are more important than grammatical correctness. \
This style matches the style found in most other compilers.

## Prefer to use a `Source` location instead of quoting the code in the message

**Don't:**

```
shader.wgsl:5:7 error: cannot multiply 'expr_a * expr_b' with types i32 and f32

var res : f32 = expr_a * expr_b
                ^^^^^^^^^^^^^^^
```

**Do:**

```
shader.wgsl:5:7 error: cannot multiply types i32 and f32

var res : f32 = expr_a * expr_b
                ^^^^^^^^^^^^^^^
```

**Justification:**

The highlighted line provides even more contextual information than the quoted
source, and duplicating this information doesn't provide any more help to the
developer. \
Quoting single word identifiers or keywords from the source is not discouraged.

## Use `note` diagnostics for providing additional links to relevant code

**Don't:**

```
shader.wgsl:5:11 error: type cannot be used in address space 'storage' as it is non-host-shareable

    cond : bool;
           ^^^^
```

**Do:**

```
shader.wgsl:5:11 error: type cannot be used in address space 'storage' as it is non-host-shareable

    cond : bool;
           ^^^^

shader.wgsl:8:4 note: while instantiating variable 'StorageBuffer'

var<storage> sb : StorageBuffer;
             ^^
```

**Justification:**

To properly understand some diagnostics requires looking at more than a single
line. \
Multi-source links can greatly reduce the time it takes to properly
understand a diagnostic message. \
This is especially important for diagnostics raised from complex whole-program
analysis, but can also greatly aid simple diagnostics like symbol collision errors.

## Use simple terminology

**Don't:**

```
shader.wgsl:7:1 error: the originating variable of the left-hand side of an assignment expression must not be declared with read access control.
```

**Do:**

```
shader.wgsl:7:1 error: cannot assign to variable with read access control

x.y = 1;
^^^^^^^

shader.wgsl:2:8 note: read access control declared here

var<storage, read> x : i32;
             ^^^^
```

**Justification:**

Diagnostics will be read by Web developers who may not be native English
speakers and are unlikely to be familiar with WGSL specification terminology.
Too much technical jargon can be intimidating and confusing. \
Diagnostics should give enough information to explain what's wrong, and most
importantly, give enough information so that a fix actionable.

**Caution:** Be careful to not over simplify. Use the specification terminology
if there's potential ambiguity by not including it.
