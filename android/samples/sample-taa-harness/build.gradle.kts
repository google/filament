plugins {
    id("com.google.android.filament.sample")
    id("com.google.android.filament-tools")
}

val generatedAssetsDir = layout.buildDirectory.dir("generated/assets/filament")
val generatedJniLibsDir = layout.buildDirectory.dir("generated/jniLibs")

filament {
    iblInputFile.set(
        project.layout.projectDirectory.file("../../../third_party/environments/lightroom_14b.hdr")
    )
    iblOutputDir.set(generatedAssetsDir.map { it.dir("ibl") })
    cmgenArgs.set(
        generatedAssetsDir.map {
            "--quiet --format=ktx --size=256 --extract-blur=0.1 -x " +
                it.dir("ibl/lightroom_14b").asFile.absolutePath
        }
    )
}

val stageHarnessModels = tasks.register<Copy>("stageHarnessModels") {
    from("../../../") {
        include("third_party/models/Fox/Fox.glb")
        include("third_party/models/DamagedHelmet/DamagedHelmet.glb")
        include("third_party/models/BusterDrone/BusterDrone.glb")
        include("tools/taa/models/SubpixelStar/SubpixelStar.glb")
        include("tools/taa/models/SSRReflection/SSRReflection.glb")
    }
    into(generatedAssetsDir)
}

tasks.named("filamentGenerateAssets") {
    dependsOn(stageHarnessModels)
}

val distDirProp = providers.gradleProperty("com.google.android.filament.dist-dir").get()
val distFile = File(distDirProp).let {
    if (it.isAbsolute) it else File(rootDir, distDirProp)
}
val filamentPath = distFile.canonicalFile.absolutePath.replace(File.separator, "/")

val copyNativeLibs = tasks.register<Copy>("copyNativeLibs") {
    from("$filamentPath/lib") {
        include("*/libtaa_harness.so")
    }
    into(generatedJniLibsDir)
}

tasks.matching { it.name.startsWith("merge") && it.name.endsWith("JniLibFolders") }.configureEach {
    dependsOn(copyNativeLibs)
}

android {
    namespace = "com.google.android.filament.taaharness"

    defaultConfig {
        applicationId = "com.google.android.filament.taaharness"
        versionCode = 1
        versionName = "1.0"
        testInstrumentationRunner = "com.google.android.filament.taaharness.TaaHarnessInstrumentation"
    }

    sourceSets {
        getByName("main") {
            jniLibs.directories.add(generatedJniLibsDir.get().asFile.path)
        }
    }

    androidResources {
        noCompress += listOf("glb")
    }
}

dependencies {
    implementation(libs.kotlin.stdlib)
}
