float4 main() : SV_Position
{
    float f1 = -1.#INF;
    float f2 =  1.#INF;
    float f3 = +1.#INF;
    float f4 = f2 * 1.#INF + 1.#INF;
    const float f5 = -1.#INF;
    // An infinity times zero is a NaN.
    // In IEEE 754, the sign of a NaN is significant only for
    // abs, copy, negate, or copySign.  Use abs(.) here to
    // set the sign bit to zero. Otherwise, some platforms will
    // have a 1 sign bit and others will have a 0 sign bit.
    const float f6 = abs(f5 * 0.0f);

    return (float4)(f1 + f2 + f3 + f4 + f5 + f6);
}
