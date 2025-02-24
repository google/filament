# Floating Point Primer

This document is meant to be a primer of the concepts related to floating point
numbers that are needed to be understood when working on tests in WebGPU's CTS.

WebGPU's CTS is responsible for testing if an implementation of WebGPU
satisfies the spec, and thus meets the expectations of programmers based on the
contract defined by the spec.

Floating point math makes up a significant portion of the WGSL spec, and has
many subtle corner cases to get correct.

Additionally, floating point math, unlike integer math, is broadly not exact, so
how inaccurate a calculation is allowed to be is required to be stated in the
spec and tested in the CTS, as opposed to testing for a singular correct
response.

Thus, the WebGPU CTS has a significant amount of machinery around how to
correctly test floating point expectations in a fluent manner.

## Floating Point Numbers

For some of the following discussion of floating point numbers 32-bit
floating numbers are assumed, also known as single precision IEEE floating
point numbers or `f32`s. Most of the discussions that apply to this format apply
to other concrete formats that are handled, i.e. 16-bit/f16/half-precision.
There are some significant differences with respect to AbstractFloats, which
will be discussed in its own section.

Details of how these formats work are discussed as needed below, but for a more
involved discussion, please see the references in the Resources sections.

Additionally, in the Appendix there is a table of interesting/common values that
are often referenced in tests or this document.

A floating point number system defines
- A finite set of values to stand as representatives for the infinite set of
  real numbers, and
- Arithmetic operations on those representatives, trying to approximate the
  ideal operations on real numbers.

The cardinality mismatch alone implies that any floating point number system
necessarily loses information.

This means that not all numbers in the bounds can be exactly represented as a
floating point value.

For example, the integer `1` is exactly represented as a f32 as `0x3f800000`,
but the next nearest number `0x3f800001` is `1.00000011920928955`.

So any number between `1` and `1.00000011920928955` is not exactly representable
as a f32 and instead is approximated as either `1` or `1.00000011920928955`.

When a number X is not exactly representable by a floating point value, there
are normally two neighbouring numbers that could reasonably represent X: the
nearest floating point value above X, and the nearest floating point value below
X. Which of these values gets used is dictated by the rounding mode being used,
which may be something like always round towards 0 or go to the nearest
neighbour, or something else entirely.

The process of converting numbers between different precisions is called
quantization. WGSL does not prescribe a specific rounding mode when
quantizing, so either of the neighbouring values is considered valid when
converting a non-exactly representable value to a floating point value. This has
significant implications on the CTS that are discussed later.

From here on, we assume you are familiar with the internal structure of a
floating point number (a sign bit, a biased exponent, and a mantissa). For
reference, see
[binary64 on Wikipedia](https://en.wikipedia.org/wiki/Double-precision_floating-point_format),
[binary32 on Wikipedia](https://en.wikipedia.org/wiki/Single-precision_floating-point_format),
and
[binary16 on Wikipedia](https://en.wikipedia.org/wiki/Half-precision_floating-point_format).

In the floating points formats described above, there are two possible zero
values, one with all bits being 0, called positive zero, and one all the same
except with the sign bit being 1, called negative zero.

For WGSL, and thus the CTS's purposes, these values are considered equivalent.
Typescript, which the CTS is written in, treats all zeros as positive zeros,
unless you explicitly escape hatch to differentiate between them, so most of the
time there being two zeros doesn't materially affect code.

### Normal Numbers

Normal numbers are floating point numbers whose biased exponent is not all 0s or
all 1s. When working with normal numbers the mantissa starts with an implied
leading 1. For WGSL these numbers behave as you expect for floating point values
with no interesting caveats.

### Subnormal Numbers

Subnormal numbers are finite non-zero numbers whose biased exponent is all 0s,
sometimes called denorms.

These are the closest numbers to zero, both positive and negative, and fill in
the gap between the normal numbers with the smallest magnitude, and 0.

Some devices, for performance reasons, do not handle operations on the
subnormal numbers, and instead treat them as being zero, this is called *flush
to zero* or FTZ behaviour.

This means in the CTS that when a subnormal number is consumed or produced by an
operation, an implementation may choose to replace it with zero.

Like the rounding mode for quantization, this adds significant complexity to the
CTS, which will be discussed later.

### Inf & NaNs

Floating point numbers include positive and negative infinity to represent
values that are out of the bounds supported by the current precision.

Implementations may assume that infinities are not present. When an evaluation
at runtime would produce an infinity, an indeterminate value is produced
instead.

When a value goes out-of-bounds for a specific precision there are special
rounding rules that apply. If it is 'near' the edge of finite values for that
precision, it is considered to be near-overflowing, and the implementation may
choose to round it to the edge value or the appropriate infinity. If it is not
near the finite values, which it is considered to be far-overflowing, then it
must be rounded to the appropriate infinity.

This of course is vague, but the spec does have a precise definition where the
transition from near to far overflow is.

Let `x` be our value.

Let `exp_max` be the (unbiased) exponent of the largest finite value for the
floating point type.

If `|x|` < `2 ** (exp_max + 1)`, but not in
the finite range, than it is considered to be near-overflowing for the
floating point type.

If the magnitude is equal to or greater than this limit, then it is
far-overflowing for the floating point type.

This concept of near-overflow vs far-overflow divides the real number line into
5 distinct regions.

| Region                                        | Rule                            |
|-----------------------------------------------|---------------------------------|
| -∞ < `x` <= `-(2 ** (exp_max + 1))`           | must round to -∞                |
| `-(2 ** (exp_max + 1))` < `x` <= min fp value | must round to -∞ or min value   |
| min fp value < `x` < max fp value             | round as discussed below        |
| max fp value <= `x` < `2 ** (exp_max + 1)`    | must round to max value or ∞    |
| `2 ** (exp_max + 1))` < `x`                   | implementations must round to ∞ |


The CTS encodes the least restrictive interpretation of the rules in the spec,
i.e. assuming someone has made a slightly adversarial implementation that always
chooses the thing with the least accuracy.

This means that the above rules about infinities and overflow combine to say
that any time a non-finite value for the specific floating point type is seen,
any finite value is acceptable afterward. This is because the non-finite value
may be converted to an infinity and then an indeterminate value can be used
instead of the infinity.

(This comes with the caveat that this is only for runtime execution on a GPU,
the rules for compile time execution will be discussed below.)

Signaling NaNs are treated as quiet NaNs in the WGSL spec. And quiet NaNs have
the same "may-convert-to-indeterminate-value" behaviour that infinities have, so
for the purpose of the CTS they are handled by the infinite/out-of-bounds logic
normally.

## Notation/Terminology

When discussing floating point values in the CTS, there are a few terms used
with precise meanings, which will be elaborated here.

Additionally, any specific notation used will be specified here to avoid
confusion.

### Operations

The CTS tests for the proper execution of builtins, i.e. `sin`, `sqrt`, `abs`,
etc, and expressions, i.e. `*`, `/`, `<`, etc, when provided with floating
point inputs. These collectively can be referred to as floating point
operations.

Operations, which can be thought of as mathematical functions, are mappings from
a set of inputs to a set of outputs.

Denoted `f(x, y) = X`, where `f` is a placeholder or the name of the operation,
lower case variables are the inputs to the function, and uppercase variables are
the outputs of the function.

Operations have one or more inputs and an output value.

Values are generally defined as floats, integers, booleans, vectors, and
matrices. Consult the [WGSL Spec](https://www.w3.org/TR/WGSL/) for the exact
list of types and their definitions.

Most operations inputs and output are the same type of value. There are some
exceptions that accept or emit heterogeneous data types, normally a floating
point type and a integer type or a boolean.

There are a couple of builtins (`frexp` and `modf`) that return composite
outputs where there are multiple values being returned, there is a single result
value made of structured data. Whereas composite inputs are handle by having
multiple input parameters.

Some examples of different types of operations:

`multiplication(x, y) = X`, which represents the WGSL expression `x * y`, takes
in floating point values, `x` and `y`, and produces a floating point value `X`.

`lessThan(x, y) = X`, which represents the WGSL expression `x < y`, again takes
in floating point values, but in this case returns a boolean value.

`ldexp(x, y) = X`, which builds a floating point value, takes in a floating
point value `x` and a restricted integer `y`.

### Domain, Range, and Intervals

For an operation `f(x) = X`, the interval of valid values for the input, `x`, is
called the *domain*, and the interval for valid results, `X`, is called the
*range*.

An interval, `[a, b]`, is a set of real numbers that contains  `a`, `b`, and all
the real numbers between them.

Open-ended intervals, i.e. ones that don't include `a` and/or `b`, are avoided,
and are called out explicitly when they occur.

The convention in this doc and the CTS code is that `a <= b`, so `a` can be
referred to as the beginning of the interval and `b` as the end of the interval.

When talking about intervals, this doc and the code endeavours to avoid using
the term **range** to refer to the span of values that an interval covers,
instead using the term **endpoints** to avoid confusion of terminology around
output of operations.

The term **endpoints** is generally used to refer to the conceptual numeric
spaces, i.e. f32 or abstract float.

Thus a specific interval can have **endpoints** that are either in or out of
bounds for a specific floating point precision.

## Accuracy

As mentioned above floating point numbers are not able to represent all the
possible values over their range, but instead represent discrete values in that
space, and approximate the remainder.

Additionally, floating point numbers are not evenly distributed over the real
number line, but instead are more densely clustered around zero, with the space
between values increasing in steps as the magnitude increases.

When discussing operations on floating point numbers, there is often reference
to a true value. This is the value that given no performance constraints and
infinite precision you would get, i.e `acos(1) = π`, where π has infinite
digits of precision.

For the CTS it is often sufficient to calculate the true value using TypeScript,
since its native number format is higher precision (double-precision/f64), so
all f64, f32, and f16 values can be represented in it. Where this breaks down
will be discussed in the section on compile time vs runtime execution.

The true value is sometimes representable exactly as a floating point value, but
often is not.

Additionally, many operations are implemented using approximations from
numerical analysis, where there is a tradeoff between the precision of the
result and the cost.

Thus, the spec specifies what the accuracy constraints for specific operations
is, how close to truth an implementation is required to be, to be
considered conforming.

There are 5 different ways that accuracy requirements are defined in the spec:

1. *Exact*

   This is the situation where it is expected that true value for an operation
   is always expected to be exactly representable. This doesn't happen for any
   of the operations that return floating point values, but does occur for
   logical operations that return boolean values.


2. *Correctly Rounded*

   For the case that the true value is exactly representable as a floating
   point, this is the equivalent of exactly from above. In the event that the
   true value is not exact, then the acceptable answer for most numbers is
   either the nearest representable value above or below the true value.

   For values near the subnormal range, e.g. close to zero, this becomes more
   complex, since an implementation may FTZ at any point. So if the exact
   solution is subnormal or either of the neighbours of the true value are
   subnormal, zero becomes a possible result, thus the acceptance interval is
   wider than naively expected.

   On the edge of and beyond the bounds of a floating point type the definition
   of correctly rounded becomes complex, which is discussed in detail in the
   section on overflow.


3. *Absolute Error*

   This type of accuracy specifies an error value, ε, and the calculated result
   is expected to be within that distance from the true value, i.e.
   `[ X - ε, X + ε ]`.

   The main drawback with this manner of specifying accuracy is that it doesn't
   scale with the level of precision in floating point numbers themselves at a
   specific value. Thus, it tends to be only used for specifying accuracy over
   specific limited intervals, i.e. [-π, π].


4. *Units of Least Precision (ULP)*

   The solution to the issue of not scaling with precision of floating point is
   to use units of least precision.

   ULP(X) is min (b-a) over all pairs (a,b) of representable floating point
   numbers such that (a <= X <= b and a =/= b). For a more formal discussion of
   ULP see
   [On the definition of ulp(x)](https://hal.inria.fr/inria-00070503/document).

   n * ULP or nULP means `[X - n * ULP @ X, X + n * ULP @ X]`.


5. *Inherited*

   When an operation's accuracy is defined in terms of other operations, then
   its accuracy is said to be inherited. Handling of inherited accuracies is
   one of the main driving factors in the design of testing framework, so will
   need to be discussed in detail.

## Acceptance Intervals

The first four accuracy types; Exact, Correctly Rounded, Absolute Error, and
ULP, sometimes called simple accuracies, can be defined in isolation from each
other, and by association can be implemented using relatively independent
implementations.

The original implementation of the floating point framework did this as it was
being built out, but ran into difficulties when defining the inherited
accuracies.

For examples, `tan(x) inherits from sin(x)/cos(x)`, one can take the defined
rules and manually build up a bespoke solution for checking the results, but
this is tedious, error-prone, and doesn't allow for code re-use.

Instead, it would be better if there was a single conceptual framework that one
can express all the 'simple' accuracy requirements in, and then have a mechanism
for composing them to define inherited accuracies.

In the WebGPU CTS this is done via the concept of acceptance intervals, which is
derived from a similar concept in the Vulkan CTS, though implemented
significantly differently.

The core of this idea is that each of different accuracy types can be integrated
into the definition of the operation, so that instead of transforming an input
from the domain to a point in the range, the operation is producing an interval
in the range, that is the acceptable values an implementation may emit.


The simple accuracies can be defined as follows:

1. *Exact*

   `f(x) => [X, X]`


2. *Correctly Rounded*

   If `X` is precisely defined as a floating point value

   `f(x) => [X, X]`

   otherwise,

   `[a, b]` where `a` is the largest representable number with `a <= X`, and `b`
   is the smallest representable number with `X <= b`


3. *Absolute Error*

   `f(x) => [ X - ε, X + ε ]`, where ε is the absolute error value


4. **ULP Error**

   `f(x) = X => [X - n*ULP(X), X + n*ULP(X)]`

As defined, these definitions handle mapping from a point in the domain into an
interval in the range.

This is insufficient for implementing inherited accuracies, since inheritance
sometimes involve mapping domain intervals to range intervals.

Here we use the convention for naturally extending a function on real numbers
into a function on intervals of real numbers, i.e. `f([a, b]) = [A, B]`.

Given that floating point numbers have a finite number of precise values for any
given interval, one could implement just running the accuracy computation for
every point in the interval and then spanning together the resultant intervals.
That would be very inefficient though and make your reviewer sad to read.

For mapping intervals to intervals the key insight is that we only need to be
concerned with the extrema of the operation in the interval, since the
acceptance interval is defined by largest and smallest of the possible outputs.

In more precise terms:
```
  f(x) => X, x = [a, b] and X = [A, B]

  X = [min(f(x)), max(f(x))]
  X = [min(f([a, b])), max(f([a, b]))]
  X = [f(m), f(n)]
```
where `m` and `n` are in `[a, b]`, `m <= n`, and produce the min and max results
for `f` on the interval, respectively.

So how do we find the minima and maxima for our operation in the domain?

The common general solution for this requires using calculus to calculate the
derivative of `f`, `f'`, and then find the zeroes `f'` to find inflection
points of `f`.

This solution wouldn't be sufficient for all builtins, i.e. `step` which is not
differentiable at edge values.

Thankfully we do not need a general solution for the CTS, since all the builtin
operations are defined in the spec, so `f` is from a known set of options.

These operations can be divided into two broad categories: monotonic, and
non-monotonic, with respect to an interval.

The monotonic operations are ones that preserve the order of inputs in their
outputs (or reverse it). Their graph only ever decreases or increases,
never changing from one or the other, though it can have flat sections.

The non-monotonic operations are ones whose graph would have both regions of
increase and decrease.

The monotonic operations, when mapping an interval to an interval, are simple to
handle, since the extrema are guaranteed to be the ends of the domain, `a` and
`b`.

So `f([a, b])` = `[f(a), f(b)]` or `[f(b), f(a)]`. We could figure out if `f` is
increasing or decreasing beforehand to determine if it should be `[f(a), f(b)]`
or `[f(b), f(a)]`.

It is simpler to just use min & max to have an implementation that is agnostic
to the details of `f`.
```
  A = f(a), B = f(b)
  X = [min(A, B), max(A, B)]
```

The non-monotonic functions that we need to handle for interval-to-interval
mappings are more complex. Thankfully are a small number of the overall
operations that need to be handled, since they are only the operations that are
used in an inherited accuracy and take in the output of another operation as
part of that inherited accuracy.

So in the CTS we just have bespoke implementations for each of them.

Part of the operation definition in the CTS is a function that takes in the
domain interval, and returns a sub-interval such that the subject function is
monotonic over that sub-interval, and hence the function's minima and maxima are
at the ends.

This adjusted domain interval can then be fed through the same machinery as the
monotonic functions.

### Inherited Accuracy

So with all of that background out of the way, we can now define an inherited
accuracy in terms of acceptance intervals.

The crux of this is the insight that the range of one operation can become the
domain of another operation to compose them together.

And since we have defined how to do this interval to interval mapping above,
transforming things becomes mechanical and thus implementable in reusable code.

When talking about inherited accuracies `f(x) => g(x)` is used to denote that
`f`'s accuracy is a defined as `g`.

An example to illustrate inherited accuracies, in f32:

```
  tan(x) => sin(x)/cos(x)

  sin(x) => [sin(x) - 2 ** -11, sin(x) + 2 ** -11]`
  cos(x) => [cos(x) - 2 ** -11, cos(x) + 2-11]

  x/y => [x/y - 2.5 * ULP(x/y), x/y + 2.5 * ULP(x/y)]
```

`sin(x)` and `cos(x)` are non-monotonic, so calculating out a closed generic
form over an interval is a pain, since the min and max vary depending on the
value of x. Let's isolate this to a single point, so you don't have to read
literally pages of expanded intervals.

```
  x = π/2

  sin(π/2) => [sin(π/2) - 2 ** -11, sin(π/2) + 2 ** -11]
           => [0 - 2 ** -11, 0 + 2 ** -11]
           => [-0.000488…, 0.000488…]
  cos(π/2) => [cos(π/2) - 2 ** -11, cos(π/2) + 2 ** -11]
           => [-0.500488…, -0.499511…]

  tan(π/2) => sin(π/2)/cos(π/2)
           => [-0.000488…, 0.000488…]/[-0.500488…, -0.499511…]
           => [min(-0.000488…/-0.500488…, -0.000488…/-0.499511…, 0.000488…/-0.500488…, 0.000488…/-0.499511…),
               max(-0.000488…/-0.500488…, -0.000488…/-0.499511…, 0.000488…/-0.500488…, 0.000488…/-0.499511…)]
           => [0.000488…/-0.499511…, 0.000488…/0.499511…]
           => [-0.0009775171, 0.0009775171]
```

For clarity this has omitted a bunch of complexity around FTZ behaviours, and
that these operations are only defined for specific domains, but the high-level
concepts hold.

For each of the inherited operations we could implement a manually written out
closed form solution, but that would be quite error-prone and not be
re-using code between builtins.

Instead, the CTS takes advantage of the fact in addition to testing
implementations of `tan(x)` we are going to be testing implementations of
`sin(x)`, `cos(x)` and `x/y`, so there should be functions to generate
acceptance intervals for those operations.

The `tan(x)` acceptance interval can be constructed by generating the acceptance
intervals for `sin(x)`, `cos(x)` and `x/y` via function calls and composing the
results.

This algorithmically looks something like this:

```
  tan(x):
    Calculate sin(x) interval
    Calculate cos(x) interval
    Calculate sin(x) result divided by cos(x) result
    Return division result
```

### Out of Bounds
When calculating inherited intervals, if a intermediate calculation goes out of
bounds this will flow through to later calculations, even if a later calculation
would pull the result back inbounds.

For example, `fp.positive.max + fp.positive.max - fp.positive.max` could be
simplified to just `fp.positive.max` before execution, but it would also be
valid for an implementation to naively perform left to right evaluation. Thus
the addition would produce an intermediate value of `2 * fp.positive.max`. Again
the implementation may hoist intermediate calculation to a higher precision to
avoid overflow here, but is not required to. So a conforming implementation at
this point may just return any value since the calculation when out of bounds.
Thus the execution tests in the CTS should accept any value returned, so the
case is just effectively confirming the computation completes.

When looking at validation tests there is some subtleties about out of bounds
behaviour, specifically how far out of bounds the value went that will influence
the expected results, which will be discussed in more detail below.

#### Vectors and Matrices
The above discussion about inheritance of out of bounds intervals assumed scalar
calculations, so all the result intervals were dependent on the input intervals,
so if an out-of-bounds input occurred naturally all the output values were
effectively out of bounds.

For vector and matrix operations, this is not always true. Operations on these
data structures can either define an element-wise mapping, where for each output
element the result is calculated by executing a scalar operation on a input
element (sometimes referred to as component-wise), or where the operation is
defined such the output elements are dependent on the entire input.

For concrete examples, constant scaling (`c * vec` of `c * mat`) is an
element-wise operation, because one can define a simple mapping
`o[i]` = `c * i[i]`, where the ith output only depends on the ith input.

A non-element-wise operation would be something like cross product of vectors
or the determinant of a matrix, where each output element is dependent on
multiple input elements.

For component-wise operations, out of bounds-ness flows through per element,
i.e. if the ith input element was considered to be have gone out of bounds, then
the ith output is considered to have too also regardless of the operation
performed. Thus an input may be a mixture of out of bounds elements & inbounds
elements, and produce another mixture, assuming the operation being performed
itself does not push elements out of bounds.

For non-component-wise operations, out of bounds-ness flows through the entire
operation, i.e. if any of the input elements is out of bounds, then all the
output elements are considered to be out of bounds. Additionally, if the
calculation for any of the elements in output goes out of bounds, then the
entire output is considered to have gone out of bounds, even if other individual
elements stayed inbounds.

For some non-element-wise operations one could define mappings for individual
output elements that do not depend on all the input elements and consider only
if those inputs that are used, but for the purposes of WGSL and the CTS, OOB
inheritance is not so finely defined as to consider the difference between using
some and all the input elements for non-element-wise operations.

## Compile vs Run Time Evaluation

The above discussions have been primarily agnostic to when and where a
calculation is occurring, with an implicit bias to runtime execution on a GPU.

In reality where/when a computation is occurring has a significant impact on the
expected outcome when dealing with edge cases.

### Terminology

There are two related axes that will be referred to when it comes to evaluation.
These are compile vs run time, and CPU vs GPU. Broadly speaking compile time
execution happens on the host CPU, and run time evaluation occurs on a dedicated
GPU.

(Software graphics implementations like WARP and SwiftShader technically break
this by being a software emulation of a GPU that runs on the CPU, but
conceptually one can think of these implementations being a type of GPU in this
context, since it has similar constraints when it comes to precision, etc.)

Compile time evaluation is execution that occurs when setting up a shader
module, i.e. when compiling WGSL to a platform specific shading language. It is
part of resolving values for things like constants, and occurs once, before the
shader is run by the caller. It includes constant evaluation and override
evaluation. All AbstractFloat operations are compile time evaluated.

Runtime evaluation is execution that occurs every time the shader is run, and
may include dynamic data that is provided between invocations. It is work that
is sent to the GPU for execution in the shader.

WGSL const-expressions and override-expressions are evaluated before runtime and
both are considered "compile time" in this discussion. WGSL runtime-expressions
are evaluated at runtime.

### Behavioural Differences

For a well-defined operation with a finite result, runtime and compile time
evaluation should be indistinguishable.

For example:
```
// runtime
@group(0) @binding(0) var a : f32;
@group(0) @binding(1) var b : f32;

let c: f32 = a + b
```
and
```
// compile time
const c: f32 = 1.0f + 2.0f
```
should produce the same result of `3.0` in the variable `c`, assuming `1.0` and `2.0`
were passed in as `a` and `b`.

The only difference, is when/where the execution occurs.

The difference in behaviour between these two occur when the result of the
operation is not finite for the underlying floating point type.

If instead of `1.0` and `2.0`, we had `10.0` and `f32.max`, so the true result is
`f32.max + 10.0`, the behaviours differ. Specifically the runtime
evaluated version will still run, but the result in `c` will be an indeterminate
value, which is any finite f32 value. For the compile time example instead,
compiling the shader will fail validation.

This applies to any operation, and isn't restricted to just addition. Anytime a
value goes outside the finite values the shader will hit these results,
indeterminate for runtime execution and validation failure for compile time
execution.

Unfortunately we are dealing with intervals of results and not precise results.
So this leads to more even conceptual complexity. For runtime evaluation, this
isn't too bad, because the rule becomes: if any part of the interval is
non-finite then an indeterminate value can be a result, and the interval for an
indeterminate result `[fp min, fp max]`, will include any finite portions of the
interval.

Compile time evaluation becomes significantly more complex, because difference
isn't what interval is returned, but does this shader compile or not, which are
mutually exclusive. This is compounded even further by having to consider
near-overflow vs far-overflow behaviour. Thankfully this can be broken down into
a case by case basis based on where an interval falls.

Assuming `X`, is the well-defined result of an operation, i.e. not indeterminate
due to the operation not being defined for the inputs:

| Region                       |                                                      | Result                         |
|------------------------------|------------------------------------------------------|--------------------------------|
| `abs(X) <= fp max`           | interval falls completely in the finite bounds       | validation succeeds            |
| `abs(X) >= 2 ** (exp_max+1)` | interval falls completely in the far-overflow bounds | validation fails               |
| Otherwise                    | interval intersects the near-overflow region         | validation may succeed or fail |

The final case is somewhat difficult from a CTS perspective, because now it
isn't sufficient to know that a non-finite result has occurred, but what the
specific result is needs to be tracked. Additionally, the expected result is
somewhat ambiguous, since a shader may or may not compile. This could in theory
still be tested by the CTS, via having switching logic that determines in this
region, if the shader compiles expect these results, otherwise pass the test.
This adds a significant amount of complexity to the testing code for thoroughly
testing a relatively small segment of values. Other environments do not have the
behaviour in this region as rigorously defined nor tested, so fully testing
here would likely find lots of issues that would just need to be mitigated in
the CTS.

Currently, we have chosen to not test validation of near-overflow scenarios to
avoid this complexity. If this becomes a significant source of bugs and/or
incompatibility between implementations this can be revisited in the future.

### Additional Technical Limitations

The above description of compile and runtime evaluation was somewhat based in
the theoretical world that the intervals being used for testing are infinitely
precise, when in actuality they are implemented by the ECMAScript `number` type,
which is implemented as a f64 value.

For the vast majority of cases, even out-of-bounds and overflow, this is
sufficient. There is one small slice where this breaks down. Specifically if
the result just outside the finite range by less than 1 f64 ULP of the edge
value. An example of this is `2 ** -11 + f32.max`. This will be between `f32.max`
and `f32.max + ULPF64(f32.max)`. This becomes a problem, because this value
technically falls into the out-of-bounds region, but depending on how
quantization for f64 is handled in the test runner will be either `f32.max` or
`f32.max + ULPF64(f32.max)`. So as a compile time evaluation either we expect an
implementation to always handle this, or it might fail, but we cannot easily
detect it, since this is pushing hard on the limits of precision of the testing
environment.

(A parallel version of this probably exists on the other side of the
out-of-bounds region, but I don't have a proven example of this)

The high road fix to this problem is to use an arbitrary precision floating
point implementation. Unfortunately such a library is not on the standards
track for ECMAScript at this time, so we would have to evaluate and pick a
third party dependency to use. Beyond the selection process, this would also
require a significant refactoring of the existing framework code for fixing a
very marginal case.

(This differs from Float16 support, where the prototyped version of the proposed
API has been pulled in, and the long term plan it use the ECMAScript
implementation's version, once all the major runtimes support it. So it can
be viewed as a polyfill).

This region currently is not tested as part of the decision to defer testing on
the entire out-of-bounds but not overflowing region.

In the future if we decide to add testing to the out-of-bounds region, to avoid
perfect being the enemy of good here, it is likely the CTS would still avoid
testing these regions where f64 precision breaks down. If someone is interested
in taking on the effort needed to migrate to an arbitrary precision float
library, or if this turns out to be a significant issue in the future, this
decision can be revisited.

## Abstract Float

### Accuracy

For the concrete floating point types (f32 & f16) the accuracy of operations are
defined in terms of their own type. Specifically for f32, correctly rounded
refers to the nearest f32 values, and ULP is in terms of the distance between
f32 values.

AbstractFloat internally is defined as a f64, and this applies for exact and
correctly rounded accuracies. Thus, correctly rounded refers to the nearest f64
values. However, AbstractFloat differs for ULP and absolute errors. Reading
the spec strictly, these all have unbounded accuracies, but it is recommended
that their accuracies be at least as good as the f32 equivalent.

The difference between f32 and f64 ULP at a specific value X are significant, so
at least as good as f32 requirement is always less strict than if it was
calculated in terms of f64. Similarly, for absolute accuracies the interval
`[x - epsilon, x + epsilon]` is always equal or wider if calculated as f32s
vs f64s.

If an inherited accuracy is only defined in terms of correctly rounded
accuracies, then the interval is calculated in terms of f64s. If any of the
defining accuracies are ULP or absolute errors, then the result falls into the
unbounded accuracy, but recommended to be at least as good as f32 bucket.

What this means from a CTS implementation is that for these "at least as good as
f32" error intervals, if the infinitely accurate result is finite for f32, then
the error interval for f64 is just the f32 interval. If the result is not finite
for f32, then the accuracy interval is just the unbounded interval.

How this is implemented in the CTS is by having the FPTraits for AbstractFloat
forward to the f32 implementation for the operations that are tested to be as
good as f32.

### Implementation

AbstractFloats are a compile time construct that exist in WGSL. They are
expressible as literal values or the result of operations that return them, but
a variable cannot be typed as an AbstractFloat. Instead, the variable needs be a
concrete type, i.e. f32 or f16, and the AbstractFloat value will be quantized
on assignment.

Because they cannot be stored nor passed via buffers, it is tricky to test them.
There are two approaches that have been proposed for testing the results of
operations that return AbstractFloats.

As of the writing of this doc, this second option for testing AbstractFloats
is the one being pursued in the CTS.

#### const_assert

The first proposal is to lean on the `const_assert` statement that exists in
WGSL. For each test case a snippet of code would be written out that has a form
something like this

```
// foo(x) is the operation under test
const_assert lower < foo(x) // Result was below the acceptance interval
const_assert upper > foo(x) // Result was above the acceptance interval
```

where lower and upper would actually be string replaced with literals for the
endpoints of the acceptance interval when generating the shader text.

This approach has a number of limitations that made it unacceptable for the CTS.
First, how errors are reported is a pain to debug. Someone working with the CTS
would either get a report of a failed shader compile, or a failed compile with
the line number, but they will not get the result of `foo(x)`. Just that it is
out of range. Additionally, if you place many of these stanzas in the same
shader to optimize dispatch, you will not get a report that these 3 of 32 cases
failed with these results, you will just get this batch failed. All of these
makes for a very poor experience in attempting to understand what is failing.

Beyond the lack of ergonomics, this approach also makes things like AF
comparison and const_assert very load bearing for the CTS. It is possible that
a bug could exist in an implementation of const_assert for example that would
cause it to not fail shader compilation, which could lead to silent passing of
tests. Conceptually you can think of this instead of depending on a signal to
indicate something is working, we would be depending on a signal that it isn't
working, and assuming if we don't receive that signal everything is good, not
that our signal mechanism was broken.

#### Extracting Bits

The other proposal that was developed depends on the fact that AbstractFloat is
spec'd to be a f64 internally. So the CTS could store the result of an operation
as two 32-bit unsigned integers (or broken up into sign, exponent, and
mantissa). These stored integers could be exported to the testing framework via
a buffer, which could in turn rebuild the f64 values.

This approach allows the CTS to test values directly in the testing framework,
thus provide the same diagnostics as other tests, as well as reusing the same
running harness.

The major downsides come from actually implementing extracting the bits. Due to
the restrictions on AbstractFloats the actual code to extract the bits is
tricky. Specifically there is no simple bit cast to something like an
AbstractInt that can be used. Instead, `frexp` needs to be used with additional
operations. This leads to problems, since as is `frexp` is not defined for
subnormal values, so it is impossible to extract a subnormal AbstractFloat,
though 0 could be returned when one is encountered.

Test that do try to extract bits to determine the result should either avoid
cases with subnormal results or check for the nearest normal or zero number.

The inability to store AbstractFloats in non-lossy fashion also has additional
issues, since this means that user defined functions that take in or return
them do not exist in WGSL. Thus, the snippet of code for extracting
AbstractFloats cannot just be inserted as a function at the top of a testing
shader, and then invoked on each test case. Instead, it needs to be inlined
into the shader at each call-site. Actually implementing this in the CTS isn't
difficult, but it does make the shaders significantly longer and more
difficult to read. It also may have an impact on how many test cases can be in
a batch, since runtime for some backends is sensitive to the length of the
shader being run.

# Appendix

### Significant f64 Values

| Name                   |     Decimal (~) |                   Hex | Sign Bit | Exponent Bits |                                                 Significand Bits |
|------------------------|----------------:|----------------------:|---------:|--------------:|-----------------------------------------------------------------:|
| Negative Infinity      |              -∞ | 0xfff0 0000 0000 0000 |        1 | 111 1111 1111 | 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 |
| Min Negative Normal    | -1.79769313E308 | 0xffef ffff ffff ffff |        1 | 111 1111 1110 | 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 |
| Max Negative Normal    | -2.2250738E−308 | 0x8010 0000 0000 0000 |        1 | 000 0000 0001 | 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 |
| Min Negative Subnormal | -2.2250738E−308 | 0x800f ffff ffff ffff |        1 | 000 0000 0000 | 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 |
| Max Negative Subnormal | -4.9406564E−324 | 0x8000 0000 0000 0001 |        1 | 000 0000 0000 | 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0001 |
| Negative Zero          |              -0 | 0x8000 0000 0000 0000 |        1 | 000 0000 0000 | 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 |
| Positive Zero          |               0 | 0x0000 0000 0000 0000 |        0 | 000 0000 0000 | 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 |
| Min Positive Subnormal |  4.9406564E−324 | 0x0000 0000 0000 0001 |        0 | 000 0000 0000 | 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0001 |
| Max Positive Subnormal |  2.2250738E−308 | 0x000f ffff ffff ffff |        0 | 000 0000 0000 | 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 |
| Min Positive Normal    |  2.2250738E−308 | 0x0010 0000 0000 0000 |        0 | 000 0000 0001 | 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 |
| Max Positive Normal    |  1.79769313E308 | 0x7fef ffff ffff ffff |        0 | 111 1111 1110 | 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 |
| Negative Infinity      |               ∞ | 0x7ff0 0000 0000 0000 |        0 | 111 1111 1111 | 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 |

### Significant f32 Values

| Name                   |    Decimal (~) |         Hex | Sign Bit | Exponent Bits |             Significand Bits |
|------------------------|---------------:|------------:|---------:|--------------:|-----------------------------:|
| Negative Infinity      |             -∞ | 0xff80 0000 |        1 |     1111 1111 | 0000 0000 0000 0000 0000 000 |
| Min Negative Normal    | -3.40282346E38 | 0xff7f ffff |        1 |     1111 1110 | 1111 1111 1111 1111 1111 111 |
| Max Negative Normal    | -1.1754943E−38 | 0x8080 0000 |        1 |     0000 0001 | 0000 0000 0000 0000 0000 000 |
| Min Negative Subnormal | -1.1754942E-38 | 0x807f ffff |        1 |     0000 0000 | 1111 1111 1111 1111 1111 111 |
| Max Negative Subnormal | -1.4012984E−45 | 0x8000 0001 |        1 |     0000 0000 | 0000 0000 0000 0000 0000 001 |
| Negative Zero          |             -0 | 0x8000 0000 |        1 |     0000 0000 | 0000 0000 0000 0000 0000 000 |
| Positive Zero          |              0 | 0x0000 0000 |        0 |     0000 0000 | 0000 0000 0000 0000 0000 000 |
| Min Positive Subnormal |  1.4012984E−45 | 0x0000 0001 |        0 |     0000 0000 | 0000 0000 0000 0000 0000 001 |
| Max Positive Subnormal |  1.1754942E-38 | 0x007f ffff |        0 |     0000 0000 | 1111 1111 1111 1111 1111 111 |
| Min Positive Normal    |  1.1754943E−38 | 0x0080 0000 |        0 |     0000 0001 | 0000 0000 0000 0000 0000 000 |
| Max Positive Normal    |  3.40282346E38 | 0x7f7f ffff |        0 |     1111 1110 | 1111 1111 1111 1111 1111 111 |
| Negative Infinity      |              ∞ | 0x7f80 0000 |        0 |     1111 1111 | 0000 0000 0000 0000 0000 000 |

### Significant f16 Values

| Name                   |   Decimal (~) |    Hex | Sign Bit | Exponent Bits | Significand Bits |
|------------------------|--------------:|-------:|---------:|--------------:|-----------------:|
| Negative Infinity      |            -∞ | 0xfc00 |        1 |        111 11 |     00 0000 0000 |
| Min Negative Normal    |        -65504 | 0xfbff |        1 |        111 10 |     11 1111 1111 |
| Max Negative Normal    | -6.1035156E−5 | 0x8400 |        1 |        000 01 |     00 0000 0000 |
| Min Negative Subnormal | -6.0975552E−5 | 0x83ff |        1 |        000 00 |     11 1111 1111 |
| Max Negative Subnormal | -5.9604645E−8 | 0x8001 |        1 |        000 00 |     00 0000 0001 |
| Negative Zero          |            -0 | 0x8000 |        1 |        000 00 |     00 0000 0000 |
| Positive Zero          |             0 | 0x0000 |        0 |        000 00 |     00 0000 0000 |
| Min Positive Subnormal |  5.9604645E−8 | 0x0001 |        0 |        000 00 |     00 0000 0001 |
| Max Positive Subnormal |  6.0975552E−5 | 0x03ff |        0 |        000 00 |     11 1111 1111 |
| Min Positive Normal    |  6.1035156E−5 | 0x0400 |        0 |        000 01 |     00 0000 0000 |
| Max Positive Normal    |         65504 | 0x7bff |        0 |        111 10 |     11 1111 1111 |
| Negative Infinity      |             ∞ | 0x7c00 |        0 |        111 11 |     00 0000 0000 |

# Resources
- [WebGPU Spec](https://www.w3.org/TR/webgpu/)
- [WGSL Spec](https://www.w3.org/TR/WGSL/)
- [binary64 on Wikipedia](https://en.wikipedia.org/wiki/Double-precision_floating-point_format)
- [binary32 on Wikipedia](https://en.wikipedia.org/wiki/Single-precision_floating-point_format)
- [binary16 on Wikipedia](https://en.wikipedia.org/wiki/Half-precision_floating-point_format)
- [IEEE-754 Floating Point Converter](https://www.h-schmidt.net/FloatConverter/IEEE754.html)
- [IEEE 754 Calculator](http://weitz.de/ieee/)
- [On the definition of ulp(x)](https://hal.inria.fr/inria-00070503/document)
- [Float Exposed](https://float.exposed/)
