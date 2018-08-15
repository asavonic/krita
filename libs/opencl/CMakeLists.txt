find_package(OpenCL REQUIRED)

include_directories(
    ${OpenCL_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/libs/global
    ${CMAKE_SOURCE_DIR}/libs/pigment
)

# TODO: which one to use?
include_directories(${CMAKE_BINARY_DIR}/libs/pigment)


set(kritaopencl_LIB_SRCS
    kis_opencl_utils.cpp
    kis_opencl_platform.cpp
    kis_opencl_device.cpp
    kis_opencl_context.cpp
)

if(NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
endif()

add_library(kritaopencl SHARED ${kritaopencl_LIB_SRCS} )
generate_export_header(kritaopencl BASE_NAME kritaopencl)


target_link_libraries(kritaopencl
  PUBLIC
    kritaglobal
    ${OpenCL_LIBRARY}
    Qt5::Core
)

set_target_properties(kritaopencl PROPERTIES
    VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)

add_subdirectory(tests)

install(TARGETS kritaopencl  ${INSTALL_TARGETS_DEFAULT_ARGS})

