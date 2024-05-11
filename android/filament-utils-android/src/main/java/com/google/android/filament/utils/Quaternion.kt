/*
 * Copyright (C) 2022 Romain Guy
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

import kotlin.math.*

enum class QuaternionComponent {
    X, Y, Z, W
}

/**
 * Construct Quaternion and set each value.
 * The Quaternion will be normalized during construction
 * Default: Identity
 */
data class Quaternion(
        var x: Float = 0.0f,
        var y: Float = 0.0f,
        var z: Float = 0.0f,
        var w: Float = 1.0f) {

    constructor(v: Float3, w: Float = 1.0f) : this(v.x, v.y, v.z, w)
    constructor(v: Float4) : this(v.x, v.y, v.z, v.w)
    constructor(q: Quaternion) : this(q.x, q.y, q.z, q.w)

    companion object {
        /**
         * Construct a Quaternion from an axis and angle in degrees
         *
         * @param axis Rotation direction
         * @param angle Angle size in degrees
         */
        fun fromAxisAngle(axis: Float3, angle: Float): Quaternion {
            val r = radians(angle)
            return Quaternion(sin(r * 0.5f) * normalize(axis), cos(r * 0.5f))
        }

        /**
         * Construct a Quaternion from Euler angles using YPR around a specified order
         *
         * Uses intrinsic Tait-Bryan angles. This means that rotations are performed with respect to
         * the local coordinate system.
         * That is, for order 'XYZ', the rotation is first around the X axis (which is the same as
         * the world-X axis), then around local-Y (which may now be different from the world
         * Y-axis), then local-Z (which may be different from the world Z-axis)
         *
         * @param d Per axis Euler angles in degrees
         * Yaw, pitch, roll (YPR) are taken accordingly to the rotations order input.
         * @param order The order in which to apply rotations.
         * Default is [RotationsOrder.ZYX] which means that the object will first be rotated around
         * its Z axis, then its Y axis and finally its X axis.
         */
        fun fromEuler(d: Float3, order: RotationsOrder = RotationsOrder.ZYX): Quaternion {
            val r = transform(d, ::radians)
            return fromEuler(r[order.yaw], r[order.pitch], r[order.roll], order)
        }

        /**
         * Construct a Quaternion from Euler yaw, pitch, roll around a specified order.
         *
         * @param roll about 1st rotation axis in radians. Z in case of ZYX order
         * @param pitch about 2nd rotation axis in radians. Y in case of ZYX order
         * @param yaw about 3rd rotation axis in radians. X in case of ZYX order
         * @param order The order in which to apply rotations.
         * Default is [RotationsOrder.ZYX] which means that the object will first be rotated around its Z
         * axis, then its Y axis and finally its X axis.
         */
        fun fromEuler(
            yaw: Float = 0.0f,
            pitch: Float = 0.0f,
            roll: Float = 0.0f,
            order: RotationsOrder = RotationsOrder.ZYX
        ): Quaternion {
            val c1 = cos(yaw * 0.5f)
            val s1 = sin(yaw * 0.5f)
            val c2 = cos(pitch * 0.5f)
            val s2 = sin(pitch * 0.5f)
            val c3 = cos(roll * 0.5f)
            val s3 = sin(roll * 0.5f)
            return when (order) {
                RotationsOrder.XZY -> Quaternion(
                        s1 * c2 * c3 - c1 * s2 * s3,
                        c1 * c2 * s3 - s1 * s2 * c3,
                        s1 * c2 * s3 + c1 * s2 * c3,
                        s1 * s2 * s3 + c1 * c2 * c3)
                RotationsOrder.XYZ -> Quaternion(
                        s1 * c2 * c3 + s2 * s3 * c1,
                        s2 * c1 * c3 - s1 * s3 * c2,
                        s1 * s2 * c3 + s3 * c1 * c2,
                        c1 * c2 * c3 - s1 * s2 * s3
                )
                RotationsOrder.YXZ -> Quaternion(
                        s1 * c2 * s3 + c1 * s2 * c3,
                        s1 * c2 * c3 - c1 * s2 * s3,
                        c1 * c2 * s3 - s1 * s2 * c3,
                        s1 * s2 * s3 + c1 * c2 * c3
                )
                RotationsOrder.YZX -> Quaternion(
                        s1 * s2 * c3 + c1 * c2 * s3,
                        s1 * c2 * c3 + c1 * s2 * s3,
                        c1 * s2 * c3 - s1 * c2 * s3,
                        c1 * c2 * c3 - s1 * s2 * s3
                )
                RotationsOrder.ZYX -> Quaternion(
                        c1 * c2 * s3 - s1 * s2 * c3,
                        s1 * c2 * s3 + c1 * s2 * c3,
                        s1 * c2 * c3 - c1 * s2 * s3,
                        s1 * s2 * s3 + c1 * c2 * c3
                )
                RotationsOrder.ZXY -> Quaternion(
                        c1 * s2 * c3 - s1 * c2 * s3,
                        s1 * s2 * c3 + c1 * c2 * s3,
                        s1 * c2 * c3 + c1 * s2 * s3,
                        c1 * c2 * c3 - s1 * s2 * s3
                )
            }
        }
    }

    inline var xyz: Float3
        get() = Float3(x, y, z)
        set(value) {
            x = value.x
            y = value.y
            z = value.z
        }

    inline var imaginary: Float3
        get() = xyz
        set(value) {
            x = value.x
            y = value.y
            z = value.z
        }

    inline var real: Float
        get() = w
        set(value) {
            w = value
        }

    inline var xyzw: Float4
        get() = Float4(x, y, z, w)
        set(value) {
            x = value.x
            y = value.y
            z = value.z
            w = value.w
        }

    operator fun get(index: QuaternionComponent) = when (index) {
        QuaternionComponent.X -> x
        QuaternionComponent.Y -> y
        QuaternionComponent.Z -> z
        QuaternionComponent.W -> w
    }

    operator fun get(
            index1: QuaternionComponent,
            index2: QuaternionComponent,
            index3: QuaternionComponent): Float3 {
        return Float3(get(index1), get(index2), get(index3))
    }

    operator fun get(
            index1: QuaternionComponent,
            index2: QuaternionComponent,
            index3: QuaternionComponent,
            index4: QuaternionComponent): Quaternion {
        return Quaternion(get(index1), get(index2), get(index3), get(index4))
    }

    operator fun get(index: Int) = when (index) {
        0 -> x
        1 -> y
        2 -> z
        3 -> w
        else -> throw IllegalArgumentException("index must be in 0..3")
    }

    operator fun get(index1: Int, index2: Int, index3: Int): Float3 {
        return Float3(get(index1), get(index2), get(index3))
    }

    operator fun get(index1: Int, index2: Int, index3: Int, index4: Int): Quaternion {
        return Quaternion(get(index1), get(index2), get(index3), get(index4))
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

    operator fun set(index: QuaternionComponent, v: Float) = when (index) {
        QuaternionComponent.X -> x = v
        QuaternionComponent.Y -> y = v
        QuaternionComponent.Z -> z = v
        QuaternionComponent.W -> w = v
    }

    operator fun set(index1: QuaternionComponent, index2: QuaternionComponent, v: Float) {
        set(index1, v)
        set(index2, v)
    }

    operator fun set(
            index1: QuaternionComponent, index2: QuaternionComponent, index3: QuaternionComponent, v: Float) {
        set(index1, v)
        set(index2, v)
        set(index3, v)
    }

    operator fun set(
            index1: QuaternionComponent, index2: QuaternionComponent,
            index3: QuaternionComponent, index4: QuaternionComponent, v: Float) {
        set(index1, v)
        set(index2, v)
        set(index3, v)
        set(index4, v)
    }

    operator fun unaryMinus() = Quaternion(-x, -y, -z, -w)

    inline operator fun plus(v: Float) = Quaternion(x + v, y + v, z + v, w + v)
    inline operator fun minus(v: Float) = Quaternion(x - v, y - v, z - v, w - v)
    inline operator fun times(v: Float) = Quaternion(x * v, y * v, z * v, w * v)
    inline operator fun div(v: Float) = Quaternion(x / v, y / v, z / v, w / v)
    inline fun compareTo(v: Float, delta: Float = 0.0f) = Float4(
        x.compareTo(v, delta),
        y.compareTo(v, delta),
        z.compareTo(v, delta),
        w.compareTo(v, delta)
    )

    inline fun equals(v: Float, delta: Float = 0.0f) = Bool4(
        x.equals(v, delta),
        y.equals(v, delta),
        z.equals(v, delta),
        w.equals(v, delta)
    )

    inline operator fun times(v: Float3) = (this * Quaternion(v, 0.0f) * inverse(this)).xyz

    inline operator fun plus(q: Quaternion) = Quaternion(x + q.x, y + q.y, z + q.z, w + q.w)
    inline operator fun minus(q: Quaternion) = Quaternion(x - q.x, y - q.y, z - q.z, w - q.w)
    inline operator fun times(q: Quaternion) = Quaternion(
        w * q.x + x * q.w + y * q.z - z * q.y,
        w * q.y - x * q.z + y * q.w + z * q.x,
        w * q.z + x * q.y - y * q.x + z * q.w,
        w * q.w - x * q.x - y * q.y - z * q.z
    )

    inline fun compareTo(v: Float4, delta: Float = 0.0f) = Float4(
        x.compareTo(v.x, delta),
        y.compareTo(v.y, delta),
        z.compareTo(v.z, delta),
        w.compareTo(v.w, delta)
    )

    inline fun equals(v: Float4, delta: Float = 0.0f) = Bool4(
        x.equals(v.x, delta),
        y.equals(v.y, delta),
        z.equals(v.z, delta),
        w.equals(v.w, delta)
    )

    inline fun transform(block: (Float) -> Float): Quaternion {
        x = block(x)
        y = block(y)
        z = block(z)
        w = block(w)
        return this
    }

    fun toEulerAngles() = eulerAngles(this)

    fun toMatrix() = rotation(this)

    fun toFloatArray() = floatArrayOf(x, y, z, w)
}

inline operator fun Float.plus(q: Quaternion) = Quaternion(this + q.x, this + q.y, this + q.z, this + q.w)
inline operator fun Float.minus(q: Quaternion) = Quaternion(this - q.x, this - q.y, this - q.z, this - q.w)
inline operator fun Float.times(q: Quaternion) = Quaternion(this * q.x, this * q.y, this * q.z, this * q.w)
inline operator fun Float.div(q: Quaternion) = Quaternion(this / q.x, this / q.y, this / q.z, this / q.w)

inline fun lessThan(a: Quaternion, b: Float) = Bool4(
    a.x < b,
    a.y < b,
    a.z < b,
    a.w < b
)

inline fun lessThan(a: Quaternion, b: Quaternion) = Bool4(
    a.x < b.x,
    a.y < b.y,
    a.z < b.z,
    a.w < b.w
)

inline fun lessThanEqual(a: Quaternion, b: Float) = Bool4(
    a.x <= b,
    a.y <= b,
    a.z <= b,
    a.w <= b
)

inline fun lessThanEqual(a: Quaternion, b: Quaternion) = Bool4(
    a.x <= b.x,
    a.y <= b.y,
    a.z <= b.z,
    a.w <= b.w
)

inline fun greaterThan(a: Quaternion, b: Float) = Bool4(
    a.x > b,
    a.y > b,
    a.z > b,
    a.w > b
)

inline fun greaterThan(a: Quaternion, b: Quaternion) = Bool4(
    a.x > b.y,
    a.y > b.y,
    a.z > b.z,
    a.w > b.w
)

inline fun greaterThanEqual(a: Quaternion, b: Float) = Bool4(
    a.x >= b,
    a.y >= b,
    a.z >= b,
    a.w >= b
)

inline fun greaterThanEqual(a: Quaternion, b: Quaternion) = Bool4(
    a.x >= b.x,
    a.y >= b.y,
    a.z >= b.z,
    a.w >= b.w
)

inline fun equal(a: Quaternion, b: Float, delta: Float = 0.0f) = Bool4(
    a.x.equals(b, delta),
    a.y.equals(b, delta),
    a.z.equals(b, delta),
    a.w.equals(b, delta)
)

inline fun equal(a: Quaternion, b: Quaternion, delta: Float = 0.0f) = Bool4(
    a.x.equals(b.x, delta),
    a.y.equals(b.y, delta),
    a.z.equals(b.z, delta),
    a.w.equals(b.w, delta)
)

inline fun notEqual(a: Quaternion, b: Float, delta: Float = 0.0f) = Bool4(
    !a.x.equals(b, delta),
    !a.y.equals(b, delta),
    !a.z.equals(b, delta),
    !a.w.equals(b, delta)
)

inline fun notEqual(a: Quaternion, b: Quaternion, delta: Float = 0.0f) = Bool4(
    !a.x.equals(b.x, delta),
    !a.y.equals(b.y, delta),
    !a.z.equals(b.z, delta),
    !a.w.equals(b.w, delta)
)

inline infix fun Quaternion.lt(b: Float) = Bool4(x < b, y < b, z < b, w < b)
inline infix fun Quaternion.lt(b: Float4) = Bool4(x < b.x, y < b.y, z < b.z, w < b.w)
inline infix fun Quaternion.lte(b: Float) = Bool4(x <= b, y <= b, z <= b, w <= b)
inline infix fun Quaternion.lte(b: Float4) = Bool4(x <= b.x, y <= b.y, z <= b.z, w <= b.w)
inline infix fun Quaternion.gt(b: Float) = Bool4(x > b, y > b, z > b, w > b)
inline infix fun Quaternion.gt(b: Float4) = Bool4(x > b.x, y > b.y, z > b.z, w > b.w)
inline infix fun Quaternion.gte(b: Float) = Bool4(x >= b, y >= b, z >= b, w >= b)
inline infix fun Quaternion.gte(b: Float4) = Bool4(x >= b.x, y >= b.y, z >= b.z, w >= b.w)
inline infix fun Quaternion.eq(b: Float) = Bool4(x == b, y == b, z == b, w == b)
inline infix fun Quaternion.eq(b: Float4) = Bool4(x == b.x, y == b.y, z == b.z, w == b.w)
inline infix fun Quaternion.neq(b: Float) = Bool4(x != b, y != b, z != b, w != b)
inline infix fun Quaternion.neq(b: Float4) = Bool4(x != b.x, y != b.y, z != b.z, w != b.w)

inline fun abs(q: Quaternion) = Quaternion(abs(q.x), abs(q.y), abs(q.z), abs(q.w))
inline fun length(q: Quaternion) = sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w)
inline fun length2(q: Quaternion) = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w
inline fun dot(a: Quaternion, b: Quaternion) = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w

/**
 * Rescale the Quaternion to the unit length
 */
fun normalize(q: Quaternion): Quaternion {
    val l = 1.0f / length(q)
    return Quaternion(q.x * l, q.y * l, q.z * l, q.w * l)
}

fun conjugate(q: Quaternion): Quaternion = Quaternion(-q.x, -q.y, -q.z, q.w)

fun inverse(q: Quaternion): Quaternion {
    val d = 1.0f / dot(q, q)
    return Quaternion(-q.x * d, -q.y * d, -q.z * d, q.w * d)
}

fun cross(a: Quaternion, b: Quaternion): Quaternion {
    val m = a * b
    return Quaternion(m.x, m.y, m.z, 0.0f)
}

fun angle(a: Quaternion, b: Quaternion): Float {
    return 2.0f * acos(abs(clamp(dot(a, b), -1.0f, 1.0f)))
}

/**
 * Spherical linear interpolation between two given orientations
 *
 * If t is 0 this returns a.
 * As t approaches 1 slerp may approach either b or -b (whichever is closest to a)
 * If t is above 1 or below 0 the result will be extrapolated.
 * @param a The beginning value
 * @param b The ending value
 * @param t The ratio between the two floats
 * @param dotThreshold If the quaternion dot product is greater than this value
 * (i.e. the quaternions are very close to each other), then the quaternions are
 * linearly interpolated instead of spherically interpolated.
 *
 * @return Interpolated value between the two floats
 */
fun slerp(a: Quaternion, b: Quaternion, t: Float, dotThreshold: Float = 0.9995f): Quaternion {
    // could also be computed as: pow(q * inverse(p), t) * p;
    var dot = dot(a, b)
    var b1 = b

    // If the dot product is negative, then the interpolation won't follow the shortest angular path
    // between the two quaterions. In this case, invert the end quaternion to produce an equivalent
    // rotation that will give us the path we want.
    if (dot < 0.0f) {
        dot = -dot
        b1 = -b
    }

    // Prevent blowing up when slerping between two quaternions that are very near each other.
    return if (dot < dotThreshold) {
        val angle = acos(dot)
        val s = sin(angle)
        a * sin((1.0f - t) * angle) / s + b1 * sin(t * angle) / s
    } else {
        // If the angle is too small, use linear interpolation
        nlerp(a, b1, t)
    }
}

fun lerp(a: Quaternion, b: Quaternion, t: Float): Quaternion {
    return ((1.0f - t) * a) + (t * b)
}

fun nlerp(a: Quaternion, b: Quaternion, t: Float): Quaternion {
    return normalize(lerp(a, b, t))
}

/**
 * Convert a Quaternion to Euler angles
 *
 * @param order The order in which to apply rotations.
 * Default is [RotationsOrder.ZYX] which means that the object will first be rotated around its Z
 * axis, then its Y axis and finally its X axis.
 */
fun eulerAngles(q: Quaternion, order: RotationsOrder = RotationsOrder.ZYX): Float3 {
    return eulerAngles(rotation(q), order)
}
