plugins {
    id("com.android.application")
    id("com.google.android.filament-tools")
}

project.extra["isSample"] = true

kotlin {
    jvmToolchain(libs.versions.jdk.get().toInt())
}

filament {
    meshInputFile.set(project.layout.projectDirectory.file("../../../third_party/models/shader_ball/shader_ball.obj"))
    meshOutputDir.set(project.layout.projectDirectory.dir("src/main/assets/models"))

    iblInputFile.set(project.layout.projectDirectory.file("../../../third_party/environments/flower_road_no_sun_2k.hdr"))
    iblOutputDir.set(project.layout.projectDirectory.dir("src/main/assets/envs"))
}

tasks.named("clean") {
    doFirst {
        delete("src/main/assets")
    }
}

android {
    namespace = "com.google.android.filament.material_builder"

    compileSdk = libs.versions.compileSdk.get().toInt()
    defaultConfig {
        applicationId = "com.google.android.filament.material_builder"
        minSdk = libs.versions.minSdk.get().toInt()
        targetSdk = libs.versions.targetSdk.get().toInt()
    }

    // NOTE: This is a workaround required because the AGP task collectReleaseDependencies
    //       is not configuration-cache friendly yet; this is only useful for Play publication
    dependenciesInfo {
        includeInApk = false
    }

    // We use the .filamat extension for materials compiled with matc
    // Telling aapt to not compress them allows to load them efficiently
    androidResources {
        noCompress += listOf("filamat")
    }

    compileOptions {
        val jdkVersion = JavaVersion.toVersion(libs.versions.jdk.get())
        sourceCompatibility = jdkVersion
        targetCompatibility = jdkVersion
    }
}

dependencies {
    implementation(libs.kotlin.stdlib)
    implementation(project(":filament-android"))
    implementation(project(":filamat-android"))
}
