package android.dawn

import android.graphics.Bitmap
import kotlin.math.pow
import kotlin.math.sqrt

fun Int.floatFrom(pos: Int) = (this shr 8 * pos and 255).toFloat() / 255

val Int.blue get() = floatFrom(0)
val Int.green get() = floatFrom(1)
val Int.red get() = floatFrom(2)
val Int.alpha get() = floatFrom(3)

fun imageSimilarity(a: Bitmap, b: Bitmap): Float {
    if (a.width != b.width || a.height != b.height) {
        return 0f
    }

    var sumSimilarity = 0f

    for (y in 0 until a.height) {
        for (x in 0 until a.width) {
            val ac = a.getPixel(x, y)
            val bc = b.getPixel(x, y)
            sumSimilarity += (1 - sqrt(
                (ac.red - bc.red).pow(2.0f) +
                        (ac.green - bc.green).pow(2.0f) +
                        (ac.blue - bc.blue).pow(2.0f) +
                        (ac.alpha - bc.alpha).pow(2.0f)
            ) / 2)
        }
    }

    return sumSimilarity / (a.width * a.height)
}
