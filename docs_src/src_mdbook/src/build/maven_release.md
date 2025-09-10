## Register for a Sonatype Account

First, you'll need to register for a Sonatype account at the [Central
Portal](https://central.sonatype.org/register/central-portal/).

To publish under the `com.google.android` namespace, you'll also need to email
`central-support@sonatype.com` with your account information to request access.

Then, generate a user token through the Sonatype website. Navigate to
[https://central.sonatype.com/account](https://central.sonatype.com/account) and select **Generate
User Token**.

Finally, add the generated token credentials to `~/.gradle/gradle.properties`. (Note: these are
different from the credentials used to log into your Sonatype account):

```gradle
# Generated Sonatype token
sonatypeUsername=<username>
sonatypePassword=<password>
```

-----

## Signing Key

All release artifacts must be signed. You'll need to create an OpenPGP key pair. See Gradle's
documentation on [Signatory
credentials](https://docs.gradle.org/current/userguide/signing_plugin.html#sec:signatory_credentials)
for instructions.

Update `~/.gradle/gradle.properties` with your new key's credentials:

```gradle
signing.keyId=<key id>
signing.password=<key password>
signing.secretKeyRingFile=<secret key ring file>
```

-----

## Build the Android Release

Make sure `JAVA_HOME` is set correctly. For example:

```bash
export JAVA_HOME=/Library/Java/JavaVirtualMachines/zulu-17.jdk/Contents/Home
./build.sh -C -i -p android release
```

-----

## Publish to Sonatype

### A Note on the Legacy Staging Service

Previously, Filament was published to Maven via OSSRH (Open Source Software Repository Hosting). In
2025, this service was [sunsetted](https://central.sonatype.org/pages/ossrh-eol/). Now, we use
Sonatype's [Central Publisher Portal](https://central.sonatype.org/).

The new Central Publisher Portal does not officially support Gradle. However, Sonatype provides a
staging API compatibility service, which works with Filament's Gradle setup.

-----

### 1\. Upload to the Staging API Compatibility Service

```bash
cd android
./gradlew publishToSonatype
```

### 2\. Move the Repository to the Central Publisher Portal

We have a script to automate this. It reads the `sonatypeUsername` and `sonatypePassword` from your
`~/.gradle/gradle.properties` file.

```bash
python3 build/common/close-sonatype-staging-repository.py
```

### 3\. Publish the Release on Sonatype

Navigate to [Maven Central Repository Deployments](https://central.sonatype.com/publishing/deployments).

Here, you should see a new deployment with a **Validated** status and all your artifacts listed. Click
the **Publish** button to publish the artifacts. It typically takes around 5 minutes after clicking
**Publish** for the artifacts to go live.
