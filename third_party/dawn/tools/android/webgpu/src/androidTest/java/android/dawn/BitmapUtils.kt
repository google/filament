package android.dawn

import android.graphics.Bitmap
import android.os.Environment
import java.io.BufferedOutputStream
import java.io.File
import java.io.FileOutputStream

fun writeReferenceImage(bitmap: Bitmap) {
    val path =
        Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS)
    val file = File("${path}${File.separator}${"reference.png"}")
    BufferedOutputStream(FileOutputStream(file)).use {
        bitmap.compress(Bitmap.CompressFormat.PNG, 100, it)
        it.close()
    }
}
