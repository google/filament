plugins {
    id("com.android.application")
    id("com.google.android.filament-tools")
}

project.extra["isSample"] = true

kotlin {
    jvmToolchain(libs.versions.jdk.get().toInt())
}

filament {
    cmgenArgs.set("-q --format=ktx --size=256 --extract-blur=0.1 --deploy=src/main/assets/envs/default_env")
    iblInputFile.set(project.layout.projectDirectory.file("../../../third_party/environments/lightroom_14b.hdr"))
    iblOutputDir.set(project.layout.projectDirectory.dir("src/main/assets/envs"))
}

// don't forget to update MainACtivity.kt when/if changing this.
val copyBusterGltf = tasks.register<Copy>("copyBusterGltf") {
    from("../../../third_party/models/BusterDrone")
    into("src/main/assets/models")
}

tasks.named("preBuild") {
    dependsOn(copyBusterGltf)
}

tasks.named("clean") {
    doFirst {
        delete("src/main/assets")
    }
}

android {
    namespace = "com.google.android.filament.gltf"

    compileSdk = libs.versions.compileSdk.get().toInt()
    defaultConfig {
        applicationId = "com.google.android.filament.gltf"
        minSdk = libs.versions.minSdk.get().toInt()
        targetSdk = libs.versions.targetSdk.get().toInt()
    }

    // NOTE: This is a workaround required because the AGP task collectReleaseDependencies
    //       is not configuration-cache friendly yet; this is only useful for Play publication
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
    implementation(libs.androidx.annotations)
    implementation(libs.kotlin.stdlib)
    implementation(libs.kotlinx.coroutines.core)
    implementation(project(":filament-android"))
    implementation(project(":gltfio-android"))
    implementation(project(":filament-utils-android"))
}
