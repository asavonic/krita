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

#include "kis_opencl_context.h"

#include "kis_opencl_device.h"
#include "kis_opencl_platform.h"
#include "kis_opencl_utils.h"

#include <kis_debug.h>

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QString>
#include <QTextStream>

namespace ocl {

KisCLContext GlobalContext;

static QString getProgramSource(QString name) {
    if (auto* prefix = getenv("KIS_OCL_PROGRAM_SRC_PREFIX")) {
        name = QString(prefix) + QDir::separator() + name;
    }

    // TODO: relative path or lookup in a binary for OCL program source

    QFile file(name);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }

    QTextStream fs(&file);
    return fs.readAll();
}

static cl_program createProgram(cl_context context, QString src) {
    QByteArray srcBytes = src.toLatin1();
    const char* data = srcBytes.data();

    cl_int error;
    cl_program program = clCreateProgramWithSource(context, 1, &data,
                                                   NULL, &error);
    if (error != CL_SUCCESS) {
        errOpenCL << "clCreateProgramWithSource failed: " << errorStr(error);
        return nullptr;
    }

    return program;
}

static bool buildProgram(cl_program program, cl_device_id device,
                         QString buildOptions, QString& buildLog) {
    QByteArray options = buildOptions.toLatin1();

    cl_int buildError =
        clBuildProgram(program, 1, &device, options.data(), NULL, NULL);

    size_t logSize;
    cl_int error = clGetProgramBuildInfo(
            program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);

    if (error != CL_SUCCESS) {
        Q_ASSERT(false && "Cannot query OpenCL device build log.");
        return false;
    }

    QByteArray logBytes(logSize, '\0');

    error = clGetProgramBuildInfo(
            program, device, CL_PROGRAM_BUILD_LOG,
            logSize, logBytes.data(), NULL);

    if (error != CL_SUCCESS) {
        Q_ASSERT(false && "Cannot query OpenCL device build log.");
        return false;
    }

    buildLog = logBytes.data();

    return buildError == CL_SUCCESS;
}

KisCLContext::KisCLContext() :
        m_avail(false),
        m_platform(nullptr),
        m_device(nullptr),
        m_context(nullptr),
        m_queue(nullptr),
        m_program(nullptr) {

    if (!getenv("KIS_OCL_ENABLE")) {
        return;
    }

    m_platform = getDefaultPlatform();
    if (!m_platform) {
        dbgOpenCL << "no OpenCL platform available";
        return;
    }

    m_device = getDefaultDevice(m_platform);
    if (!m_device) {
        dbgOpenCL << "no OpenCL device available";
        return;
    }

    cl_int error;
    m_context = clCreateContext(NULL, 1, &m_device, NULL, NULL, &error);
    if (error != CL_SUCCESS) {
        errOpenCL << "clCreateContext failed: " << errorStr(error);
        return;
    }

    cl_command_queue_properties prop =
        getenv("KIS_OCL_PROFILE") ? CL_QUEUE_PROFILING_ENABLE : 0;

    m_queue = clCreateCommandQueue(m_context, m_device, prop, &error);
    if (error != CL_SUCCESS) {
        errOpenCL << "clCreateCommandQueue failed: " << errorStr(error);
        return;
    }

    QString compositeSrc = getProgramSource("composite.cl");
    if (compositeSrc.isEmpty()) {
        errOpenCL << "composite.cl not found";
        return;
    }

    m_program = createProgram(m_context, compositeSrc);
    if (!m_program) {
        return;
    }

    QString buildLog;
    QString buildOptions;
    if (const char* options = getenv("KIS_OCL_PROGRAM_OPTS_COMPOSITE")) {
        buildOptions = options;
    }

    if (auto* prefix = getenv("KIS_OCL_PROGRAM_SRC_PREFIX")) {
        buildOptions += " -I";
        buildOptions += prefix;

        buildOptions += " -I";
        buildOptions += prefix;
        buildOptions += "/include";
    }

    if (!buildProgram(m_program, m_device, buildOptions, buildLog)) {
        errOpenCL.noquote() << "composite.cl build failed:\n" << buildLog;
        clReleaseProgram(m_program);
        m_program = nullptr;
        return;
    }

    m_avail = true;
}

KisCLContext::~KisCLContext() {
    if (m_program)        clReleaseProgram(m_program);
    if (m_queue)          clReleaseCommandQueue(m_queue);
    if (m_context)        clReleaseContext(m_context);
    if (m_device)         clReleaseDevice(m_device);
}

bool KisCLContext::available() {
    return m_avail;
}

cl_context KisCLContext::context() {
    return m_context;
}

cl_command_queue KisCLContext::defaultQueue() {
    return m_queue;
}

cl_kernel KisCLContext::kernelByName(const char* name) {
    cl_int error = CL_SUCCESS;
    cl_kernel kernel = clCreateKernel(m_program, name, &error);
    if (error != CL_SUCCESS) {
        errOpenCL << "clCreateKernel failed: " << errorStr(error);

        if (error == CL_INVALID_KERNEL_NAME) {
            errOpenCL << "cannot find kernel" << name;
            errOpenCL << "candidates are:";
            printKernelNames(m_program);
        }
        return nullptr;
    }

    return kernel;
}


} // namespace ocl
