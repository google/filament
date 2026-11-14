plugins {
    id("com.google.android.filament.sample")
    id("com.google.android.filament-tools")
}

filament {
    val envOutputDir = layout.buildDirectory.dir("generated/assets/filament/envs/default_env")
    cmgenArgs.set(envOutputDir.map { "-q --format=ktx --size=256 --extract-blur=0.1 --deploy=${it.asFile.absolutePath}" })
    iblInputFile.set(project.layout.projectDirectory.file("../../../third_party/environments/lightroom_14b.hdr"))
}

// don't forget to update MainACtivity.kt when/if changing this.
val copyBusterGltf = tasks.register<Copy>("copyBusterGltf") {
    from("../../../third_party/models/BusterDrone")
    into(layout.buildDirectory.dir("generated/assets/filament/models"))
}

tasks.named("filamentGenerateAssets") {
    dependsOn(copyBusterGltf)
}

android {
    namespace = "com.google.android.filament.gltf"
    defaultConfig {
        applicationId = "com.google.android.filament.gltf"
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
