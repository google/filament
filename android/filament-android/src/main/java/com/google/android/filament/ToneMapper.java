package com.google.android.filament;

/**
 * Interface for tone mapping operators. A tone mapping operator, or tone mapper,
 * is responsible for compressing the dynamic range of the rendered scene to a
 * dynamic range suitable for display.
 *
 * In Filament, tone mapping is a color grading step. ToneMapper instances are
 * created and passed to the ColorGrading::Builder to produce a 3D LUT that will
 * be used during post-processing to prepare the final color buffer for display.
 *
 * Filament provides several default tone mapping operators that fall into three
 * categories:
 *
 * <ul>
 * <li>Configurable tone mapping operators</li>
 * <ul>
 *   <li>GenericToneMapper</li>
 *   <li>AgXToneMapper</li>
 * </ul>
 * <li>Fixed-aesthetic tone mapping operators</li>
 * <ul>
 *   <li>ACESToneMapper</li>
 *   <li>ACESLegacyToneMapper</li>
 *   <li>FilmicToneMapper</li>
 *   <li>PBRNeutralToneMapper</li>
 * </ul>
 * <li>Debug/validation tone mapping operators</li>
 * <ul>
 *   <li>LinearToneMapper</li>
 *   <li>DisplayRangeToneMapper</li>
 * </ul>
 * </ul>
 *
 * You can create custom tone mapping operators by subclassing ToneMapper.
 */
public class ToneMapper {
    private final long mNativeObject;

    private ToneMapper(long nativeObject) {
        mNativeObject = nativeObject;
    }

    public long getNativeObject() {
        if (mNativeObject == 0) {
            throw new IllegalStateException("Calling method on destroyed ToneMapper");
        }
        return mNativeObject;
    }

    @Override
    protected void finalize() throws Throwable {
        try {
            super.finalize();
        } finally {
            nDestroyToneMapper(mNativeObject);
        }
    }

    /**
     * Linear tone mapping operator that returns the input color but clamped to
     * the 0..1 range. This operator is mostly useful for debugging.
     */
    public static class Linear extends ToneMapper {
        public Linear() {
            super(nCreateLinearToneMapper());
        }
    }

    /**
     * ACES tone mapping operator. This operator is an implementation of the
     * ACES Reference Rendering Transform (RRT) combined with the Output Device
     * Transform (ODT) for sRGB monitors (dim surround, 100 nits).
     */
    public static class ACES extends ToneMapper {
        public ACES() {
            super(nCreateACESToneMapper());
        }
    }

    /**
     * ACES tone mapping operator, modified to match the perceived brightness
     * of FilmicToneMapper. This operator is the same as ACESToneMapper but
     * applies a brightness multiplier of ~1.6 to the input color value to
     * target brighter viewing environments.
     */
    public static class ACESLegacy extends ToneMapper {
        public ACESLegacy() {
            super(nCreateACESLegacyToneMapper());
        }
    }

    /**
     * "Filmic" tone mapping operator. This tone mapper was designed to
     * approximate the aesthetics of the ACES RRT + ODT for Rec.709
     * and historically Filament's default tone mapping operator. It exists
     * only for backward compatibility purposes and is not otherwise recommended.
     */
    public static class Filmic extends ToneMapper {
        public Filmic() {
            super(nCreateFilmicToneMapper());
        }
    }

    /**
     * Khronos PBR Neutral tone mapping operator. This tone mapper was designed
     * to preserve the appearance of materials across lighting conditions while
     * avoiding artifacts in the highlights in high dynamic range conditions.
     */
    public static class PBRNeutralToneMapper extends ToneMapper {
        public PBRNeutralToneMapper() {
            super(nCreatePBRNeutralToneMapper());
        }
    }

    /**
     * AgX tone mapping operator.
     */
    public static class Agx extends ToneMapper {
        public enum AgxLook {
            /**
             * Base contrast with no look applied
             */
            NONE,

            /**
             * A punchy and more chroma laden look for sRGB displays
             */
            PUNCHY,

            /**
             * A golden tinted, slightly washed look for BT.1886 displays
             */
            GOLDEN
        }

        /**
         * Builds a new AgX tone mapper with no look applied.
         */
        public Agx() {
            this(AgxLook.NONE);
        }

        /**
         * Builds a new AgX tone mapper.
         *
         * @param look: an optional creative adjustment to contrast and saturation
         */
        public Agx(AgxLook look) {
            super(nCreateAgxToneMapper(look.ordinal()));
        }
    }

    /**
     * Generic tone mapping operator that gives control over the tone mapping
     * curve. This operator can be used to control the aesthetics of the final
     * image. This operator also allows to control the dynamic range of the
     * scene referred values.
     *
     * The tone mapping curve is defined by 5 parameters:
     * <ul>
     * <li>contrast: controls the contrast of the curve</li>
     * referred values map to output white</li>
     * <li>midGrayIn: sets the input middle gray</li>
     * <li>midGrayOut: sets the output middle gray</li>
     * <li>hdrMax: defines the maximum input value that will be mapped to
     * output white</li>
     * </ul>
     */
    public static class Generic extends ToneMapper {
        /**
         * Builds a new generic tone mapper parameterized to closely approximate
         * the {@link ACESLegacy} tone mapper. The default values are:
         *
         * <ul>
         * <li>contrast = 1.55f</li>
         * <li>midGrayIn = 0.18f</li>
         * <li>midGrayOut = 0.215f</li>
         * <li>hdrMax = 10.0f</li>
         * </ul>
         */
        public Generic() {
            this(1.55f, 0.18f, 0.215f, 10.0f);
        }

        /**
         * Builds a new generic tone mapper.
         *
         * @param contrast: controls the contrast of the curve, must be > 0.0, values
         *                  in the range 0.5..2.0 are recommended.
         * @param midGrayIn: sets the input middle gray, between 0.0 and 1.0.
         * @param midGrayOut: sets the output middle gray, between 0.0 and 1.0.
         * @param hdrMax: defines the maximum input value that will be mapped to
         *                output white. Must be >= 1.0.
         */
        public Generic(
                float contrast, float midGrayIn, float midGrayOut, float hdrMax) {
            super(nCreateGenericToneMapper(contrast, midGrayIn, midGrayOut, hdrMax));
        }

        /** Returns the contrast of the curve as a strictly positive value. */
        public float getContrast() {
            return nGenericGetContrast(getNativeObject());
        }

        /** Sets the contrast of the curve, must be > 0.0, values in the range 0.5..2.0 are recommended. */
        public void setContrast(float contrast) {
            nGenericSetContrast(getNativeObject(), contrast);
        }

        /** Returns the middle gray point for input values as a value between 0.0 and 1.0. */
        public float getMidGrayIn() {
            return nGenericGetMidGrayIn(getNativeObject());
        }

        /** Sets the input middle gray, between 0.0 and 1.0. */
        public void setMidGrayIn(float midGrayIn) {
            nGenericSetMidGrayIn(getNativeObject(), midGrayIn);
        }

        /** Returns the middle gray point for output values as a value between 0.0 and 1.0. */
        public float getMidGrayOut() {
            return nGenericGetMidGrayOut(getNativeObject());
        }

        /** Sets the output middle gray, between 0.0 and 1.0. */
        public void setMidGrayOut(float midGrayOut) {
            nGenericSetMidGrayOut(getNativeObject(), midGrayOut);
        }

        /** Returns the maximum input value that will map to output white, as a value >= 1.0. */
        public float getHdrMax() {
            return nGenericGetHdrMax(getNativeObject());
        }

        /** Defines the maximum input value that will be mapped to output white. Must be >= 1.0. */
        public void setHdrMax(float hdrMax) {
            nGenericSetHdrMax(getNativeObject(), hdrMax);
        }
    }

    private static native void nDestroyToneMapper(long nativeObject);

    private static native long nCreateLinearToneMapper();
    private static native long nCreateACESToneMapper();
    private static native long nCreateACESLegacyToneMapper();
    private static native long nCreateFilmicToneMapper();
    private static native long nCreatePBRNeutralToneMapper();
    private static native long nCreateAgxToneMapper(int look);
    private static native long nCreateGenericToneMapper(
            float contrast, float midGrayIn, float midGrayOut, float hdrMax);

    // Generic tone mappper
    private static native float nGenericGetContrast(long nativeObject);
    private static native float nGenericGetMidGrayIn(long nativeObject);
    private static native float nGenericGetMidGrayOut(long nativeObject);
    private static native float nGenericGetHdrMax(long nativeObject);

    private static native void nGenericSetContrast(long nativeObject, float contrast);
    private static native void nGenericSetMidGrayIn(long nativeObject, float midGrayIn);
    private static native void nGenericSetMidGrayOut(long nativeObject, float midGrayOut);
    private static native void nGenericSetHdrMax(long nativeObject, float hdrMax);
}
