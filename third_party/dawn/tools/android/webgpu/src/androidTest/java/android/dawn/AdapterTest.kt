package android.dawn

import android.dawn.helper.Util
import androidx.test.ext.junit.runners.AndroidJUnit4
import kotlinx.coroutines.runBlocking
import org.junit.Assert.assertEquals
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4::class)
class AdapterTest {
    @Test
    fun adaptorTest() {
        Util  // Hack to force library initialization.

        val instance = createInstance()
        runBlocking {
            val (status1, adapter, message1) = instance.requestAdapter()
            check(status1 == RequestAdapterStatus.Success && adapter != null) {
                message1 ?: "Error requesting the adapter"
            }
            val adapterInfo = adapter.getInfo()

            assertEquals("The backend type should be Vulkan",
                BackendType.Vulkan.value, adapterInfo.backendType.value
            )
        }
    }
}
