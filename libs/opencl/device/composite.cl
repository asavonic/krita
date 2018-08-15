/*
 *  Copyright (c) 2018 Andrew Savonichev <andrew.savonichev@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

static uchar round_float_to_uchar(float value) {
    return (uchar)(value + 0.5f);
}

static uchar lerp_mixed_u8_float(uchar a, uchar b, float alpha) {
    return round_float_to_uchar(((int)b - a) * alpha + a);
}

static float blend(float a, float b, float alpha) {
    return (a - b) * alpha + b;
}

static float lerp(float a, float b, float alpha) {
    return blend(b, a, alpha);
}

float unionShapeOpacity(float a, float b) {
    return (float)((double)a + b - (a * b));
}

__kernel void compositeAlphaDarken32(__global const uchar* src,
                                     __global uchar* dst,
                                     __global const uchar* mask,
                                     int srcStride,
                                     int dstStride,
                                     int maskStride,
                                     float opacity,
                                     float lastOpacity,
                                     float flow) {

    size_t srcOffset = get_global_id(0) * srcStride
        + get_global_id(1) * sizeof(uchar) * 4;
    size_t dstOffset = get_global_id(0) * dstStride
        + get_global_id(1) * sizeof(uchar) * 4;

    if (srcStride == 0) {
        // single pixel input
        srcOffset = 0;
    }

    src = src + srcOffset;
    dst = dst + dstOffset;

    if (mask != NULL) {
        size_t maskOffset = get_global_id(0) * maskStride + get_global_id(1);
        mask = mask + maskOffset;
    }

    const int alphaPos = 3;
    const float uint8Rec1 = 1.0f / 255.0f;
    const float uint8Rec2 = 1.0f / (255.0f * 255.0f);
    const float uint8Max = 255.0f;

    uchar dstAlphaInt = dst[alphaPos];
    float dstAlphaNorm =
        dstAlphaInt ? dstAlphaInt * uint8Rec1 : 0.0;

    opacity = opacity * flow;
    float srcAlphaNorm;
    float mskAlphaNorm;

    if (mask) {
        mskAlphaNorm = (float)*mask * uint8Rec2 * src[alphaPos];
        srcAlphaNorm = mskAlphaNorm * opacity;
    } else {
        mskAlphaNorm = src[alphaPos] * uint8Rec1;
        srcAlphaNorm = mskAlphaNorm * opacity;
    }

    if (dstAlphaInt != 0) {
        dst[0] = lerp_mixed_u8_float(dst[0], src[0], srcAlphaNorm);
        dst[1] = lerp_mixed_u8_float(dst[1], src[1], srcAlphaNorm);
        dst[2] = lerp_mixed_u8_float(dst[2], src[2], srcAlphaNorm);
    } else {
        dst[0] = src[0];
        dst[1] = src[1];
        dst[2] = src[2];
        dst[3] = src[3];
    }

    float averageOpacity = lastOpacity * flow;

    float fullFlowAlpha;
    if (averageOpacity > opacity) {
        fullFlowAlpha = (averageOpacity > dstAlphaNorm)
            ? lerp(srcAlphaNorm, averageOpacity, dstAlphaNorm / averageOpacity)
            : dstAlphaNorm;
    } else {
        fullFlowAlpha = (opacity > dstAlphaNorm)
            ? lerp(dstAlphaNorm, opacity, mskAlphaNorm)
            : dstAlphaNorm;
    }

    float dstAlpha;

    if (fabs(flow - 1.0f) < FLT_EPSILON) {
        dstAlpha = fullFlowAlpha * uint8Max;
    } else {
        float zeroFlowAlpha = unionShapeOpacity(srcAlphaNorm, dstAlphaNorm);
        dstAlpha = lerp(zeroFlowAlpha, fullFlowAlpha, flow) * uint8Max;
    }

    dst[alphaPos] = round_float_to_uchar(dstAlpha);
}

