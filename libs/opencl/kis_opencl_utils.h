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

#ifndef KIS_OPENCL_UTILS_H
#define KIS_OPENCL_UTILS_H

#include <QDebug>
#include <CL/cl.h>

#include <kritaglobal_export.h>

#define CLCHECK(error, api)                                                   \
    if (error) { dbgOpenCL << api << "failed:" << ocl::errorStr(error);       \
        return;                                                               \
    }
#define CLCHECKRET(error, api)                                                \
    if (error) { dbgOpenCL << api << "failed:" << ocl::errorStr(error);       \
        return err;                                                           \
    }

namespace ocl {

/// Returns an error string for the OpenCL error codes defined in CL/cl.h
KRITAGLOBAL_EXPORT const char* errorStr(cl_int error);

template<typename T>
cl_int setKernelArg(cl_kernel kernel, int num, T& arg) {
    return clSetKernelArg(kernel, num, sizeof(T), &arg);
}

struct CLKernelArg {
    template<typename T>
    CLKernelArg(T& arg) : ptr(&arg), size(sizeof(arg)) {}
    const void* ptr;
    size_t size;
};

/// Set kernel arguments from \p args array. In case of an error, return a
/// number of argument that caused it. If all arguments were set correctly,
/// return -1 and set \p error to CL_SUCCESS.
template<int N>
int setKernelArgs(cl_kernel kernel, CLKernelArg (&args)[N], cl_int* error) {
    *error = CL_SUCCESS;
    for (int i = 0; i < N; ++i) {
        *error = clSetKernelArg(kernel, i, args[i].size, args[i].ptr);
        if (*error != CL_SUCCESS) {
            return i;
        }
    }
    return -1;
}

void printKernelNames(cl_program program);

} // namespace ocl

#endif // KIS_OPENCL_UTILS_H
