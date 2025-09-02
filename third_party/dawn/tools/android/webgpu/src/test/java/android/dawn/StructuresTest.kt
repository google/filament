package android.dawn

import org.junit.Assert.assertEquals
import org.junit.Test

class StructuresTest {
  @Test
  fun defaultZeroTest() {
    val bindingLayoutEntry = BindGroupLayoutEntry(binding = 0, visibility = ShaderStage.Vertex)
    assertEquals("default=zero member structures not initializing enums to their zero value",
      SamplerBindingType.BindingNotUsed, bindingLayoutEntry.sampler.type)
  }

  @Test
  fun defaultTest() {
    val sampler = SamplerBindingLayout()
    assertEquals("Non-defaulted structure not initialized enum to IDL-specified default",
      SamplerBindingType.Filtering, sampler.type)
  }
}
