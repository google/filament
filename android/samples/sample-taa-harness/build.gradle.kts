plugins {
    id("com.android.application")
    id("com.google.android.filament-tools")
}

project.extra["isSample"] = true

kotlin {
    jvmToolchain(libs.versions.jdk.get().toInt())
}

val generatedAssetsDir = project.layout.projectDirectory.dir("src/main/assets")
val generatedJniLibsDir = layout.buildDirectory.dir("generated/jniLibs")

filament {
    iblInputFile.set(
        project.layout.projectDirectory.file("../../../third_party/environments/lightroom_14b.hdr")
    )
    iblOutputDir.set(generatedAssetsDir.dir("ibl"))
    cmgenArgs.set(
        "--quiet --format=ktx --size=256 --extract-blur=0.1 -x " +
            generatedAssetsDir.dir("ibl/lightroom_14b").asFile.absolutePath
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

val copyNativeLibs = tasks.register<Copy>("copyNativeLibs") {
    from("${rootProject.extra["filamentPath"]}/lib") {
        include("*/libtaa_harness.so")
    }
    into(generatedJniLibsDir)
}

tasks.named("preBuild") {
    dependsOn(stageHarnessModels, copyNativeLibs)
}

tasks.matching { it.name.startsWith("merge") && it.name.endsWith("JniLibFolders") }.configureEach {
    dependsOn(copyNativeLibs)
}

tasks.named("clean") {
    doFirst {
        delete("src/main/assets")
    }
}

android {
    namespace = "com.google.android.filament.taaharness"
    compileSdk = libs.versions.compileSdk.get().toInt()

    defaultConfig {
        applicationId = "com.google.android.filament.taaharness"
        minSdk = libs.versions.minSdk.get().toInt()
        targetSdk = libs.versions.targetSdk.get().toInt()
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
        noCompress += listOf("glb", "ktx", "filamat")
    }

    dependenciesInfo {
        includeInApk = false
    }

    compileOptions {
        val jdkVersion = JavaVersion.toVersion(libs.versions.jdk.get())
        sourceCompatibility = jdkVersion
        targetCompatibility = jdkVersion
    }
}

dependencies {
    implementation(libs.kotlin.stdlib)
}
