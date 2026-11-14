plugins {
    id("com.google.android.filament.sample")
    id("com.google.android.filament-tools")
}

filament {
    val envOutputDir = layout.buildDirectory.dir("generated/assets/filament/envs/default_env")
    cmgenArgs.set(envOutputDir.map { "-q --format=ktx --size=256 --extract-blur=0.1 --deploy=${it.asFile.absolutePath}" })
    iblInputFile.set(project.layout.projectDirectory.file("../../../third_party/environments/lightroom_14b.hdr"))
}

// don't forget to update MainActivity.kt when/if changing this.
val copyModels = tasks.register<Copy>("copyModels") {
    from(file("../../../third_party/models/DamagedHelmet/DamagedHelmet.glb"))
    from(file("../../../third_party/models/Fox/Fox.glb"))
    from(file("../../../third_party/models/TransmissionRoughnessTest/TransmissionRoughnessTest.glb"))
    into(layout.buildDirectory.dir("generated/assets/filament/models"))
}

tasks.named("filamentGenerateAssets") {
    dependsOn(copyModels)
}

android {
    namespace = "com.google.android.filament.validation"
    defaultConfig {
        applicationId = "com.google.android.filament.validation"
    }
}

dependencies {
    implementation(libs.kotlin.stdlib)
    implementation(libs.androidx.constraintlayout)
    implementation(libs.android.material)
    implementation(libs.kotlinx.coroutines.core)
    implementation(project(":filament-android"))
    implementation(project(":gltfio-android"))
    implementation(project(":filament-utils-android"))
}
