package android.dawn

import org.junit.Test

import org.junit.Assert.assertTrue

class EnumsTest {
    @Test
    fun uniqueEnumsBufferBindingType() {
        val values = HashSet<BufferBindingType>()
        arrayOf(
            BufferBindingType.BindingNotUsed,
            BufferBindingType.Uniform,
            BufferBindingType.Storage,
            BufferBindingType.ReadOnlyStorage
        ).forEach { value -> assertTrue("Multiple enums share value $value", values.add(value)) }
    }
}
