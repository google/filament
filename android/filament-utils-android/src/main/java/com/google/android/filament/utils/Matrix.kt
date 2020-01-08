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

package com.google.android.filament.utils

import kotlin.math.*

enum class MatrixColumn {
    X, Y, Z, W
}

data class Mat2(
        var x: Float2 = Float2(x = 1.0f),
        var y: Float2 = Float2(y = 1.0f)) {
    constructor(m: Mat2) : this(m.x.copy(), m.y.copy())

    companion object {
        fun of(vararg a: Float): Mat2 {
            require(a.size >= 4)
            return Mat2(
                    Float2(a[0], a[2]),
                    Float2(a[1], a[3])
            )
        }

        fun identity() = Mat2()
    }

    operator fun get(column: Int) = when(column) {
        0 -> x
        1 -> y
        else -> throw IllegalArgumentException("column must be in 0..1")
    }
    operator fun get(column: Int, row: Int) = get(column)[row]

    operator fun get(column: MatrixColumn) = when(column) {
        MatrixColumn.X -> x
        MatrixColumn.Y -> y
        else -> throw IllegalArgumentException("column must be X or Y")
    }
    operator fun get(column: MatrixColumn, row: Int) = get(column)[row]

    operator fun invoke(row: Int, column: Int) = get(column - 1)[row - 1]
    operator fun invoke(row: Int, column: Int, v: Float) = set(column - 1, row - 1, v)

    operator fun set(column: Int, v: Float2) {
        this[column].xy = v
    }
    operator fun set(column: Int, row: Int, v: Float) {
        this[column][row] = v
    }

    operator fun unaryMinus() = Mat2(-x, -y)
    operator fun inc(): Mat2 {
        x++
        y++
        return this
    }
    operator fun dec(): Mat2 {
        x--
        y--
        return this
    }

    operator fun plus(v: Float) = Mat2(x + v, y + v)
    operator fun minus(v: Float) = Mat2(x - v, y - v)
    operator fun times(v: Float) = Mat2(x * v, y * v)
    operator fun div(v: Float) = Mat2(x / v, y / v)

    operator fun times(m: Mat2): Mat2 {
        val t = transpose(this)
        return Mat2(
                Float2(dot(t.x, m.x), dot(t.y, m.x)),
                Float2(dot(t.x, m.y), dot(t.y, m.y))
        )
    }

    operator fun times(v: Float2): Float2 {
        val t = transpose(this)
        return Float2(dot(t.x, v), dot(t.y, v))
    }

    fun toFloatArray() = floatArrayOf(
            x.x, y.x,
            x.y, y.y
    )

    override fun toString(): String {
        return """
            |${x.x} ${y.x}|
            |${x.y} ${y.y}|
            """.trimIndent()
    }

}

data class Mat3(
        var x: Float3 = Float3(x = 1.0f),
        var y: Float3 = Float3(y = 1.0f),
        var z: Float3 = Float3(z = 1.0f)) {
    constructor(m: Mat3) : this(m.x.copy(), m.y.copy(), m.z.copy())

    companion object {
        fun of(vararg a: Float): Mat3 {
            require(a.size >= 9)
            return Mat3(
                    Float3(a[0], a[3], a[6]),
                    Float3(a[1], a[4], a[7]),
                    Float3(a[2], a[5], a[8])
            )
        }

        fun identity() = Mat3()
    }

    operator fun get(column: Int) = when(column) {
        0 -> x
        1 -> y
        2 -> z
        else -> throw IllegalArgumentException("column must be in 0..2")
    }
    operator fun get(column: Int, row: Int) = get(column)[row]

    operator fun get(column: MatrixColumn) = when(column) {
        MatrixColumn.X -> x
        MatrixColumn.Y -> y
        MatrixColumn.Z -> z
        else -> throw IllegalArgumentException("column must be X, Y or Z")
    }
    operator fun get(column: MatrixColumn, row: Int) = get(column)[row]

    operator fun invoke(row: Int, column: Int) = get(column - 1)[row - 1]
    operator fun invoke(row: Int, column: Int, v: Float) = set(column - 1, row - 1, v)

    operator fun set(column: Int, v: Float3) {
        this[column].xyz = v
    }
    operator fun set(column: Int, row: Int, v: Float) {
        this[column][row] = v
    }

    operator fun unaryMinus() = Mat3(-x, -y, -z)
    operator fun inc(): Mat3 {
        x++
        y++
        z++
        return this
    }
    operator fun dec(): Mat3 {
        x--
        y--
        z--
        return this
    }

    operator fun plus(v: Float) = Mat3(x + v, y + v, z + v)
    operator fun minus(v: Float) = Mat3(x - v, y - v, z - v)
    operator fun times(v: Float) = Mat3(x * v, y * v, z * v)
    operator fun div(v: Float) = Mat3(x / v, y / v, z / v)

    operator fun times(m: Mat3): Mat3 {
        val t = transpose(this)
        return Mat3(
                Float3(dot(t.x, m.x), dot(t.y, m.x), dot(t.z, m.x)),
                Float3(dot(t.x, m.y), dot(t.y, m.y), dot(t.z, m.y)),
                Float3(dot(t.x, m.z), dot(t.y, m.z), dot(t.z, m.z))
        )
    }

    operator fun times(v: Float3): Float3 {
        val t = transpose(this)
        return Float3(dot(t.x, v), dot(t.y, v), dot(t.z, v))
    }

    fun toFloatArray() = floatArrayOf(
            x.x, y.x, z.x,
            x.y, y.y, z.y,
            x.z, y.z, z.z
    )

    override fun toString(): String {
        return """
            |${x.x} ${y.x} ${z.x}|
            |${x.y} ${y.y} ${z.y}|
            |${x.z} ${y.z} ${z.z}|
            """.trimIndent()
    }
}

data class Mat4(
        var x: Float4 = Float4(x = 1.0f),
        var y: Float4 = Float4(y = 1.0f),
        var z: Float4 = Float4(z = 1.0f),
        var w: Float4 = Float4(w = 1.0f)) {
    constructor(right: Float3, up: Float3, forward: Float3, position: Float3 = Float3()) :
            this(Float4(right), Float4(up), Float4(forward), Float4(position, 1.0f))
    constructor(m: Mat4) : this(m.x.copy(), m.y.copy(), m.z.copy(), m.w.copy())

    companion object {
        fun of(vararg a: Float): Mat4 {
            require(a.size >= 16)
            return Mat4(
                    Float4(a[0], a[4], a[8],  a[12]),
                    Float4(a[1], a[5], a[9],  a[13]),
                    Float4(a[2], a[6], a[10], a[14]),
                    Float4(a[3], a[7], a[11], a[15])
            )
        }

        fun identity() = Mat4()
    }

    inline var right: Float3
        get() = x.xyz
        set(value) {
            x.xyz = value
        }
    inline var up: Float3
        get() = y.xyz
        set(value) {
            y.xyz = value
        }
    inline var forward: Float3
        get() = z.xyz
        set(value) {
            z.xyz = value
        }
    inline var position: Float3
        get() = w.xyz
        set(value) {
            w.xyz = value
        }

    inline val scale: Float3
        get() = Float3(length(x.xyz), length(y.xyz), length(z.xyz))
    inline val translation: Float3
        get() = w.xyz
    val rotation: Float3
        get() {
            val x = normalize(right)
            val y = normalize(up)
            val z = normalize(forward)

            return when {
                z.y <= -1.0f -> Float3(degrees(-HALF_PI), 0.0f, degrees(atan2( x.z,  y.z)))
                z.y >=  1.0f -> Float3(degrees( HALF_PI), 0.0f, degrees(atan2(-x.z, -y.z)))
                else -> Float3(
                        degrees(-asin(z.y)), degrees(-atan2(z.x, z.z)), degrees(atan2( x.y,  y.y)))
            }
        }

    inline val upperLeft: Mat3
        get() = Mat3(x.xyz, y.xyz, z.xyz)

    operator fun get(column: Int) = when(column) {
        0 -> x
        1 -> y
        2 -> z
        3 -> w
        else -> throw IllegalArgumentException("column must be in 0..3")
    }
    operator fun get(column: Int, row: Int) = get(column)[row]

    operator fun get(column: MatrixColumn) = when(column) {
        MatrixColumn.X -> x
        MatrixColumn.Y -> y
        MatrixColumn.Z -> z
        MatrixColumn.W -> w
    }
    operator fun get(column: MatrixColumn, row: Int) = get(column)[row]

    operator fun invoke(row: Int, column: Int) = get(column - 1)[row - 1]
    operator fun invoke(row: Int, column: Int, v: Float) = set(column - 1, row - 1, v)

    operator fun set(column: Int, v: Float4) {
        this[column].xyzw = v
    }
    operator fun set(column: Int, row: Int, v: Float) {
        this[column][row] = v
    }

    operator fun unaryMinus() = Mat4(-x, -y, -z, -w)
    operator fun inc(): Mat4 {
        x++
        y++
        z++
        w++
        return this
    }
    operator fun dec(): Mat4 {
        x--
        y--
        z--
        w--
        return this
    }

    operator fun plus(v: Float) = Mat4(x + v, y + v, z + v, w + v)
    operator fun minus(v: Float) = Mat4(x - v, y - v, z - v, w - v)
    operator fun times(v: Float) = Mat4(x * v, y * v, z * v, w * v)
    operator fun div(v: Float) = Mat4(x / v, y / v, z / v, w / v)

    operator fun times(m: Mat4): Mat4 {
        val t = transpose(this)
        return Mat4(
                Float4(dot(t.x, m.x), dot(t.y, m.x), dot(t.z, m.x), dot(t.w, m.x)),
                Float4(dot(t.x, m.y), dot(t.y, m.y), dot(t.z, m.y), dot(t.w, m.y)),
                Float4(dot(t.x, m.z), dot(t.y, m.z), dot(t.z, m.z), dot(t.w, m.z)),
                Float4(dot(t.x, m.w), dot(t.y, m.w), dot(t.z, m.w), dot(t.w, m.w))
        )
    }

    operator fun times(v: Float4): Float4 {
        val t = transpose(this)
        return Float4(dot(t.x, v), dot(t.y, v), dot(t.z, v), dot(t.w, v))
    }

    fun toFloatArray() = floatArrayOf(
            x.x, y.x, z.x, w.x,
            x.y, y.y, z.y, w.y,
            x.z, y.z, z.z, w.z,
            x.w, y.w, z.w, w.w
    )

    override fun toString(): String {
        return """
            |${x.x} ${y.x} ${z.x} ${w.x}|
            |${x.y} ${y.y} ${z.y} ${w.y}|
            |${x.z} ${y.z} ${z.z} ${w.z}|
            |${x.w} ${y.w} ${z.w} ${w.w}|
            """.trimIndent()
    }
}

fun transpose(m: Mat2) = Mat2(
        Float2(m.x.x, m.y.x),
        Float2(m.x.y, m.y.y)
)

fun transpose(m: Mat3) = Mat3(
        Float3(m.x.x, m.y.x, m.z.x),
        Float3(m.x.y, m.y.y, m.z.y),
        Float3(m.x.z, m.y.z, m.z.z)
)
fun inverse(m: Mat3): Mat3 {
    val a = m.x.x
    val b = m.x.y
    val c = m.x.z
    val d = m.y.x
    val e = m.y.y
    val f = m.y.z
    val g = m.z.x
    val h = m.z.y
    val i = m.z.z

    val A = e * i - f * h
    val B = f * g - d * i
    val C = d * h - e * g

    val det = a * A + b * B + c * C

    return Mat3.of(
            A / det,               B / det,               C / det,
            (c * h - b * i) / det, (a * i - c * g) / det, (b * g - a * h) / det,
            (b * f - c * e) / det, (c * d - a * f) / det, (a * e - b * d) / det
    )
}

fun transpose(m: Mat4) = Mat4(
        Float4(m.x.x, m.y.x, m.z.x, m.w.x),
        Float4(m.x.y, m.y.y, m.z.y, m.w.y),
        Float4(m.x.z, m.y.z, m.z.z, m.w.z),
        Float4(m.x.w, m.y.w, m.z.w, m.w.w)
)
fun inverse(m: Mat4): Mat4 {
    val result = Mat4()

    var pair0  = m.z.z * m.w.w
    var pair1  = m.w.z * m.z.w
    var pair2  = m.y.z * m.w.w
    var pair3  = m.w.z * m.y.w
    var pair4  = m.y.z * m.z.w
    var pair5  = m.z.z * m.y.w
    var pair6  = m.x.z * m.w.w
    var pair7  = m.w.z * m.x.w
    var pair8  = m.x.z * m.z.w
    var pair9  = m.z.z * m.x.w
    var pair10 = m.x.z * m.y.w
    var pair11 = m.y.z * m.x.w

    result.x.x  = pair0 * m.y.y + pair3 * m.z.y + pair4 * m.w.y
    result.x.x -= pair1 * m.y.y + pair2 * m.z.y + pair5 * m.w.y
    result.x.y  = pair1 * m.x.y + pair6 * m.z.y + pair9 * m.w.y
    result.x.y -= pair0 * m.x.y + pair7 * m.z.y + pair8 * m.w.y
    result.x.z  = pair2 * m.x.y + pair7 * m.y.y + pair10 * m.w.y
    result.x.z -= pair3 * m.x.y + pair6 * m.y.y + pair11 * m.w.y
    result.x.w  = pair5 * m.x.y + pair8 * m.y.y + pair11 * m.z.y
    result.x.w -= pair4 * m.x.y + pair9 * m.y.y + pair10 * m.z.y
    result.y.x  = pair1 * m.y.x + pair2 * m.z.x + pair5 * m.w.x
    result.y.x -= pair0 * m.y.x + pair3 * m.z.x + pair4 * m.w.x
    result.y.y  = pair0 * m.x.x + pair7 * m.z.x + pair8 * m.w.x
    result.y.y -= pair1 * m.x.x + pair6 * m.z.x + pair9 * m.w.x
    result.y.z  = pair3 * m.x.x + pair6 * m.y.x + pair11 * m.w.x
    result.y.z -= pair2 * m.x.x + pair7 * m.y.x + pair10 * m.w.x
    result.y.w  = pair4 * m.x.x + pair9 * m.y.x + pair10 * m.z.x
    result.y.w -= pair5 * m.x.x + pair8 * m.y.x + pair11 * m.z.x

    pair0 = m.z.x * m.w.y
    pair1 = m.w.x * m.z.y
    pair2 = m.y.x * m.w.y
    pair3 = m.w.x * m.y.y
    pair4 = m.y.x * m.z.y
    pair5 = m.z.x * m.y.y
    pair6 = m.x.x * m.w.y
    pair7 = m.w.x * m.x.y
    pair8 = m.x.x * m.z.y
    pair9 = m.z.x * m.x.y
    pair10 = m.x.x * m.y.y
    pair11 = m.y.x * m.x.y

    result.z.x  = pair0 * m.y.w + pair3 * m.z.w + pair4 * m.w.w
    result.z.x -= pair1 * m.y.w + pair2 * m.z.w + pair5 * m.w.w
    result.z.y  = pair1 * m.x.w + pair6 * m.z.w + pair9 * m.w.w
    result.z.y -= pair0 * m.x.w + pair7 * m.z.w + pair8 * m.w.w
    result.z.z  = pair2 * m.x.w + pair7 * m.y.w + pair10 * m.w.w
    result.z.z -= pair3 * m.x.w + pair6 * m.y.w + pair11 * m.w.w
    result.z.w  = pair5 * m.x.w + pair8 * m.y.w + pair11 * m.z.w
    result.z.w -= pair4 * m.x.w + pair9 * m.y.w + pair10 * m.z.w
    result.w.x  = pair2 * m.z.z + pair5 * m.w.z + pair1 * m.y.z
    result.w.x -= pair4 * m.w.z + pair0 * m.y.z + pair3 * m.z.z
    result.w.y  = pair8 * m.w.z + pair0 * m.x.z + pair7 * m.z.z
    result.w.y -= pair6 * m.z.z + pair9 * m.w.z + pair1 * m.x.z
    result.w.z  = pair6 * m.y.z + pair11 * m.w.z + pair3 * m.x.z
    result.w.z -= pair10 * m.w.z + pair2 * m.x.z + pair7 * m.y.z
    result.w.w  = pair10 * m.z.z + pair4 * m.x.z + pair9 * m.y.z
    result.w.w -= pair8 * m.y.z + pair11 * m.z.z + pair5 * m.x.z

    val determinant = m.x.x * result.x.x + m.y.x * result.x.y + m.z.x * result.x.z + m.w.x * result.x.w

    return result / determinant
}

fun scale(s: Float3) = Mat4(Float4(x = s.x), Float4(y = s.y), Float4(z = s.z))
fun scale(m: Mat4) = scale(m.scale)

fun translation(t: Float3) = Mat4(w = Float4(t, 1.0f))
fun translation(m: Mat4) = translation(m.translation)

fun rotation(m: Mat4) = Mat4(normalize(m.right), normalize(m.up), normalize(m.forward))
fun rotation(d: Float3): Mat4 {
    val r = transform(d, ::radians)
    val c = transform(r, { x -> cos(x) })
    val s = transform(r, { x -> sin(x) })

    return Mat4.of(
             c.y * c.z, -c.x * s.z + s.x * s.y * c.z,  s.x * s.z + c.x * s.y * c.z, 0.0f,
             c.y * s.z,  c.x * c.z + s.x * s.y * s.z, -s.x * c.z + c.x * s.y * s.z, 0.0f,
            -s.y      ,  s.x * c.y                  ,  c.x * c.y                  , 0.0f,
             0.0f     ,  0.0f                       ,  0.0f                       , 1.0f
    )
}
fun rotation(axis: Float3, angle: Float): Mat4 {
    val x = axis.x
    val y = axis.y
    val z = axis.z

    val r = radians(angle)
    val c = cos(r)
    val s = sin(r)
    val d = 1.0f - c

    return Mat4.of(
            x * x * d + c    , x * y * d - z * s, x * z * d + y * s, 0.0f,
            y * x * d + z * s, y * y * d + c    , y * z * d - x * s, 0.0f,
            z * x * d - y * s, z * y * d + x * s, z * z * d + c    , 0.0f,
            0.0f             , 0.0f             , 0.0f             , 1.0f
    )
}

fun normal(m: Mat4) = scale(1.0f / Float3(length2(m.right), length2(m.up), length2(m.forward))) * m

fun lookAt(eye: Float3, target: Float3, up: Float3 = Float3(z = 1.0f)): Mat4 {
    return lookTowards(eye, target - eye, up)
}

fun lookTowards(eye: Float3, forward: Float3, up: Float3 = Float3(z = 1.0f)): Mat4 {
    val f = normalize(forward)
    val r = normalize(f x up)
    val u = normalize(r x f)
    return Mat4(Float4(r), Float4(u), Float4(f), Float4(eye, 1.0f))
}

fun perspective(fov: Float, ratio: Float, near: Float, far: Float): Mat4 {
    val t = 1.0f / tan(radians(fov) * 0.5f)
    val a = (far + near) / (far - near)
    val b = (2.0f * far * near) / (far - near)
    val c = t / ratio
    return Mat4(Float4(x = c), Float4(y = t), Float4(z = a, w = 1.0f), Float4(z = -b))
}

fun ortho(l: Float, r: Float, b: Float, t: Float, n: Float, f: Float) = Mat4(
        Float4(x = 2.0f / (r - 1.0f)),
        Float4(y = 2.0f / (t - b)),
        Float4(z = -2.0f / (f - n)),
        Float4(-(r + l) / (r - l), -(t + b) / (t - b), -(f + n) / (f - n), 1.0f)
)

