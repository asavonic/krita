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

#ifndef KIS_OPENCL_CONTEXT_H
#define KIS_OPENCL_CONTEXT_H

#include <CL/cl.h>
#include "kritaglobal_export.h"

#include <QMap>
#include <tuple>

namespace ocl {

class KRITAGLOBAL_EXPORT KisCLContext {
public:
    KisCLContext();
    ~KisCLContext();

    KisCLContext(const KisCLContext&) = delete;
    KisCLContext& operator=(const KisCLContext&) = delete;

    bool available();

    cl_context context();
    cl_command_queue defaultQueue();
    cl_kernel  kernelByName(const char* name);

private:
    bool m_avail;

    cl_platform_id m_platform;
    cl_device_id m_device;
    cl_context m_context;
    cl_command_queue m_queue;
    cl_program m_program;
};

extern KRITAGLOBAL_EXPORT KisCLContext GlobalContext;


} // namespace ocl

#endif // KIS_OPENCL_CONTEXT_H
