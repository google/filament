/*
 * Copyright (C) 2017 Romain Guy
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

@file:Suppress("NOTHING_TO_INLINE", "unused")

package com.google.android.filament.utils

import kotlin.math.abs
import kotlin.math.max
import kotlin.math.min
import kotlin.math.sqrt
import kotlin.math.acos
import kotlin.math.absoluteValue

enum class VectorComponent {
    X, Y, Z, W,
    R, G, B, A,
    S, T, P, Q
}

data class Float2(var x: Float = 0.0f, var y: Float = 0.0f) {
    constructor(v: Float) : this(v, v)
    constructor(v: Float2) : this(v.x, v.y)

    inline var r: Float
        get() = x
        set(value) {
            x = value
        }
    inline var g: Float
        get() = y
        set(value) {
            y = value
        }

    inline var s: Float
        get() = x
        set(value) {
            x = value
        }
    inline var t: Float
        get() = y
        set(value) {
            y = value
        }

    inline var xy: Float2
        get() = Float2(x, y)
        set(value) {
            x = value.x
            y = value.y
        }
    inline var rg: Float2
        get() = Float2(x, y)
        set(value) {
            x = value.x
            y = value.y
        }
    inline var st: Float2
        get() = Float2(x, y)
        set(value) {
            x = value.x
            y = value.y
        }

    operator fun get(index: VectorComponent) = when (index) {
        VectorComponent.X, VectorComponent.R, VectorComponent.S -> x
        VectorComponent.Y, VectorComponent.G, VectorComponent.T -> y
        else -> throw IllegalArgumentException("index must be X, Y, R, G, S or T")
    }

    operator fun get(index1: VectorComponent, index2: VectorComponent): Float2 {
        return Float2(get(index1), get(index2))
    }

    operator fun get(index: Int) = when (index) {
        0 -> x
        1 -> y
        else -> throw IllegalArgumentException("index must be in 0..1")
    }

    operator fun get(index1: Int, index2: Int) = Float2(get(index1), get(index2))

    inline operator fun invoke(index: Int) = get(index - 1)

    operator fun set(index: Int, v: Float) = when (index) {
        0 -> x = v
        1 -> y = v
        else -> throw IllegalArgumentException("index must be in 0..1")
    }

    operator fun set(index1: Int, index2: Int, v: Float) {
        set(index1, v)
        set(index2, v)
    }

    operator fun set(index: VectorComponent, v: Float) = when (index) {
        VectorComponent.X, VectorComponent.R, VectorComponent.S -> x = v
        VectorComponent.Y, VectorComponent.G, VectorComponent.T -> y = v
        else -> throw IllegalArgumentException("index must be X, Y, R, G, S or T")
    }

    operator fun set(index1: VectorComponent, index2: VectorComponent, v: Float) {
        set(index1, v)
        set(index2, v)
    }

    operator fun unaryMinus() = Float2(-x, -y)
    operator fun inc() = Float2(x++, y++)
    operator fun dec() = Float2(x--, y--)

    inline operator fun plus(v: Float) = Float2(x + v, y + v)
    inline operator fun minus(v: Float) = Float2(x - v, y - v)
    inline operator fun times(v: Float) = Float2(x * v, y * v)
    inline operator fun div(v: Float) = Float2(x / v, y / v)
    inline fun compareTo(v: Float, delta: Float = 0.0f) = Float2(
        x.compareTo(v, delta),
        y.compareTo(v, delta)
    )

    inline fun equals(v: Float, delta: Float = 0.0f) = x.equals(v, delta) && y.equals(v, delta)

    inline operator fun plus(v: Float2) = Float2(x + v.x, y + v.y)
    inline operator fun minus(v: Float2) = Float2(x - v.x, y - v.y)
    inline operator fun times(v: Float2) = Float2(x * v.x, y * v.y)
    inline operator fun div(v: Float2) = Float2(x / v.x, y / v.y)
    inline fun compareTo(v: Float2, delta: Float = 0.0f) = Float2(
        x.compareTo(v.x, delta),
        y.compareTo(v.y, delta)
    )

    inline fun equals(v: Float2, delta: Float = 0.0f) = x.equals(v.x, delta) && y.equals(v.y, delta)

    inline fun transform(block: (Float) -> Float): Float2 {
        x = block(x)
        y = block(y)
        return this
    }

    fun toFloatArray() = floatArrayOf(x, y)
}

data class Float3(var x: Float = 0.0f, var y: Float = 0.0f, var z: Float = 0.0f) {
    constructor(v: Float) : this(v, v, v)
    constructor(v: Float2, z: Float = 0.0f) : this(v.x, v.y, z)
    constructor(v: Float3) : this(v.x, v.y, v.z)

    inline var r: Float
        get() = x
        set(value) {
            x = value
        }
    inline var g: Float
        get() = y
        set(value) {
            y = value
        }
    inline var b: Float
        get() = z
        set(value) {
            z = value
        }

    inline var s: Float
        get() = x
        set(value) {
            x = value
        }
    inline var t: Float
        get() = y
        set(value) {
            y = value
        }
    inline var p: Float
        get() = z
        set(value) {
            z = value
        }

    inline var xy: Float2
        get() = Float2(x, y)
        set(value) {
            x = value.x
            y = value.y
        }
    inline var rg: Float2
        get() = Float2(x, y)
        set(value) {
            x = value.x
            y = value.y
        }
    inline var st: Float2
        get() = Float2(x, y)
        set(value) {
            x = value.x
            y = value.y
        }

    inline var rgb: Float3
        get() = Float3(x, y, z)
        set(value) {
            x = value.x
            y = value.y
            z = value.z
        }
    inline var xyz: Float3
        get() = Float3(x, y, z)
        set(value) {
            x = value.x
            y = value.y
            z = value.z
        }
    inline var stp: Float3
        get() = Float3(x, y, z)
        set(value) {
            x = value.x
            y = value.y
            z = value.z
        }

    operator fun get(index: VectorComponent) = when (index) {
        VectorComponent.X, VectorComponent.R, VectorComponent.S -> x
        VectorComponent.Y, VectorComponent.G, VectorComponent.T -> y
        VectorComponent.Z, VectorComponent.B, VectorComponent.P -> z
        else -> throw IllegalArgumentException("index must be X, Y, Z, R, G, B, S, T or P")
    }

    operator fun get(index1: VectorComponent, index2: VectorComponent): Float2 {
        return Float2(get(index1), get(index2))
    }
    operator fun get(
            index1: VectorComponent, index2: VectorComponent, index3: VectorComponent): Float3 {
        return Float3(get(index1), get(index2), get(index3))
    }

    operator fun get(index: Int) = when (index) {
        0 -> x
        1 -> y
        2 -> z
        else -> throw IllegalArgumentException("index must be in 0..2")
    }

    operator fun get(index1: Int, index2: Int) = Float2(get(index1), get(index2))
    operator fun get(index1: Int, index2: Int, index3: Int): Float3 {
        return Float3(get(index1), get(index2), get(index3))
    }

    inline operator fun invoke(index: Int) = get(index - 1)

    operator fun set(index: Int, v: Float) = when (index) {
        0 -> x = v
        1 -> y = v
        2 -> z = v
        else -> throw IllegalArgumentException("index must be in 0..2")
    }

    operator fun set(index1: Int, index2: Int, v: Float) {
        set(index1, v)
        set(index2, v)
    }

    operator fun set(index1: Int, index2: Int, index3: Int, v: Float) {
        set(index1, v)
        set(index2, v)
        set(index3, v)
    }

    operator fun set(index: VectorComponent, v: Float) = when (index) {
        VectorComponent.X, VectorComponent.R, VectorComponent.S -> x = v
        VectorComponent.Y, VectorComponent.G, VectorComponent.T -> y = v
        VectorComponent.Z, VectorComponent.B, VectorComponent.P -> z = v
        else -> throw IllegalArgumentException("index must be X, Y, Z, R, G, B, S, T or P")
    }

    operator fun set(index1: VectorComponent, index2: VectorComponent, v: Float) {
        set(index1, v)
        set(index2, v)
    }

    operator fun set(
            index1: VectorComponent, index2: VectorComponent, index3: VectorComponent, v: Float) {
        set(index1, v)
        set(index2, v)
        set(index3, v)
    }

    operator fun unaryMinus() = Float3(-x, -y, -z)
    operator fun inc() = Float3(x++, y++, z++)
    operator fun dec() = Float3(x--, y--, z--)

    inline operator fun plus(v: Float) = Float3(x + v, y + v, z + v)
    inline operator fun minus(v: Float) = Float3(x - v, y - v, z - v)
    inline operator fun times(v: Float) = Float3(x * v, y * v, z * v)
    inline operator fun div(v: Float) = Float3(x / v, y / v, z / v)
    inline fun compareTo(v: Float, delta: Float = 0.0f) = Float3(
        x.compareTo(v, delta),
        y.compareTo(v, delta),
        z.compareTo(v, delta)
    )

    inline fun equals(v: Float, delta: Float = 0.0f) =
        x.equals(v, delta) && y.equals(v, delta) && z.equals(v, delta)

    inline operator fun plus(v: Float2) = Float3(x + v.x, y + v.y, z)
    inline operator fun minus(v: Float2) = Float3(x - v.x, y - v.y, z)
    inline operator fun times(v: Float2) = Float3(x * v.x, y * v.y, z)
    inline operator fun div(v: Float2) = Float3(x / v.x, y / v.y, z)

    inline operator fun plus(v: Float3) = Float3(x + v.x, y + v.y, z + v.z)
    inline operator fun minus(v: Float3) = Float3(x - v.x, y - v.y, z - v.z)
    inline operator fun times(v: Float3) = Float3(x * v.x, y * v.y, z * v.z)
    inline operator fun div(v: Float3) = Float3(x / v.x, y / v.y, z / v.z)
    inline fun compareTo(v: Float3, delta: Float = 0.0f) = Float3(
        x.compareTo(v.x, delta),
        y.compareTo(v.y, delta),
        z.compareTo(v.z, delta)
    )

    inline fun equals(v: Float3, delta: Float = 0.0f) =
        x.equals(v.x, delta) && y.equals(v.y, delta) && z.equals(v.z, delta)

    inline fun transform(block: (Float) -> Float): Float3 {
        x = block(x)
        y = block(y)
        z = block(z)
        return this
    }

    fun toFloatArray() = floatArrayOf(x, y, z)
}

data class Float4(
        var x: Float = 0.0f,
        var y: Float = 0.0f,
        var z: Float = 0.0f,
        var w: Float = 0.0f) {
    constructor(v: Float) : this(v, v, v, v)
    constructor(v: Float2, z: Float = 0.0f, w: Float = 0.0f) : this(v.x, v.y, z, w)
    constructor(v: Float3, w: Float = 0.0f) : this(v.x, v.y, v.z, w)
    constructor(v: Float4) : this(v.x, v.y, v.z, v.w)

    inline var r: Float
        get() = x
        set(value) {
            x = value
        }
    inline var g: Float
        get() = y
        set(value) {
            y = value
        }
    inline var b: Float
        get() = z
        set(value) {
            z = value
        }
    inline var a: Float
        get() = w
        set(value) {
            w = value
        }

    inline var s: Float
        get() = x
        set(value) {
            x = value
        }
    inline var t: Float
        get() = y
        set(value) {
            y = value
        }
    inline var p: Float
        get() = z
        set(value) {
            z = value
        }
    inline var q: Float
        get() = w
        set(value) {
            w = value
        }

    inline var xy: Float2
        get() = Float2(x, y)
        set(value) {
            x = value.x
            y = value.y
        }
    inline var rg: Float2
        get() = Float2(x, y)
        set(value) {
            x = value.x
            y = value.y
        }
    inline var st: Float2
        get() = Float2(x, y)
        set(value) {
            x = value.x
            y = value.y
        }

    inline var rgb: Float3
        get() = Float3(x, y, z)
        set(value) {
            x = value.x
            y = value.y
            z = value.z
        }
    inline var xyz: Float3
        get() = Float3(x, y, z)
        set(value) {
            x = value.x
            y = value.y
            z = value.z
        }
    inline var stp: Float3
        get() = Float3(x, y, z)
        set(value) {
            x = value.x
            y = value.y
            z = value.z
        }

    inline var rgba: Float4
        get() = Float4(x, y, z, w)
        set(value) {
            x = value.x
            y = value.y
            z = value.z
            w = value.w
        }
    inline var xyzw: Float4
        get() = Float4(x, y, z, w)
        set(value) {
            x = value.x
            y = value.y
            z = value.z
            w = value.w
        }
    inline var stpq: Float4
        get() = Float4(x, y, z, w)
        set(value) {
            x = value.x
            y = value.y
            z = value.z
            w = value.w
        }

    operator fun get(index: VectorComponent) = when (index) {
        VectorComponent.X, VectorComponent.R, VectorComponent.S -> x
        VectorComponent.Y, VectorComponent.G, VectorComponent.T -> y
        VectorComponent.Z, VectorComponent.B, VectorComponent.P -> z
        VectorComponent.W, VectorComponent.A, VectorComponent.Q -> w
    }

    operator fun get(index1: VectorComponent, index2: VectorComponent): Float2 {
        return Float2(get(index1), get(index2))
    }
    operator fun get(
            index1: VectorComponent,
            index2: VectorComponent,
            index3: VectorComponent): Float3 {
        return Float3(get(index1), get(index2), get(index3))
    }
    operator fun get(
            index1: VectorComponent,
            index2: VectorComponent,
            index3: VectorComponent,
            index4: VectorComponent): Float4 {
        return Float4(get(index1), get(index2), get(index3), get(index4))
    }

    operator fun get(index: Int) = when (index) {
        0 -> x
        1 -> y
        2 -> z
        3 -> w
        else -> throw IllegalArgumentException("index must be in 0..3")
    }

    operator fun get(index1: Int, index2: Int) = Float2(get(index1), get(index2))
    operator fun get(index1: Int, index2: Int, index3: Int): Float3 {
        return Float3(get(index1), get(index2), get(index3))
    }
    operator fun get(index1: Int, index2: Int, index3: Int, index4: Int): Float4 {
        return Float4(get(index1), get(index2), get(index3), get(index4))
    }

    inline operator fun invoke(index: Int) = get(index - 1)

    operator fun set(index: Int, v: Float) = when (index) {
        0 -> x = v
        1 -> y = v
        2 -> z = v
        3 -> w = v
        else -> throw IllegalArgumentException("index must be in 0..3")
    }

    operator fun set(index1: Int, index2: Int, v: Float) {
        set(index1, v)
        set(index2, v)
    }

    operator fun set(index1: Int, index2: Int, index3: Int, v: Float) {
        set(index1, v)
        set(index2, v)
        set(index3, v)
    }

    operator fun set(index1: Int, index2: Int, index3: Int, index4: Int, v: Float) {
        set(index1, v)
        set(index2, v)
        set(index3, v)
        set(index4, v)
    }

    operator fun set(index: VectorComponent, v: Float) = when (index) {
        VectorComponent.X, VectorComponent.R, VectorComponent.S -> x = v
        VectorComponent.Y, VectorComponent.G, VectorComponent.T -> y = v
        VectorComponent.Z, VectorComponent.B, VectorComponent.P -> z = v
        VectorComponent.W, VectorComponent.A, VectorComponent.Q -> w = v
    }

    operator fun set(index1: VectorComponent, index2: VectorComponent, v: Float) {
        set(index1, v)
        set(index2, v)
    }

    operator fun set(
            index1: VectorComponent, index2: VectorComponent, index3: VectorComponent, v: Float) {
        set(index1, v)
        set(index2, v)
        set(index3, v)
    }

    operator fun set(
            index1: VectorComponent, index2: VectorComponent,
            index3: VectorComponent, index4: VectorComponent, v: Float) {
        set(index1, v)
        set(index2, v)
        set(index3, v)
        set(index4, v)
    }

    operator fun unaryMinus() = Float4(-x, -y, -z, -w)
    operator fun inc() = Float4(x++, y++, z++, w++)
    operator fun dec() = Float4(x--, y--, z--, w--)

    inline operator fun plus(v: Float) = Float4(x + v, y + v, z + v, w + v)
    inline operator fun minus(v: Float) = Float4(x - v, y - v, z - v, w - v)
    inline operator fun times(v: Float) = Float4(x * v, y * v, z * v, w * v)
    inline operator fun div(v: Float) = Float4(x / v, y / v, z / v, w / v)
    inline fun compareTo(v: Float, delta: Float = 0.0f) = Float4(
        x.compareTo(v, delta),
        y.compareTo(v, delta),
        z.compareTo(v, delta),
        w.compareTo(v, delta)
    )

    inline fun equals(v: Float, delta: Float = 0.0f) =
        x.equals(v, delta) && y.equals(v, delta) && z.equals(v, delta) && w.equals(v, delta)

    inline operator fun plus(v: Float2) = Float4(x + v.x, y + v.y, z, w)
    inline operator fun minus(v: Float2) = Float4(x - v.x, y - v.y, z, w)
    inline operator fun times(v: Float2) = Float4(x * v.x, y * v.y, z, w)
    inline operator fun div(v: Float2) = Float4(x / v.x, y / v.y, z, w)

    inline operator fun plus(v: Float3) = Float4(x + v.x, y + v.y, z + v.z, w)
    inline operator fun minus(v: Float3) = Float4(x - v.x, y - v.y, z - v.z, w)
    inline operator fun times(v: Float3) = Float4(x * v.x, y * v.y, z * v.z, w)
    inline operator fun div(v: Float3) = Float4(x / v.x, y / v.y, z / v.z, w)

    inline operator fun plus(v: Float4) = Float4(x + v.x, y + v.y, z + v.z, w + v.w)
    inline operator fun minus(v: Float4) = Float4(x - v.x, y - v.y, z - v.z, w - v.w)
    inline operator fun times(v: Float4) = Float4(x * v.x, y * v.y, z * v.z, w * v.w)
    inline operator fun div(v: Float4) = Float4(x / v.x, y / v.y, z / v.z, w / v.w)
    inline fun compareTo(v: Float4, delta: Float = 0.0f) = Float4(
        x.compareTo(v.x, delta),
        y.compareTo(v.y, delta),
        z.compareTo(v.z, delta),
        w.compareTo(v.w, delta)
    )

    inline fun equals(v: Float4, delta: Float = 0.0f) =
        x.equals(v.x, delta) && y.equals(v.y, delta) && z.equals(v.z, delta) && w.equals(v.w, delta)

    inline fun transform(block: (Float) -> Float): Float4 {
        x = block(x)
        y = block(y)
        z = block(z)
        w = block(w)
        return this
    }

    fun toFloatArray() = floatArrayOf(x, y, z, w)
}

inline operator fun Float.plus(v: Float2) = Float2(this + v.x, this + v.y)
inline operator fun Float.minus(v: Float2) = Float2(this - v.x, this - v.y)
inline operator fun Float.times(v: Float2) = Float2(this * v.x, this * v.y)
inline operator fun Float.div(v: Float2) = Float2(this / v.x, this / v.y)

inline fun Float.compareTo(v: Float, delta: Float): Float = when {
    equals(v, delta) -> 0.0f
    else -> compareTo(v).toFloat()
}

inline fun Float.equals(v: Float, delta: Float) = (this - v).absoluteValue < delta
inline fun abs(v: Float2) = Float2(abs(v.x), abs(v.y))
inline fun length(v: Float2) = sqrt(v.x * v.x + v.y * v.y)
inline fun length2(v: Float2) = v.x * v.x + v.y * v.y
inline fun distance(a: Float2, b: Float2) = length(a - b)
inline fun dot(a: Float2, b: Float2) = a.x * b.x + a.y * b.y
fun normalize(v: Float2): Float2 {
    val l = 1.0f / length(v)
    return Float2(v.x * l, v.y * l)
}

inline fun reflect(i: Float2, n: Float2) = i - 2.0f * dot(n, i) * n
fun refract(i: Float2, n: Float2, eta: Float): Float2 {
    val d = dot(n, i)
    val k = 1.0f - eta * eta * (1.0f - sqr(d))
    return if (k < 0.0f) Float2(0.0f) else eta * i - (eta * d + sqrt(k)) * n
}

inline fun angle(a: Float2, b: Float2): Float {
    val l = length(a) * length(b)
    return if (l == 0.0f) 0.0f else acos(clamp(dot(a, b) / l, -1.0f, 1.0f))
}

inline fun clamp(v: Float2, min: Float, max: Float): Float2 {
    return Float2(
            clamp(v.x, min, max),
            clamp(v.y, min, max)
    )
}

inline fun clamp(v: Float2, min: Float2, max: Float2): Float2 {
    return Float2(
            clamp(v.x, min.x, max.x),
            clamp(v.y, min.y, max.y)
    )
}

inline fun mix(a: Float2, b: Float2, x: Float): Float2 {
    return Float2(
            mix(a.x, b.x, x),
            mix(a.y, b.y, x)
    )
}

inline fun mix(a: Float2, b: Float2, x: Float2): Float2 {
    return Float2(
            mix(a.x, b.x, x.x),
            mix(a.y, b.y, x.y)
    )
}

inline fun min(v: Float2) = min(v.x, v.y)
inline fun min(a: Float2, b: Float2) = Float2(min(a.x, b.x), min(a.y, b.y))
inline fun max(v: Float2) = max(v.x, v.y)
inline fun max(a: Float2, b: Float2) = Float2(max(a.x, b.x), max(a.y, b.y))

inline fun transform(v: Float2, block: (Float) -> Float) = v.copy().transform(block)

inline fun lessThan(a: Float2, b: Float) = Bool2(a.x < b, a.y < b)
inline fun lessThan(a: Float2, b: Float2) = Bool2(a.x < b.x, a.y < b.y)
inline fun lessThanEqual(a: Float2, b: Float) = Bool2(a.x <= b, a.y <= b)
inline fun lessThanEqual(a: Float2, b: Float2) = Bool2(a.x <= b.x, a.y <= b.y)
inline fun greaterThan(a: Float2, b: Float) = Bool2(a.x > b, a.y > b)
inline fun greaterThan(a: Float2, b: Float2) = Bool2(a.x > b.y, a.y > b.y)
inline fun greaterThanEqual(a: Float2, b: Float) = Bool2(a.x >= b, a.y >= b)
inline fun greaterThanEqual(a: Float2, b: Float2) = Bool2(a.x >= b.x, a.y >= b.y)
inline fun equal(a: Float2, b: Float, delta: Float = 0.0f) = Bool2(
    a.x.equals(b, delta),
    a.y.equals(b, delta)
)

inline fun equal(a: Float2, b: Float2, delta: Float = 0.0f) = Bool2(
    a.x.equals(b.x, delta),
    a.y.equals(b.y, delta)
)

inline fun notEqual(a: Float2, b: Float, delta: Float = 0.0f) = Bool2(
    !a.x.equals(b, delta),
    !a.y.equals(b, delta)
)

inline fun notEqual(a: Float2, b: Float2, delta: Float = 0.0f) = Bool2(
    !a.x.equals(b.x, delta),
    !a.y.equals(b.y, delta)
)

inline infix fun Float2.lt(b: Float) = Bool2(x < b, y < b)
inline infix fun Float2.lt(b: Float2) = Bool2(x < b.x, y < b.y)
inline infix fun Float2.lte(b: Float) = Bool2(x <= b, y <= b)
inline infix fun Float2.lte(b: Float2) = Bool2(x <= b.x, y <= b.y)
inline infix fun Float2.gt(b: Float) = Bool2(x > b, y > b)
inline infix fun Float2.gt(b: Float2) = Bool2(x > b.x, y > b.y)
inline infix fun Float2.gte(b: Float) = Bool2(x >= b, y >= b)
inline infix fun Float2.gte(b: Float2) = Bool2(x >= b.x, y >= b.y)
inline infix fun Float2.eq(b: Float) = Bool2(x == b, y == b)
inline infix fun Float2.eq(b: Float2) = Bool2(x == b.x, y == b.y)
inline infix fun Float2.neq(b: Float) = Bool2(x != b, y != b)
inline infix fun Float2.neq(b: Float2) = Bool2(x != b.x, y != b.y)

inline fun any(v: Bool2) = v.x || v.y
inline fun all(v: Bool2) = v.x && v.y

inline operator fun Float.plus(v: Float3) = Float3(this + v.x, this + v.y, this + v.z)
inline operator fun Float.minus(v: Float3) = Float3(this - v.x, this - v.y, this - v.z)
inline operator fun Float.times(v: Float3) = Float3(this * v.x, this * v.y, this * v.z)
inline operator fun Float.div(v: Float3) = Float3(this / v.x, this / v.y, this / v.z)

inline fun abs(v: Float3) = Float3(abs(v.x), abs(v.y), abs(v.z))
inline fun length(v: Float3) = sqrt(v.x * v.x + v.y * v.y + v.z * v.z)
inline fun length2(v: Float3) = v.x * v.x + v.y * v.y + v.z * v.z
inline fun distance(a: Float3, b: Float3) = length(a - b)
inline fun dot(a: Float3, b: Float3) = a.x * b.x + a.y * b.y + a.z * b.z
inline fun cross(a: Float3, b: Float3): Float3 {
    return Float3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x)
}
inline infix fun Float3.x(v: Float3): Float3 {
    return Float3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x)
}
fun normalize(v: Float3): Float3 {
    val l = 1.0f / length(v)
    return Float3(v.x * l, v.y * l, v.z * l)
}

inline fun reflect(i: Float3, n: Float3) = i - 2.0f * dot(n, i) * n
fun refract(i: Float3, n: Float3, eta: Float): Float3 {
    val d = dot(n, i)
    val k = 1.0f - eta * eta * (1.0f - sqr(d))
    return if (k < 0.0f) Float3(0.0f) else eta * i - (eta * d + sqrt(k)) * n
}

inline fun angle(a: Float3, b: Float3): Float {
    val l = length(a) * length(b)
    return if (l == 0.0f) 0.0f else acos(clamp(dot(a, b) / l, -1.0f, 1.0f))
}

inline fun clamp(v: Float3, min: Float, max: Float): Float3 {
    return Float3(
            clamp(v.x, min, max),
            clamp(v.y, min, max),
            clamp(v.z, min, max)
    )
}

inline fun clamp(v: Float3, min: Float3, max: Float3): Float3 {
    return Float3(
            clamp(v.x, min.x, max.x),
            clamp(v.y, min.y, max.y),
            clamp(v.z, min.z, max.z)
    )
}

inline fun mix(a: Float3, b: Float3, x: Float): Float3 {
    return Float3(
            mix(a.x, b.x, x),
            mix(a.y, b.y, x),
            mix(a.z, b.z, x)
    )
}

inline fun mix(a: Float3, b: Float3, x: Float3): Float3 {
    return Float3(
            mix(a.x, b.x, x.x),
            mix(a.y, b.y, x.y),
            mix(a.z, b.z, x.z)
    )
}

inline fun min(v: Float3) = min(v.x, min(v.y, v.z))
inline fun min(a: Float3, b: Float3) = Float3(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z))
inline fun max(v: Float3) = max(v.x, max(v.y, v.z))
inline fun max(a: Float3, b: Float3) = Float3(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z))

inline fun transform(v: Float3, block: (Float) -> Float) = v.copy().transform(block)

inline fun lessThan(a: Float3, b: Float) = Bool3(a.x < b, a.y < b, a.z < b)
inline fun lessThan(a: Float3, b: Float3) = Bool3(a.x < b.x, a.y < b.y, a.z < b.z)
inline fun lessThanEqual(a: Float3, b: Float) = Bool3(a.x <= b, a.y <= b, a.z <= b)
inline fun lessThanEqual(a: Float3, b: Float3) = Bool3(a.x <= b.x, a.y <= b.y, a.z <= b.z)
inline fun greaterThan(a: Float3, b: Float) = Bool3(a.x > b, a.y > b, a.z > b)
inline fun greaterThan(a: Float3, b: Float3) = Bool3(a.x > b.y, a.y > b.y, a.z > b.z)
inline fun greaterThanEqual(a: Float3, b: Float) = Bool3(a.x >= b, a.y >= b, a.z >= b)
inline fun greaterThanEqual(a: Float3, b: Float3) = Bool3(a.x >= b.x, a.y >= b.y, a.z >= b.z)
inline fun equal(a: Float3, b: Float, delta: Float = 0.0f) = Bool3(
    a.x.equals(b, delta),
    a.y.equals(b, delta),
    a.z.equals(b, delta)
)

inline fun equal(a: Float3, b: Float3, delta: Float = 0.0f) = Bool3(
    a.x.equals(b.x, delta),
    a.y.equals(b.y, delta),
    a.z.equals(b.z, delta)
)

inline fun notEqual(a: Float3, b: Float, delta: Float = 0.0f) = Bool3(
    !a.x.equals(b, delta),
    !a.y.equals(b, delta),
    !a.z.equals(b, delta)
)

inline fun notEqual(a: Float3, b: Float3, delta: Float = 0.0f) = Bool3(
    !a.x.equals(b.x, delta),
    !a.y.equals(b.y, delta),
    !a.z.equals(b.z, delta)
)

inline infix fun Float3.lt(b: Float) = Bool3(x < b, y < b, z < b)
inline infix fun Float3.lt(b: Float3) = Bool3(x < b.x, y < b.y, z < b.z)
inline infix fun Float3.lte(b: Float) = Bool3(x <= b, y <= b, z <= b)
inline infix fun Float3.lte(b: Float3) = Bool3(x <= b.x, y <= b.y, z <= b.z)
inline infix fun Float3.gt(b: Float) = Bool3(x > b, y > b, z > b)
inline infix fun Float3.gt(b: Float3) = Bool3(x > b.x, y > b.y, z > b.z)
inline infix fun Float3.gte(b: Float) = Bool3(x >= b, y >= b, z >= b)
inline infix fun Float3.gte(b: Float3) = Bool3(x >= b.x, y >= b.y, z >= b.z)
inline infix fun Float3.eq(b: Float) = Bool3(x == b, y == b, z == b)
inline infix fun Float3.eq(b: Float3) = Bool3(x == b.x, y == b.y, z == b.z)
inline infix fun Float3.neq(b: Float) = Bool3(x != b, y != b, z != b)
inline infix fun Float3.neq(b: Float3) = Bool3(x != b.x, y != b.y, z != b.z)

inline fun any(v: Bool3) = v.x || v.y || v.z
inline fun all(v: Bool3) = v.x && v.y && v.z

inline operator fun Float.plus(v: Float4) = Float4(this + v.x, this + v.y, this + v.z, this + v.w)
inline operator fun Float.minus(v: Float4) = Float4(this - v.x, this - v.y, this - v.z, this - v.w)
inline operator fun Float.times(v: Float4) = Float4(this * v.x, this * v.y, this * v.z, this * v.w)
inline operator fun Float.div(v: Float4) = Float4(this / v.x, this / v.y, this / v.z, this / v.w)

inline fun abs(v: Float4) = Float4(abs(v.x), abs(v.y), abs(v.z), abs(v.w))
inline fun length(v: Float4) = sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w)
inline fun length2(v: Float4) = v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w
inline fun distance(a: Float4, b: Float4) = length(a - b)
inline fun dot(a: Float4, b: Float4) = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w
fun normalize(v: Float4): Float4 {
    val l = 1.0f / length(v)
    return Float4(v.x * l, v.y * l, v.z * l, v.w * l)
}

inline fun clamp(v: Float4, min: Float, max: Float): Float4 {
    return Float4(
            clamp(v.x, min, max),
            clamp(v.y, min, max),
            clamp(v.z, min, max),
            clamp(v.w, min, max)
    )
}

inline fun clamp(v: Float4, min: Float4, max: Float4): Float4 {
    return Float4(
            clamp(v.x, min.x, max.x),
            clamp(v.y, min.y, max.y),
            clamp(v.z, min.z, max.z),
            clamp(v.w, min.z, max.w)
    )
}

inline fun mix(a: Float4, b: Float4, x: Float): Float4 {
    return Float4(
            mix(a.x, b.x, x),
            mix(a.y, b.y, x),
            mix(a.z, b.z, x),
            mix(a.w, b.w, x)
    )
}

inline fun mix(a: Float4, b: Float4, x: Float4): Float4 {
    return Float4(
            mix(a.x, b.x, x.x),
            mix(a.y, b.y, x.y),
            mix(a.z, b.z, x.z),
            mix(a.w, b.w, x.w))
}

inline fun min(v: Float4) = min(v.x, min(v.y, min(v.z, v.w)))
inline fun min(a: Float4, b: Float4): Float4 {
    return Float4(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z), min(a.w, b.w))
}
inline fun max(v: Float4) = max(v.x, max(v.y, max(v.z, v.w)))
inline fun max(a: Float4, b: Float4): Float4 {
    return Float4(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z), max(a.w, b.w))
}

inline fun transform(v: Float4, block: (Float) -> Float) = v.copy().transform(block)

inline fun lessThan(a: Float4, b: Float) = Bool4(a.x < b, a.y < b, a.z < b, a.w < b)
inline fun lessThan(a: Float4, b: Float4) = Bool4(a.x < b.x, a.y < b.y, a.z < b.z, a.w < b.w)
inline fun lessThanEqual(a: Float4, b: Float) = Bool4(a.x <= b, a.y <= b, a.z <= b, a.w <= b)
inline fun lessThanEqual(a: Float4, b: Float4) =
    Bool4(a.x <= b.x, a.y <= b.y, a.z <= b.z, a.w <= b.w)

inline fun greaterThan(a: Float4, b: Float) = Bool4(a.x > b, a.y > b, a.z > b, a.w > b)
inline fun greaterThan(a: Float4, b: Float4) = Bool4(a.x > b.y, a.y > b.y, a.z > b.z, a.w > b.w)
inline fun greaterThanEqual(a: Float4, b: Float) = Bool4(a.x >= b, a.y >= b, a.z >= b, a.w >= b)
inline fun greaterThanEqual(a: Float4, b: Float4) =
    Bool4(a.x >= b.x, a.y >= b.y, a.z >= b.z, a.w >= b.w)

inline fun equal(a: Float4, b: Float, delta: Float = 0.0f) = Bool4(
    a.x.equals(b, delta),
    a.y.equals(b, delta),
    a.z.equals(b, delta),
    a.w.equals(b, delta)
)

inline fun equal(a: Float4, b: Float4, delta: Float = 0.0f) = Bool4(
    a.x.equals(b.x, delta),
    a.y.equals(b.y, delta),
    a.z.equals(b.z, delta),
    a.w.equals(b.w, delta)
)

inline fun notEqual(a: Float4, b: Float, delta: Float = 0.0f) = Bool4(
    !a.x.equals(b, delta),
    !a.y.equals(b, delta),
    !a.z.equals(b, delta),
    !a.w.equals(b, delta)
)

inline fun notEqual(a: Float4, b: Float4, delta: Float = 0.0f) = Bool4(
    !a.x.equals(b.x, delta),
    !a.y.equals(b.y, delta),
    !a.z.equals(b.z, delta),
    !a.w.equals(b.w, delta)
)

inline infix fun Float4.lt(b: Float) = Bool4(x < b, y < b, z < b, w < b)
inline infix fun Float4.lt(b: Float4) = Bool4(x < b.x, y < b.y, z < b.z, w < b.w)
inline infix fun Float4.lte(b: Float) = Bool4(x <= b, y <= b, z <= b, w <= b)
inline infix fun Float4.lte(b: Float4) = Bool4(x <= b.x, y <= b.y, z <= b.z, w <= b.w)
inline infix fun Float4.gt(b: Float) = Bool4(x > b, y > b, z > b, w > b)
inline infix fun Float4.gt(b: Float4) = Bool4(x > b.x, y > b.y, z > b.z, w > b.w)
inline infix fun Float4.gte(b: Float) = Bool4(x >= b, y >= b, z >= b, w >= b)
inline infix fun Float4.gte(b: Float4) = Bool4(x >= b.x, y >= b.y, z >= b.z, w >= b.w)
inline infix fun Float4.eq(b: Float) = Bool4(x == b, y == b, z == b, w == b)
inline infix fun Float4.eq(b: Float4) = Bool4(x == b.x, y == b.y, z == b.z, w == b.w)
inline infix fun Float4.neq(b: Float) = Bool4(x != b, y != b, z != b, w != b)
inline infix fun Float4.neq(b: Float4) = Bool4(x != b.x, y != b.y, z != b.z, w != b.w)

inline fun any(v: Bool4) = v.x || v.y || v.z || v.w
inline fun all(v: Bool4) = v.x && v.y && v.z && v.w

data class Bool2(var x: Boolean = false, var y: Boolean = false) {
    constructor(v: Bool2) : this(v.x, v.y)

    inline var r: Boolean
        get() = x
        set(value) {
            x = value
        }
    inline var g: Boolean
        get() = y
        set(value) {
            y = value
        }

    inline var s: Boolean
        get() = x
        set(value) {
            x = value
        }
    inline var t: Boolean
        get() = y
        set(value) {
            y = value
        }

    inline var xy: Bool2
        get() = Bool2(x, y)
        set(value) {
            x = value.x
            y = value.y
        }
    inline var rg: Bool2
        get() = Bool2(x, y)
        set(value) {
            x = value.x
            y = value.y
        }
    inline var st: Bool2
        get() = Bool2(x, y)
        set(value) {
            x = value.x
            y = value.y
        }

    operator fun get(index: VectorComponent) = when (index) {
        VectorComponent.X, VectorComponent.R, VectorComponent.S -> x
        VectorComponent.Y, VectorComponent.G, VectorComponent.T -> y
        else -> throw IllegalArgumentException("index must be X, Y, R, G, S or T")
    }

    operator fun get(index1: VectorComponent, index2: VectorComponent): Bool2 {
        return Bool2(get(index1), get(index2))
    }

    operator fun get(index: Int) = when (index) {
        0 -> x
        1 -> y
        else -> throw IllegalArgumentException("index must be in 0..1")
    }

    operator fun get(index1: Int, index2: Int) = Bool2(get(index1), get(index2))

    inline operator fun invoke(index: Int) = get(index - 1)

    operator fun set(index: Int, v: Boolean) = when (index) {
        0 -> x = v
        1 -> y = v
        else -> throw IllegalArgumentException("index must be in 0..1")
    }

    operator fun set(index1: Int, index2: Int, v: Boolean) {
        set(index1, v)
        set(index2, v)
    }

    operator fun set(index: VectorComponent, v: Boolean) = when (index) {
        VectorComponent.X, VectorComponent.R, VectorComponent.S -> x = v
        VectorComponent.Y, VectorComponent.G, VectorComponent.T -> y = v
        else -> throw IllegalArgumentException("index must be X, Y, R, G, S or T")
    }

    operator fun set(index1: VectorComponent, index2: VectorComponent, v: Boolean) {
        set(index1, v)
        set(index2, v)
    }
}

data class Bool3(var x: Boolean = false, var y: Boolean = false, var z: Boolean = false) {
    constructor(v: Bool2, z: Boolean = false) : this(v.x, v.y, z)
    constructor(v: Bool3) : this(v.x, v.y, v.z)

    inline var r: Boolean
        get() = x
        set(value) {
            x = value
        }
    inline var g: Boolean
        get() = y
        set(value) {
            y = value
        }
    inline var b: Boolean
        get() = z
        set(value) {
            z = value
        }

    inline var s: Boolean
        get() = x
        set(value) {
            x = value
        }
    inline var t: Boolean
        get() = y
        set(value) {
            y = value
        }
    inline var p: Boolean
        get() = z
        set(value) {
            z = value
        }

    inline var xy: Bool2
        get() = Bool2(x, y)
        set(value) {
            x = value.x
            y = value.y
        }
    inline var rg: Bool2
        get() = Bool2(x, y)
        set(value) {
            x = value.x
            y = value.y
        }
    inline var st: Bool2
        get() = Bool2(x, y)
        set(value) {
            x = value.x
            y = value.y
        }

    inline var rgb: Bool3
        get() = Bool3(x, y, z)
        set(value) {
            x = value.x
            y = value.y
            z = value.z
        }
    inline var xyz: Bool3
        get() = Bool3(x, y, z)
        set(value) {
            x = value.x
            y = value.y
            z = value.z
        }
    inline var stp: Bool3
        get() = Bool3(x, y, z)
        set(value) {
            x = value.x
            y = value.y
            z = value.z
        }

    operator fun get(index: VectorComponent) = when (index) {
        VectorComponent.X, VectorComponent.R, VectorComponent.S -> x
        VectorComponent.Y, VectorComponent.G, VectorComponent.T -> y
        VectorComponent.Z, VectorComponent.B, VectorComponent.P -> z
        else -> throw IllegalArgumentException("index must be X, Y, Z, R, G, B, S, T or P")
    }

    operator fun get(index1: VectorComponent, index2: VectorComponent): Bool2 {
        return Bool2(get(index1), get(index2))
    }
    operator fun get(
            index1: VectorComponent, index2: VectorComponent, index3: VectorComponent): Bool3 {
        return Bool3(get(index1), get(index2), get(index3))
    }

    operator fun get(index: Int) = when (index) {
        0 -> x
        1 -> y
        2 -> z
        else -> throw IllegalArgumentException("index must be in 0..2")
    }

    operator fun get(index1: Int, index2: Int) = Bool2(get(index1), get(index2))
    operator fun get(index1: Int, index2: Int, index3: Int): Bool3 {
        return Bool3(get(index1), get(index2), get(index3))
    }

    inline operator fun invoke(index: Int) = get(index - 1)

    operator fun set(index: Int, v: Boolean) = when (index) {
        0 -> x = v
        1 -> y = v
        2 -> z = v
        else -> throw IllegalArgumentException("index must be in 0..2")
    }

    operator fun set(index1: Int, index2: Int, v: Boolean) {
        set(index1, v)
        set(index2, v)
    }

    operator fun set(index1: Int, index2: Int, index3: Int, v: Boolean) {
        set(index1, v)
        set(index2, v)
        set(index3, v)
    }

    operator fun set(index: VectorComponent, v: Boolean) = when (index) {
        VectorComponent.X, VectorComponent.R, VectorComponent.S -> x = v
        VectorComponent.Y, VectorComponent.G, VectorComponent.T -> y = v
        VectorComponent.Z, VectorComponent.B, VectorComponent.P -> z = v
        else -> throw IllegalArgumentException("index must be X, Y, Z, R, G, B, S, T or P")
    }

    operator fun set(index1: VectorComponent, index2: VectorComponent, v: Boolean) {
        set(index1, v)
        set(index2, v)
    }

    operator fun set(
            index1: VectorComponent, index2: VectorComponent, index3: VectorComponent, v: Boolean) {
        set(index1, v)
        set(index2, v)
        set(index3, v)
    }
}

data class Bool4(
        var x: Boolean = false,
        var y: Boolean = false,
        var z: Boolean = false,
        var w: Boolean = false) {
    constructor(v: Bool2, z: Boolean = false, w: Boolean = false) : this(v.x, v.y, z, w)
    constructor(v: Bool3, w: Boolean = false) : this(v.x, v.y, v.z, w)
    constructor(v: Bool4) : this(v.x, v.y, v.z, v.w)

    inline var r: Boolean
        get() = x
        set(value) {
            x = value
        }
    inline var g: Boolean
        get() = y
        set(value) {
            y = value
        }
    inline var b: Boolean
        get() = z
        set(value) {
            z = value
        }
    inline var a: Boolean
        get() = w
        set(value) {
            w = value
        }

    inline var s: Boolean
        get() = x
        set(value) {
            x = value
        }
    inline var t: Boolean
        get() = y
        set(value) {
            y = value
        }
    inline var p: Boolean
        get() = z
        set(value) {
            z = value
        }
    inline var q: Boolean
        get() = w
        set(value) {
            w = value
        }

    inline var xy: Bool2
        get() = Bool2(x, y)
        set(value) {
            x = value.x
            y = value.y
        }
    inline var rg: Bool2
        get() = Bool2(x, y)
        set(value) {
            x = value.x
            y = value.y
        }
    inline var st: Bool2
        get() = Bool2(x, y)
        set(value) {
            x = value.x
            y = value.y
        }

    inline var rgb: Bool3
        get() = Bool3(x, y, z)
        set(value) {
            x = value.x
            y = value.y
            z = value.z
        }
    inline var xyz: Bool3
        get() = Bool3(x, y, z)
        set(value) {
            x = value.x
            y = value.y
            z = value.z
        }
    inline var stp: Bool3
        get() = Bool3(x, y, z)
        set(value) {
            x = value.x
            y = value.y
            z = value.z
        }

    inline var rgba: Bool4
        get() = Bool4(x, y, z, w)
        set(value) {
            x = value.x
            y = value.y
            z = value.z
            w = value.w
        }
    inline var xyzw: Bool4
        get() = Bool4(x, y, z, w)
        set(value) {
            x = value.x
            y = value.y
            z = value.z
            w = value.w
        }
    inline var stpq: Bool4
        get() = Bool4(x, y, z, w)
        set(value) {
            x = value.x
            y = value.y
            z = value.z
            w = value.w
        }

    operator fun get(index: VectorComponent) = when (index) {
        VectorComponent.X, VectorComponent.R, VectorComponent.S -> x
        VectorComponent.Y, VectorComponent.G, VectorComponent.T -> y
        VectorComponent.Z, VectorComponent.B, VectorComponent.P -> z
        VectorComponent.W, VectorComponent.A, VectorComponent.Q -> w
    }

    operator fun get(index1: VectorComponent, index2: VectorComponent): Bool2 {
        return Bool2(get(index1), get(index2))
    }
    operator fun get(
            index1: VectorComponent,
            index2: VectorComponent,
            index3: VectorComponent): Bool3 {
        return Bool3(get(index1), get(index2), get(index3))
    }
    operator fun get(
            index1: VectorComponent,
            index2: VectorComponent,
            index3: VectorComponent,
            index4: VectorComponent): Bool4 {
        return Bool4(get(index1), get(index2), get(index3), get(index4))
    }

    operator fun get(index: Int) = when (index) {
        0 -> x
        1 -> y
        2 -> z
        3 -> w
        else -> throw IllegalArgumentException("index must be in 0..3")
    }

    operator fun get(index1: Int, index2: Int) = Bool2(get(index1), get(index2))
    operator fun get(index1: Int, index2: Int, index3: Int): Bool3 {
        return Bool3(get(index1), get(index2), get(index3))
    }
    operator fun get(index1: Int, index2: Int, index3: Int, index4: Int): Bool4 {
        return Bool4(get(index1), get(index2), get(index3), get(index4))
    }

    inline operator fun invoke(index: Int) = get(index - 1)

    operator fun set(index: Int, v: Boolean) = when (index) {
        0 -> x = v
        1 -> y = v
        2 -> z = v
        3 -> w = v
        else -> throw IllegalArgumentException("index must be in 0..3")
    }

    operator fun set(index1: Int, index2: Int, v: Boolean) {
        set(index1, v)
        set(index2, v)
    }

    operator fun set(index1: Int, index2: Int, index3: Int, v: Boolean) {
        set(index1, v)
        set(index2, v)
        set(index3, v)
    }

    operator fun set(index1: Int, index2: Int, index3: Int, index4: Int, v: Boolean) {
        set(index1, v)
        set(index2, v)
        set(index3, v)
        set(index4, v)
    }

    operator fun set(index: VectorComponent, v: Boolean) = when (index) {
        VectorComponent.X, VectorComponent.R, VectorComponent.S -> x = v
        VectorComponent.Y, VectorComponent.G, VectorComponent.T -> y = v
        VectorComponent.Z, VectorComponent.B, VectorComponent.P -> z = v
        VectorComponent.W, VectorComponent.A, VectorComponent.Q -> w = v
    }

    operator fun set(index1: VectorComponent, index2: VectorComponent, v: Boolean) {
        set(index1, v)
        set(index2, v)
    }

    operator fun set(
            index1: VectorComponent, index2: VectorComponent, index3: VectorComponent, v: Boolean) {
        set(index1, v)
        set(index2, v)
        set(index3, v)
    }

    operator fun set(
            index1: VectorComponent, index2: VectorComponent,
            index3: VectorComponent, index4: VectorComponent, v: Boolean) {
        set(index1, v)
        set(index2, v)
        set(index3, v)
        set(index4, v)
    }
}

data class Half2(var x: Half = Half.POSITIVE_ZERO, var y: Half = Half.POSITIVE_ZERO) {
    constructor(v: Half) : this(v, v)
    constructor(v: Half2) : this(v.x, v.y)

    inline var r: Half
        get() = x
        set(value) {
            x = value
        }
    inline var g: Half
        get() = y
        set(value) {
            y = value
        }

    inline var s: Half
        get() = x
        set(value) {
            x = value
        }
    inline var t: Half
        get() = y
        set(value) {
            y = value
        }

    inline var xy: Half2
        get() = Half2(x, y)
        set(value) {
            x = value.x
            y = value.y
        }
    inline var rg: Half2
        get() = Half2(x, y)
        set(value) {
            x = value.x
            y = value.y
        }
    inline var st: Half2
        get() = Half2(x, y)
        set(value) {
            x = value.x
            y = value.y
        }

    operator fun get(index: VectorComponent) = when (index) {
        VectorComponent.X, VectorComponent.R, VectorComponent.S -> x
        VectorComponent.Y, VectorComponent.G, VectorComponent.T -> y
        else -> throw IllegalArgumentException("index must be X, Y, R, G, S or T")
    }

    operator fun get(index1: VectorComponent, index2: VectorComponent): Half2 {
        return Half2(get(index1), get(index2))
    }

    operator fun get(index: Int) = when (index) {
        0 -> x
        1 -> y
        else -> throw IllegalArgumentException("index must be in 0..1")
    }

    operator fun get(index1: Int, index2: Int) = Half2(get(index1), get(index2))

    inline operator fun invoke(index: Int) = get(index - 1)

    operator fun set(index: Int, v: Half) = when (index) {
        0 -> x = v
        1 -> y = v
        else -> throw IllegalArgumentException("index must be in 0..1")
    }

    operator fun set(index1: Int, index2: Int, v: Half) {
        set(index1, v)
        set(index2, v)
    }

    operator fun set(index: VectorComponent, v: Half) = when (index) {
        VectorComponent.X, VectorComponent.R, VectorComponent.S -> x = v
        VectorComponent.Y, VectorComponent.G, VectorComponent.T -> y = v
        else -> throw IllegalArgumentException("index must be X, Y, R, G, S or T")
    }

    operator fun set(index1: VectorComponent, index2: VectorComponent, v: Half) {
        set(index1, v)
        set(index2, v)
    }

    operator fun unaryMinus() = Half2(-x, -y)
    operator fun inc() = Half2(x++, y++)
    operator fun dec() = Half2(x--, y--)

    inline operator fun plus(v: Half) = Half2(x + v, y + v)
    inline operator fun minus(v: Half) = Half2(x - v, y - v)
    inline operator fun times(v: Half) = Half2(x * v, y * v)
    inline operator fun div(v: Half) = Half2(x / v, y / v)

    inline operator fun plus(v: Half2) = Half2(x + v.x, y + v.y)
    inline operator fun minus(v: Half2) = Half2(x - v.x, y - v.y)
    inline operator fun times(v: Half2) = Half2(x * v.x, y * v.y)
    inline operator fun div(v: Half2) = Half2(x / v.x, y / v.y)

    inline fun transform(block: (Half) -> Half): Half2 {
        x = block(x)
        y = block(y)
        return this
    }

    fun toFloatArray() = floatArrayOf(x.toFloat(), y.toFloat())
}

data class Half3(
    var x: Half = Half.POSITIVE_ZERO,
    var y: Half = Half.POSITIVE_ZERO,
    var z: Half = Half.POSITIVE_ZERO
) {
    constructor(v: Half) : this(v, v, v)
    constructor(v: Half2, z: Half = Half.POSITIVE_ZERO) : this(v.x, v.y, z)
    constructor(v: Half3) : this(v.x, v.y, v.z)

    inline var r: Half
        get() = x
        set(value) {
            x = value
        }
    inline var g: Half
        get() = y
        set(value) {
            y = value
        }
    inline var b: Half
        get() = z
        set(value) {
            z = value
        }

    inline var s: Half
        get() = x
        set(value) {
            x = value
        }
    inline var t: Half
        get() = y
        set(value) {
            y = value
        }
    inline var p: Half
        get() = z
        set(value) {
            z = value
        }

    inline var xy: Half2
        get() = Half2(x, y)
        set(value) {
            x = value.x
            y = value.y
        }
    inline var rg: Half2
        get() = Half2(x, y)
        set(value) {
            x = value.x
            y = value.y
        }
    inline var st: Half2
        get() = Half2(x, y)
        set(value) {
            x = value.x
            y = value.y
        }

    inline var rgb: Half3
        get() = Half3(x, y, z)
        set(value) {
            x = value.x
            y = value.y
            z = value.z
        }
    inline var xyz: Half3
        get() = Half3(x, y, z)
        set(value) {
            x = value.x
            y = value.y
            z = value.z
        }
    inline var stp: Half3
        get() = Half3(x, y, z)
        set(value) {
            x = value.x
            y = value.y
            z = value.z
        }

    operator fun get(index: VectorComponent) = when (index) {
        VectorComponent.X, VectorComponent.R, VectorComponent.S -> x
        VectorComponent.Y, VectorComponent.G, VectorComponent.T -> y
        VectorComponent.Z, VectorComponent.B, VectorComponent.P -> z
        else -> throw IllegalArgumentException("index must be X, Y, Z, R, G, B, S, T or P")
    }

    operator fun get(index1: VectorComponent, index2: VectorComponent): Half2 {
        return Half2(get(index1), get(index2))
    }
    operator fun get(
        index1: VectorComponent, index2: VectorComponent, index3: VectorComponent): Half3 {
        return Half3(get(index1), get(index2), get(index3))
    }

    operator fun get(index: Int) = when (index) {
        0 -> x
        1 -> y
        2 -> z
        else -> throw IllegalArgumentException("index must be in 0..2")
    }

    operator fun get(index1: Int, index2: Int) = Half2(get(index1), get(index2))
    operator fun get(index1: Int, index2: Int, index3: Int): Half3 {
        return Half3(get(index1), get(index2), get(index3))
    }

    inline operator fun invoke(index: Int) = get(index - 1)

    operator fun set(index: Int, v: Half) = when (index) {
        0 -> x = v
        1 -> y = v
        2 -> z = v
        else -> throw IllegalArgumentException("index must be in 0..2")
    }

    operator fun set(index1: Int, index2: Int, v: Half) {
        set(index1, v)
        set(index2, v)
    }

    operator fun set(index1: Int, index2: Int, index3: Int, v: Half) {
        set(index1, v)
        set(index2, v)
        set(index3, v)
    }

    operator fun set(index: VectorComponent, v: Half) = when (index) {
        VectorComponent.X, VectorComponent.R, VectorComponent.S -> x = v
        VectorComponent.Y, VectorComponent.G, VectorComponent.T -> y = v
        VectorComponent.Z, VectorComponent.B, VectorComponent.P -> z = v
        else -> throw IllegalArgumentException("index must be X, Y, Z, R, G, B, S, T or P")
    }

    operator fun set(index1: VectorComponent, index2: VectorComponent, v: Half) {
        set(index1, v)
        set(index2, v)
    }

    operator fun set(
        index1: VectorComponent, index2: VectorComponent, index3: VectorComponent, v: Half) {
        set(index1, v)
        set(index2, v)
        set(index3, v)
    }

    operator fun unaryMinus() = Half3(-x, -y, -z)
    operator fun inc() = Half3(x++, y++, z++)
    operator fun dec() = Half3(x--, y--, z--)

    inline operator fun plus(v: Half) = Half3(x + v, y + v, z + v)
    inline operator fun minus(v: Half) = Half3(x - v, y - v, z - v)
    inline operator fun times(v: Half) = Half3(x * v, y * v, z * v)
    inline operator fun div(v: Half) = Half3(x / v, y / v, z / v)

    inline operator fun plus(v: Half2) = Half3(x + v.x, y + v.y, z)
    inline operator fun minus(v: Half2) = Half3(x - v.x, y - v.y, z)
    inline operator fun times(v: Half2) = Half3(x * v.x, y * v.y, z)
    inline operator fun div(v: Half2) = Half3(x / v.x, y / v.y, z)

    inline operator fun plus(v: Half3) = Half3(x + v.x, y + v.y, z + v.z)
    inline operator fun minus(v: Half3) = Half3(x - v.x, y - v.y, z - v.z)
    inline operator fun times(v: Half3) = Half3(x * v.x, y * v.y, z * v.z)
    inline operator fun div(v: Half3) = Half3(x / v.x, y / v.y, z / v.z)

    inline fun transform(block: (Half) -> Half): Half3 {
        x = block(x)
        y = block(y)
        z = block(z)
        return this
    }

    fun toFloatArray() = floatArrayOf(x.toFloat(), y.toFloat(), z.toFloat())
}

data class Half4(
    var x: Half = Half.POSITIVE_ZERO,
    var y: Half = Half.POSITIVE_ZERO,
    var z: Half = Half.POSITIVE_ZERO,
    var w: Half = Half.POSITIVE_ZERO
) {
    constructor(v: Half) : this(v, v, v, v)
    constructor(v: Half2, z: Half = Half.POSITIVE_ZERO, w: Half = Half.POSITIVE_ZERO) : this(v.x, v.y, z, w)
    constructor(v: Half3, w: Half = Half.POSITIVE_ZERO) : this(v.x, v.y, v.z, w)
    constructor(v: Half4) : this(v.x, v.y, v.z, v.w)

    inline var r: Half
        get() = x
        set(value) {
            x = value
        }
    inline var g: Half
        get() = y
        set(value) {
            y = value
        }
    inline var b: Half
        get() = z
        set(value) {
            z = value
        }
    inline var a: Half
        get() = w
        set(value) {
            w = value
        }

    inline var s: Half
        get() = x
        set(value) {
            x = value
        }
    inline var t: Half
        get() = y
        set(value) {
            y = value
        }
    inline var p: Half
        get() = z
        set(value) {
            z = value
        }
    inline var q: Half
        get() = w
        set(value) {
            w = value
        }

    inline var xy: Half2
        get() = Half2(x, y)
        set(value) {
            x = value.x
            y = value.y
        }
    inline var rg: Half2
        get() = Half2(x, y)
        set(value) {
            x = value.x
            y = value.y
        }
    inline var st: Half2
        get() = Half2(x, y)
        set(value) {
            x = value.x
            y = value.y
        }

    inline var rgb: Half3
        get() = Half3(x, y, z)
        set(value) {
            x = value.x
            y = value.y
            z = value.z
        }
    inline var xyz: Half3
        get() = Half3(x, y, z)
        set(value) {
            x = value.x
            y = value.y
            z = value.z
        }
    inline var stp: Half3
        get() = Half3(x, y, z)
        set(value) {
            x = value.x
            y = value.y
            z = value.z
        }

    inline var rgba: Half4
        get() = Half4(x, y, z, w)
        set(value) {
            x = value.x
            y = value.y
            z = value.z
            w = value.w
        }
    inline var xyzw: Half4
        get() = Half4(x, y, z, w)
        set(value) {
            x = value.x
            y = value.y
            z = value.z
            w = value.w
        }
    inline var stpq: Half4
        get() = Half4(x, y, z, w)
        set(value) {
            x = value.x
            y = value.y
            z = value.z
            w = value.w
        }

    operator fun get(index: VectorComponent) = when (index) {
        VectorComponent.X, VectorComponent.R, VectorComponent.S -> x
        VectorComponent.Y, VectorComponent.G, VectorComponent.T -> y
        VectorComponent.Z, VectorComponent.B, VectorComponent.P -> z
        VectorComponent.W, VectorComponent.A, VectorComponent.Q -> w
    }

    operator fun get(index1: VectorComponent, index2: VectorComponent): Half2 {
        return Half2(get(index1), get(index2))
    }
    operator fun get(
        index1: VectorComponent,
        index2: VectorComponent,
        index3: VectorComponent): Half3 {
        return Half3(get(index1), get(index2), get(index3))
    }
    operator fun get(
        index1: VectorComponent,
        index2: VectorComponent,
        index3: VectorComponent,
        index4: VectorComponent): Half4 {
        return Half4(get(index1), get(index2), get(index3), get(index4))
    }

    operator fun get(index: Int) = when (index) {
        0 -> x
        1 -> y
        2 -> z
        3 -> w
        else -> throw IllegalArgumentException("index must be in 0..3")
    }

    operator fun get(index1: Int, index2: Int) = Half2(get(index1), get(index2))
    operator fun get(index1: Int, index2: Int, index3: Int): Half3 {
        return Half3(get(index1), get(index2), get(index3))
    }
    operator fun get(index1: Int, index2: Int, index3: Int, index4: Int): Half4 {
        return Half4(get(index1), get(index2), get(index3), get(index4))
    }

    inline operator fun invoke(index: Int) = get(index - 1)

    operator fun set(index: Int, v: Half) = when (index) {
        0 -> x = v
        1 -> y = v
        2 -> z = v
        3 -> w = v
        else -> throw IllegalArgumentException("index must be in 0..3")
    }

    operator fun set(index1: Int, index2: Int, v: Half) {
        set(index1, v)
        set(index2, v)
    }

    operator fun set(index1: Int, index2: Int, index3: Int, v: Half) {
        set(index1, v)
        set(index2, v)
        set(index3, v)
    }

    operator fun set(index1: Int, index2: Int, index3: Int, index4: Int, v: Half) {
        set(index1, v)
        set(index2, v)
        set(index3, v)
        set(index4, v)
    }

    operator fun set(index: VectorComponent, v: Half) = when (index) {
        VectorComponent.X, VectorComponent.R, VectorComponent.S -> x = v
        VectorComponent.Y, VectorComponent.G, VectorComponent.T -> y = v
        VectorComponent.Z, VectorComponent.B, VectorComponent.P -> z = v
        VectorComponent.W, VectorComponent.A, VectorComponent.Q -> w = v
    }

    operator fun set(index1: VectorComponent, index2: VectorComponent, v: Half) {
        set(index1, v)
        set(index2, v)
    }

    operator fun set(
        index1: VectorComponent, index2: VectorComponent, index3: VectorComponent, v: Half) {
        set(index1, v)
        set(index2, v)
        set(index3, v)
    }

    operator fun set(
        index1: VectorComponent, index2: VectorComponent,
        index3: VectorComponent, index4: VectorComponent, v: Half) {
        set(index1, v)
        set(index2, v)
        set(index3, v)
        set(index4, v)
    }

    operator fun unaryMinus() = Half4(-x, -y, -z, -w)
    operator fun inc() = Half4(x++, y++, z++, w++)
    operator fun dec() = Half4(x--, y--, z--, w--)

    inline operator fun plus(v: Half) = Half4(x + v, y + v, z + v, w + v)
    inline operator fun minus(v: Half) = Half4(x - v, y - v, z - v, w - v)
    inline operator fun times(v: Half) = Half4(x * v, y * v, z * v, w * v)
    inline operator fun div(v: Half) = Half4(x / v, y / v, z / v, w / v)

    inline operator fun plus(v: Half2) = Half4(x + v.x, y + v.y, z, w)
    inline operator fun minus(v: Half2) = Half4(x - v.x, y - v.y, z, w)
    inline operator fun times(v: Half2) = Half4(x * v.x, y * v.y, z, w)
    inline operator fun div(v: Half2) = Half4(x / v.x, y / v.y, z, w)

    inline operator fun plus(v: Half3) = Half4(x + v.x, y + v.y, z + v.z, w)
    inline operator fun minus(v: Half3) = Half4(x - v.x, y - v.y, z - v.z, w)
    inline operator fun times(v: Half3) = Half4(x * v.x, y * v.y, z * v.z, w)
    inline operator fun div(v: Half3) = Half4(x / v.x, y / v.y, z / v.z, w)

    inline operator fun plus(v: Half4) = Half4(x + v.x, y + v.y, z + v.z, w + v.w)
    inline operator fun minus(v: Half4) = Half4(x - v.x, y - v.y, z - v.z, w - v.w)
    inline operator fun times(v: Half4) = Half4(x * v.x, y * v.y, z * v.z, w * v.w)
    inline operator fun div(v: Half4) = Half4(x / v.x, y / v.y, z / v.z, w / v.w)

    inline fun transform(block: (Half) -> Half): Half4 {
        x = block(x)
        y = block(y)
        z = block(z)
        w = block(w)
        return this
    }

    fun toFloatArray() = floatArrayOf(x.toFloat(), y.toFloat(), z.toFloat(), w.toFloat())
}

inline fun min(v: Half2) = min(v.x, v.y)
inline fun min(a: Half2, b: Half2) = Half2(min(a.x, b.x), min(a.y, b.y))
inline fun max(v: Half2) = max(v.x, v.y)
inline fun max(a: Half2, b: Half2) = Half2(max(a.x, b.x), max(a.y, b.y))

inline fun transform(v: Half2, block: (Half) -> Half) = v.copy().transform(block)

inline fun lessThan(a: Half2, b: Half) = Bool2(a.x < b, a.y < b)
inline fun lessThan(a: Half2, b: Half2) = Bool2(a.x < b.x, a.y < b.y)
inline fun lessThanEqual(a: Half2, b: Half) = Bool2(a.x <= b, a.y <= b)
inline fun lessThanEqual(a: Half2, b: Half2) = Bool2(a.x <= b.x, a.y <= b.y)
inline fun greaterThan(a: Half2, b: Half) = Bool2(a.x > b, a.y > b)
inline fun greaterThan(a: Half2, b: Half2) = Bool2(a.x > b.y, a.y > b.y)
inline fun greaterThanEqual(a: Half2, b: Half) = Bool2(a.x >= b, a.y >= b)
inline fun greaterThanEqual(a: Half2, b: Half2) = Bool2(a.x >= b.x, a.y >= b.y)
inline fun equal(a: Half2, b: Half) = Bool2(a.x == b, a.y == b)
inline fun equal(a: Half2, b: Half2) = Bool2(a.x == b.x, a.y == b.y)
inline fun notEqual(a: Half2, b: Half) = Bool2(a.x != b, a.y != b)
inline fun notEqual(a: Half2, b: Half2) = Bool2(a.x != b.x, a.y != b.y)

inline infix fun Half2.lt(b: Half) = Bool2(x < b, y < b)
inline infix fun Half2.lt(b: Half2) = Bool2(x < b.x, y < b.y)
inline infix fun Half2.lte(b: Half) = Bool2(x <= b, y <= b)
inline infix fun Half2.lte(b: Half2) = Bool2(x <= b.x, y <= b.y)
inline infix fun Half2.gt(b: Half) = Bool2(x > b, y > b)
inline infix fun Half2.gt(b: Half2) = Bool2(x > b.x, y > b.y)
inline infix fun Half2.gte(b: Half) = Bool2(x >= b, y >= b)
inline infix fun Half2.gte(b: Half2) = Bool2(x >= b.x, y >= b.y)
inline infix fun Half2.eq(b: Half) = Bool2(x == b, y == b)
inline infix fun Half2.eq(b: Half2) = Bool2(x == b.x, y == b.y)
inline infix fun Half2.neq(b: Half) = Bool2(x != b, y != b)
inline infix fun Half2.neq(b: Half2) = Bool2(x != b.x, y != b.y)

inline operator fun Half.plus(v: Half3) = Half3(this + v.x, this + v.y, this + v.z)
inline operator fun Half.minus(v: Half3) = Half3(this - v.x, this - v.y, this - v.z)
inline operator fun Half.times(v: Half3) = Half3(this * v.x, this * v.y, this * v.z)
inline operator fun Half.div(v: Half3) = Half3(this / v.x, this / v.y, this / v.z)

inline fun abs(v: Half3) = Half3(abs(v.x), abs(v.y), abs(v.z))
inline fun length(v: Half3) = sqrt(v.x * v.x + v.y * v.y + v.z * v.z)
inline fun length2(v: Half3) = v.x * v.x + v.y * v.y + v.z * v.z
inline fun distance(a: Half3, b: Half3) = length(a - b)
inline fun dot(a: Half3, b: Half3) = a.x * b.x + a.y * b.y + a.z * b.z
inline fun cross(a: Half3, b: Half3): Half3 {
    return Half3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x)
}
inline infix fun Half3.x(v: Half3): Half3 {
    return Half3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x)
}
fun normalize(v: Half3): Half3 {
    val l = HALF_ONE / length(v)
    return Half3(v.x * l, v.y * l, v.z * l)
}

inline fun reflect(i: Half3, n: Half3) = i - HALF_TWO * dot(n, i) * n
fun refract(i: Half3, n: Half3, eta: Half): Half3 {
    val d = dot(n, i)
    val k = HALF_ONE - eta * eta * (HALF_ONE - sqr(d))
    return if (k < Half.POSITIVE_ZERO) Half3() else eta * i - (eta * d + sqrt(k)) * n
}

inline fun clamp(v: Half3, min: Half, max: Half): Half3 {
    return Half3(
        clamp(v.x, min, max),
        clamp(v.y, min, max),
        clamp(v.z, min, max)
    )
}

inline fun clamp(v: Half3, min: Half3, max: Half3): Half3 {
    return Half3(
        clamp(v.x, min.x, max.x),
        clamp(v.y, min.y, max.y),
        clamp(v.z, min.z, max.z)
    )
}

inline fun mix(a: Half3, b: Half3, x: Half): Half3 {
    return Half3(
        mix(a.x, b.x, x),
        mix(a.y, b.y, x),
        mix(a.z, b.z, x)
    )
}

inline fun mix(a: Half3, b: Half3, x: Half3): Half3 {
    return Half3(
        mix(a.x, b.x, x.x),
        mix(a.y, b.y, x.y),
        mix(a.z, b.z, x.z)
    )
}

inline fun min(v: Half3) = min(v.x, min(v.y, v.z))
inline fun min(a: Half3, b: Half3) = Half3(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z))
inline fun max(v: Half3) = max(v.x, max(v.y, v.z))
inline fun max(a: Half3, b: Half3) = Half3(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z))

inline fun transform(v: Half3, block: (Half) -> Half) = v.copy().transform(block)

inline fun lessThan(a: Half3, b: Half) = Bool3(a.x < b, a.y < b, a.z < b)
inline fun lessThan(a: Half3, b: Half3) = Bool3(a.x < b.x, a.y < b.y, a.z < b.z)
inline fun lessThanEqual(a: Half3, b: Half) = Bool3(a.x <= b, a.y <= b, a.z <= b)
inline fun lessThanEqual(a: Half3, b: Half3) = Bool3(a.x <= b.x, a.y <= b.y, a.z <= b.z)
inline fun greaterThan(a: Half3, b: Half) = Bool3(a.x > b, a.y > b, a.z > b)
inline fun greaterThan(a: Half3, b: Half3) = Bool3(a.x > b.y, a.y > b.y, a.z > b.z)
inline fun greaterThanEqual(a: Half3, b: Half) = Bool3(a.x >= b, a.y >= b, a.z >= b)
inline fun greaterThanEqual(a: Half3, b: Half3) = Bool3(a.x >= b.x, a.y >= b.y, a.z >= b.z)
inline fun equal(a: Half3, b: Half) = Bool3(a.x == b, a.y == b, a.z == b)
inline fun equal(a: Half3, b: Half3) = Bool3(a.x == b.x, a.y == b.y, a.z == b.z)
inline fun notEqual(a: Half3, b: Half) = Bool3(a.x != b, a.y != b, a.z != b)
inline fun notEqual(a: Half3, b: Half3) = Bool3(a.x != b.x, a.y != b.y, a.z != b.z)

inline infix fun Half3.lt(b: Half) = Bool3(x < b, y < b, z < b)
inline infix fun Half3.lt(b: Half3) = Bool3(x < b.x, y < b.y, z < b.z)
inline infix fun Half3.lte(b: Half) = Bool3(x <= b, y <= b, z <= b)
inline infix fun Half3.lte(b: Half3) = Bool3(x <= b.x, y <= b.y, z <= b.z)
inline infix fun Half3.gt(b: Half) = Bool3(x > b, y > b, z > b)
inline infix fun Half3.gt(b: Half3) = Bool3(x > b.x, y > b.y, z > b.z)
inline infix fun Half3.gte(b: Half) = Bool3(x >= b, y >= b, z >= b)
inline infix fun Half3.gte(b: Half3) = Bool3(x >= b.x, y >= b.y, z >= b.z)
inline infix fun Half3.eq(b: Half) = Bool3(x == b, y == b, z == b)
inline infix fun Half3.eq(b: Half3) = Bool3(x == b.x, y == b.y, z == b.z)
inline infix fun Half3.neq(b: Half) = Bool3(x != b, y != b, z != b)
inline infix fun Half3.neq(b: Half3) = Bool3(x != b.x, y != b.y, z != b.z)

inline operator fun Half.plus(v: Half4) = Half4(this + v.x, this + v.y, this + v.z, this + v.w)
inline operator fun Half.minus(v: Half4) = Half4(this - v.x, this - v.y, this - v.z, this - v.w)
inline operator fun Half.times(v: Half4) = Half4(this * v.x, this * v.y, this * v.z, this * v.w)
inline operator fun Half.div(v: Half4) = Half4(this / v.x, this / v.y, this / v.z, this / v.w)

inline fun abs(v: Half4) = Half4(abs(v.x), abs(v.y), abs(v.z), abs(v.w))
inline fun length(v: Half4) = sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w)
inline fun length2(v: Half4) = v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w
inline fun distance(a: Half4, b: Half4) = length(a - b)
inline fun dot(a: Half4, b: Half4) = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w
fun normalize(v: Half4): Half4 {
    val l = HALF_ONE / length(v)
    return Half4(v.x * l, v.y * l, v.z * l, v.w * l)
}

inline fun clamp(v: Half4, min: Half, max: Half): Half4 {
    return Half4(
        clamp(v.x, min, max),
        clamp(v.y, min, max),
        clamp(v.z, min, max),
        clamp(v.w, min, max)
    )
}

inline fun clamp(v: Half4, min: Half4, max: Half4): Half4 {
    return Half4(
        clamp(v.x, min.x, max.x),
        clamp(v.y, min.y, max.y),
        clamp(v.z, min.z, max.z),
        clamp(v.w, min.z, max.w)
    )
}

inline fun mix(a: Half4, b: Half4, x: Half): Half4 {
    return Half4(
        mix(a.x, b.x, x),
        mix(a.y, b.y, x),
        mix(a.z, b.z, x),
        mix(a.w, b.w, x)
    )
}

inline fun mix(a: Half4, b: Half4, x: Half4): Half4 {
    return Half4(
        mix(a.x, b.x, x.x),
        mix(a.y, b.y, x.y),
        mix(a.z, b.z, x.z),
        mix(a.w, b.w, x.w))
}

inline fun min(v: Half4) = min(v.x, min(v.y, min(v.z, v.w)))
inline fun min(a: Half4, b: Half4): Half4 {
    return Half4(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z), min(a.w, b.w))
}
inline fun max(v: Half4) = max(v.x, max(v.y, max(v.z, v.w)))
inline fun max(a: Half4, b: Half4): Half4 {
    return Half4(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z), max(a.w, b.w))
}

inline fun transform(v: Half4, block: (Half) -> Half) = v.copy().transform(block)

inline fun lessThan(a: Half4, b: Half) = Bool4(a.x < b, a.y < b, a.z < b, a.w < b)
inline fun lessThan(a: Half4, b: Half4) = Bool4(a.x < b.x, a.y < b.y, a.z < b.z, a.w < b.w)
inline fun lessThanEqual(a: Half4, b: Half) = Bool4(a.x <= b, a.y <= b, a.z <= b, a.w <= b)
inline fun lessThanEqual(a: Half4, b: Half4) = Bool4(a.x <= b.x, a.y <= b.y, a.z <= b.z, a.w <= b.w)
inline fun greaterThan(a: Half4, b: Half) = Bool4(a.x > b, a.y > b, a.z > b, a.w > b)
inline fun greaterThan(a: Half4, b: Half4) = Bool4(a.x > b.y, a.y > b.y, a.z > b.z, a.w > b.w)
inline fun greaterThanEqual(a: Half4, b: Half) = Bool4(a.x >= b, a.y >= b, a.z >= b, a.w >= b)
inline fun greaterThanEqual(a: Half4, b: Half4) = Bool4(a.x >= b.x, a.y >= b.y, a.z >= b.z, a.w >= b.w)
inline fun equal(a: Half4, b: Half) = Bool4(a.x == b, a.y == b, a.z == b, a.w == b)
inline fun equal(a: Half4, b: Half4) = Bool4(a.x == b.x, a.y == b.y, a.z == b.z, a.w == b.w)
inline fun notEqual(a: Half4, b: Half) = Bool4(a.x != b, a.y != b, a.z != b, a.w != b)
inline fun notEqual(a: Half4, b: Half4) = Bool4(a.x != b.x, a.y != b.y, a.z != b.z, a.w != b.w)

inline infix fun Half4.lt(b: Half) = Bool4(x < b, y < b, z < b, a < b)
inline infix fun Half4.lt(b: Half4) = Bool4(x < b.x, y < b.y, z < b.z, w < b.w)
inline infix fun Half4.lte(b: Half) = Bool4(x <= b, y <= b, z <= b, w <= b)
inline infix fun Half4.lte(b: Half4) = Bool4(x <= b.x, y <= b.y, z <= b.z, w <= b.w)
inline infix fun Half4.gt(b: Half) = Bool4(x > b, y > b, z > b, w > b)
inline infix fun Half4.gt(b: Half4) = Bool4(x > b.x, y > b.y, z > b.z, w > b.w)
inline infix fun Half4.gte(b: Half) = Bool4(x >= b, y >= b, z >= b, w >= b)
inline infix fun Half4.gte(b: Half4) = Bool4(x >= b.x, y >= b.y, z >= b.z, w >= b.w)
inline infix fun Half4.eq(b: Half) = Bool4(x == b, y == b, z == b, w == b)
inline infix fun Half4.eq(b: Half4) = Bool4(x == b.x, y == b.y, z == b.z, w == b.w)
inline infix fun Half4.neq(b: Half) = Bool4(x != b, y != b, z != b, w != b)
inline infix fun Half4.neq(b: Half4) = Bool4(x != b.x, y != b.y, z != b.z, w != b.w)
