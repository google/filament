plugins {
    id("com.google.android.filament.sample")
    id("com.google.android.filament-tools")
}

filament {
    materialInputDir.set(project.layout.projectDirectory.dir("src/main/materials"))
}

android {
    namespace = "com.google.android.filament.streamtest"
    defaultConfig {
        applicationId = "com.google.android.filament.streamtest"
        minSdk = 23
    }
}

dependencies {
    implementation(libs.kotlin.stdlib)
    implementation(libs.androidx.core)
    implementation(project(":filament-android"))
}
