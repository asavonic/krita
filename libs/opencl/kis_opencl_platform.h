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

#ifndef KIS_OPENCL_PLATFORM_H
#define KIS_OPENCL_PLATFORM_H

#include <CL/cl.h>

namespace ocl {

/// Returns a 'default' OpenCL platform.
/// Default platform would be either the first platform on the machine, or the
/// one (at least partially) matched with KIS_OCL_PLATFORM_VENDOR and
/// KIS_OCL_PLATFORM_NAME environment variables.
cl_platform_id getDefaultPlatform();

} // namespace ocl

#endif // KIS_OPENCL_PLATFORM_H
