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

#include "kis_opencl_platform.h"
#include <kis_debug.h>

namespace ocl {

static QString getPlatformInfo(cl_platform_id platform,
                               cl_platform_info query) {
    size_t infoSize = 0;
    cl_int error = clGetPlatformInfo(platform, query, 0, NULL, &infoSize);
    if (error != CL_SUCCESS || infoSize == 0) {
        return QString();
    }

    std::vector<char> info(infoSize);
    error = clGetPlatformInfo(platform, query, info.size(), &info[0], NULL);
    if (error != CL_SUCCESS) {
        return QString();
    }

    return QString(info.data());
}


/// Returns first OpenCL platform that matches given criteria
/// Pass nullptr if you dont't care.
static cl_platform_id selectPlatform(std::vector<cl_platform_id>& platforms,
                                     const char* vendorPattern = nullptr,
                                     const char* namePattern = nullptr) {
    for (auto platform : platforms) {
        QString vendor = getPlatformInfo(platform, CL_PLATFORM_VENDOR);
        QString name   = getPlatformInfo(platform, CL_PLATFORM_NAME);

        bool vendorMatch = !vendorPattern || vendor.contains(vendorPattern);
        bool nameMatch = !namePattern || name.contains(namePattern);

        if (vendorMatch && nameMatch) {
            dbgOpenCL.nospace() << "platform [" << vendor << "] "
                                << name << " matches";
            return platform;
        }
    }

    return nullptr;
}


cl_platform_id getDefaultPlatform() {
    cl_uint numPlatforms = 0;
    cl_int error = clGetPlatformIDs(0, NULL, &numPlatforms);
    if (error != CL_SUCCESS || numPlatforms == 0) {
        return nullptr;
    }

    std::vector<cl_platform_id> platforms(numPlatforms);
    error = clGetPlatformIDs(platforms.size(), &platforms[0], NULL);
    if (error != CL_SUCCESS) {
        return nullptr;
    }

    auto* vendor = getenv("KIS_OCL_PLATFORM_VENDOR");
    auto* name   = getenv("KIS_OCL_PLATFORM_NAME");

    cl_platform_id platform = selectPlatform(platforms, vendor, name);
    if (!platform && !platforms.empty()) {
        dbgOpenCL << "all OpenCL platforms were filtered out by the selection criteria";
        if (vendor) {
            dbgOpenCL << "vendor required:" << vendor;
        }
        if (name) {
            dbgOpenCL << "platform name required:" << name;
        }
    }

    return platform;
}

} // namespace ocl
