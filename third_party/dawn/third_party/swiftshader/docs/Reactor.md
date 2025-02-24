Reactor Documentation
=====================

Reactor is an embedded language for C++ to facilitate dynamic code generation and specialization.

Introduction
------------

To generate the code for an expression such as
```C++
float y = 1 - x;
```
using the LLVM compiler framework, one needs to execute
```C++
Value *valueY = BinaryOperator::CreateSub(ConstantInt::get(Type::getInt32Ty(Context), 1), valueX, "y", basicBlock);
```

For large expressions this quickly becomes hard to read, and tedious to write and modify.

With Reactor, it becomes as simple as writing
```C++
Float y = 1 - x;
```
Note the capital letter for the type. This is not the code to perform the calculation. It's the code that when executed will record the calculation to be performed.

This is possible through the use of C++ operator overloading. Reactor also supports control flow constructs and pointer arithmetic with C-like syntax.

Motivation
----------

Just-in-time (JIT) compiled code has the potential to be faster than statically compiled code, through [run-time specialization](http://en.wikipedia.org/wiki/Run-time_algorithm_specialisation). However, this is rarely achieved in practice.

Specialization in general is the use of a more optimal routine that is specific for a certain set of conditions. For example when sorting two numbers it is faster to swap them if they are not yet in order, than to call a generic quicksort function. Specialization can be done statically, by explicitly writing each variant or by using metaprogramming to generate multiple variants at static compile time, or dynamically by examining the parameters at run-time and generating a specialized path.

Because specialization can be done statically, sometimes aided by metaprogramming, the ability of a JIT-compiler to do it at run-time is often disregarded. Specialized benchmarks show no advantage of JIT code over static code. However, having a specialized benchmark does not take into account that a typical real-world application deals with many unpredictable conditions. Systems can have one core or several dozen cores, and many different ISA extensions. This alone can make it impractical to write fully specialized routines manually, and with the help of metaprogramming it results in code bloat. Worse yet, any non-trivial application has a layered architecture in which lower layers (e.g. framework APIs) know very little or nothing about the usage by higher layers. Various parameters also depend on user input. Run-time specialization can have access to the full context in which each routine executes, and although the optimization contribution of specialization for a single parameter is small, the combined speedup can be huge. As an extreme example, interpreters can execute any kind of program in any language, but by specializing for a specific program you get a compiled version of that program. But you don't need a full-blown language to observe a huge difference between interpretation and specialization through compilation. Most applications process some form of list of commands in an interpreted fashion, and even the series of calls into a framework API can be compiled into a more efficient whole at run-time.

While the benefit of run-time specialization should now be apparent, JIT-compiled languages lack many of the practical advantages of static compilation. JIT-compilers are very constrained in how much time they can spend on compiling the bytecode into machine code. This limits their ability to even reach parity with static compilation, let alone attempt to exceed it by performing run-time specialization. Also, even if the compilation time was not as constrained, they can't specialize at every opportunity because it would result in an explosive growth of the amount of generated code. There's a need to be very selective in only specializing the hotspots for often recurring conditions, and to manage a cache of the different variants. Even just selecting the size of the set of variables that form the entire condition to specialize for can get immensely complicated.

Clearly we need a manageable way to benefit from run-time specialization where it would help significantly, while still resorting to static compilation for anything else. A crucial observation is that the developer has expectations about the application's behavior, which is valuable information which can be exploited to choose between static or JIT-compilation. One way to do that is to use an API which JIT-compiles the commands provided by the application developer. An example of this is an advanced DBMS which compiles the query into an optimized sequence of routines, each specialized to the data types involved, the sizes of the CPU caches, etc. Another example is a modern graphics API, which takes shaders (a routine executed per pixel or other element) and a set of parameters which affect their execution, and compiles them into GPU-specific code. However, these examples have a very hard divide between what goes on inside the API and outside. You can't exchange data between the statically compiled outside world and the JIT-compiled routines, unless through the API, and they have very different execution models. In other words they are highly domain specific and not generic ways to exploit run-time specialization in arbitrary code.

This is becoming especially problematic for GPUs, as they are now just as programmable as CPUs but you can still only command them through an API. Attempts to disguise this by using a single language, such as C++AMP and SYCL, still have difficulties expressing how data is exchanged, don't actually provide control over the specialization, they have hidden overhead, and they have unpredictable performance characteristics across devices. Meanwhile CPUs gain ever more cores and wider SIMD vector units, but statically compiled languages don't readily exploit this and can't deal with the many code paths required to extract optimal performance. A different language and framework is required.

Concepts and Syntax
-------------------

### Routine and Function<>

Reactor allows you to create new functions at run-time. Their generation happens in C++, and after materializing them they can be called during the execution of the same C++ program. We call these dynamically generated functions "routines", to discern them from statically compiled functions and methods. Reactor's `Routine` class encapsulates a routine. Deleting a Routine object also frees the memory used to store the routine.

To declare the function signature of a routine, use the `Function<>` template. The template argument is the signature of a function, using Reactor variable types. Here's a complete definition of a routine taking no arguments and returning an integer:

`C++
Function<Int(Void)> function;
{
    Return(1);
}
`

The braces are superfluous. They just make the syntax look more like regular C++, and they offer a new scope for Reactor variables.

The Routine is obtained and materialized by "calling" the `Function<>` object to give it a name:

```C++
auto routine = function("one");
```

Finally, we can obtain the function pointer to the entry point of the routine, and call it:

```C++
int (*callable)() = (int(*)())routine->getEntry();

int result = callable();
assert(result == 1);
```

Note that `Function<>` objects are relatively heavyweight, since they have the entire JIT-compiler behind them, while `Routine` objects are lightweight and merely provide storage and lifetime management of generated routines. So we typically allow the `Function<>` object to be destroyed (by going out of scope), while the `Routine` object is retained until we no longer need to call the routine. Hence the distinction between them and the need for a couple of lines of boilerplate code.

### Arguments and Expressions

Routines can take various arguments. The following example illustrates the syntax for accessing the arguments of a routine which takes two integer arguments and returns their sum:

```C++
Function<Int(Int, Int)> function;
{
    Int x = function.Arg<0>();
    Int y = function.Arg<1>();

    Int sum = x + y;

    Return(sum);
}
```

Reactor supports various types which correspond to C++ types:

| Class name    | C++ equivalent |
| ------------- |----------------|
| Int           | int32_t        |
| UInt          | uint32_t       |
| Short         | int16_t        |
| UShort        | uint16_t       |
| Byte          | uint8_t        |
| SByte         | int8_t         |
| Long          | int64_t        |
| ULong         | uint64_t       |
| Float         | float          |

Note that bytes are unsigned unless prefixed with S, while larger integers are signed unless prefixed with U.

These scalar types support all of the C++ arithmetic operations.

Reactor also supports several vector types. For example `Float4` is a vector of four floats. They support a select number of C++ operators, and several "intrinsic" functions such as `Max()` to compute the element-wise maximum and return a bit mask. Check [Reactor.hpp](../src/Reactor/Reactor.hpp) for all the types, operators and intrinsics.

### Casting and Reinterpreting

Types can be cast using the constructor-style syntax:

```C++
Function<Int(Float)> function;
{
    Float x = function.Arg<0>();

    Int cast = Int(x);

    Return(cast);
}
```

You can reinterpret-cast a variable using `As<>`:

```C++
Function<Int(Float)> function;
{
    Float x = function.Arg<0>();

    Int reinterpret = As<Int>(x);

    Return(reinterpret);
}
```

Note that this is a bitwise cast. Unlike C++'s `reinterpret_cast<>`, it does not allow casting between different sized types. Think of it as storing the value in memory and then loading from that same address into the casted type.

An important exception is that 16-, 8-, and 4-byte vectors can be cast to other vectors of one of these sizes. Casting to a longer vector leaves the upper contents undefined.

### Pointers

Pointers also use a template class:

```C++
Function<Int(Pointer<Int>)> function;
{
    Pointer<Int> x = function.Arg<0>();

    Int dereference = *x;

    Return(dereference);
}
```

Pointer arithmetic is only supported on `Pointer<Byte>`, and can be used to access structure fields:

```C++
struct S
{
    int x;
    int y;
};

Function<Int(Pointer<Byte>)> function;
{
    Pointer<Byte> s = function.Arg<0>();

    Int y = *Pointer<Int>(s + offsetof(S, y));

    Return(y);
}
```

Reactor also defines an `OFFSET()` macro, which is a generalization of the `offsetof()` macro defined in `<cstddef>`. It allows e.g. getting the offset of array elements, even when indexed dynamically.

### Conditionals

To generate for example the [unit step](https://en.wikipedia.org/wiki/Heaviside_step_function) function:

```C++
Function<Float(Float)> function;
{
    Pointer<Float> x = function.Arg<0>();

    If(x > 0.0f)
    {
        Return(1.0f);
    }
    Else If(x < 0.0f)
    {
        Return(0.0f);
    }
    Else
    {
        Return(0.5f);
    }
}
```

There's also an IfThenElse() intrinsic function which corresponds with the C++ ?: operator.

### Loops

Loops also have a syntax similar to C++:

```C++
Function<Int(Pointer<Int>, Int)> function;
{
    Pointer<Int> p = function.Arg<0>();
    Int n = function.Arg<1>();
    Int total = 0;

    For(Int i = 0, i < n, i++)
    {
        total += p[i];
    }

    Return(total);
}
```

Note the use of commas instead of semicolons to separate the loop expressions.

`While(expr) {}` also works as expected, but there is no `Do {} While(expr)` equivalent because we can't discern between them. Instead, there's a `Do {} Until(expr)` where you can use the inverse expression to exit the loop.

Specialization
--------------

The above examples don't illustrate anything that can't be written as regular C++ function. The real power of Reactor is to generate routines that are specialized for a certain set of conditions, or "state".

```C++
Function<Int(Pointer<Int>, Int)> function;
{
    Pointer<Int> p = function.Arg<0>();
    Int n = function.Arg<1>();
    Int total = 0;

    For(Int i = 0, i < n, i++)
    {
        if(state.operation == ADD)
        {
            total += p[i];
        }
        else if(state.operation == SUBTRACT)
        {
            total -= p[i];
        }
        else if(state.operation == AND)
        {
            total &= p[i];
        }
        else if(...)
        {
            ...
        }
    }

    Return(total);
}
```

Note that this example uses regular C++ `if` and `else` constructs. They only determine which code ends up in the generated routine, and don't end up in the generated code themselves. Thus the routine contains a loop with just one arithmetic or logical operation, making it more efficient than if this was written in regular C++.

Of course one could write an equivalent efficient function in regular C++ like this:

```C++
int function(int *p, int n)
{
    int total = 0;

    if(state.operation == ADD)
    {
        for(int i = 0; i < n; i++)
        {
            total += p[i];
        }
    }
    else if(state.operation == SUBTRACT)
    {
        for(int i = 0; i < n; i++)
        {
            total -= p[i];
        }
    }
    else if(state.operation == AND)
    {
        for(int i = 0; i < n; i++)
        {
            total &= p[i];
        }
    }
    else if(...)
    {
        ...
    }

    return total;
}
```

But now there's a lot of repeated code. It could be made more manageable using macros or templates, but that doesn't help reduce the binary size of the statically compiled code. That's fine when there are only a handful of state conditions to specialize for, but when you have multiple state variables with many possible values each, the total number of combinations can be prohibitive.

This is especially the case when implementing APIs which offer a broad set of features but developers are likely to only use a select set. The quintessential example is graphics processing, where there are are long pipelines of optional operations and both fixed-function and programmable stages. Applications configure the state of these stages between each draw call.

With Reactor, we can write the code for such pipelines in a syntax that is as easy to read as a naive unoptimized implementation, while at the same time specializing the code for exactly the operations required by the pipeline configuration.
