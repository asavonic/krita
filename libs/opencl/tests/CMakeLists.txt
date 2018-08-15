set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include(ECMAddTests)

include_directories(..)

ecm_add_test(
    kis_opencl_context_test.cpp
    TEST_NAME libs-opencl-context
    LINK_LIBRARIES kritaopencl Qt5::Test)
