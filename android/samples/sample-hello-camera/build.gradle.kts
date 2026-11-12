plugins {
    id("com.google.android.filament.sample")
    id("com.google.android.filament-tools")
}

filament {
    materialInputDir.set(project.layout.projectDirectory.dir("src/main/materials"))
}

android {
    namespace = "com.google.android.filament.hellocam"
    defaultConfig {
        applicationId = "com.google.android.filament.hellocam"
        minSdk = 23
    }
}

dependencies {
    implementation(libs.kotlin.stdlib)
    implementation(libs.androidx.core)
    implementation(project(":filament-android"))
}
