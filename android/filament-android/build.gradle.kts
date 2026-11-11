plugins {
    id("com.android.library")
    alias(libs.plugins.dokka)
    `maven-publish`
}

android {
    namespace = "com.google.android.filament"

    publishing {
        singleVariant("release") {
            withSourcesJar()
            withJavadocJar()
        }
    }
}

dependencies {
    implementation(libs.androidx.annotations)
}

apply(from = rootProject.file("gradle/gradle-mvn-push.gradle.kts"))

afterEvaluate {
    publishing {
        publications {
            register<MavenPublication>("release") {
                artifactId = project.property("POM_ARTIFACT_ID") as String
                from(components["release"])
            }
        }
    }
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
