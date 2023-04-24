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

@file:Suppress("NOTHING_TO_INLINE")

package com.google.android.filament.utils

import kotlin.math.pow

const val FPI         = 3.1415926536f
const val HALF_PI     = FPI * 0.5f
const val TWO_PI      = FPI * 2.0f
const val FOUR_PI     = FPI * 4.0f
const val INV_PI      = 1.0f / FPI
const val INV_TWO_PI  = INV_PI * 0.5f
const val INV_FOUR_PI = INV_PI * 0.25f

val HALF_ONE = Half(0x3c00.toUShort())
val HALF_TWO = Half(0x4000.toUShort())

inline fun clamp(x: Float, min: Float, max: Float) = if (x < min) min else (if (x > max) max else x)

inline fun clamp(x: Half, min: Half, max: Half) = if (x < min) min else (if (x > max) max else x)

inline fun saturate(x: Float) = clamp(x, 0.0f, 1.0f)

inline fun saturate(x: Half) = clamp(x, Half.POSITIVE_ZERO, HALF_ONE)

inline fun mix(a: Float, b: Float, x: Float) = a * (1.0f - x) + b * x

inline fun mix(a: Half, b: Half, x: Half) = a * (HALF_ONE - x) + b * x

inline fun degrees(v: Float) = v * (180.0f * INV_PI)

inline fun radians(v: Float) = v * (FPI / 180.0f)

inline fun fract(v: Float) = v % 1

inline fun sqr(v: Float) = v * v

inline fun sqr(v: Half) = v * v

inline fun pow(x: Float, y: Float) = (x.toDouble().pow(y.toDouble())).toFloat()
