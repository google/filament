package android.dawn

import android.dawn.helper.Util
import androidx.test.ext.junit.runners.AndroidJUnit4
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4::class)
class ObjectTest {
    init {
        Util  // Hack to force library initialization.
    }

    @Test
    fun sameObjectCompare() {
        val surface = createInstance().createSurface()

        val texture1 = surface.getCurrentTexture().texture

        val texture2 = surface.getCurrentTexture().texture

        assert(texture1 == texture2) {
            "== matches two Kotlin objects representing the same Dawn object"
        }

        assert(texture1.equals(texture2)) {
            ".equals() matches two Kotlin objects representing the same Dawn object"
        }
    }

    @Test
    fun differentObjectCompare() {
        val instance = createInstance()

        val surface1 = instance.createSurface()

        val surface2 = instance.createSurface()

        assert(!(surface1 == surface2)) {
            "== fails to match two Kotlin objects representing different Dawn objects"
        }

        assert(!surface1.equals(surface2)) {
            ".equals fails to match two Kotlin objects representing different Dawn objects"
        }
    }
}
