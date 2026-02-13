package android.dawn

import java.net.URL
import java.util.jar.JarFile
import kotlin.reflect.KCallable
import kotlin.reflect.KClass
import kotlin.reflect.KProperty1
import kotlin.reflect.full.companionObjectInstance
import kotlin.reflect.full.primaryConstructor
import kotlin.test.assertContains
import kotlin.test.assertEquals
import kotlin.test.fail
import org.junit.Test

/**
 * Yield all classes under the specified namespace.
 */
private fun classNames(namespace: String) = sequence {
    // Use the thread's class loader to find all the class resource files.
    val classLoader = Thread.currentThread().contextClassLoader!!
    for (jarUrl in classLoader.getResources(namespace.replace('.', '/'))) {
        if (jarUrl.protocol != "jar") {
            continue
        }

        val url = URL(jarUrl.file.substringBefore('!')) // Trim prefix jar: and suffix !...

        // Find the class files inside the jar file.
        for (entry in JarFile(url.file).entries()) {
            if (entry.name.endsWith(".class")) {  // Not every file is a class.
                yield(entry.name.removeSuffix(".class").replace('/', '.'))
            }
        }
    }
}

class MappedNamedConstantsTest {

    val TYPES_WITH_MAPPED_NAMED_CONSTANTS = arrayOf(
        AdapterType::class,
        AddressMode::class,
        BackendType::class,
        BlendFactor::class,
        BlendOperation::class,
        BufferBindingType::class,
        BufferMapState::class,
        BufferUsage::class,
        CallbackMode::class,
        ColorWriteMask::class,
        CompareFunction::class,
        CompilationInfoRequestStatus::class,
        CompilationMessageType::class,
        CompositeAlphaMode::class,
        CreatePipelineAsyncStatus::class,
        CullMode::class,
        DeviceLostReason::class,
        ErrorFilter::class,
        ErrorType::class,
        FeatureLevel::class,
        FeatureName::class,
        FilterMode::class,
        FrontFace::class,
        IndexFormat::class,
        InstanceFeatureName::class,
        LoadOp::class,
        MapAsyncStatus::class,
        MapMode::class,
        MipmapFilterMode::class,
        OptionalBool::class,
        PopErrorScopeStatus::class,
        PowerPreference::class,
        PredefinedColorSpace::class,
        PresentMode::class,
        PrimitiveTopology::class,
        QueryType::class,
        QueueWorkDoneStatus::class,
        RequestAdapterStatus::class,
        RequestDeviceStatus::class,
        SamplerBindingType::class,
        ShaderStage::class,
        Status::class,
        StencilOperation::class,
        StorageTextureAccess::class,
        StoreOp::class,
        SType::class,
        SurfaceGetCurrentTextureStatus::class,
        TextureAspect::class,
        TextureDimension::class,
        TextureFormat::class,
        TextureSampleType::class,
        TextureUsage::class,
        TextureViewDimension::class,
        ToneMappingMode::class,
        VertexFormat::class,
        VertexStepMode::class,
        WaitStatus::class,
        WGSLLanguageFeatureName::class
    )

    /**
     * Test that the actual classes in our generated Kotlin are all tested in this unit test,
     * and that there are no unexpected new classes.
     *
     * It's worth hardcoding the list above (rather than just listing the target classes to
     * inspect dynamically) because this way, if the number of classes found dynamically were
     * to suddenly drop to zero (as in, the code changed and the test filter dropped all the
     * classes) then we'd know the test was no longer testing the correct thing. Similarly,
     * if the number of classes found dynamically were to suddenly increase, then we'd know
     * that the test was no longer testing the correct thing.
     */
    @Test
    fun testPackageClassesMatchTestTargets() {
        val dawnClasses = classNames("android.dawn")
        val actual = dawnClasses.filter { clazz ->
            isAndroidDawn(clazz) && hasCompanionObjectWithNames(clazz)
        }.map { it.removePrefix("android.dawn.") }

        val expected = TYPES_WITH_MAPPED_NAMED_CONSTANTS.mapNotNull { it.simpleName }

        // Test that the two lists match, throw a useful failure if they don't
        actual.forEach { className -> assertContains(expected, className) }
        expected.forEach { className -> assertContains(actual, className) }
    }

    /**
     * Test that every class listed above (a) has a names field, and (b) each name is
     * mapped to the correct string and instance value.
     */
    @Test
    fun testMappedConstantsNamesAreCorrect() {
        for (clazz in TYPES_WITH_MAPPED_NAMED_CONSTANTS) {
            val companionObject = getCompanionObjectOrFail(clazz)
            val namesMap = getNamesMapOrFail(companionObject, clazz)
            val companionConstants = getCompanionConstants(companionObject)

            for ((key, constantName) in namesMap) {
                val constantProperty = companionConstants[constantName]
                val actual = (constantProperty as KProperty1<*, *>).getter.call(companionObject)
                val expected = clazz.primaryConstructor?.call(key)
                assertEquals(expected, actual)
            }
        }
    }

    private fun isAndroidDawn(clazz: String): Boolean {
        return clazz.startsWith("android.dawn") && clazz.count { it == '.' } == 2
    }

    private fun hasCompanionObjectWithNames(clazz: String): Boolean {
        val kClass = Class.forName(clazz).kotlin
        val companionObject = kClass.companionObjectInstance
        return companionObject != null && companionObject::class.members.any { it.name == "names" }
    }

    private fun getCompanionConstants(companionObject: Any): Map<String, KCallable<*>> {
        return companionObject::class.members.filter { it.isFinal && it is KProperty1<*, *> }
            .associateBy { it.name }
    }

    private fun getCompanionObjectOrFail(clazz: KClass<out Any>): Any {
        return clazz.companionObjectInstance
            ?: fail("No companion object found in ${clazz.simpleName}")
    }

    private fun getNamesMapOrFail(companionObject: Any, clazz: KClass<out Any>): Map<Int, String> {
        val namesProperty = companionObject::class.members.find { it.name == "names" }
            ?: fail("Property 'names' not found in companion object of ${clazz.simpleName}")

        @Suppress("UNCHECKED_CAST")
        return namesProperty.call(companionObject) as? Map<Int, String>
            ?: fail("Property 'names' is not of type Map<Int, String>")
    }
}
