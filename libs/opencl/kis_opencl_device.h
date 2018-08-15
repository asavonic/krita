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

#ifndef KIS_OPENCL_DEVICE_H
#define KIS_OPENCL_DEVICE_H

#include <CL/cl.h>

namespace ocl {

/// Returns a default device with respect to KIS_OCL_DEVICE_TYPE environment
/// variable.
///
/// KIS_OCL_DEVICE_TYPE can be be one of following:
///  - CL_DEVICE_TYPE_CPU
///  - CL_DEVICE_TYPE_GPU
///  - CL_DEVICE_TYPE_ACCELERATOR
///  - CL_DEVICE_TYPE_DEFAULT
cl_device_id getDefaultDevice(cl_platform_id platform);

} // namespace ocl

#endif // KIS_OPENCL_DEVICE_H
