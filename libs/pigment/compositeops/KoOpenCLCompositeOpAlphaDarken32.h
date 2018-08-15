/*
 * Copyright (c) 2018 Andrew Savonichev <andrew.savonichev@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KOOPENCLCOMPOSITEOPALPHADARKEN32_H_
#define KOOPENCLCOMPOSITEOPALPHADARKEN32_H_

#include "KoCompositeOpBase.h"
#include "KoCompositeOpRegistry.h"
#include <klocalizedstring.h>
#include "KoStreamedMath.h"

#include <kis_opencl_context.h>
#include <kis_opencl_utils.h>
#include <kis_debug.h>


/**
 * An OpenCL version of a composite op for the use in 4 byte
 * colorspaces with alpha channel placed at the last byte of
 * the pixel: C1_C2_C3_A.
 */
class KoOpenCLCompositeOpAlphaDarken32 : public KoCompositeOp
{
public:
    KoOpenCLCompositeOpAlphaDarken32(const KoColorSpace* cs)
            : KoCompositeOp(cs, COMPOSITE_ALPHA_DARKEN, i18n("Alpha darken"),
                            KoCompositeOp::categoryMix()),
              m_context(ocl::GlobalContext),
              m_kernel(m_context.kernelByName("compositeAlphaDarken32")) {}

    virtual ~KoOpenCLCompositeOpAlphaDarken32() {
        clReleaseKernel(m_kernel);
    }

    using KoCompositeOp::composite;

    struct CLCompositeBuffers {
        CLCompositeBuffers(ocl::KisCLContext& context,
                           const KoCompositeOp::ParameterInfo& params,
                           size_t pixelSize):
                srcBuf(nullptr), srcSize(0),
                dstBuf(nullptr), dstSize(0),
                maskBuf(nullptr), maskSize(0) {
            //              stride
            //  +-  --  --  --  --  --+
            //  -          cols       -
            //  +--------+--------+---+       +--------+--------+---+
            //  |    x ->.        |r  |       +--------+xxxxxxxx|yyy|
            //  |x+stride.        |o  |       |yyyyyyyy|xxxxxxxx|yyy|
            //  |        |        |w  |       |yyyyyyyy|xxxxxxxx+---+
            //  |        +--------+s  |       +--------+--------+   |
            //  |                     |       |                     |
            //  |                     |       |                     |
            //  +---------------------+       +---------------------+
            //        figure 1                      figure 2
            //
            // We have a working area of COLSxROWS size, which is a part of a
            // larger block of continuous memory (see fig.1). We have a pointer
            // to the beginning of our working area (X), and a STRIDE (in
            // bytes), which is an offset from the beginning of one row, to the
            // beginning of the next row.
            //
            // In order to transfer our working area COLSxROWS (marked X on
            // fig.2), we need to transfer continuous memory block surrounding
            // it (both X and Y on fig.2).
            //
            // We already have a pointer to X, so we can compute size to
            // transfer:
            //
            //     buffer_size = (rows-1)*stride + cols*pixelsize
            //

            srcSize = pixelSize * params.cols +
                params.srcRowStride * (params.rows - 1);

            dstSize = pixelSize * params.cols +
                params.dstRowStride * (params.rows - 1);

            maskSize = sizeof(quint8) * params.cols +
                params.maskRowStride * (params.rows - 1);

            if (params.srcRowStride == 0) {
                // Special case: we have only one pixel as input, when all pixels in
                // src[cols:rows] have the same color.
                srcSize = pixelSize;
            }

            // TODO: re-use buffers
            srcBuf = clCreateBuffer(context.context(),
                                    CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                    srcSize,
                                    const_cast<quint8*>(params.srcRowStart),
                                    &error);
            CLCHECK(error, "clCreateBuffer")

            dstBuf = clCreateBuffer(context.context(),
                                    CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
                                    dstSize,
                                    params.dstRowStart,
                                    &error);
            CLCHECK(error, "clCreateBuffer")

            maskBuf = nullptr;
            if (params.maskRowStart) {
                maskBuf = clCreateBuffer(context.context(),
                                         CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                         maskSize,
                                         (void*)params.maskRowStart,
                                         &error);
                CLCHECK(error, "clCreateBuffer")
            }
        }

        ~CLCompositeBuffers() {
            clReleaseMemObject(srcBuf);
            clReleaseMemObject(dstBuf);
            if (maskBuf) {
                clReleaseMemObject(maskBuf);
            }
        }

        cl_mem srcBuf;
        size_t srcSize;

        cl_mem dstBuf;
        size_t dstSize;

        cl_mem maskBuf;
        size_t maskSize;

        cl_int error;
    };

    virtual void composite(const KoCompositeOp::ParameterInfo& params) const
    {
        if (!m_kernel) {
            return;
        }
        cl_int error = CL_SUCCESS;

        size_t pixelSize = 4;
        CLCompositeBuffers buffers(m_context, params, pixelSize);
        if (buffers.error != CL_SUCCESS) {
            dbgOpenCL << "failed to allocate device memory" <<
                ocl::errorStr(buffers.error);
            return;
        }

        ocl::CLKernelArg args[] = {
            buffers.srcBuf,
            buffers.dstBuf,
            buffers.maskBuf,
            params.srcRowStride,
            params.dstRowStride,
            params.maskRowStride,
            params.opacity,
            *params.lastOpacity,
            params.flow,
        };
        int failedArg = ocl::setKernelArgs(m_kernel, args, &error);
        if (error != CL_SUCCESS) {
            dbgOpenCL << "failed to set kernel arg" << failedArg << ":"
                      << ocl::errorStr(error);
            return;
        }

        size_t globalWorkSize[] = { (size_t)params.rows, (size_t)params.cols };
        size_t globalWorkDim =
            sizeof(globalWorkSize) / sizeof(*globalWorkSize);

        cl_event kernelEvent;
        error = clEnqueueNDRangeKernel(m_context.defaultQueue(),
                                       m_kernel,
                                       /*work_dim=*/globalWorkDim,
                                       /*global_work_offset=*/NULL,
                                       globalWorkSize,
                                       /*local_work_size=*/NULL,
                                       /*num_events=*/0,
                                       /*events=*/NULL,
                                       &kernelEvent);
        CLCHECK(error, "clEnqueueNDRangeKernel");

        error = clEnqueueReadBuffer(m_context.defaultQueue(),
                                    buffers.dstBuf,
                                    /*blocking=*/CL_TRUE,
                                    /*offset=*/ 0,
                                    buffers.dstSize,
                                    params.dstRowStart,
                                    /*num_events=*/ 0,
                                    /*events=*/ 0,
                                    /*event=*/ nullptr);
        CLCHECK(error, "clEnqueueReadBuffer");
    }

private:
    ocl::KisCLContext& m_context;
    cl_kernel m_kernel;
};

#endif // KOOPENCLCOMPOSITEOPALPHADARKEN32_H_
