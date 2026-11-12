plugins {
    id("com.android.library")
    `maven-publish`
}

android {
    namespace = "com.google.android.filament.filamat"

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
                artifactId = project.property("POM_ARTIFACT_ID_FULL") as String
                from(components["release"])
            }
        }
    }
}
