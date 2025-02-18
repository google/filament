Used to build Dawn as an Android Archive package via Maven.

To build in Android Studio:

View -> Tool Windows -> Gradle -> 'Execute Gradle Task' -> `gradle publishToMavenLocal`

To build in a bash shell, install the Gradle Wrapper in this folder.

https://docs.gradle.org/current/userguide/gradle_wrapper.html#header

Then:

``
./gradlew publishToMavenLocal
``