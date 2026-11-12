plugins {
    id("com.android.application")
    id("com.google.android.filament-tools")
}

project.extra["isSample"] = true

kotlin {
    jvmToolchain(libs.versions.jdk.get().toInt())
}

filament {
    iblInputFile.set(project.layout.projectDirectory.file("../../../third_party/environments/studio_small_02_2k.hdr"))
    iblOutputDir.set(project.layout.projectDirectory.dir("src/main/assets/envs"))
    materialInputDir.set(project.layout.projectDirectory.dir("src/main/materials"))
    materialOutputDir.set(project.layout.projectDirectory.dir("src/main/assets/materials"))
}

tasks.named("clean") {
    doFirst {
        delete("src/main/assets")
    }
}

android {
    namespace = "com.google.android.filament.pagecurl"

    compileSdk = libs.versions.compileSdk.get().toInt()
    defaultConfig {
        applicationId = "com.google.android.filament.pagecurl"
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
    implementation(project(":filament-android"))
    implementation(libs.androidx.annotations)
}
