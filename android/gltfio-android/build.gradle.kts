plugins {
    id("com.google.android.filament.library")
}

android {
    namespace = "com.google.android.filament.gltfio"

    packaging {
        jniLibs {
            excludes += listOf("lib/*/libfilament-jni.so")
        }
    }
}

tasks.configureEach {
    if (name.startsWith("buildCMakeRelWithDebInfo")) {
        dependsOn(":filament-android:externalNativeBuildRelease")
    } else if (name.startsWith("buildCMakeDebug")) {
        dependsOn(":filament-android:externalNativeBuildDebug")
    }
}

dependencies {
    implementation(libs.androidx.annotations)

    api(project(":filament-android"))
}
