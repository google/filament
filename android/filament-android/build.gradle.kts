plugins {
    id("com.google.android.filament.library")
    alias(libs.plugins.dokka)
}

android {
    namespace = "com.google.android.filament"
}

dependencies {
    implementation(libs.androidx.annotations)
}

tasks.dokkaGfm {
    dokkaSourceSets {
        // Use 'register' instead of 'configureEach' to force Dokka
        // to create a new source set from scratch.
        register("main") {
            sourceRoots.from(file("src/main/java"))
        }
    }
}
