DXC Cookbook: HLSL Coding Patterns for SPIR-V
=============================================

Author: Steven Perron

Date: Oct 22, 2018

Introduction
============

This document provides a set of examples that demonstrate what will and
will not be accepted by the DXC compiler when generating SPIR-V. The
difficulty in defining what is acceptable is that it cannot be specified
by a grammar. The entire program must be taken into consideration.
Hopefully this will be useful.

We are interested in how global resources are used. For a SPIR-V shader
to be valid, accesses to global resources like structured buffers and
images must be done directly on the global resources. They cannot be
copied or have their address returned from functions. However, in HLSL,
it is possible to copy a global resource or to pass it by reference to a
function. Since this can be arbitrarily complex, DXC can generate valid
SPIR-V only if the compiler is able to remove all of these copies.

The transformations that are used to remove the copies will be the same
for both structured buffers and images, so we have chosen to focus on
structured buffer. The process of transforming the code in this way is
called *legalization.*

Support evolves over time as the optimizations in SPIRV-Tools are
improved. At GDC 2018, Greg Fischer from LunarG
`presented <http://schedule.gdconf.com/session/hlsl-in-vulkan-there-and-back-again-presented-by-khronos-group/856616>`__
earlier results in this space. The DXC, Glslang, and SPIRV-Tools
maintainers work together to handle new HLSL code patterns. This
document represents the state of the DXC compiler in October 2018.

Glslang does legalization as well. However, what it is able to legalize
is different from DXC because of features it chooses to support, and the
optimizations from SPIRV-Tools it choose to run. For example, Glslang
does not support structured buffer aliasing yet, so many of these
examples will not work with Glslang.

All of the examples are available in the DXC repository, at
https://github.com/Microsoft/DirectXShaderCompiler/tree/main/tools/clang/test/CodeGenSPIRV/legal-examples
. To open a link to Tim Jones' Shader Playground for an example, you can
follow the url in the comments of each example.

Examples for structured buffers
===============================

Desired code
------------

.. code-block:: hlsl

    // 0-copy-sbuf-ok.hlsl
    // http://shader-playground.timjones.io/e6af2bdce0c61ed07d3a826aa8a95d45

    struct S {
      float4 f;
    };

    int i;

    StructuredBuffer<S> gSBuffer;
    RWStructuredBuffer<S> gRWSBuffer;

    void main() {
      gRWSBuffer[i] = gSBuffer[i];
    }

This example shows code that directly translates to valid SPIR-V. In
this case, we have two structured buffers. When one of their elements is
accessed, it is done by naming the resource from which to get the
element.

Note that it is fine to copy an element of the structured buffer.

Single copy to a local
----------------------

Cases that can be easily legalized are those where there is exactly one
assignment to the local copy of the structured buffer. In this context,
a local is either a global static or a function scope symbol. Something
that can be accessed by only a single instance of the shader. When you
have a single copy to a local, it is obvious which global is actually be
used. This allows the compiler to replace a reference to the local
symbol with the global resource.

Initialization of a static
~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: hlsl

    // 1-copy-global-static-ok.hlsl
    // http://shader-playground.timjones.io/815543dc91a4e6855a8d0c6a345d4a5a

    struct S {
      float4 f;
    };

    int i;

    StructuredBuffer<S> gSBuffer;
    RWStructuredBuffer<S> gRWSBuffer;

    static StructuredBuffer<S> sSBuffer = gSBuffer;

    void main() {
      gRWSBuffer[i] = sSBuffer[i];
    }

This example shows an implicitly addressed structured buffer
``gSBuffer`` assigned to a static ``sSBuffer``. This copy is treated
like a shallow copy. This is implemented by making ``sSBuffer`` a
pointer to ``gSBuffer``.

This example can be legalized because the compiler is able to see that
``sSbuffer`` is points to ``gSBuffer``, which does not move, so uses of
``sSbuffer`` can be replaced by ``gSBuffer``.

.. code-block:: hlsl

    // 2-write-global-static-ok.hlsl
    // http://shader-playground.timjones.io/1c65c467e395383945d219a60edbe10c

    struct S {
      float4 f;
    };

    int i;

    RWStructuredBuffer<S> gRWSBuffer;

    static RWStructuredBuffer<S> sRWSBuffer = gRWSBuffer;

    void main() {
      sRWSBuffer[i].f = 0.0;
    }

This example is similar to the previous example, except in this case the
shallow copy becomes important. ``sRWSBuffer`` is treated like a pointer
to ``gRWSBuffer``. As before, the references to ``sRWSBuffer`` can be
replaced by ``gRWSBuffer``. This means that the write that occurs will
be visible outside of the shader.

Copy to function scope
~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: hlsl

    // 3-copy-local-struct-ok.hlsl
    // http://shader-playground.timjones.io/77dd20774e4943044c2f1b630c539f07

    struct S {
      float4 f;
    };

    struct CombinedBuffers {
      StructuredBuffer<S> SBuffer;
      RWStructuredBuffer<S> RWSBuffer;
    };


    int i;

    StructuredBuffer<S> gSBuffer;
    RWStructuredBuffer<S> gRWSBuffer;

    void main() {
      CombinedBuffers cb;
      cb.SBuffer = gSBuffer;
      cb.RWSBuffer = gRWSBuffer;
      cb.RWSBuffer[i] = cb.SBuffer[i];
    }

It is also possible to copy a structured buffer to a function scope
symbol. This is similar to a copy to a static scope symbol. The local
copy is really a pointer to the original. This example demonstrates that
DXC can legalize the copy even if it is a copy to part of a structure.
There are no specific restrictions on the structure. The structured
buffers can be anywhere in the structure, and there can be any number of
members. Structured buffers can be in nested structures of any depth.
The following is a move complicated example.

.. code-block:: hlsl

    // 4-copy-local-nested-struct-ok.hlsl
    // http://shader-playground.timjones.io/14f59ff2a28c0a0180daf6ce4393cf6b

    struct S {
      float4 f;
    };

    struct CombinedBuffers {
      StructuredBuffer<S> SBuffer;
      RWStructuredBuffer<S> RWSBuffer;
    };

    struct S2 {
      CombinedBuffers cb;
    };

    struct S1 {
      S2 s2;
    };

    int i;

    StructuredBuffer<S> gSBuffer;
    RWStructuredBuffer<S> gRWSBuffer;

    void main() {
      S1 s1;
      s1.s2.cb.SBuffer = gSBuffer;
      s1.s2.cb.RWSBuffer = gRWSBuffer;
      s1.s2.cb.RWSBuffer[i] = s1.s2.cb.SBuffer[i];
    }

Function parameters
~~~~~~~~~~~~~~~~~~~

.. code-block:: hlsl

    // 5-func-param-sbuf-ok.hlsl
    // http://shader-playground.timjones.io/aeb06f527c5390d82d63bdb4eafc9ae7

    struct S {
      float4 f;
    };

    struct CombinedBuffers {
      StructuredBuffer<S> SBuffer;
      RWStructuredBuffer<S> RWSBuffer;
    };


    int i;

    StructuredBuffer<S> gSBuffer;
    RWStructuredBuffer<S> gRWSBuffer;

    void foo(StructuredBuffer<S> pSBuffer) {
      gRWSBuffer[i] = pSBuffer[i];
    }

    void main() {
      foo(gSBuffer);
    }

It is possible to pass a structured buffer as a parameter to a function.
As with the copies in the previous section, it is a pointer to the
structured buffer that is actually being passed to ``foo``. This is the
same way that arrays work in C/C++.

.. code-block:: hlsl

    // 6-func-param-rwsbuf-ok.hlsl
    // http://shader-playground.timjones.io/f4e0194ce78118c0a709d85080ccea93

    struct S {
      float4 f;
    };

    int i;

    StructuredBuffer<S> gSBuffer;
    RWStructuredBuffer<S> gRWSBuffer;

    void foo(RWStructuredBuffer<S> pRWSBuffer) {
      pRWSBuffer[i] = gSBuffer[i];
    }

    void main() {
      foo(gRWSBuffer);
    }

The same is true for RW structured buffers. So in this case, the write
to ``pRWSBuffer`` is changing ``gRWSBuffer``. This means that the write
to ``pRWSBuffer`` will be visible outside of the function, and outside
of the shader.

Return values
~~~~~~~~~~~~~

The next two examples show that structured buffers can be a function's
return value. As before, the return value of ``foo`` is really a pointer
to the global resource.

.. code-block:: hlsl

    // 7-func-ret-tmp-var-ok.hlsl
    // http://shader-playground.timjones.io/d6b706423f02dad58fbb01841282c6a1

    struct S {
      float4 f;
    };

    int i;

    StructuredBuffer<S> gSBuffer;
    RWStructuredBuffer<S> gRWSBuffer;

    RWStructuredBuffer<S> foo() {
      return gRWSBuffer;
    }

    void main() {
      RWStructuredBuffer<S> lRWSBuffer = foo();
      lRWSBuffer[i] = gSBuffer[i];
    }

| In this case, the compiler will replace ``lRWSBuffer`` by
  ``gRWSBuffer``.

.. code-block:: hlsl

    // 8-func-ret-direct-ok.hlsl
    // http://shader-playground.timjones.io/6edbbc1aa6c6b6533c5a728135f87fb9

    struct S {
      float4 f;
    };

    int i;

    StructuredBuffer<S> gSBuffer;
    RWStructuredBuffer<S> gRWSBuffer;

    StructuredBuffer<S> foo() {
      return gSBuffer;
    }

    void main() {
      gRWSBuffer[i] = foo()[i];
    }

This example is similar to the previous, but shows that you do not have
to use an explicit temporary value.

Conditional control flow
------------------------

The examples so far have do not have any conditional control flow. This
makes it obvious which resources are being used. The introduction of
conditional control flow makes the job of the compiler much harder, and
in some cases impossible. Remember that the compiler is trying to
determine at compile time which resource will be used at run time. In
this section, we will look at how control flow affects the compiler's
ability to do this. The bottom line is that the compiler has to be able
to turn all of the conditional control flow that affects which resources
are used into straight line code.

Inputs in if-statement
~~~~~~~~~~~~~~~~~~~~~~

The first example is one where the compiler cannot determine which
resource is actually being accessed.

.. code-block:: hlsl

    // 9-if-stmt-select-fail.hlsl
    // http://shader-playground.timjones.io/2896e95627fd8a6689ca96c81a5c7c68

    struct S {
      float4 f;
    };

    struct CombinedBuffers {
      StructuredBuffer<S> SBuffer;
      RWStructuredBuffer<S> RWSBuffer;
    };


    int i;

    StructuredBuffer<S> gSBuffer1;
    StructuredBuffer<S> gSBuffer2;
    RWStructuredBuffer<S> gRWSBuffer;

    #define constant 0

    void main() {

      StructuredBuffer<S> lSBuffer;
      if (constant > i) {          // Condition can't be computed at compile time.
        lSBuffer = gSBuffer1;      // Will produce invalid SPIR-V for Vulkan.
      } else {
        lSBuffer = gSBuffer2;
      }
      gRWSBuffer[i] = lSBuffer[i];
    }

In this example, ``lsBuffer`` could be either ``gSBuffer1`` or
``gSBuffer2``. It depends on the value of ``i`` which is a parameter to
the shader and cannot be known at compile time. At this time, the
compiler is not able to convert this code into something that drivers
will accept.

If this is the pattern that your code, I would suggest rewriting the
code into the following:

.. code-block:: hlsl

    // 10-if-stmt-select-ok.hlsl
    // http://shader-playground.timjones.io/5063d8a0a7ad1f9d0839cd34a6d94dd2

    struct S {
      float4 f;
    };

    struct CombinedBuffers {
      StructuredBuffer<S> SBuffer;
      RWStructuredBuffer<S> RWSBuffer;
    };


    int i;

    StructuredBuffer<S> gSBuffer1;
    StructuredBuffer<S> gSBuffer2;
    RWStructuredBuffer<S> gRWSBuffer;

    #define constant 0

    void main() {

      StructuredBuffer<S> lSBuffer;
      if (constant > i) {
        lSBuffer = gSBuffer1;
        gRWSBuffer[i] = lSBuffer[i];
      } else {
        lSBuffer = gSBuffer2;
        gRWSBuffer[i] = lSBuffer[i];
      }
    }

Notice that this involves replicating code. If the code that follows the
if-statement is long, you could consider moving it to a function, and
having two calls to that function.

If-statements with constants
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Not all control flow is a problem. There are situations where the
compiler is able to determine that a condition is always true or always
false. For example, in the following code, the compiler looks at "0>2",
and knows that is always false.

.. code-block:: hlsl

    // 11-if-stmt-const-ok.hlsl
    // http://shader-playground.timjones.io/7ef5b89b3ec3d56c22e1bca45b40516a

    struct S {
      float4 f;
    };

    int i;

    StructuredBuffer<S> gSBuffer1;
    StructuredBuffer<S> gSBuffer2;
    RWStructuredBuffer<S> gRWSBuffer;

    #define constant 0

    void main() {

      StructuredBuffer<S> lSBuffer;
      if (constant > 2) {
        lSBuffer = gSBuffer1;
      } else {
        lSBuffer = gSBuffer2;
      }
      gRWSBuffer[i] = lSBuffer[i];
    }

The compiler will turn this code into

.. code-block:: hlsl

    struct S {
      float4 f;
    };

    int i;

    StructuredBuffer<S> gSBuffer1;
    StructuredBuffer<S> gSBuffer2;
    RWStructuredBuffer<S> gRWSBuffer;

    #define constant 0

    void main() {
      gRWSBuffer[i] = gSBuffer2[i];
    }

The two previous examples show that handling control flow depends on
what the compiler can do. This depends on the amount of optimization
that is done, and which optimizations are done. In general, when you are
writing code that will select a resource, keep the conditions as simple
as possible to make it as easy as possible for the compiler to determine
which path is taken.

Switch statements
~~~~~~~~~~~~~~~~~

Switch statements are similar to if-statements. If the selector is a
constant, then the compiler will be able to propagate the copies.

.. code-block:: hlsl

    // 12-switch-stmt-select-fail.hlsl
    // http://shader-playground.timjones.io/b079f878daeba5d77842725b90a476ca

    struct S {
      float4 f;
    };

    struct CombinedBuffers {
      StructuredBuffer<S> SBuffer;
      RWStructuredBuffer<S> RWSBuffer;
    };


    int i;

    StructuredBuffer<S> gSBuffer1;
    StructuredBuffer<S> gSBuffer2;
    RWStructuredBuffer<S> gRWSBuffer;

    #define constant 0

    void main() {

      StructuredBuffer<S> lSBuffer;
      switch(i) {                   // Compiler can't determine which case will run.
        case 0:
          lSBuffer = gSBuffer1;     // Will produce invalid SPIR-V for Vulkan.
          break;
        default:
          lSBuffer = gSBuffer2;
          break;
      }
      gRWSBuffer[i] = lSBuffer[i];
    }

The compiler is not able to remove the copies in this example because it
does not know the value of ``i`` at compile time.

.. code-block:: hlsl

    // 13-switch-stmt-const-ok.hlsl
    // http://shader-playground.timjones.io/a46dd1f1a84eba38c047439741ec08ab

    struct S {
      float4 f;
    };

    struct CombinedBuffers {
      StructuredBuffer<S> SBuffer;
      RWStructuredBuffer<S> RWSBuffer;
    };


    int i;

    StructuredBuffer<S> gSBuffer1;
    StructuredBuffer<S> gSBuffer2;
    RWStructuredBuffer<S> gRWSBuffer;

    const static int constant = 0;

    void main() {

      StructuredBuffer<S> lSBuffer;
      switch(constant) {
        case 0:
          lSBuffer = gSBuffer1;
          break;
        default:
          lSBuffer = gSBuffer2;
          break;
      }
      gRWSBuffer[i] = lSBuffer[i];
    }

However, if the selector is turned into a constant, the compiler can
replace uses of ``lSBuffer`` by ``gSBuffer1``.

Loop Induction Variables in conditions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Besides inputs, another type of variable that hinders the compiler are
loop induction variables. These are variables that change value for each
iteration of the loop. Consider this example.

.. code-block:: hlsl

    // 14-loop-var-fail.hlsl
    // http://shader-playground.timjones.io/8df364770e3f425e6321e71f817bcd1a

    struct S {
      float4 f;
    };

    struct CombinedBuffers {
      StructuredBuffer<S> SBuffer;
      RWStructuredBuffer<S> RWSBuffer;
    };

    StructuredBuffer<S> gSBuffer1;
    StructuredBuffer<S> gSBuffer2;
    RWStructuredBuffer<S> gRWSBuffer;

    #define constant 0

    void main() {

      StructuredBuffer<S> lSBuffer;

      for( int j = 0; j < 2; j++ ) {
        if (constant > j) {         // Condition is different for different iterations
          lSBuffer = gSBuffer1;     // Will produces invalid SPIR-V for Vulkan.
        } else {
          lSBuffer = gSBuffer2;
        }
        gRWSBuffer[j] = lSBuffer[j];
      }
    }

In this example, ``j`` is an induction variable. It takes on the values
``0`` and ``1``. The information is there to be able to determine which
path is taken in each iteration, but the compiler does not figure this
out by default.

If you want the compiler to be able to legalize this code, then you will
have to direct the compiler to unroll this loop using the unroll
attribute. The following example can be legalized by the compiler:

.. code-block:: hlsl

    // 15-loop-var-unroll-ok.hlsl
    // http://shader-playground.timjones.io/3d0f6f830fc4a5102714e19c748e81c7

    struct S {
      float4 f;
    };

    struct CombinedBuffers {
      StructuredBuffer<S> SBuffer;
      RWStructuredBuffer<S> RWSBuffer;
    };

    StructuredBuffer<S> gSBuffer1;
    StructuredBuffer<S> gSBuffer2;
    RWStructuredBuffer<S> gRWSBuffer;

    #define constant 0

    void main() {

      StructuredBuffer<S> lSBuffer;

      [unroll]
      for( int j = 0; j < 2; j++ ) {
        if (constant > j) {
          lSBuffer = gSBuffer1;
        } else {
          lSBuffer = gSBuffer2;
        }
        gRWSBuffer[j] = lSBuffer[j];
      }
    }

Variable iteration counts
~~~~~~~~~~~~~~~~~~~~~~~~~

Adding the unroll attribute to loops does not guarantee that the
compiler is able to legalize the code. The compiler has to be able to
fully unroll the loop. That means the compiler will have to create a
copy of the body of the loop for each iteration so that there is no loop
anymore. That can only be done if the number of iterations can be known
at compile time.

This means that the compiler must be able to determine the initial
value, the final value, and the step for the induction variable, ``j``
in the example. None of ``foo1``, ``foo2``, or ``foo3`` can be legalized
because the number of iterations cannot be known at compile time.

.. code-block:: hlsl

    // 16-loop-var-range-fail.hlsl
    // http://shader-playground.timjones.io/376f5f985c3ceceea004ab58edb336f2

    struct S {
      float4 f;
    };

    struct CombinedBuffers {
      StructuredBuffer<S> SBuffer;
      RWStructuredBuffer<S> RWSBuffer;
    };

    StructuredBuffer<S> gSBuffer1;
    StructuredBuffer<S> gSBuffer2;
    RWStructuredBuffer<S> gRWSBuffer;

    int i;

    #define constant 0

    void foo1() {
      StructuredBuffer<S> lSBuffer;

      [unroll]
      for( int j = i; j < 2; j++ ) {  // Compiler can't determine the initial value
        if (constant > j) {
          lSBuffer = gSBuffer1;
        } else {
          lSBuffer = gSBuffer2;
        }
        gRWSBuffer[j] = lSBuffer[j];
      }
    }

    void foo2() {
      StructuredBuffer<S> lSBuffer;

      [unroll]
      for( int j = 0; j < i; j++ ) {  // Compiler can't determine the end value
        if (constant > j) {
          lSBuffer = gSBuffer1;
        } else {
          lSBuffer = gSBuffer2;
        }
        gRWSBuffer[j] = lSBuffer[j];
      }
    }

    void foo3() {
      StructuredBuffer<S> lSBuffer;

      [unroll]
      for( int j = 0; j < 2; j += i ) { // Compiler can't determine the step count
        if (constant > j) {
          lSBuffer = gSBuffer1;
        } else {
          lSBuffer = gSBuffer2;
        }
        gRWSBuffer[j] = lSBuffer[j];
      }
    }


    void main() {
      foo1(); foo2(); foo3();
    }

As before the compiler will try to simplify expressions to determine
their value at compile time, but it may not always be successful. We
would recommend that you keep the expressions for the loop bounds as
simple as possible to increase the chances the compiler can figure it
out.

Other restrictions on unrolling
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Not being able to determine the iteration count at compile time is a
fundamental problem. No matter how good the compiler is, it will never
be able to fully unroll the loop. However, due to the internal details
(algorithms in the SPIRV-Tools optimizer), other cases cannot be
handled. The most notable one is that the induction variable must be an
integral type.

.. code-block:: hlsl

    // 17-loop-var-float-fail.hlsl
    // http://shader-playground.timjones.io/d5d2598699378688684a4a074553dddf

    struct S {
      float4 f;
    };

    struct CombinedBuffers {
      StructuredBuffer<S> SBuffer;
      RWStructuredBuffer<S> RWSBuffer;
    };

    StructuredBuffer<S> gSBuffer1;
    StructuredBuffer<S> gSBuffer2;
    RWStructuredBuffer<S> gRWSBuffer;

    #define constant 0

    void main() {

      StructuredBuffer<S> lSBuffer;

      [unroll]
      for( float j = 0; j < 2; j++ ) {  // Can't infer floating point induction values
        if (constant > j) {
          lSBuffer = gSBuffer1;
        } else {
          lSBuffer = gSBuffer2;
        }
        gRWSBuffer[j] = lSBuffer[j];
      }
    }

This example cannot be legalized because ``j`` is a ``float``.

Other interesting cases
-----------------------

Multiple calls to a function
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: hlsl

    // 18-multi-func-call-ok.hlsl
    // http://shader-playground.timjones.io/e7b3ac1262a291c92902fd3f1fd3343c

    struct S {
      float4 f;
    };

    int i;

    StructuredBuffer<S> gSBuffer;
    RWStructuredBuffer<S> gRWSBuffer1;
    RWStructuredBuffer<S> gRWSBuffer2;


    void foo(RWStructuredBuffer<S> pRWSBuffer) {
      pRWSBuffer[i] = gSBuffer[i];
    }

    void main() {
      foo(gRWSBuffer1);
      foo(gRWSBuffer2);
    }

In this example, we see the same function is called twice. Each call has
a different parameter. This can look like a problem because
``pRWSBuffer`` could be either ``gRWSBuffer1`` or ``gRWSBuffer2``.
However, the compiler is able to work around this by creating a separate
copy of ``foo`` for each call site. In fact, these copies will be placed
inline.

Multiple returns
~~~~~~~~~~~~~~~~

As we have already seen, a return from a function is a copy. At this
point, it would be fair to ask what happens if there are multiple
returns.

.. code-block:: hlsl

    // 19-multi-func-ret-fail.hlsl
    // http://shader-playground.timjones.io/922facb688a5ba09b153d64cf1fc4557

    struct S {
      float4 f;
    };

    int i;

    StructuredBuffer<S> gSBuffer;
    RWStructuredBuffer<S> gRWSBuffer1;
    RWStructuredBuffer<S> gRWSBuffer2;

    RWStructuredBuffer<S> foo(int l) {
      if (l == 0) {       // Compiler does not know which branch will be taken:
                          // Branch taken depends on input i.
        return gRWSBuffer1;
      } else {
        return gRWSBuffer2;
      }
    }

    void main() {
      RWStructuredBuffer<S> lRWSBuffer = foo(i);
      lRWSBuffer[i] = gSBuffer[i];
    }

The compiler is not able to legalize this example because it does not
know which value will be returned. However, if the compiler is able to
determine which path will be taken, then it can be legalized.

.. code-block:: hlsl

    // 20-multi-func-ret-const-ok.hlsl
    // http://shader-playground.timjones.io/84b093c7cf9e3932c5f0d9691533bafe

    struct S {
      float4 f;
    };

    int i;

    StructuredBuffer<S> gSBuffer1;
    StructuredBuffer<S> gSBuffer2;
    RWStructuredBuffer<S> gRWSBuffer1;
    RWStructuredBuffer<S> gRWSBuffer2;

    StructuredBuffer<S> foo(int l) {
      if (l == 0) {
        return gSBuffer1;
      } else {
        return gSBuffer2;
      }
    }

    void main() {
      gRWSBuffer1[i] = foo(0)[i];
      gRWSBuffer2[i] = foo(1)[i];
    }

For each call to ``foo``, the compiler is able to determine which value
will be returned. In this case, the code can be legalized.

Combining elements
~~~~~~~~~~~~~~~~~~

Individually, these examples are simple; however, these elements can be
combined in arbitrary ways. As one last example, consider this HLSL
source code.

.. code-block:: hlsl

    // 21-combined-ok.hlsl
    // http://shader-playground.timjones.io/9f00d2d359da0731cdf8d0b68520e2c4

    struct S {
      float4 f;
    };

    int i;

    StructuredBuffer<S> gSBuffer1;
    StructuredBuffer<S> gSBuffer2;
    RWStructuredBuffer<S> gRWSBuffer1;
    RWStructuredBuffer<S> gRWSBuffer2;

    #define constant 0

    StructuredBuffer<S> bar() {
      if (constant > 2) {
        return gSBuffer1;
      } else {
        return gSBuffer2;
      }
    }

    void foo(RWStructuredBuffer<S> pRWSBuffer) {
      StructuredBuffer<S> lSBuffer = bar();
      pRWSBuffer[i] = lSBuffer[i];
    }

    void main() {
      foo(gRWSBuffer1);
      foo(gRWSBuffer2);
    }

The compiler will do all of the transformations that mentioned earlier
to identify a single resource for each load and store from a resource.

Conclusion
==========

It is impossible to enumerate all of the possible code sequences that
work or do not work, but hopefully this will give a guide as to what is
possible or not. The general rule of thumb is that there must be a
straightforward way to transform the code so that there are no copies of
global resources.
