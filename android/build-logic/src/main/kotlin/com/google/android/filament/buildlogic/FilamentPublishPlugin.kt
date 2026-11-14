/*
 * Copyright (C) 2026 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.google.android.filament.buildlogic

import java.net.URI
import org.gradle.api.Plugin
import org.gradle.api.Project
import org.gradle.api.publish.PublishingExtension
import org.gradle.api.publish.maven.MavenPublication
import org.gradle.plugins.signing.SigningExtension

class FilamentPublishPlugin : Plugin<Project> {
    override fun apply(project: Project) {
        project.pluginManager.apply("maven-publish")
        project.pluginManager.apply("signing")

        val providers = project.providers
        val versionName = providers.gradleProperty("VERSION_NAME").get()
        val groupName = providers.gradleProperty("GROUP").get()

        project.version = versionName
        project.group = groupName

        val isReleaseBuild = !versionName.contains("SNAPSHOT")
        val releaseRepoUrl = providers.gradleProperty("RELEASE_REPOSITORY_URL")
            .getOrElse("https://oss.sonatype.org/service/local/staging/deploy/maven2/")
        val snapshotRepoUrl = providers.gradleProperty("SNAPSHOT_REPOSITORY_URL")
            .getOrElse("https://oss.sonatype.org/content/repositories/snapshots/")
        val repoUsername = providers.gradleProperty("SONATYPE_NEXUS_USERNAME").getOrElse("")
        val repoPassword = providers.gradleProperty("SONATYPE_NEXUS_PASSWORD").getOrElse("")

        val pomName = project.findProperty("POM_NAME") as String?
        val pomPackaging = project.findProperty("POM_PACKAGING") as String?
        val pomDescription = providers.gradleProperty("POM_DESCRIPTION")
        val pomUrl = providers.gradleProperty("POM_URL")
        val pomScmUrl = providers.gradleProperty("POM_SCM_URL")
        val pomScmConnection = providers.gradleProperty("POM_SCM_CONNECTION")
        val pomScmDevConnection = providers.gradleProperty("POM_SCM_DEV_CONNECTION")
        val pomLicenceName = providers.gradleProperty("POM_LICENCE_NAME")
        val pomLicenceUrl = providers.gradleProperty("POM_LICENCE_URL")
        val pomLicenceDist = providers.gradleProperty("POM_LICENCE_DIST")
        val pomDeveloperId = providers.gradleProperty("POM_DEVELOPER_ID")
        val pomDeveloperName = providers.gradleProperty("POM_DEVELOPER_NAME")

        project.extensions.configure(PublishingExtension::class.java) { publishing ->
            publishing.repositories { repositories ->
                repositories.maven { maven ->
                    maven.url = URI(if (isReleaseBuild) releaseRepoUrl else snapshotRepoUrl)
                    maven.credentials { credentials ->
                        credentials.username = repoUsername
                        credentials.password = repoPassword
                    }
                }
            }

            publishing.publications.withType(MavenPublication::class.java).configureEach { pub ->
                pub.groupId = groupName
                pub.version = versionName

                pub.pom { pom ->
                    if (pomName != null) {
                        pom.name.set(pomName)
                    }
                    if (pomPackaging != null) {
                        pom.packaging = pomPackaging
                    }
                    pom.description.set(pomDescription)
                    pom.url.set(pomUrl)

                    pom.scm { scm ->
                        scm.url.set(pomScmUrl)
                        scm.connection.set(pomScmConnection)
                        scm.developerConnection.set(pomScmDevConnection)
                    }

                    pom.licenses { licenses ->
                        licenses.license { license ->
                            license.name.set(pomLicenceName)
                            license.url.set(pomLicenceUrl)
                            license.distribution.set(pomLicenceDist)
                        }
                    }

                    pom.developers { developers ->
                        developers.developer { developer ->
                            developer.id.set(pomDeveloperId)
                            developer.name.set(pomDeveloperName)
                        }
                    }
                }
            }
        }

        project.extensions.configure(SigningExtension::class.java) { signing ->
            val signingKey = providers.gradleProperty("signingKey").orNull
            val signingPassword = providers.gradleProperty("signingPassword").orNull
            if (!signingKey.isNullOrEmpty() && !signingPassword.isNullOrEmpty()) {
                signing.useInMemoryPgpKeys(signingKey, signingPassword)
            }

            val publishing = project.extensions.getByType(PublishingExtension::class.java)
            signing.sign(publishing.publications)
        }
    }
}
