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

// don't forget to update MainActivity.kt when/if changing this.
val copyModels = tasks.register<Copy>("copyModels") {
    from(file("../../../third_party/models/DamagedHelmet/DamagedHelmet.glb"))
    from(file("../../../third_party/models/Fox/Fox.glb"))
    from(file("../../../third_party/models/TransmissionRoughnessTest/TransmissionRoughnessTest.glb"))
    into(file("src/main/assets/models"))
}

tasks.named("preBuild") {
    dependsOn(copyModels)
}

tasks.named("clean") {
    doFirst {
        delete("src/main/assets/envs")
        delete("src/main/assets/models")
    }
}

android {
    namespace = "com.google.android.filament.validation"

    compileSdk = libs.versions.compileSdk.get().toInt()
    defaultConfig {
        applicationId = "com.google.android.filament.validation"
        minSdk = libs.versions.minSdk.get().toInt()
        targetSdk = libs.versions.targetSdk.get().toInt()
    }

    compileOptions {
        val jdkVersion = JavaVersion.toVersion(libs.versions.jdk.get())
        sourceCompatibility = jdkVersion
        targetCompatibility = jdkVersion
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
