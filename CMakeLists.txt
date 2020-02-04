cmake_minimum_required(VERSION 3.0.0)
project(Endian)

option(BUILD_STL_20
    "Build for STL 20"
    OFF
)

set(installHeaders 
    endianness.h
)
add_library(Endian INTERFACE)
if(${CMAKE_VERSION}` VERSION_GREATER_EQUAL "3.15.0")
    set_target_properties(Endian PROPERTIES PUBLIC_HEADER ${installHeaders})
else()
    install(
        FILES ${installHeaders}
        DESTINATION include/endianness )
endif()
add_library(ENDIAN::Endian ALIAS Endian)
target_include_directories(Endian INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
if(BUILD_STL_20)
    target_compile_definitions(Endian INTERFACE TRY_STL_20)
endif()
if(MSVC)
    add_custom_target(Endian.headers SOURCES ${installHeaders} README.md LICENSE)
endif()
install(TARGETS Endian
    EXPORT endiannessConfig
    PUBLIC_HEADER DESTINATION include/endianness
    INCLUDES DESTINATION include/endianness
)
install(EXPORT endiannessConfig NAMESPACE ENDIAN:: DESTINATION lib/cmake/endianness)
