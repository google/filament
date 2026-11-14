/*
 * Copyright 2013 Chris Banes
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import java.net.URI
import org.gradle.api.publish.PublishingExtension
import org.gradle.api.publish.maven.MavenPom
import org.gradle.api.publish.maven.MavenPublication
import org.gradle.plugins.signing.SigningExtension

apply(plugin = "maven-publish")
apply(plugin = "signing")

val versionName = project.property("VERSION_NAME") as String
val groupName = project.property("GROUP") as String

version = versionName
group = groupName

fun isReleaseBuild(): Boolean = !versionName.contains("SNAPSHOT")

fun getReleaseRepositoryUrl(): String =
    (project.findProperty("RELEASE_REPOSITORY_URL") as String?)
        ?: "https://oss.sonatype.org/service/local/staging/deploy/maven2/"

fun getSnapshotRepositoryUrl(): String =
    (project.findProperty("SNAPSHOT_REPOSITORY_URL") as String?)
        ?: "https://oss.sonatype.org/content/repositories/snapshots/"

fun getRepositoryUsername(): String =
    (project.findProperty("SONATYPE_NEXUS_USERNAME") as String?) ?: ""

fun getRepositoryPassword(): String =
    (project.findProperty("SONATYPE_NEXUS_PASSWORD") as String?) ?: ""

fun configurePom(pom: MavenPom) {
    pom.name.set(project.property("POM_NAME") as String)
    pom.packaging = project.property("POM_PACKAGING") as String
    pom.description.set(project.property("POM_DESCRIPTION") as String)
    pom.url.set(project.property("POM_URL") as String)

    pom.scm {
        url.set(project.property("POM_SCM_URL") as String)
        connection.set(project.property("POM_SCM_CONNECTION") as String)
        developerConnection.set(project.property("POM_SCM_DEV_CONNECTION") as String)
    }

    pom.licenses {
        license {
            name.set(project.property("POM_LICENCE_NAME") as String)
            url.set(project.property("POM_LICENCE_URL") as String)
            distribution.set(project.property("POM_LICENCE_DIST") as String)
        }
    }

    pom.developers {
        developer {
            id.set(project.property("POM_DEVELOPER_ID") as String)
            name.set(project.property("POM_DEVELOPER_NAME") as String)
        }
    }
}

afterEvaluate {
    extensions.configure<PublishingExtension> {
        repositories {
            maven {
                val releasesRepoUrl = getReleaseRepositoryUrl()
                val snapshotsRepoUrl = getSnapshotRepositoryUrl()
                url = URI(if (isReleaseBuild()) releasesRepoUrl else snapshotsRepoUrl)

                credentials {
                    username = getRepositoryUsername()
                    password = getRepositoryPassword()
                }
            }
        }

        publications.withType<MavenPublication>().all {
            groupId = groupName
            version = versionName

            configurePom(pom)
        }
    }

    extensions.configure<SigningExtension> {
        val signingKey = project.findProperty("signingKey") as String?
        val signingPassword = project.findProperty("signingPassword") as String?
        if (!signingKey.isNullOrEmpty() && !signingPassword.isNullOrEmpty()) {
            println("Signing with in-memory keys")
            useInMemoryPgpKeys(signingKey, signingPassword)
        }

        val publishing = extensions.getByType<PublishingExtension>()
        publishing.publications.all {
            sign(this)
        }
    }
}
