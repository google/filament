package android.dawn.helper

import android.view.Surface

public object Util {
    init {
        System.loadLibrary("webgpu_c_bundled")
    }

    public external fun windowFromSurface(surface: Surface?): Long
}