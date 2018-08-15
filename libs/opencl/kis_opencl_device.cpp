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

#include "kis_opencl_device.h"
#include <cstring>

namespace ocl {

static cl_device_type deviceTypeFromStr(const char* str) {
    if (std::strcmp("CL_DEVICE_TYPE_CPU", str) == 0) {
        return CL_DEVICE_TYPE_CPU;
    }
    if (std::strcmp("CL_DEVICE_TYPE_GPU", str) == 0) {
        return CL_DEVICE_TYPE_GPU;
    }
    if (std::strcmp("CL_DEVICE_TYPE_ACCELERATOR", str) == 0) {
        return CL_DEVICE_TYPE_ACCELERATOR;
    }
    if (std::strcmp("CL_DEVICE_TYPE_DEFAULT", str) == 0) {
        return CL_DEVICE_TYPE_ACCELERATOR;
    }

    return CL_DEVICE_TYPE_ALL;
}


cl_device_id getDefaultDevice(cl_platform_id platform) {
    cl_device_type deviceType = CL_DEVICE_TYPE_ALL;
    if (auto* env = getenv("KIS_OCL_DEVICE_TYPE")) {
        deviceType = deviceTypeFromStr(env);
    }

    cl_uint numDevices = 0;
    cl_int error = clGetDeviceIDs(platform, deviceType, 0, NULL, &numDevices);
    if (error != CL_SUCCESS || !numDevices) {
        return nullptr;
    }

    cl_device_id device;
    error = clGetDeviceIDs(platform, deviceType, 1, &device, NULL);
    if (error != CL_SUCCESS) {
        return nullptr;
    }

    return device;
}

} // namespace ocl
