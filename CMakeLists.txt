cmake_minimum_required(VERSION 3.0.0)
project(Endian)
set(installHeaders 
    endianness.h
)
add_library(Endian INTERFACE)
set_target_properties(Endian PROPERTIES PUBLIC_HEADER ${installHeaders})
install(TARGETS Endian
    EXPORT endiannessConfig
    PUBLIC_HEADER DESTINATION include/endianness
    INCLUDES DESTINATION include/endianness
)
install(EXPORT endiannessConfig NAMESPACE ENDIAN:: DESTINATION lib/cmake/endianness)