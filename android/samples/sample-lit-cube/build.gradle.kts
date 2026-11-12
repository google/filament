plugins {
    id("com.android.application")
    id("com.google.android.filament-tools")
}

project.extra["isSample"] = true

kotlin {
    jvmToolchain(libs.versions.jdk.get().toInt())
}

filament {
    materialInputDir.set(project.layout.projectDirectory.dir("src/main/materials"))
    materialOutputDir.set(project.layout.projectDirectory.dir("src/main/assets/materials"))
}

tasks.named("clean") {
    doFirst {
        delete("src/main/assets")
    }
}

android {
    namespace = "com.google.android.filament.litcube"

    compileSdk = libs.versions.compileSdk.get().toInt()
    defaultConfig {
        applicationId = "com.google.android.filament.litcube"
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
        noCompress += listOf("filamat", "ktx")
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
}
