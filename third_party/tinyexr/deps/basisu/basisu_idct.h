// ------------------------------------------------------------
// 1D ORTHONORMAL IDCT (DCT-III), SIZE 2, FLOAT
// out[x*dst_stride] = sum_k C[k][x] * src[k*src_stride]
// C[k][x] = alpha(k) * cos(pi * (2*x+1) * k / (2*N)),
// alpha(0) = sqrt(1/N), alpha(k>0) = sqrt(2/N)
static inline void idct_1d_2(
    const float* src, int src_stride,
    float* dst,       int dst_stride)
{
    float s0 = 0.0f;
    float s1 = 0.0f;

    {
        float v = src[0 * src_stride];
        if (v != 0.0f)
        {
            s0 += 7.071067691e-01f * v;
            s1 += 7.071067691e-01f * v;
        }
    }

    {
        float v = src[1 * src_stride];
        if (v != 0.0f)
        {
            s0 += 7.071067691e-01f * v;
            s1 += -7.071067691e-01f * v;
        }
    }

    dst[0 * dst_stride] = s0;
    dst[1 * dst_stride] = s1;
}

// ------------------------------------------------------------
// 1D ORTHONORMAL IDCT (DCT-III), SIZE 3, FLOAT
// out[x*dst_stride] = sum_k C[k][x] * src[k*src_stride]
// C[k][x] = alpha(k) * cos(pi * (2*x+1) * k / (2*N)),
// alpha(0) = sqrt(1/N), alpha(k>0) = sqrt(2/N)
static inline void idct_1d_3(
    const float* src, int src_stride,
    float* dst,       int dst_stride)
{
    float s0 = 0.0f;
    float s1 = 0.0f;
    float s2 = 0.0f;

    {
        float v = src[0 * src_stride];
        if (v != 0.0f)
        {
            s0 += 5.773502588e-01f * v;
            s1 += 5.773502588e-01f * v;
            s2 += 5.773502588e-01f * v;
        }
    }

    {
        float v = src[1 * src_stride];
        if (v != 0.0f)
        {
            s0 += 7.071067691e-01f * v;
            s2 += -7.071068883e-01f * v;
        }
    }

    {
        float v = src[2 * src_stride];
        if (v != 0.0f)
        {
            s0 += 4.082482755e-01f * v;
            s1 += -8.164966106e-01f * v;
            s2 += 4.082486033e-01f * v;
        }
    }

    dst[0 * dst_stride] = s0;
    dst[1 * dst_stride] = s1;
    dst[2 * dst_stride] = s2;
}

// ------------------------------------------------------------
// 1D ORTHONORMAL IDCT (DCT-III), SIZE 4, FLOAT
// out[x*dst_stride] = sum_k C[k][x] * src[k*src_stride]
// C[k][x] = alpha(k) * cos(pi * (2*x+1) * k / (2*N)),
// alpha(0) = sqrt(1/N), alpha(k>0) = sqrt(2/N)
static inline void idct_1d_4(
    const float* src, int src_stride,
    float* dst,       int dst_stride)
{
    float s0 = 0.0f;
    float s1 = 0.0f;
    float s2 = 0.0f;
    float s3 = 0.0f;

    {
        float v = src[0 * src_stride];
        if (v != 0.0f)
        {
            s0 += 5.000000000e-01f * v;
            s1 += 5.000000000e-01f * v;
            s2 += 5.000000000e-01f * v;
            s3 += 5.000000000e-01f * v;
        }
    }

    {
        float v = src[1 * src_stride];
        if (v != 0.0f)
        {
            s0 += 6.532814503e-01f * v;
            s1 += 2.705980539e-01f * v;
            s2 += -2.705981135e-01f * v;
            s3 += -6.532815099e-01f * v;
        }
    }

    {
        float v = src[2 * src_stride];
        if (v != 0.0f)
        {
            s0 += 4.999999702e-01f * v;
            s1 += -4.999999702e-01f * v;
            s2 += -4.999999106e-01f * v;
            s3 += 5.000001788e-01f * v;
        }
    }

    {
        float v = src[3 * src_stride];
        if (v != 0.0f)
        {
            s0 += 2.705980539e-01f * v;
            s1 += -6.532814503e-01f * v;
            s2 += 6.532815099e-01f * v;
            s3 += -2.705983818e-01f * v;
        }
    }

    dst[0 * dst_stride] = s0;
    dst[1 * dst_stride] = s1;
    dst[2 * dst_stride] = s2;
    dst[3 * dst_stride] = s3;
}

// ------------------------------------------------------------
// 1D ORTHONORMAL IDCT (DCT-III), SIZE 5, FLOAT
// out[x*dst_stride] = sum_k C[k][x] * src[k*src_stride]
// C[k][x] = alpha(k) * cos(pi * (2*x+1) * k / (2*N)),
// alpha(0) = sqrt(1/N), alpha(k>0) = sqrt(2/N)
static inline void idct_1d_5(
    const float* src, int src_stride,
    float* dst,       int dst_stride)
{
    float s0 = 0.0f;
    float s1 = 0.0f;
    float s2 = 0.0f;
    float s3 = 0.0f;
    float s4 = 0.0f;

    {
        float v = src[0 * src_stride];
        if (v != 0.0f)
        {
            s0 += 4.472135901e-01f * v;
            s1 += 4.472135901e-01f * v;
            s2 += 4.472135901e-01f * v;
            s3 += 4.472135901e-01f * v;
            s4 += 4.472135901e-01f * v;
        }
    }

    {
        float v = src[1 * src_stride];
        if (v != 0.0f)
        {
            s0 += 6.015009880e-01f * v;
            s1 += 3.717480302e-01f * v;
            s3 += -3.717481494e-01f * v;
            s4 += -6.015009284e-01f * v;
        }
    }

    {
        float v = src[2 * src_stride];
        if (v != 0.0f)
        {
            s0 += 5.116672516e-01f * v;
            s1 += -1.954395324e-01f * v;
            s2 += -6.324555278e-01f * v;
            s3 += -1.954392791e-01f * v;
            s4 += 5.116672516e-01f * v;
        }
    }

    {
        float v = src[3 * src_stride];
        if (v != 0.0f)
        {
            s0 += 3.717480302e-01f * v;
            s1 += -6.015009284e-01f * v;
            s3 += 6.015008688e-01f * v;
            s4 += -3.717483282e-01f * v;
        }
    }

    {
        float v = src[4 * src_stride];
        if (v != 0.0f)
        {
            s0 += 1.954394877e-01f * v;
            s1 += -5.116672516e-01f * v;
            s2 += 6.324555278e-01f * v;
            s3 += -5.116675496e-01f * v;
            s4 += 1.954394132e-01f * v;
        }
    }

    dst[0 * dst_stride] = s0;
    dst[1 * dst_stride] = s1;
    dst[2 * dst_stride] = s2;
    dst[3 * dst_stride] = s3;
    dst[4 * dst_stride] = s4;
}

// ------------------------------------------------------------
// 1D ORTHONORMAL IDCT (DCT-III), SIZE 6, FLOAT
// out[x*dst_stride] = sum_k C[k][x] * src[k*src_stride]
// C[k][x] = alpha(k) * cos(pi * (2*x+1) * k / (2*N)),
// alpha(0) = sqrt(1/N), alpha(k>0) = sqrt(2/N)
static inline void idct_1d_6(
    const float* src, int src_stride,
    float* dst,       int dst_stride)
{
    float s0 = 0.0f;
    float s1 = 0.0f;
    float s2 = 0.0f;
    float s3 = 0.0f;
    float s4 = 0.0f;
    float s5 = 0.0f;

    {
        float v = src[0 * src_stride];
        if (v != 0.0f)
        {
            s0 += 4.082483053e-01f * v;
            s1 += 4.082483053e-01f * v;
            s2 += 4.082483053e-01f * v;
            s3 += 4.082483053e-01f * v;
            s4 += 4.082483053e-01f * v;
            s5 += 4.082483053e-01f * v;
        }
    }

    {
        float v = src[1 * src_stride];
        if (v != 0.0f)
        {
            s0 += 5.576775074e-01f * v;
            s1 += 4.082482755e-01f * v;
            s2 += 1.494291872e-01f * v;
            s3 += -1.494293064e-01f * v;
            s4 += -4.082482755e-01f * v;
            s5 += -5.576775670e-01f * v;
        }
    }

    {
        float v = src[2 * src_stride];
        if (v != 0.0f)
        {
            s0 += 4.999999702e-01f * v;
            s2 += -5.000000596e-01f * v;
            s3 += -4.999999106e-01f * v;
            s5 += 5.000000596e-01f * v;
        }
    }

    {
        float v = src[3 * src_stride];
        if (v != 0.0f)
        {
            s0 += 4.082482755e-01f * v;
            s1 += -4.082482755e-01f * v;
            s2 += -4.082483053e-01f * v;
            s3 += 4.082484245e-01f * v;
            s4 += 4.082480669e-01f * v;
            s5 += -4.082485437e-01f * v;
        }
    }

    {
        float v = src[4 * src_stride];
        if (v != 0.0f)
        {
            s0 += 2.886750996e-01f * v;
            s1 += -5.773502588e-01f * v;
            s2 += 2.886753380e-01f * v;
            s3 += 2.886748910e-01f * v;
            s4 += -5.773502588e-01f * v;
            s5 += 2.886753976e-01f * v;
        }
    }

    {
        float v = src[5 * src_stride];
        if (v != 0.0f)
        {
            s0 += 1.494291872e-01f * v;
            s1 += -4.082483053e-01f * v;
            s2 += 5.576775074e-01f * v;
            s3 += -5.576776266e-01f * v;
            s4 += 4.082483053e-01f * v;
            s5 += -1.494295001e-01f * v;
        }
    }

    dst[0 * dst_stride] = s0;
    dst[1 * dst_stride] = s1;
    dst[2 * dst_stride] = s2;
    dst[3 * dst_stride] = s3;
    dst[4 * dst_stride] = s4;
    dst[5 * dst_stride] = s5;
}

// ------------------------------------------------------------
// 1D ORTHONORMAL IDCT (DCT-III), SIZE 7, FLOAT
// out[x*dst_stride] = sum_k C[k][x] * src[k*src_stride]
// C[k][x] = alpha(k) * cos(pi * (2*x+1) * k / (2*N)),
// alpha(0) = sqrt(1/N), alpha(k>0) = sqrt(2/N)
static inline void idct_1d_7(
    const float* src, int src_stride,
    float* dst,       int dst_stride)
{
    float s0 = 0.0f;
    float s1 = 0.0f;
    float s2 = 0.0f;
    float s3 = 0.0f;
    float s4 = 0.0f;
    float s5 = 0.0f;
    float s6 = 0.0f;

    {
        float v = src[0 * src_stride];
        if (v != 0.0f)
        {
            s0 += 3.779644668e-01f * v;
            s1 += 3.779644668e-01f * v;
            s2 += 3.779644668e-01f * v;
            s3 += 3.779644668e-01f * v;
            s4 += 3.779644668e-01f * v;
            s5 += 3.779644668e-01f * v;
            s6 += 3.779644668e-01f * v;
        }
    }

    {
        float v = src[1 * src_stride];
        if (v != 0.0f)
        {
            s0 += 5.211208463e-01f * v;
            s1 += 4.179065228e-01f * v;
            s2 += 2.319205552e-01f * v;
            s4 += -2.319206595e-01f * v;
            s5 += -4.179066122e-01f * v;
            s6 += -5.211208463e-01f * v;
        }
    }

    {
        float v = src[2 * src_stride];
        if (v != 0.0f)
        {
            s0 += 4.815880954e-01f * v;
            s1 += 1.189424619e-01f * v;
            s2 += -3.332694173e-01f * v;
            s3 += -5.345224738e-01f * v;
            s4 += -3.332692087e-01f * v;
            s5 += 1.189427450e-01f * v;
            s6 += 4.815880954e-01f * v;
        }
    }

    {
        float v = src[3 * src_stride];
        if (v != 0.0f)
        {
            s0 += 4.179065228e-01f * v;
            s1 += -2.319206595e-01f * v;
            s2 += -5.211208463e-01f * v;
            s4 += 5.211208463e-01f * v;
            s5 += 2.319205403e-01f * v;
            s6 += -4.179067314e-01f * v;
        }
    }

    {
        float v = src[4 * src_stride];
        if (v != 0.0f)
        {
            s0 += 3.332692981e-01f * v;
            s1 += -4.815880954e-01f * v;
            s2 += -1.189422309e-01f * v;
            s3 += 5.345224738e-01f * v;
            s4 += -1.189426631e-01f * v;
            s5 += -4.815878570e-01f * v;
            s6 += 3.332692981e-01f * v;
        }
    }

    {
        float v = src[5 * src_stride];
        if (v != 0.0f)
        {
            s0 += 2.319205552e-01f * v;
            s1 += -5.211208463e-01f * v;
            s2 += 4.179064631e-01f * v;
            s4 += -4.179064035e-01f * v;
            s5 += 5.211209059e-01f * v;
            s6 += -2.319207191e-01f * v;
        }
    }

    {
        float v = src[6 * src_stride];
        if (v != 0.0f)
        {
            s0 += 1.189424619e-01f * v;
            s1 += -3.332692087e-01f * v;
            s2 += 4.815881252e-01f * v;
            s3 += -5.345224738e-01f * v;
            s4 += 4.815881550e-01f * v;
            s5 += -3.332694471e-01f * v;
            s6 += 1.189431697e-01f * v;
        }
    }

    dst[0 * dst_stride] = s0;
    dst[1 * dst_stride] = s1;
    dst[2 * dst_stride] = s2;
    dst[3 * dst_stride] = s3;
    dst[4 * dst_stride] = s4;
    dst[5 * dst_stride] = s5;
    dst[6 * dst_stride] = s6;
}

// ------------------------------------------------------------
// 1D ORTHONORMAL IDCT (DCT-III), SIZE 8, FLOAT
// out[x*dst_stride] = sum_k C[k][x] * src[k*src_stride]
// C[k][x] = alpha(k) * cos(pi * (2*x+1) * k / (2*N)),
// alpha(0) = sqrt(1/N), alpha(k>0) = sqrt(2/N)
static inline void idct_1d_8(
    const float* src, int src_stride,
    float* dst,       int dst_stride)
{
    float s0 = 0.0f;
    float s1 = 0.0f;
    float s2 = 0.0f;
    float s3 = 0.0f;
    float s4 = 0.0f;
    float s5 = 0.0f;
    float s6 = 0.0f;
    float s7 = 0.0f;

    {
        float v = src[0 * src_stride];
        if (v != 0.0f)
        {
            s0 += 3.535533845e-01f * v;
            s1 += 3.535533845e-01f * v;
            s2 += 3.535533845e-01f * v;
            s3 += 3.535533845e-01f * v;
            s4 += 3.535533845e-01f * v;
            s5 += 3.535533845e-01f * v;
            s6 += 3.535533845e-01f * v;
            s7 += 3.535533845e-01f * v;
        }
    }

    {
        float v = src[1 * src_stride];
        if (v != 0.0f)
        {
            s0 += 4.903926253e-01f * v;
            s1 += 4.157347977e-01f * v;
            s2 += 2.777850926e-01f * v;
            s3 += 9.754511714e-02f * v;
            s4 += -9.754516184e-02f * v;
            s5 += -2.777851820e-01f * v;
            s6 += -4.157348275e-01f * v;
            s7 += -4.903926551e-01f * v;
        }
    }

    {
        float v = src[2 * src_stride];
        if (v != 0.0f)
        {
            s0 += 4.619397521e-01f * v;
            s1 += 1.913417131e-01f * v;
            s2 += -1.913417578e-01f * v;
            s3 += -4.619398117e-01f * v;
            s4 += -4.619397521e-01f * v;
            s5 += -1.913415641e-01f * v;
            s6 += 1.913418025e-01f * v;
            s7 += 4.619397819e-01f * v;
        }
    }

    {
        float v = src[3 * src_stride];
        if (v != 0.0f)
        {
            s0 += 4.157347977e-01f * v;
            s1 += -9.754516184e-02f * v;
            s2 += -4.903926551e-01f * v;
            s3 += -2.777850032e-01f * v;
            s4 += 2.777852118e-01f * v;
            s5 += 4.903926253e-01f * v;
            s6 += 9.754503518e-02f * v;
            s7 += -4.157348871e-01f * v;
        }
    }

    {
        float v = src[4 * src_stride];
        if (v != 0.0f)
        {
            s0 += 3.535533845e-01f * v;
            s1 += -3.535533845e-01f * v;
            s2 += -3.535533249e-01f * v;
            s3 += 3.535535038e-01f * v;
            s4 += 3.535533845e-01f * v;
            s5 += -3.535536230e-01f * v;
            s6 += -3.535532653e-01f * v;
            s7 += 3.535534143e-01f * v;
        }
    }

    {
        float v = src[5 * src_stride];
        if (v != 0.0f)
        {
            s0 += 2.777850926e-01f * v;
            s1 += -4.903926551e-01f * v;
            s2 += 9.754520655e-02f * v;
            s3 += 4.157346785e-01f * v;
            s4 += -4.157348871e-01f * v;
            s5 += -9.754510969e-02f * v;
            s6 += 4.903926551e-01f * v;
            s7 += -2.777854204e-01f * v;
        }
    }

    {
        float v = src[6 * src_stride];
        if (v != 0.0f)
        {
            s0 += 1.913417131e-01f * v;
            s1 += -4.619397521e-01f * v;
            s2 += 4.619397819e-01f * v;
            s3 += -1.913419515e-01f * v;
            s4 += -1.913414896e-01f * v;
            s5 += 4.619396627e-01f * v;
            s6 += -4.619398713e-01f * v;
            s7 += 1.913419515e-01f * v;
        }
    }

    {
        float v = src[7 * src_stride];
        if (v != 0.0f)
        {
            s0 += 9.754511714e-02f * v;
            s1 += -2.777850032e-01f * v;
            s2 += 4.157346785e-01f * v;
            s3 += -4.903925955e-01f * v;
            s4 += 4.903927147e-01f * v;
            s5 += -4.157347977e-01f * v;
            s6 += 2.777855694e-01f * v;
            s7 += -9.754577279e-02f * v;
        }
    }

    dst[0 * dst_stride] = s0;
    dst[1 * dst_stride] = s1;
    dst[2 * dst_stride] = s2;
    dst[3 * dst_stride] = s3;
    dst[4 * dst_stride] = s4;
    dst[5 * dst_stride] = s5;
    dst[6 * dst_stride] = s6;
    dst[7 * dst_stride] = s7;
}

// ------------------------------------------------------------
// 1D ORTHONORMAL IDCT (DCT-III), SIZE 9, FLOAT
// out[x*dst_stride] = sum_k C[k][x] * src[k*src_stride]
// C[k][x] = alpha(k) * cos(pi * (2*x+1) * k / (2*N)),
// alpha(0) = sqrt(1/N), alpha(k>0) = sqrt(2/N)
static inline void idct_1d_9(
    const float* src, int src_stride,
    float* dst,       int dst_stride)
{
    float s0 = 0.0f;
    float s1 = 0.0f;
    float s2 = 0.0f;
    float s3 = 0.0f;
    float s4 = 0.0f;
    float s5 = 0.0f;
    float s6 = 0.0f;
    float s7 = 0.0f;
    float s8 = 0.0f;

    {
        float v = src[0 * src_stride];
        if (v != 0.0f)
        {
            s0 += 3.333333433e-01f * v;
            s1 += 3.333333433e-01f * v;
            s2 += 3.333333433e-01f * v;
            s3 += 3.333333433e-01f * v;
            s4 += 3.333333433e-01f * v;
            s5 += 3.333333433e-01f * v;
            s6 += 3.333333433e-01f * v;
            s7 += 3.333333433e-01f * v;
            s8 += 3.333333433e-01f * v;
        }
    }

    {
        float v = src[1 * src_stride];
        if (v != 0.0f)
        {
            s0 += 4.642428160e-01f * v;
            s1 += 4.082482755e-01f * v;
            s2 += 3.030129671e-01f * v;
            s3 += 1.612297893e-01f * v;
            s5 += -1.612298936e-01f * v;
            s6 += -3.030129969e-01f * v;
            s7 += -4.082482755e-01f * v;
            s8 += -4.642428458e-01f * v;
        }
    }

    {
        float v = src[2 * src_stride];
        if (v != 0.0f)
        {
            s0 += 4.429753423e-01f * v;
            s1 += 2.357022464e-01f * v;
            s2 += -8.185859025e-02f * v;
            s3 += -3.611168861e-01f * v;
            s4 += -4.714045227e-01f * v;
            s5 += -3.611167669e-01f * v;
            s6 += -8.185851574e-02f * v;
            s7 += 2.357022166e-01f * v;
            s8 += 4.429753721e-01f * v;
        }
    }

    {
        float v = src[3 * src_stride];
        if (v != 0.0f)
        {
            s0 += 4.082482755e-01f * v;
            s2 += -4.082482755e-01f * v;
            s3 += -4.082482159e-01f * v;
            s5 += 4.082483649e-01f * v;
            s6 += 4.082482755e-01f * v;
            s8 += -4.082485437e-01f * v;
        }
    }

    {
        float v = src[4 * src_stride];
        if (v != 0.0f)
        {
            s0 += 3.611168265e-01f * v;
            s1 += -2.357022911e-01f * v;
            s2 += -4.429753125e-01f * v;
            s3 += 8.185874671e-02f * v;
            s4 += 4.714045227e-01f * v;
            s5 += 8.185835928e-02f * v;
            s6 += -4.429753721e-01f * v;
            s7 += -2.357023507e-01f * v;
            s8 += 3.611169457e-01f * v;
        }
    }

    {
        float v = src[5 * src_stride];
        if (v != 0.0f)
        {
            s0 += 3.030129671e-01f * v;
            s1 += -4.082482755e-01f * v;
            s2 += -1.612298042e-01f * v;
            s3 += 4.642428458e-01f * v;
            s5 += -4.642428160e-01f * v;
            s6 += 1.612296849e-01f * v;
            s7 += 4.082482159e-01f * v;
            s8 += -3.030129373e-01f * v;
        }
    }

    {
        float v = src[6 * src_stride];
        if (v != 0.0f)
        {
            s0 += 2.357022464e-01f * v;
            s1 += -4.714045227e-01f * v;
            s2 += 2.357022166e-01f * v;
            s3 += 2.357020825e-01f * v;
            s4 += -4.714045227e-01f * v;
            s5 += 2.357024848e-01f * v;
            s6 += 2.357022017e-01f * v;
            s7 += -4.714045227e-01f * v;
            s8 += 2.357031256e-01f * v;
        }
    }

    {
        float v = src[7 * src_stride];
        if (v != 0.0f)
        {
            s0 += 1.612297893e-01f * v;
            s1 += -4.082482159e-01f * v;
            s2 += 4.642428458e-01f * v;
            s3 += -3.030130565e-01f * v;
            s5 += 3.030129075e-01f * v;
            s6 += -4.642427862e-01f * v;
            s7 += 4.082485735e-01f * v;
            s8 += -1.612301171e-01f * v;
        }
    }

    {
        float v = src[8 * src_stride];
        if (v != 0.0f)
        {
            s0 += 8.185850084e-02f * v;
            s1 += -2.357022166e-01f * v;
            s2 += 3.611166775e-01f * v;
            s3 += -4.429752231e-01f * v;
            s4 += 4.714045227e-01f * v;
            s5 += -4.429754615e-01f * v;
            s6 += 3.611168563e-01f * v;
            s7 += -2.357021123e-01f * v;
            s8 += 8.185899258e-02f * v;
        }
    }

    dst[0 * dst_stride] = s0;
    dst[1 * dst_stride] = s1;
    dst[2 * dst_stride] = s2;
    dst[3 * dst_stride] = s3;
    dst[4 * dst_stride] = s4;
    dst[5 * dst_stride] = s5;
    dst[6 * dst_stride] = s6;
    dst[7 * dst_stride] = s7;
    dst[8 * dst_stride] = s8;
}

// ------------------------------------------------------------
// 1D ORTHONORMAL IDCT (DCT-III), SIZE 10, FLOAT
// out[x*dst_stride] = sum_k C[k][x] * src[k*src_stride]
// C[k][x] = alpha(k) * cos(pi * (2*x+1) * k / (2*N)),
// alpha(0) = sqrt(1/N), alpha(k>0) = sqrt(2/N)
static inline void idct_1d_10(
    const float* src, int src_stride,
    float* dst,       int dst_stride)
{
    float s0 = 0.0f;
    float s1 = 0.0f;
    float s2 = 0.0f;
    float s3 = 0.0f;
    float s4 = 0.0f;
    float s5 = 0.0f;
    float s6 = 0.0f;
    float s7 = 0.0f;
    float s8 = 0.0f;
    float s9 = 0.0f;

    {
        float v = src[0 * src_stride];
        if (v != 0.0f)
        {
            s0 += 3.162277639e-01f * v;
            s1 += 3.162277639e-01f * v;
            s2 += 3.162277639e-01f * v;
            s3 += 3.162277639e-01f * v;
            s4 += 3.162277639e-01f * v;
            s5 += 3.162277639e-01f * v;
            s6 += 3.162277639e-01f * v;
            s7 += 3.162277639e-01f * v;
            s8 += 3.162277639e-01f * v;
            s9 += 3.162277639e-01f * v;
        }
    }

    {
        float v = src[1 * src_stride];
        if (v != 0.0f)
        {
            s0 += 4.417076707e-01f * v;
            s1 += 3.984702229e-01f * v;
            s2 += 3.162277639e-01f * v;
            s3 += 2.030306906e-01f * v;
            s4 += 6.995963305e-02f * v;
            s5 += -6.995966285e-02f * v;
            s6 += -2.030307651e-01f * v;
            s7 += -3.162277639e-01f * v;
            s8 += -3.984702528e-01f * v;
            s9 += -4.417076707e-01f * v;
        }
    }

    {
        float v = src[2 * src_stride];
        if (v != 0.0f)
        {
            s0 += 4.253254235e-01f * v;
            s1 += 2.628655434e-01f * v;
            s3 += -2.628656328e-01f * v;
            s4 += -4.253253937e-01f * v;
            s5 += -4.253253639e-01f * v;
            s6 += -2.628654838e-01f * v;
            s8 += 2.628656626e-01f * v;
            s9 += 4.253254235e-01f * v;
        }
    }

    {
        float v = src[3 * src_stride];
        if (v != 0.0f)
        {
            s0 += 3.984702229e-01f * v;
            s1 += 6.995963305e-02f * v;
            s2 += -3.162277639e-01f * v;
            s3 += -4.417076409e-01f * v;
            s4 += -2.030306011e-01f * v;
            s5 += 2.030307949e-01f * v;
            s6 += 4.417076707e-01f * v;
            s7 += 3.162277639e-01f * v;
            s8 += -6.995979697e-02f * v;
            s9 += -3.984701931e-01f * v;
        }
    }

    {
        float v = src[4 * src_stride];
        if (v != 0.0f)
        {
            s0 += 3.618034124e-01f * v;
            s1 += -1.381966174e-01f * v;
            s2 += -4.472135901e-01f * v;
            s3 += -1.381964386e-01f * v;
            s4 += 3.618033826e-01f * v;
            s5 += 3.618032932e-01f * v;
            s6 += -1.381967962e-01f * v;
            s7 += -4.472135901e-01f * v;
            s8 += -1.381963789e-01f * v;
            s9 += 3.618034124e-01f * v;
        }
    }

    {
        float v = src[5 * src_stride];
        if (v != 0.0f)
        {
            s0 += 3.162277639e-01f * v;
            s1 += -3.162277639e-01f * v;
            s2 += -3.162277043e-01f * v;
            s3 += 3.162278533e-01f * v;
            s4 += 3.162277639e-01f * v;
            s5 += -3.162276745e-01f * v;
            s6 += -3.162276447e-01f * v;
            s7 += 3.162280619e-01f * v;
            s8 += 3.162278533e-01f * v;
            s9 += -3.162281811e-01f * v;
        }
    }

    {
        float v = src[6 * src_stride];
        if (v != 0.0f)
        {
            s0 += 2.628655434e-01f * v;
            s1 += -4.253253937e-01f * v;
            s3 += 4.253253639e-01f * v;
            s4 += -2.628657520e-01f * v;
            s5 += -2.628654242e-01f * v;
            s6 += 4.253254235e-01f * v;
            s8 += -4.253252745e-01f * v;
            s9 += 2.628654540e-01f * v;
        }
    }

    {
        float v = src[7 * src_stride];
        if (v != 0.0f)
        {
            s0 += 2.030306906e-01f * v;
            s1 += -4.417076409e-01f * v;
            s2 += 3.162278533e-01f * v;
            s3 += 6.995949894e-02f * v;
            s4 += -3.984701633e-01f * v;
            s5 += 3.984702528e-01f * v;
            s6 += -6.996008009e-02f * v;
            s7 += -3.162274361e-01f * v;
            s8 += 4.417077899e-01f * v;
            s9 += -2.030310780e-01f * v;
        }
    }

    {
        float v = src[8 * src_stride];
        if (v != 0.0f)
        {
            s0 += 1.381965876e-01f * v;
            s1 += -3.618033826e-01f * v;
            s2 += 4.472135901e-01f * v;
            s3 += -3.618035913e-01f * v;
            s4 += 1.381965429e-01f * v;
            s5 += 1.381962299e-01f * v;
            s6 += -3.618031442e-01f * v;
            s7 += 4.472135901e-01f * v;
            s8 += -3.618036509e-01f * v;
            s9 += 1.381966770e-01f * v;
        }
    }

    {
        float v = src[9 * src_stride];
        if (v != 0.0f)
        {
            s0 += 6.995963305e-02f * v;
            s1 += -2.030306011e-01f * v;
            s2 += 3.162277639e-01f * v;
            s3 += -3.984701633e-01f * v;
            s4 += 4.417076409e-01f * v;
            s5 += -4.417076409e-01f * v;
            s6 += 3.984701931e-01f * v;
            s7 += -3.162280619e-01f * v;
            s8 += 2.030308247e-01f * v;
            s9 += -6.995939463e-02f * v;
        }
    }

    dst[0 * dst_stride] = s0;
    dst[1 * dst_stride] = s1;
    dst[2 * dst_stride] = s2;
    dst[3 * dst_stride] = s3;
    dst[4 * dst_stride] = s4;
    dst[5 * dst_stride] = s5;
    dst[6 * dst_stride] = s6;
    dst[7 * dst_stride] = s7;
    dst[8 * dst_stride] = s8;
    dst[9 * dst_stride] = s9;
}

// ------------------------------------------------------------
// 1D ORTHONORMAL IDCT (DCT-III), SIZE 11, FLOAT
// out[x*dst_stride] = sum_k C[k][x] * src[k*src_stride]
// C[k][x] = alpha(k) * cos(pi * (2*x+1) * k / (2*N)),
// alpha(0) = sqrt(1/N), alpha(k>0) = sqrt(2/N)
static inline void idct_1d_11(
    const float* src, int src_stride,
    float* dst,       int dst_stride)
{
    float s0 = 0.0f;
    float s1 = 0.0f;
    float s2 = 0.0f;
    float s3 = 0.0f;
    float s4 = 0.0f;
    float s5 = 0.0f;
    float s6 = 0.0f;
    float s7 = 0.0f;
    float s8 = 0.0f;
    float s9 = 0.0f;
    float s10 = 0.0f;

    {
        float v = src[0 * src_stride];
        if (v != 0.0f)
        {
            s0 += 3.015113473e-01f * v;
            s1 += 3.015113473e-01f * v;
            s2 += 3.015113473e-01f * v;
            s3 += 3.015113473e-01f * v;
            s4 += 3.015113473e-01f * v;
            s5 += 3.015113473e-01f * v;
            s6 += 3.015113473e-01f * v;
            s7 += 3.015113473e-01f * v;
            s8 += 3.015113473e-01f * v;
            s9 += 3.015113473e-01f * v;
            s10 += 3.015113473e-01f * v;
        }
    }

    {
        float v = src[1 * src_stride];
        if (v != 0.0f)
        {
            s0 += 4.220612943e-01f * v;
            s1 += 3.878683746e-01f * v;
            s2 += 3.222526908e-01f * v;
            s3 += 2.305300087e-01f * v;
            s4 += 1.201311573e-01f * v;
            s6 += -1.201311946e-01f * v;
            s7 += -2.305300087e-01f * v;
            s8 += -3.222527206e-01f * v;
            s9 += -3.878683746e-01f * v;
            s10 += -4.220612943e-01f * v;
        }
    }

    {
        float v = src[2 * src_stride];
        if (v != 0.0f)
        {
            s0 += 4.091291726e-01f * v;
            s1 += 2.792335451e-01f * v;
            s2 += 6.068321317e-02f * v;
            s3 += -1.771336049e-01f * v;
            s4 += -3.587117195e-01f * v;
            s5 += -4.264014363e-01f * v;
            s6 += -3.587116897e-01f * v;
            s7 += -1.771335900e-01f * v;
            s8 += 6.068333238e-02f * v;
            s9 += 2.792335451e-01f * v;
            s10 += 4.091292024e-01f * v;
        }
    }

    {
        float v = src[3 * src_stride];
        if (v != 0.0f)
        {
            s0 += 3.878683746e-01f * v;
            s1 += 1.201311573e-01f * v;
            s2 += -2.305300087e-01f * v;
            s3 += -4.220612943e-01f * v;
            s4 += -3.222526908e-01f * v;
            s6 += 3.222527504e-01f * v;
            s7 += 4.220612645e-01f * v;
            s8 += 2.305298299e-01f * v;
            s9 += -1.201310679e-01f * v;
            s10 += -3.878685534e-01f * v;
        }
    }

    {
        float v = src[4 * src_stride];
        if (v != 0.0f)
        {
            s0 += 3.587117195e-01f * v;
            s1 += -6.068325043e-02f * v;
            s2 += -4.091292024e-01f * v;
            s3 += -2.792334855e-01f * v;
            s4 += 1.771336049e-01f * v;
            s5 += 4.264014363e-01f * v;
            s6 += 1.771334559e-01f * v;
            s7 += -2.792335153e-01f * v;
            s8 += -4.091291428e-01f * v;
            s9 += -6.068325043e-02f * v;
            s10 += 3.587118387e-01f * v;
        }
    }

    {
        float v = src[5 * src_stride];
        if (v != 0.0f)
        {
            s0 += 3.222526908e-01f * v;
            s1 += -2.305300087e-01f * v;
            s2 += -3.878683448e-01f * v;
            s3 += 1.201313213e-01f * v;
            s4 += 4.220612645e-01f * v;
            s6 += -4.220612943e-01f * v;
            s7 += -1.201310530e-01f * v;
            s8 += 3.878682852e-01f * v;
            s9 += 2.305295914e-01f * v;
            s10 += -3.222530484e-01f * v;
        }
    }

    {
        float v = src[6 * src_stride];
        if (v != 0.0f)
        {
            s0 += 2.792335451e-01f * v;
            s1 += -3.587117195e-01f * v;
            s2 += -1.771335900e-01f * v;
            s3 += 4.091292024e-01f * v;
            s4 += 6.068318710e-02f * v;
            s5 += -4.264014363e-01f * v;
            s6 += 6.068341061e-02f * v;
            s7 += 4.091290832e-01f * v;
            s8 += -1.771339774e-01f * v;
            s9 += -3.587118387e-01f * v;
            s10 += 2.792341411e-01f * v;
        }
    }

    {
        float v = src[7 * src_stride];
        if (v != 0.0f)
        {
            s0 += 2.305300087e-01f * v;
            s1 += -4.220612943e-01f * v;
            s2 += 1.201313213e-01f * v;
            s3 += 3.222525418e-01f * v;
            s4 += -3.878685534e-01f * v;
            s6 += 3.878683150e-01f * v;
            s7 += -3.222530484e-01f * v;
            s8 += -1.201303899e-01f * v;
            s9 += 4.220611751e-01f * v;
            s10 += -2.305305302e-01f * v;
        }
    }

    {
        float v = src[8 * src_stride];
        if (v != 0.0f)
        {
            s0 += 1.771335304e-01f * v;
            s1 += -4.091291726e-01f * v;
            s2 += 3.587118089e-01f * v;
            s3 += -6.068347394e-02f * v;
            s4 += -2.792334855e-01f * v;
            s5 += 4.264014363e-01f * v;
            s6 += -2.792337239e-01f * v;
            s7 += -6.068337709e-02f * v;
            s8 += 3.587115407e-01f * v;
            s9 += -4.091291726e-01f * v;
            s10 += 1.771339774e-01f * v;
        }
    }

    {
        float v = src[9 * src_stride];
        if (v != 0.0f)
        {
            s0 += 1.201311573e-01f * v;
            s1 += -3.222526908e-01f * v;
            s2 += 4.220612645e-01f * v;
            s3 += -3.878685534e-01f * v;
            s4 += 2.305301726e-01f * v;
            s6 += -2.305298299e-01f * v;
            s7 += 3.878681958e-01f * v;
            s8 += -4.220613837e-01f * v;
            s9 += 3.222527504e-01f * v;
            s10 += -1.201314703e-01f * v;
        }
    }

    {
        float v = src[10 * src_stride];
        if (v != 0.0f)
        {
            s0 += 6.068321317e-02f * v;
            s1 += -1.771335900e-01f * v;
            s2 += 2.792334557e-01f * v;
            s3 += -3.587115407e-01f * v;
            s4 += 4.091290832e-01f * v;
            s5 += -4.264014363e-01f * v;
            s6 += 4.091292620e-01f * v;
            s7 += -3.587118387e-01f * v;
            s8 += 2.792330980e-01f * v;
            s9 += -1.771344692e-01f * v;
            s10 += 6.068423390e-02f * v;
        }
    }

    dst[0 * dst_stride] = s0;
    dst[1 * dst_stride] = s1;
    dst[2 * dst_stride] = s2;
    dst[3 * dst_stride] = s3;
    dst[4 * dst_stride] = s4;
    dst[5 * dst_stride] = s5;
    dst[6 * dst_stride] = s6;
    dst[7 * dst_stride] = s7;
    dst[8 * dst_stride] = s8;
    dst[9 * dst_stride] = s9;
    dst[10 * dst_stride] = s10;
}

// ------------------------------------------------------------
// 1D ORTHONORMAL IDCT (DCT-III), SIZE 12, FLOAT
// out[x*dst_stride] = sum_k C[k][x] * src[k*src_stride]
// C[k][x] = alpha(k) * cos(pi * (2*x+1) * k / (2*N)),
// alpha(0) = sqrt(1/N), alpha(k>0) = sqrt(2/N)
static inline void idct_1d_12(
    const float* src, int src_stride,
    float* dst,       int dst_stride)
{
    float s0 = 0.0f;
    float s1 = 0.0f;
    float s2 = 0.0f;
    float s3 = 0.0f;
    float s4 = 0.0f;
    float s5 = 0.0f;
    float s6 = 0.0f;
    float s7 = 0.0f;
    float s8 = 0.0f;
    float s9 = 0.0f;
    float s10 = 0.0f;
    float s11 = 0.0f;

    {
        float v = src[0 * src_stride];
        if (v != 0.0f)
        {
            s0 += 2.886751294e-01f * v;
            s1 += 2.886751294e-01f * v;
            s2 += 2.886751294e-01f * v;
            s3 += 2.886751294e-01f * v;
            s4 += 2.886751294e-01f * v;
            s5 += 2.886751294e-01f * v;
            s6 += 2.886751294e-01f * v;
            s7 += 2.886751294e-01f * v;
            s8 += 2.886751294e-01f * v;
            s9 += 2.886751294e-01f * v;
            s10 += 2.886751294e-01f * v;
            s11 += 2.886751294e-01f * v;
        }
    }

    {
        float v = src[1 * src_stride];
        if (v != 0.0f)
        {
            s0 += 4.047556818e-01f * v;
            s1 += 3.771722317e-01f * v;
            s2 += 3.238851428e-01f * v;
            s3 += 2.485257983e-01f * v;
            s4 += 1.562298536e-01f * v;
            s5 += 5.328707024e-02f * v;
            s6 += -5.328710750e-02f * v;
            s7 += -1.562298536e-01f * v;
            s8 += -2.485258281e-01f * v;
            s9 += -3.238851428e-01f * v;
            s10 += -3.771722913e-01f * v;
            s11 += -4.047556818e-01f * v;
        }
    }

    {
        float v = src[2 * src_stride];
        if (v != 0.0f)
        {
            s0 += 3.943375647e-01f * v;
            s1 += 2.886751294e-01f * v;
            s2 += 1.056623980e-01f * v;
            s3 += -1.056624874e-01f * v;
            s4 += -2.886751294e-01f * v;
            s5 += -3.943375945e-01f * v;
            s6 += -3.943375647e-01f * v;
            s7 += -2.886751592e-01f * v;
            s8 += -1.056624129e-01f * v;
            s9 += 1.056624204e-01f * v;
            s10 += 2.886752486e-01f * v;
            s11 += 3.943375647e-01f * v;
        }
    }

    {
        float v = src[3 * src_stride];
        if (v != 0.0f)
        {
            s0 += 3.771722317e-01f * v;
            s1 += 1.562298536e-01f * v;
            s2 += -1.562298536e-01f * v;
            s3 += -3.771722913e-01f * v;
            s4 += -3.771722317e-01f * v;
            s5 += -1.562297344e-01f * v;
            s6 += 1.562299281e-01f * v;
            s7 += 3.771722615e-01f * v;
            s8 += 3.771722019e-01f * v;
            s9 += 1.562297940e-01f * v;
            s10 += -1.562300622e-01f * v;
            s11 += -3.771722317e-01f * v;
        }
    }

    {
        float v = src[4 * src_stride];
        if (v != 0.0f)
        {
            s0 += 3.535533845e-01f * v;
            s2 += -3.535534441e-01f * v;
            s3 += -3.535533547e-01f * v;
            s5 += 3.535534739e-01f * v;
            s6 += 3.535533845e-01f * v;
            s8 += -3.535534143e-01f * v;
            s9 += -3.535534143e-01f * v;
            s11 += 3.535532951e-01f * v;
        }
    }

    {
        float v = src[5 * src_stride];
        if (v != 0.0f)
        {
            s0 += 3.238851428e-01f * v;
            s1 += -1.562298536e-01f * v;
            s2 += -4.047556818e-01f * v;
            s3 += -5.328698456e-02f * v;
            s4 += 3.771722615e-01f * v;
            s5 += 2.485257536e-01f * v;
            s6 += -2.485258281e-01f * v;
            s7 += -3.771722317e-01f * v;
            s8 += 5.328687653e-02f * v;
            s9 += 4.047557116e-01f * v;
            s10 += 1.562295407e-01f * v;
            s11 += -3.238854110e-01f * v;
        }
    }

    {
        float v = src[6 * src_stride];
        if (v != 0.0f)
        {
            s0 += 2.886751294e-01f * v;
            s1 += -2.886751294e-01f * v;
            s2 += -2.886751592e-01f * v;
            s3 += 2.886752486e-01f * v;
            s4 += 2.886749804e-01f * v;
            s5 += -2.886753380e-01f * v;
            s6 += -2.886750400e-01f * v;
            s7 += 2.886751592e-01f * v;
            s8 += 2.886749506e-01f * v;
            s9 += -2.886752486e-01f * v;
            s10 += -2.886748612e-01f * v;
            s11 += 2.886750698e-01f * v;
        }
    }

    {
        float v = src[7 * src_stride];
        if (v != 0.0f)
        {
            s0 += 2.485257983e-01f * v;
            s1 += -3.771722913e-01f * v;
            s2 += -5.328698456e-02f * v;
            s3 += 4.047556818e-01f * v;
            s4 += -1.562300622e-01f * v;
            s5 += -3.238852322e-01f * v;
            s6 += 3.238853514e-01f * v;
            s7 += 1.562295407e-01f * v;
            s8 += -4.047557414e-01f * v;
            s9 += 5.328752100e-02f * v;
            s10 += 3.771720827e-01f * v;
            s11 += -2.485256344e-01f * v;
        }
    }

    {
        float v = src[8 * src_stride];
        if (v != 0.0f)
        {
            s0 += 2.041241378e-01f * v;
            s1 += -4.082483053e-01f * v;
            s2 += 2.041243017e-01f * v;
            s3 += 2.041239887e-01f * v;
            s4 += -4.082483053e-01f * v;
            s5 += 2.041243464e-01f * v;
            s6 += 2.041241080e-01f * v;
            s7 += -4.082483053e-01f * v;
            s8 += 2.041242570e-01f * v;
            s9 += 2.041241974e-01f * v;
            s10 += -4.082483053e-01f * v;
            s11 += 2.041237950e-01f * v;
        }
    }

    {
        float v = src[9 * src_stride];
        if (v != 0.0f)
        {
            s0 += 1.562298536e-01f * v;
            s1 += -3.771722317e-01f * v;
            s2 += 3.771722615e-01f * v;
            s3 += -1.562300622e-01f * v;
            s4 += -1.562296748e-01f * v;
            s5 += 3.771723211e-01f * v;
            s6 += -3.771723509e-01f * v;
            s7 += 1.562300622e-01f * v;
            s8 += 1.562293023e-01f * v;
            s9 += -3.771721721e-01f * v;
            s10 += 3.771724999e-01f * v;
            s11 += -1.562300622e-01f * v;
        }
    }

    {
        float v = src[10 * src_stride];
        if (v != 0.0f)
        {
            s0 += 1.056623980e-01f * v;
            s1 += -2.886751592e-01f * v;
            s2 += 3.943375647e-01f * v;
            s3 += -3.943376541e-01f * v;
            s4 += 2.886751592e-01f * v;
            s5 += -1.056626216e-01f * v;
            s6 += -1.056624576e-01f * v;
            s7 += 2.886750400e-01f * v;
            s8 += -3.943376839e-01f * v;
            s9 += 3.943377137e-01f * v;
            s10 += -2.886756361e-01f * v;
            s11 += 1.056632623e-01f * v;
        }
    }

    {
        float v = src[11 * src_stride];
        if (v != 0.0f)
        {
            s0 += 5.328707024e-02f * v;
            s1 += -1.562297344e-01f * v;
            s2 += 2.485257536e-01f * v;
            s3 += -3.238852322e-01f * v;
            s4 += 3.771723211e-01f * v;
            s5 += -4.047556818e-01f * v;
            s6 += 4.047556818e-01f * v;
            s7 += -3.771722913e-01f * v;
            s8 += 3.238852024e-01f * v;
            s9 += -2.485264540e-01f * v;
            s10 += 1.562305540e-01f * v;
            s11 += -5.328702182e-02f * v;
        }
    }

    dst[0 * dst_stride] = s0;
    dst[1 * dst_stride] = s1;
    dst[2 * dst_stride] = s2;
    dst[3 * dst_stride] = s3;
    dst[4 * dst_stride] = s4;
    dst[5 * dst_stride] = s5;
    dst[6 * dst_stride] = s6;
    dst[7 * dst_stride] = s7;
    dst[8 * dst_stride] = s8;
    dst[9 * dst_stride] = s9;
    dst[10 * dst_stride] = s10;
    dst[11 * dst_stride] = s11;
}
