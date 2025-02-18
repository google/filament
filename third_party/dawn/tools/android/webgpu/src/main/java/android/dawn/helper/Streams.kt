package android.dawn.helper

import java.io.InputStream
import java.io.InputStreamReader
import java.nio.charset.StandardCharsets
import java.util.Scanner

public fun InputStream.asString(): String =
    Scanner(InputStreamReader(this, StandardCharsets.UTF_8)).useDelimiter("\\A").run {
        if (hasNext()) next() else ""
    }

