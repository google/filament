/*
 * Copyright 2026 The Android Open Source Project
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

package androidx.webgpu

import android.hardware.SyncFence
import android.os.Build
import android.os.ParcelFileDescriptor
import android.os.Parcel
import androidx.annotation.RequiresApi
import androidx.annotation.RestrictTo
import kotlin.jvm.JvmStatic

/**
 * A unified abstraction for fence synchronization in WebGPU
 *
 * This class is backed by a `ParcelFileDescriptor` representing a sync fence.
 */
@ExperimentalWebGpuApi
public class GPUSyncFence internal constructor(
    private val parcelFileDescriptor: ParcelFileDescriptor
) : AutoCloseable {

    /**
     * Waits for the fence to signal.
     */
    public fun await(timeoutMillis: Long): Boolean {
        return awaitParcelFileDescriptorNative(parcelFileDescriptor, timeoutMillis)
    }

    private external fun awaitParcelFileDescriptorNative(pfd: ParcelFileDescriptor, timeoutMillis: Long): Boolean

    /**
     * Extracts the ParcelFileDescriptor.
     * This is intended for internal library use or when passing to JNI.
     */
    @RestrictTo(RestrictTo.Scope.LIBRARY_GROUP)
    public fun getParcelFileDescriptor(): ParcelFileDescriptor {
        return parcelFileDescriptor
    }

    override fun close() {
        parcelFileDescriptor.close()
    }

    public companion object {
        /**
         * Creates a GPUSyncFence directly from a ParcelFileDescriptor.
         */
        @RestrictTo(RestrictTo.Scope.LIBRARY_GROUP)
        @JvmStatic
        public fun fromParcelFileDescriptor(pfd: ParcelFileDescriptor): GPUSyncFence {
            return GPUSyncFence(pfd)
        }

        /**
         * Creates a GPUSyncFence from a platform SyncFence by extracting its file descriptor.
         */
        @RequiresApi(Build.VERSION_CODES.TIRAMISU)
        @JvmStatic
        public fun fromSyncFence(syncFence: SyncFence): GPUSyncFence {
            val pfd = readFileDescriptor(syncFence)
            return GPUSyncFence(pfd)
        }

        @RequiresApi(Build.VERSION_CODES.TIRAMISU)
        private fun readFileDescriptor(syncFence: SyncFence): ParcelFileDescriptor {
            val parcel = Parcel.obtain()
            try {
                syncFence.writeToParcel(parcel, 0)
                parcel.setDataPosition(0)
                check(parcel.readBoolean()) { "Failed to read boolean from parcel" }
                val pfd = parcel.readFileDescriptor()
                checkNotNull(pfd) { "Failed to read file descriptor from parcel" }
                return pfd
            } finally {
                parcel.recycle()
            }
        }
    }
}
