package androidx.webgpu

import org.junit.Test

import org.junit.Assert.assertTrue

class EnumsTest {
    @Test
    fun uniqueEnumsBufferBindingType() {
        val values = mutableListOf<Int>()
        arrayOf(
            BufferBindingType.BindingNotUsed,
            BufferBindingType.Uniform,
            BufferBindingType.Storage,
            BufferBindingType.ReadOnlyStorage
        ).forEach { value -> assertTrue("Multiple enums share value $value", values.add(value)) }
    }
}
