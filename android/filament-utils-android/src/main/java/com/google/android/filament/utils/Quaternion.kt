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
        var w: Float = 0.0f) {

    constructor(v: Float3, w: Float = 0.0f) : this(v.x, v.y, v.z, w)
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
         * Construct a Quaternion from Euler angles using YPR around ZYX respectively
         *
         * The Euler angles are applied in ZYX order.
         * i.e: a vector is first rotated about X (roll) then Y (pitch) and then Z (yaw).
         *
         * @param d Per axis Euler angles in degrees
         */
        fun fromEuler(d: Float3): Quaternion {
            val r = transform(d, ::radians)
            return fromEulerZYX(r.z, r.y, r.x)
        }

        /**
         * Construct a Quaternion from Euler angles using YPR around ZYX respectively
         *
         * The Euler angles are applied in ZYX order.
         * i.e: a vector is first rotated about X (roll) then Y (pitch) and then Z (yaw).
         *
         * @param roll about X axis in radians
         * @param pitch about Y axis in radians
         * @param yaw about Z axis in radians
         */
        fun fromEulerZYX(yaw: Float = 0.0f, pitch: Float = 0.0f, roll: Float = 0.0f): Quaternion {
            val cy = cos(yaw * 0.5f)
            val sy = sin(yaw * 0.5f)
            val cp = cos(pitch * 0.5f)
            val sp = sin(pitch * 0.5f)
            val cr = cos(roll * 0.5f)
            val sr = sin(roll * 0.5f)

            return Quaternion(
                    sr * cp * cy - cr * sp * sy,
                    cr * sp * cy + sr * cp * sy,
                    cr * cp * sy - sr * sp * cy,
                    cr * cp * cy + sr * sp * sy
            )
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

    inline operator fun times(v: Float3) = (this * Quaternion(v, 0.0f) * inverse(this)).xyz

    inline operator fun plus(q: Quaternion) = Quaternion(x + q.x, y + q.y, z + q.z, w + q.w)
    inline operator fun minus(q: Quaternion) = Quaternion(x - q.x, y - q.y, z - q.z, w - q.w)
    inline operator fun times(q: Quaternion) = Quaternion(
            w * q.x + x * q.w + y * q.z - z * q.y,
            w * q.y - x * q.z + y * q.w + z * q.x,
            w * q.z + x * q.y - y * q.x + z * q.w,
            w * q.w - x * q.x - y * q.y - z * q.z)

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

/**
 * Spherical linear interpolation between two given orientations
 *
 * If t is 0 this returns a.
 * As t approaches 1 slerp may approach either b or -b (whichever is closest to a)
 * If t is above 1 or below 0 the result will be extrapolated.
 * @param a The beginning value
 * @param b The ending value
 * @param t The ratio between the two floats
 * @param valueEps Prevent blowing up when slerping between two quaternions that are very near each
 * other. Linear interpolation (lerp) is returned in this case.
 *
 * @return Interpolated value between the two floats
 */
fun slerp(a: Quaternion, b: Quaternion, t: Float, valueEps: Float = 0.0000000001f): Quaternion {
    // could also be computed as: pow(q * inverse(p), t) * p;
    val d = dot(a, b)
    val absd = abs(d)
    // Prevent blowing up when slerping between two quaternions that are very near each other.
    if ((1.0f - absd) < valueEps) {
        return normalize(lerp(if (d < 0.0f) -a else a, b, t))
    }
    val npq = sqrt(dot(a, a) * dot(b, b))  // ||p|| * ||q||
    val acos = acos(clamp(absd / npq, -1.0f, 1.0f))
    val acos0 = acos * (1.0f - t)
    val acos1 = acos * t
    val sina = sin(acos)
    if (sina < valueEps) {
        return normalize(lerp(a, b, t))
    }
    val isina = 1.0f / sina
    val s0 = sin(acos0) * isina
    val s1 = sin(acos1) * isina
    // ensure we're taking the "short" side
    return normalize(s0 * a + (if (d < 0.0f) -s1 else (s1)) * b)
}

fun lerp(a: Quaternion, b: Quaternion, t: Float): Quaternion {
    return ((1 - t) * a) + (t * b)
}

fun nlerp(a: Quaternion, b: Quaternion, t: Float): Quaternion {
    return normalize(lerp(a, b, t))
}

/**
 * Convert a Quaternion to Euler angles using YPR around ZYX respectively
 *
 * The Euler angles are applied in ZYX order
 */
fun eulerAngles(q: Quaternion): Float3 {
    val nq = normalize(q)
    return Float3(
            // roll (x-axis rotation)
            degrees(atan2(2.0f * (nq.y * nq.z + nq.w * nq.x),
                    nq.w * nq.w - nq.x * nq.x - nq.y * nq.y + nq.z * nq.z)),
            // pitch (y-axis rotation)
            degrees(asin(-2.0f * (nq.x * nq.z - nq.w * nq.y))),
            // yaw (z-axis rotation)
            degrees(atan2(2.0f * (nq.x * nq.y + nq.w * nq.z),
                    nq.w * nq.w + nq.x * nq.x - nq.y * nq.y - nq.z * nq.z)))
}
