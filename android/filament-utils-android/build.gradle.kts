plugins {
    id("com.google.android.filament.library")
}

kotlin {
    jvmToolchain(libs.versions.jdk.get().toInt())
}

android {
    namespace = "com.google.android.filament.utils"

    sourceSets {
        getByName("main") {
            kotlin.directories.add("src/main/java")
        }
    }

    packaging {
        jniLibs {
            excludes += listOf("lib/*/libfilament-jni.so", "lib/*/libgltfio-jni.so")
        }
    }
}

tasks.configureEach {
    if (name.startsWith("buildCMakeRelWithDebInfo")) {
        dependsOn(":filament-android:externalNativeBuildRelease", ":gltfio-android:externalNativeBuildRelease")
    } else if (name.startsWith("buildCMakeDebug")) {
        dependsOn(":filament-android:externalNativeBuildDebug", ":gltfio-android:externalNativeBuildDebug")
    }
}

dependencies {
    implementation(libs.kotlin.stdlib)
    implementation(libs.androidx.annotations)

    implementation(libs.kotlinx.coroutines.core)
    implementation(libs.kotlinx.coroutines.android)

    api(project(":filament-android"))
    api(project(":gltfio-android"))
}
