package org.libsdl.app;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioRecord;
import android.media.AudioTrack;
import android.media.MediaRecorder;
import android.os.Build;
import android.util.Log;

public class SDLAudioManager
{
    protected static final String TAG = "SDLAudio";

    protected static AudioTrack mAudioTrack;
    protected static AudioRecord mAudioRecord;

    public static void initialize() {
        mAudioTrack = null;
        mAudioRecord = null;
    }

    // Audio

    protected static String getAudioFormatString(int audioFormat) {
        switch (audioFormat) {
        case AudioFormat.ENCODING_PCM_8BIT:
            return "8-bit";
        case AudioFormat.ENCODING_PCM_16BIT:
            return "16-bit";
        case AudioFormat.ENCODING_PCM_FLOAT:
            return "float";
        default:
            return Integer.toString(audioFormat);
        }
    }

    protected static int[] open(boolean isCapture, int sampleRate, int audioFormat, int desiredChannels, int desiredFrames) {
        int channelConfig;
        int sampleSize;
        int frameSize;

        Log.v(TAG, "Opening " + (isCapture ? "capture" : "playback") + ", requested " + desiredFrames + " frames of " + desiredChannels + " channel " + getAudioFormatString(audioFormat) + " audio at " + sampleRate + " Hz");

        /* On older devices let's use known good settings */
        if (Build.VERSION.SDK_INT < 21) {
            if (desiredChannels > 2) {
                desiredChannels = 2;
            }
            if (sampleRate < 8000) {
                sampleRate = 8000;
            } else if (sampleRate > 48000) {
                sampleRate = 48000;
            }
        }

        if (audioFormat == AudioFormat.ENCODING_PCM_FLOAT) {
            int minSDKVersion = (isCapture ? 23 : 21);
            if (Build.VERSION.SDK_INT < minSDKVersion) {
                audioFormat = AudioFormat.ENCODING_PCM_16BIT;
            }
        }
        switch (audioFormat)
        {
        case AudioFormat.ENCODING_PCM_8BIT:
            sampleSize = 1;
            break;
        case AudioFormat.ENCODING_PCM_16BIT:
            sampleSize = 2;
            break;
        case AudioFormat.ENCODING_PCM_FLOAT:
            sampleSize = 4;
            break;
        default:
            Log.v(TAG, "Requested format " + audioFormat + ", getting ENCODING_PCM_16BIT");
            audioFormat = AudioFormat.ENCODING_PCM_16BIT;
            sampleSize = 2;
            break;
        }

        if (isCapture) {
            switch (desiredChannels) {
            case 1:
                channelConfig = AudioFormat.CHANNEL_IN_MONO;
                break;
            case 2:
                channelConfig = AudioFormat.CHANNEL_IN_STEREO;
                break;
            default:
                Log.v(TAG, "Requested " + desiredChannels + " channels, getting stereo");
                desiredChannels = 2;
                channelConfig = AudioFormat.CHANNEL_IN_STEREO;
                break;
            }
        } else {
            switch (desiredChannels) {
            case 1:
                channelConfig = AudioFormat.CHANNEL_OUT_MONO;
                break;
            case 2:
                channelConfig = AudioFormat.CHANNEL_OUT_STEREO;
                break;
            case 3:
                channelConfig = AudioFormat.CHANNEL_OUT_STEREO | AudioFormat.CHANNEL_OUT_FRONT_CENTER;
                break;
            case 4:
                channelConfig = AudioFormat.CHANNEL_OUT_QUAD;
                break;
            case 5:
                channelConfig = AudioFormat.CHANNEL_OUT_QUAD | AudioFormat.CHANNEL_OUT_FRONT_CENTER;
                break;
            case 6:
                channelConfig = AudioFormat.CHANNEL_OUT_5POINT1;
                break;
            case 7:
                channelConfig = AudioFormat.CHANNEL_OUT_5POINT1 | AudioFormat.CHANNEL_OUT_BACK_CENTER;
                break;
            case 8:
                if (Build.VERSION.SDK_INT >= 23) {
                    channelConfig = AudioFormat.CHANNEL_OUT_7POINT1_SURROUND;
                } else {
                    Log.v(TAG, "Requested " + desiredChannels + " channels, getting 5.1 surround");
                    desiredChannels = 6;
                    channelConfig = AudioFormat.CHANNEL_OUT_5POINT1;
                }
                break;
            default:
                Log.v(TAG, "Requested " + desiredChannels + " channels, getting stereo");
                desiredChannels = 2;
                channelConfig = AudioFormat.CHANNEL_OUT_STEREO;
                break;
            }

/*
            Log.v(TAG, "Speaker configuration (and order of channels):");

            if ((channelConfig & 0x00000004) != 0) {
                Log.v(TAG, "   CHANNEL_OUT_FRONT_LEFT");
            }
            if ((channelConfig & 0x00000008) != 0) {
                Log.v(TAG, "   CHANNEL_OUT_FRONT_RIGHT");
            }
            if ((channelConfig & 0x00000010) != 0) {
                Log.v(TAG, "   CHANNEL_OUT_FRONT_CENTER");
            }
            if ((channelConfig & 0x00000020) != 0) {
                Log.v(TAG, "   CHANNEL_OUT_LOW_FREQUENCY");
            }
            if ((channelConfig & 0x00000040) != 0) {
                Log.v(TAG, "   CHANNEL_OUT_BACK_LEFT");
            }
            if ((channelConfig & 0x00000080) != 0) {
                Log.v(TAG, "   CHANNEL_OUT_BACK_RIGHT");
            }
            if ((channelConfig & 0x00000100) != 0) {
                Log.v(TAG, "   CHANNEL_OUT_FRONT_LEFT_OF_CENTER");
            }
            if ((channelConfig & 0x00000200) != 0) {
                Log.v(TAG, "   CHANNEL_OUT_FRONT_RIGHT_OF_CENTER");
            }
            if ((channelConfig & 0x00000400) != 0) {
                Log.v(TAG, "   CHANNEL_OUT_BACK_CENTER");
            }
            if ((channelConfig & 0x00000800) != 0) {
                Log.v(TAG, "   CHANNEL_OUT_SIDE_LEFT");
            }
            if ((channelConfig & 0x00001000) != 0) {
                Log.v(TAG, "   CHANNEL_OUT_SIDE_RIGHT");
            }
*/
        }
        frameSize = (sampleSize * desiredChannels);

        // Let the user pick a larger buffer if they really want -- but ye
        // gods they probably shouldn't, the minimums are horrifyingly high
        // latency already
        int minBufferSize;
        if (isCapture) {
            minBufferSize = AudioRecord.getMinBufferSize(sampleRate, channelConfig, audioFormat);
        } else {
            minBufferSize = AudioTrack.getMinBufferSize(sampleRate, channelConfig, audioFormat);
        }
        desiredFrames = Math.max(desiredFrames, (minBufferSize + frameSize - 1) / frameSize);

        int[] results = new int[4];

        if (isCapture) {
            if (mAudioRecord == null) {
                mAudioRecord = new AudioRecord(MediaRecorder.AudioSource.DEFAULT, sampleRate,
                        channelConfig, audioFormat, desiredFrames * frameSize);

                // see notes about AudioTrack state in audioOpen(), above. Probably also applies here.
                if (mAudioRecord.getState() != AudioRecord.STATE_INITIALIZED) {
                    Log.e(TAG, "Failed during initialization of AudioRecord");
                    mAudioRecord.release();
                    mAudioRecord = null;
                    return null;
                }

                mAudioRecord.startRecording();
            }

            results[0] = mAudioRecord.getSampleRate();
            results[1] = mAudioRecord.getAudioFormat();
            results[2] = mAudioRecord.getChannelCount();

        } else {
            if (mAudioTrack == null) {
                mAudioTrack = new AudioTrack(AudioManager.STREAM_MUSIC, sampleRate, channelConfig, audioFormat, desiredFrames * frameSize, AudioTrack.MODE_STREAM);

                // Instantiating AudioTrack can "succeed" without an exception and the track may still be invalid
                // Ref: https://android.googlesource.com/platform/frameworks/base/+/refs/heads/master/media/java/android/media/AudioTrack.java
                // Ref: http://developer.android.com/reference/android/media/AudioTrack.html#getState()
                if (mAudioTrack.getState() != AudioTrack.STATE_INITIALIZED) {
                    /* Try again, with safer values */

                    Log.e(TAG, "Failed during initialization of Audio Track");
                    mAudioTrack.release();
                    mAudioTrack = null;
                    return null;
                }

                mAudioTrack.play();
            }

            results[0] = mAudioTrack.getSampleRate();
            results[1] = mAudioTrack.getAudioFormat();
            results[2] = mAudioTrack.getChannelCount();
        }
        results[3] = desiredFrames;

        Log.v(TAG, "Opening " + (isCapture ? "capture" : "playback") + ", got " + results[3] + " frames of " + results[2] + " channel " + getAudioFormatString(results[1]) + " audio at " + results[0] + " Hz");

        return results;
    }

    /**
     * This method is called by SDL using JNI.
     */
    public static int[] audioOpen(int sampleRate, int audioFormat, int desiredChannels, int desiredFrames) {
        return open(false, sampleRate, audioFormat, desiredChannels, desiredFrames);
    }

    /**
     * This method is called by SDL using JNI.
     */
    public static void audioWriteFloatBuffer(float[] buffer) {
        if (mAudioTrack == null) {
            Log.e(TAG, "Attempted to make audio call with uninitialized audio!");
            return;
        }

        for (int i = 0; i < buffer.length;) {
            int result = mAudioTrack.write(buffer, i, buffer.length - i, AudioTrack.WRITE_BLOCKING);
            if (result > 0) {
                i += result;
            } else if (result == 0) {
                try {
                    Thread.sleep(1);
                } catch(InterruptedException e) {
                    // Nom nom
                }
            } else {
                Log.w(TAG, "SDL audio: error return from write(float)");
                return;
            }
        }
    }

    /**
     * This method is called by SDL using JNI.
     */
    public static void audioWriteShortBuffer(short[] buffer) {
        if (mAudioTrack == null) {
            Log.e(TAG, "Attempted to make audio call with uninitialized audio!");
            return;
        }

        for (int i = 0; i < buffer.length;) {
            int result = mAudioTrack.write(buffer, i, buffer.length - i);
            if (result > 0) {
                i += result;
            } else if (result == 0) {
                try {
                    Thread.sleep(1);
                } catch(InterruptedException e) {
                    // Nom nom
                }
            } else {
                Log.w(TAG, "SDL audio: error return from write(short)");
                return;
            }
        }
    }

    /**
     * This method is called by SDL using JNI.
     */
    public static void audioWriteByteBuffer(byte[] buffer) {
        if (mAudioTrack == null) {
            Log.e(TAG, "Attempted to make audio call with uninitialized audio!");
            return;
        }

        for (int i = 0; i < buffer.length; ) {
            int result = mAudioTrack.write(buffer, i, buffer.length - i);
            if (result > 0) {
                i += result;
            } else if (result == 0) {
                try {
                    Thread.sleep(1);
                } catch(InterruptedException e) {
                    // Nom nom
                }
            } else {
                Log.w(TAG, "SDL audio: error return from write(byte)");
                return;
            }
        }
    }

    /**
     * This method is called by SDL using JNI.
     */
    public static int[] captureOpen(int sampleRate, int audioFormat, int desiredChannels, int desiredFrames) {
        return open(true, sampleRate, audioFormat, desiredChannels, desiredFrames);
    }

    /** This method is called by SDL using JNI. */
    public static int captureReadFloatBuffer(float[] buffer, boolean blocking) {
        return mAudioRecord.read(buffer, 0, buffer.length, blocking ? AudioRecord.READ_BLOCKING : AudioRecord.READ_NON_BLOCKING);
    }

    /** This method is called by SDL using JNI. */
    public static int captureReadShortBuffer(short[] buffer, boolean blocking) {
        if (Build.VERSION.SDK_INT < 23) {
            return mAudioRecord.read(buffer, 0, buffer.length);
        } else {
            return mAudioRecord.read(buffer, 0, buffer.length, blocking ? AudioRecord.READ_BLOCKING : AudioRecord.READ_NON_BLOCKING);
        }
    }

    /** This method is called by SDL using JNI. */
    public static int captureReadByteBuffer(byte[] buffer, boolean blocking) {
        if (Build.VERSION.SDK_INT < 23) {
            return mAudioRecord.read(buffer, 0, buffer.length);
        } else {
            return mAudioRecord.read(buffer, 0, buffer.length, blocking ? AudioRecord.READ_BLOCKING : AudioRecord.READ_NON_BLOCKING);
        }
    }

    /** This method is called by SDL using JNI. */
    public static void audioClose() {
        if (mAudioTrack != null) {
            mAudioTrack.stop();
            mAudioTrack.release();
            mAudioTrack = null;
        }
    }

    /** This method is called by SDL using JNI. */
    public static void captureClose() {
        if (mAudioRecord != null) {
            mAudioRecord.stop();
            mAudioRecord.release();
            mAudioRecord = null;
        }
    }

    /** This method is called by SDL using JNI. */
    public static void audioSetThreadPriority(boolean iscapture, int device_id) {
        try {

            /* Set thread name */
            if (iscapture) {
                Thread.currentThread().setName("SDLAudioC" + device_id);
            } else {
                Thread.currentThread().setName("SDLAudioP" + device_id);
            }

            /* Set thread priority */
            android.os.Process.setThreadPriority(android.os.Process.THREAD_PRIORITY_AUDIO);

        } catch (Exception e) {
            Log.v(TAG, "modify thread properties failed " + e.toString());
        }
    }

    public static native int nativeSetupJNI();
}
