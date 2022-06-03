cmake_minimum_required(VERSION 3.1)

set(REDIS_PROTOBUF_VERSION "0.1.0")
message(STATUS "redis-protobuf version: ${REDIS_PROTOBUF_VERSION}")

project(redis-protobuf LANGUAGES CXX VERSION ${REDIS_PROTOBUF_VERSION})

set(REDIS_PROTOBUF_DEFAULT_BUILD_TYPE "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE ${REDIS_PROTOBUF_DEFAULT_BUILD_TYPE} CACHE STRING "Set build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()
message(STATUS "redis-protobuf build type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_FLAGS "-std=c++17 -Wall -Wextra -Werror -fPIC -Wno-unused-parameter")

set(PROJECT_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src/sw/redis-protobuf)

file(GLOB PROJECT_SOURCE_FILES "${PROJECT_SOURCE_DIR}/*.cpp" "${PROJECT_SOURCE_DIR}/*.cc")

set(SHARED_LIB shared)

add_library(${SHARED_LIB} SHARED ${PROJECT_SOURCE_FILES})

# protobuf dependency
find_path(PROTOBUF_HEADER google)
target_include_directories(${SHARED_LIB} PUBLIC ${PROTOBUF_HEADER})

find_library(PROTOBUF_LIB libprotobuf.a)
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(${SHARED_LIB} -Wl,-force_load ${PROTOBUF_LIB})
    target_link_libraries(${SHARED_LIB} z)
else()
    target_link_libraries(${SHARED_LIB} -Wl,--whole-archive ${PROTOBUF_LIB} -Wl,--no-whole-archive)
endif()

set_target_properties(${SHARED_LIB} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

set_target_properties(${SHARED_LIB} PROPERTIES CLEAN_DIRECT_OUTPUT 1)

include(GNUInstallDirs)

# Install shared lib.
install(TARGETS ${SHARED_LIB}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
