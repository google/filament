package com.google.android.filament.cppviewer

class SampleAppDispatcher {
    companion object {
        init {
            System.loadLibrary("sample-cpp-viewer-jni")
        }
        
        fun createSampleApp(sampleName: String, nativeDm: Long, nativeLoader: Long): NativeViewer {
            val app = nCreateSampleApp(sampleName, nativeDm, nativeLoader)
            return NativeViewer(app)
        }
        
        @JvmStatic
        external fun getSampleNames(): Array<String>
        
        @JvmStatic
        external fun createAssetLoader(assetManager: android.content.res.AssetManager): Long
        
        @JvmStatic
        external fun destroyAssetLoader(nativeLoader: Long)
        
        @JvmStatic
        private external fun nCreateSampleApp(sampleName: String, nativeDm: Long, nativeLoader: Long): Long
    }
}
