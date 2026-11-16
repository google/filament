plugins {
    id("com.google.android.filament.sample")
}

android {
    namespace = "com.google.android.filament.livewallpaper"
    defaultConfig {
        applicationId = "com.google.android.filament.livewallpaper"
    }
}

dependencies {
    implementation(libs.kotlin.stdlib)
    implementation(libs.androidx.annotations)
    implementation(project(":filament-android"))
}
